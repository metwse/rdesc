#include "../include/bnf_macros.h"
#include "../include/cfg.h"
#include "../include/cst_macros.h"
#include "../include/rdesc.h"
#include "../include/stack.h"
#include "detail.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>


/* Additional space for child pointers in nonterminal. */
#define rchild_list_cap(p, nt_id) \
	((p).cfg->child_caps[nt_id] * sizeof(size_t) + sizeof_node(p) - 1) \
		/ sizeof_node(p)

/* Returns the previous node's unwind size (used to navigate backwards). */
#define runwind_size(node) _rdesc_priv_node_deref(node).unwind_size


static void new_nt_node(struct rdesc *p, uint16_t nt_id);
static void new_tk_node(struct rdesc *p, uint16_t tk_id, const void *seminfo);

static void destroy_tokens(struct rdesc *p,
			   rdesc_token_destroyer_func tk_destroyer);

void rdesc_init(struct rdesc *p,
		const struct rdesc_cfg *cfg,
		size_t seminfo_size)
{
	p->cfg = cfg;
	p->seminfo_size = seminfo_size;
	p->cur = SIZE_MAX;

	rdesc_stack_init(&p->token_stack, sizeof_tk(*p));
	rdesc_stack_init(&p->cst_stack, sizeof_node(*p));
}

void rdesc_destroy(struct rdesc *p, rdesc_token_destroyer_func tk_destroyer)
{
	destroy_tokens(p, tk_destroyer);

	rdesc_stack_destroy(p->token_stack);
	rdesc_stack_destroy(p->cst_stack);
}

void rdesc_start(struct rdesc *p, int start_symbol)
{
	rdesc_assert(p->cur == SIZE_MAX, "cannot start during parse");

	rdesc_stack_reset(&p->cst_stack);

	p->top_size = 0;
	new_nt_node(p, start_symbol);
}

void rdesc_reset(struct rdesc *p,
		 rdesc_token_destroyer_func tk_destroyer) {
	destroy_tokens(p, tk_destroyer);
	p->cur = SIZE_MAX;

	rdesc_stack_reset(&p->token_stack);
	rdesc_stack_reset(&p->cst_stack);
}

static void destroy_tokens(struct rdesc *p,
			   rdesc_token_destroyer_func tk_destroyer)
{
	if (!tk_destroyer)
		return;

	/* Walk CST backwards to destroy all embedded tokens */
	p->cur = rdesc_stack_len(p->cst_stack);
	while (p->cur) {
		node_t *top = rdesc_stack_at(p->cst_stack, p->cur - p->top_size);
		uint16_t hold_top_size = p->top_size;

		if (rtype(top) == CFG_TOKEN)
			tk_destroyer(rid(top), rseminfo(top));

		p->top_size = runwind_size(top);
		p->cur = p->cur - hold_top_size;
	}

	/* Destroy tokens in backtrack stack */
	for (size_t i = 0; i < rdesc_stack_len(p->token_stack); i++) {
		tk_t *tk = rdesc_stack_at(p->token_stack, i);
		tk_destroyer(tk->id, &tk->seminfo);
	}
}

/* - THE PUMP -------------------------------------------------------------- */
#define current_variant_body(node) \
	productions(*p->cfg)[rid(node)][rvariant(node)]
#define next_symbol(node) \
	current_variant_body(node)[rchild_count(node)]

#define is_body_complete(node) \
	(next_symbol(node).id == EOB && \
	 next_symbol(node).ty == CFG_SENTINEL)

#define is_construct_end(node) \
	(current_variant_body(node)[0].id == EOC && \
	 current_variant_body(node)[0].ty == CFG_SENTINEL)

/* Backtraces to the last nonterminal that is not completed, or teardowns the
 * entire CST. */
static void nonterminal_failed(struct rdesc *p)
{
	/* Initialization: Start from the top. */
	while (true) {
		p->cur = rdesc_stack_len(p->cst_stack) - p->top_size;
		uint16_t hold_top_size = p->top_size;
		node_t *top = rdesc_stack_at(p->cst_stack, p->cur);

		/* Maintenance: Set the top size to previous element's size,
		 * to get it on the next iteration. */
		p->top_size = runwind_size(top);
		if (rtype(top) == CFG_TOKEN) {
			rdesc_stack_push(&p->token_stack, &top->n.tk);
		} else {
			if (!is_construct_end(top)) {
				rvariant(top)++;
				rchild_count(top) = 0;

				/* Termination: Found a nonterminal with
				 * remaining variants. Set top_size to this
				 * nonterminal's unwind size and return.
				 *
				 * p->cur was set at loop start, so parsing
				 * resumes on its next variant. */
				if (!is_construct_end(top)) {
					p->top_size =
						1 + rchild_list_cap(*p, rid(top));

					return;
				}
			}
		}

		/* Remove element from parent's child pointer list. */
		node_t *parent = cast(node_t *, rparent(p, top));
		if (parent)
			rchild_count(parent)--;
		rdesc_stack_multipop(&p->cst_stack, hold_top_size);

		/* Parse operation fails if removed element does not belong to
		 * any node, that is removing the node. */
		if (!parent)
			return;
	}
}

/* Internal pump state machine. Returns next action for outer pump loop.
 *
 * READY: Parse complete,
 * CONTINUE: consume next token,
 * NOMATCH: parse failed,
 * RETRY: descend into nonterminal */
static inline enum internal_pump_state {
	READY,
	CONTINUE,
	NOMATCH,
	RETRY,
} rdesc_pump_internal(struct rdesc *p, tk_t *tk)
{
	node_t *n = rdesc_stack_at(p->cst_stack, p->cur);

	if (rdesc_stack_len(p->cst_stack) == 0) {
		rdesc_stack_push(&p->token_stack, tk);

		return NOMATCH;
	}

	struct rdesc_cfg_symbol rule = next_symbol(n);

	switch (rule.ty) {
	case CFG_TOKEN:
		if (rule.id == tk->id) {
			/* Match! Add the token to nonterminal's children. */
			new_tk_node(p, tk->id, &tk->seminfo);
		} else {
			/* Push the token back to the token stack and continue
			 * on next variant. */
			rdesc_stack_push(&p->token_stack, tk);

			nonterminal_failed(p);
		}

		/* Climb the tree if to find incomplete nonterminal to
		 * continue parsing on. */
		while (true) {
			n = rdesc_stack_at(p->cst_stack, p->cur);
			if (!is_body_complete(n))
				break;

			p->cur = _rdesc_priv_parent_idx(n);

			if (p->cur == SIZE_MAX)
				return READY;
		}

		return CONTINUE;

	case CFG_NONTERMINAL:
		new_nt_node(p, rule.id);

		return RETRY;

	default: unreachable(); // GCOV_EXCL_LINE
	} // GCOV_EXCL_LINE
}

enum rdesc_result rdesc_pump(struct rdesc *p,
			     struct rdesc_node **out,
			     uint16_t id_,
			     const void *seminfo_)
{
	rdesc_assert(p->cur != SIZE_MAX, "parser is not started");

	uint8_t tk_[sizeof_tk(*p)];
	tk_t *tk = cast(tk_t *, &tk_);

	bool has_token = id_ != 0;
	if (has_token) {
		tk->id = id_;
		if (seminfo_)
			memcpy(&tk->seminfo, seminfo_, p->seminfo_size);
	}

	while (true) {
		if (!has_token && rdesc_stack_len(p->token_stack) > 0) {
			has_token = true;
			tk = rdesc_stack_pop(&p->token_stack);
		}

		if (!has_token)
			return RDESC_CONTINUE;

		enum internal_pump_state state;
		do {
			state = rdesc_pump_internal(p, tk);
		} while (state == RETRY);

		switch (state) {
		case CONTINUE:
			has_token = false;

			break;

		case NOMATCH:
			p->cur = SIZE_MAX;

			return RDESC_NOMATCH;

		case READY:
			*out = rdesc_stack_at(p->cst_stack, 0);

			return RDESC_READY;

		default: unreachable();  // GCOVR_EXCL_LINE
		}
	}
}
/* ------------------------------------------------------------------------- */

struct rdesc_node *_rdesc_priv_cst_illegal_access(struct rdesc *parser,
						  size_t index)
{
	return index == SIZE_MAX ?
		NULL : rdesc_stack_at(parser->cst_stack, index);
}

/* Makes the connection between parent and child, by adding `child_index` to
 * parent's children index list. */
static void push_child(struct rdesc *p, size_t parent_idx, size_t child_idx)
{
	node_t *parent = rdesc_stack_at(p->cst_stack, parent_idx);

	_rdesc_priv_child_idx(parent, rchild_count(parent)) = child_idx;

	rchild_count(parent)++;
}

/* Pushes a new nonterminal to parser's CST stack and reserves space for its
 * children. */
static void new_nt_node(struct rdesc *p, uint16_t nt_id)
{
	/* allocate node pointer */
	node_t *n = rdesc_stack_push(&p->cst_stack, NULL);
	/* the new node will be the p->cur, so that we need to hold parent_idx
	 * in order to add it to its parent */
	size_t parent_idx = p->cur;
	p->cur = rdesc_stack_len(p->cst_stack) - 1;  /* index of new node */

	if (parent_idx != SIZE_MAX)
		push_child(p, parent_idx, p->cur);

	_rdesc_priv_parent_idx(n) = parent_idx;
	runwind_size(n) = p->top_size;
	rtype(n) = CFG_NONTERMINAL;

	rid(n) = nt_id;
	rvariant(n) = 0;
	rchild_count(n) = 0;

	uint16_t child_list_cap = rchild_list_cap(*p, nt_id);
	rdesc_stack_multipush(&p->cst_stack, NULL, child_list_cap);

	p->top_size = 1 + child_list_cap;
}

/* Creates a new node in parser's CST stack and copies `seminfo` into it. */
static void new_tk_node(struct rdesc *p, uint16_t tk_id, const void *seminfo)
{
	node_t *n = rdesc_stack_push(&p->cst_stack, NULL);
	size_t node_id = rdesc_stack_len(p->cst_stack) - 1;

	push_child(p, p->cur, node_id);

	_rdesc_priv_parent_idx(n) = p->cur;
	runwind_size(n) = p->top_size;
	rtype(n) = CFG_TOKEN;

	rid(n) = tk_id;

	if (seminfo)
		memcpy(rseminfo(n), seminfo, p->seminfo_size);

	p->top_size = 1;
}
