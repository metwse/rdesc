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


#define child_list_size(p, nt_id) \
	((p).cfg->child_caps[nt_id] * sizeof(size_t) + sizeof_node(p) - 1) \
		/ sizeof_node(p)


static void push_child(struct rdesc *p, size_t parent_idx, size_t child_idx);
static void new_nt_node(struct rdesc *p, uint32_t nt_id);
static void new_tk_node(struct rdesc *p, uint32_t tk_id, const void *seminfo);


void rdesc_init(struct rdesc *p,
		size_t seminfo_size,
		const struct rdesc_cfg *cfg)
{
	p->cfg = cfg;
	p->seminfo_size = seminfo_size;
	p->cur = SIZE_MAX;

	rdesc_stack_init(&p->token_stack, sizeof_tk(*p));
	rdesc_stack_init(&p->cst_stack, sizeof_node(*p));
}

void rdesc_destroy(struct rdesc *p /*,
		   rdesc_token_destroyer_func token_destroyer*/) {
	rdesc_stack_destroy(p->token_stack);
	rdesc_stack_destroy(p->cst_stack);
}

void rdesc_start(struct rdesc *p,
		 int start_symbol)
{
	rdesc_assert(p->cur == SIZE_MAX, "cannot start during parse");

	p->top_size = 0;
	new_nt_node(p, start_symbol);
}

void rdesc_reset(struct rdesc *p /*,
		 rdesc_token_destroyer_func token_destroyer*/) {
	p->cur = SIZE_MAX;

	rdesc_stack_reset(&p->token_stack);
	rdesc_stack_reset(&p->cst_stack);
}

/* - THE PUMP -------------------------------------------------------------- */
#define current_variant_body(node) productions(*p->cfg)[rid(node)][rvariant(node)]
#define next_symbol(node) current_variant_body(node)[rchild_count(node)]

#define is_body_complete(node) \
	(next_symbol(node).id == EOB && \
	 next_symbol(node).ty == CFG_SENTINEL)

#define is_construct_end(node) \
	(current_variant_body(node)[0].id == EOC && \
	 current_variant_body(node)[0].ty == CFG_SENTINEL)

static void nonterminal_failed(struct rdesc *p)
{
	while (true) {
		p->cur = rdesc_stack_len(p->cst_stack) - p->top_size;
		uint16_t hold_top_size = p->top_size;
		node_t *top = rdesc_stack_at(p->cst_stack, p->cur);

		if (rtype(top) == CFG_TOKEN) {
			p->top_size = _rdesc_priv_prev_size(top);

			rdesc_stack_push(&p->token_stack, &top->n.tk);
		} else {
			p->top_size = _rdesc_priv_prev_size(top);
			if (!is_construct_end(top)) {
				rvariant(top)++;
				rchild_count(top) = 0;

				if (!is_construct_end(top)) {
					p->top_size = 1 + child_list_size(*p, rid(top));
					return;
				}
			}
		}

		node_t *parent = cast(node_t *, rparent(p, top));
		if (parent)
			rchild_count(parent)--;
		rdesc_stack_multipop(&p->cst_stack, hold_top_size);
	}
}

static inline enum internal_pump_state {
	READY,
	CONTINUE,
	RETRY,
	NOMATCH,
} rdesc_pump_internal(struct rdesc *p,
		      struct _rdesc_priv_tk *tk)
{
	node_t *n = rdesc_stack_at(p->cst_stack, p->cur);

	if (is_body_complete(n)) {
		p->cur = _rdesc_priv_parent_idx(n);

		return RETRY;
	}

	if (is_construct_end(n)) {
		rdesc_stack_push(&p->token_stack, tk);

		if (p->cur == 0) {
			return NOMATCH;
		} else {
			nonterminal_failed(p);

			return CONTINUE;
		}
	}

	struct rdesc_cfg_symbol rule = next_symbol(n);

	switch (rule.ty) {
	case CFG_TOKEN:
		if (rule.id == tk->id) {
			new_tk_node(p, tk->id, &tk->seminfo);

			while (true) {
				n = rdesc_stack_at(p->cst_stack, p->cur);
				if (!is_body_complete(n))
					break;

				p->cur = _rdesc_priv_parent_idx(n);

				if (p->cur == SIZE_MAX)
					return READY;
			}

			return CONTINUE;
		} else {
			rdesc_stack_push(&p->token_stack, tk);
			nonterminal_failed(p);

			return CONTINUE;
		}

	case CFG_NONTERMINAL:
		new_nt_node(p, rule.id);

		return RETRY;

	default: unreachable(); // GCOV_EXCL_LINE
	} // GCOV_EXCL_LINE
}

enum rdesc_result rdesc_pump(struct rdesc *p,
			     struct rdesc_node **out,
			     uint32_t id_,
			     const void *seminfo_)
{
	rdesc_assert(p->cur != SIZE_MAX, "parser is not started");

	uint8_t tk_[sizeof_tk(*p)];
	struct _rdesc_priv_tk *tk = cast(struct _rdesc_priv_tk *, &tk_);

	bool has_token = id_ != 0;
	if (has_token) {
		tk->id = id_;
		if (seminfo_)
			memcpy(&tk->seminfo, seminfo_, sizeof_tk(*p));
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
			// TODO: NOMATCH

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
static void new_nt_node(struct rdesc *p, uint32_t nt_id)
{
	node_t *n = rdesc_stack_push(&p->cst_stack, NULL);
	size_t parent_idx = p->cur;
	p->cur = rdesc_stack_len(p->cst_stack) - 1;

	if (parent_idx != SIZE_MAX)
		push_child(p, parent_idx, p->cur);

	rtype(n) = CFG_NONTERMINAL;

	_rdesc_priv_parent_idx(n) = parent_idx;
	_rdesc_priv_prev_size(n) = p->top_size;
	rid(n) = nt_id;
	rvariant(n) = 0;
	rchild_count(n) = 0;

	rdesc_stack_multipush(&p->cst_stack, NULL, child_list_size(*p, nt_id));

	p->top_size = 1 + child_list_size(*p, nt_id);
}

/* Creates a new node in parser's CST stack and copies `seminfo` into it. */
static void new_tk_node(struct rdesc *p, uint32_t tk_id, const void *seminfo)
{
	node_t *n = rdesc_stack_push(&p->cst_stack, NULL);
	size_t node_id = rdesc_stack_len(p->cst_stack) - 1;

	push_child(p, p->cur, node_id);

	rtype(n) = CFG_TOKEN;

	_rdesc_priv_parent_idx(n) = p->cur;
	_rdesc_priv_prev_size(n) = p->top_size;
	rid(n) = tk_id;

	if (seminfo)
		memcpy(rseminfo(n), seminfo, p->seminfo_size);

	p->top_size = 1;
}
