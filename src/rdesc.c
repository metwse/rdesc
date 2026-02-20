#include "../include/bnf_macros.h"
#include "../include/cst_macros.h"
#include "../include/grammar.h"
#include "../include/rdesc.h"
#include "../include/stack.h"
#include "detail.h"
#include "test_instruments.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


/* Additional space for child pointers in nonterminal. */
#define rchild_list_cap(p, nt_id) \
	((p).grammar->child_caps[nt_id] * sizeof(size_t) + sizeof_node(p) - 1) \
		/ sizeof_node(p)

/* Returns the previous node's unwind size (used to navigate backwards). */
#define runwind_size(node) _rdesc_priv_node_deref(node).unwind_size


/* Constructs nonterminal. Returns non-zero and rolls back to previous valid
 * state if construction fails. */
static int new_nt_node(struct rdesc *p, uint16_t nt_id);
/* Constructs token and returns 0 if the construction succeeded. */
static int new_tk_node(struct rdesc *p, uint16_t tk_id, const void *seminfo);

/* Destroys all tokens in CST and token stacks. */
static void destroy_tokens(struct rdesc *p);

/* Adds children to parent's child list using indexes. This function does not
 * fail even if realloc changed the stack pointer. */
static inline void push_child(struct rdesc *p,
			      size_t parent_idx,
			      size_t child_idx);

/* Similar to push child, this does not fail. */
static inline void pop_child(struct rdesc *p,
			     size_t node_idx);

int rdesc_init(struct rdesc *p,
	       const struct rdesc_grammar *grammar,
	       size_t seminfo_size,
	       void (*token_destroyer)(uint16_t, void *))
{
	p->grammar = grammar;
	p->seminfo_size = seminfo_size;
	p->token_destroyer = token_destroyer;

	p->cur = SIZE_MAX;

	if (seminfo_size > 0) {
		p->saved_seminfo = xmalloc(seminfo_size);

		if (p->saved_seminfo == NULL)
			return 1; /* Could not preallocate extra seminfo space. */
	} else {
		p->saved_seminfo = NULL;
	}

	rdesc_stack_init(&p->token_stack, sizeof_tk(*p));
	if (p->token_stack == NULL) {
		if (p->saved_seminfo != NULL)
			free(p->saved_seminfo);

		return 1;  /* Could not initialize token stack.  */
	}

	rdesc_stack_init(&p->cst_stack, sizeof_node(*p));
	if (p->cst_stack == NULL) {
		if (p->saved_seminfo != NULL)
			free(p->saved_seminfo);
		rdesc_stack_destroy(p->token_stack);

		return 1;  /* Could not intialize CST stack. */
	}

	return 0;
}

void rdesc_destroy(struct rdesc *p)
{
	destroy_tokens(p);

	rdesc_stack_destroy(p->token_stack);
	rdesc_stack_destroy(p->cst_stack);

	if (p->saved_seminfo != NULL)
		free(p->saved_seminfo);
}

int rdesc_start(struct rdesc *p, uint16_t start_symbol)
{
	rdesc_assert(p->cur == SIZE_MAX, "cannot start during parse");

	p->saved_tk = 0;
	p->top_unwind = 0;

	rdesc_stack_reset(&p->cst_stack);

	if (new_nt_node(p, start_symbol))
		return 1;  /* Start symbol creation failed. */

	return 0;
}

int rdesc_reset(struct rdesc *p) {
	destroy_tokens(p);

	p->cur = SIZE_MAX;

	rdesc_stack_reset(&p->token_stack);

	rdesc_stack_reset(&p->cst_stack);

	return 0;
}

static void destroy_tokens(struct rdesc *p)
{
	if (!p->token_destroyer)
		return;

	if (p->saved_tk)
		p->token_destroyer(p->saved_tk, p->saved_seminfo);

	/* Destroy tokens in backtrack stack */
	for (size_t i = 0; i < rdesc_stack_len(p->token_stack); i++) {
		tk_t *tk = rdesc_stack_at(p->token_stack, i);
		p->token_destroyer(tk->id, &tk->seminfo);
	}

	if (rdesc_stack_len(p->cst_stack)) {
		/* Walk CST backwards to destroy all embedded tokens */
		uint16_t top_unwind = p->top_unwind;

		for (size_t top_idx = rdesc_stack_len(p->cst_stack) - top_unwind;
		     top_idx > 0;  /* Termination: Cannot be a token */
		     top_idx -= top_unwind) {
			node_t *top = rdesc_stack_at(p->cst_stack, top_idx);

			if (rtype(top) == RDESC_TOKEN)
				p->token_destroyer(rid(top), rseminfo(top));

			top_unwind = runwind_size(top);
		}
	}
}

/* - THE PUMP -------------------------------------------------------------- */
#define current_variant_body(node) \
	productions(*p->grammar)[rid(node)][rvariant(node)]
#define next_symbol(node) \
	current_variant_body(node)[rchild_count(node)]

#define is_body_complete(node) \
	(next_symbol(node).id == EOB && \
	 next_symbol(node).ty == RDESC_SENTINEL)

#define is_construct_end(node) \
	(current_variant_body(node)[0].id == EOC && \
	 current_variant_body(node)[0].ty == RDESC_SENTINEL)

/* Backtraces to the last nonterminal that is not completed, or teardowns the
 * entire CST. */
static inline int nonterminal_failed(struct rdesc *p)
{
	size_t scan_idx = rdesc_stack_len(p->cst_stack) - p->top_unwind;
	size_t tokens_pushed = 0;

	/* FIRST traversal: Push tokens to backtracking stack. */

	/* Initialization: Start from the top. */
	while (true) {
		node_t *top = rdesc_stack_at(p->cst_stack, scan_idx);

		/* Maintenance: The slice from scan_idx to stack_len does not
		 * contain a nonterminal that have unchecked variant. */
		if (rtype(top) == RDESC_TOKEN) {
			if (rdesc_stack_push(&p->token_stack, &top->n.tk) == NULL) {
				/* Could not move token to token stack! Keep
				 * existing token in CST and report error. :*/

				rdesc_stack_multipop(&p->token_stack, tokens_pushed);

				return 1;
			}
			tokens_pushed++;
		} else /* RDESC_NONTERMINAL */ {
			uint16_t saved_child_count = rchild_count(top);
			rvariant(top)++;
			rchild_count(top) = 0;

			/* Termination: Found a nonterminal with remaining
			 * variants. */
			bool is_construct_not_end = !is_construct_end(top);

			/* Rollback changes. The second loop will update the
			 * nonterminal. */
			rvariant(top)--;
			rchild_count(top) = saved_child_count;

			if (is_construct_not_end)
				break;
		}

		scan_idx -= runwind_size(top);

		/* Parse operation fails if removed element does not belong to
		 * any node, that is removing the node. */
		if (_rdesc_priv_parent_idx(top) == SIZE_MAX)
			break;
	}

	/* Two loops exist to enable rollback to valid state in case of
	 * memory allocation failure. After the first loop ensure all the
	 * tokens pushed back to backtracking stack, the second one removes
	 * elements of first stack. */

	/* Now the traversal changes the parser state. From now on no memory
	 * failure can occur. */
	p->cur = rdesc_stack_len(p->cst_stack) - p->top_unwind;
	/* Safety: p->cur changed, so p->top_unwind MUST BE CHANGED. This is
	 * guaranteed in next loop: Before every break we update
	 * p->top_unwind. */

	while (true) {
		node_t *top = rdesc_stack_at(p->cst_stack, p->cur);

		if (rtype(top) == RDESC_NONTERMINAL) {
			/* Be careful: All children have been removed, so the
			 * nonterminal is now the topmost node. */
			rvariant(top)++;
			rchild_count(top) = 0;

			/* Found unfinished nonterminal. */
			if (!is_construct_end(top)) {
				p->top_unwind = 1 + rchild_list_cap(*p, rid(top));

				break;
			}
		}

		/* Remove element from parent's child pointer list. */
		size_t parent_idx = _rdesc_priv_parent_idx(top);
		if (parent_idx != SIZE_MAX)
			pop_child(p, parent_idx);

		/* Exit case, explained in previous loop. */
		if (parent_idx == SIZE_MAX) {
			p->top_unwind = 0;

			break;
		}

		p->cur -= runwind_size(top);
	};

	/* Remove nodes after the p->cur, which is the top. */
	rdesc_stack_multipop(&p->cst_stack,
			     rdesc_stack_len(p->cst_stack) - (p->cur + p->top_unwind));

	return 0;
}

/* Internal pump state machine. Returns next action for outer pump loop.
 *
 * - EMEM: Provided token pushed to either CST stack or token stack, but memory
 *   allocation error occurred afterwards.
 *
 * - EMEM_TK_NOT_OWNED: Provided token was not pushed to token stack or CST,
 *   and it still belong to caller.
 *
 * - READY: Parse complete.
 *
 * - CONTINUE: Request the next token.
 *
 * - NOMATCH: Parse failed.
 *
 * - RETRY: Descend into nonterminal, caller should call this function again. */
static inline enum internal_pump_state {
	EMEM,
	EMEM_TK_NOT_OWNED,
	READY,
	CONTINUE,
	NOMATCH,
	RETRY,
} rdesc_pump_internal(struct rdesc *p, tk_t *tk)
{
	node_t *n = rdesc_stack_at(p->cst_stack, p->cur);

	if (rdesc_stack_len(p->cst_stack) == 0) {
		if (rdesc_stack_push(&p->token_stack, tk) == NULL) {
			/* Token should be stored for next start, but could
			 * not because of push error. */
			return EMEM_TK_NOT_OWNED;
		}

		return NOMATCH;
	}

	struct rdesc_grammar_symbol rule = next_symbol(n);

	switch (rule.ty) {
	case RDESC_TOKEN:
		if (rule.id == tk->id) {
			/* Match! Add the token to nonterminal's children. */
			if (new_tk_node(p, tk->id, &tk->seminfo)) {
				/* Could not add token to the current
				 * nonterminal's children. */
				return EMEM_TK_NOT_OWNED;
			}
		} else {
			/* Push the token back to the token stack and continue
			 * on the next variant. */
			if (rdesc_stack_push(&p->token_stack, tk) == NULL) {
				/* Could not push token back to backtracking
				 * stack. */
				return EMEM_TK_NOT_OWNED;
			}

			if (nonterminal_failed(p)) {
				/* Memory error in backtracking. */
				return EMEM;
			}
		}

		/* Climb the tree if to find incomplete nonterminal to continue
		 * parsing on. */
		while (true) {
			n = rdesc_stack_at(p->cst_stack, p->cur);
			if (!is_body_complete(n))
				break;

			p->cur = _rdesc_priv_parent_idx(n);

			/* Every node, including the root is completed. Return
			 * ready. */
			if (p->cur == SIZE_MAX)
				return READY;
		}

		return CONTINUE;

	case RDESC_NONTERMINAL:
		if (new_nt_node(p, rule.id)) {
			/* An error occured before the token ever used. */
			return EMEM_TK_NOT_OWNED;
		}

		return RETRY;

	default: unreachable(); // GCOV_EXCL_LINE
	} // GCOV_EXCL_LINE
}

enum rdesc_result rdesc_pump(struct rdesc *p, uint16_t id, void *seminfo)
{
	rdesc_assert(p->cur != SIZE_MAX, "parser is not started");

	uint8_t tk_[sizeof_tk(*p)];
	tk_t *tk = cast(tk_t *, &tk_);

	bool has_token;
	if (p->saved_tk) {
		rdesc_assert(id == 0, "shall not provide new token during resume");

		has_token = true;
		tk->id = p->saved_tk;
		if (p->saved_seminfo != NULL)
			memcpy(&tk->seminfo, p->saved_seminfo, p->seminfo_size);

		p->saved_tk = 0;
	} else {
		has_token = id != 0;

		if (has_token) {
			tk->id = id;
			if (seminfo != NULL)
				memcpy(&tk->seminfo, seminfo, p->seminfo_size);
		}
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
		case EMEM:
			return RDESC_ENOMEM;

		case EMEM_TK_NOT_OWNED:
			p->saved_tk = tk->id;
			memcpy(p->saved_seminfo, &tk->seminfo, p->seminfo_size);

			return RDESC_ENOMEM;

		case CONTINUE:
			has_token = false;

			break;

		case NOMATCH:
			p->cur = SIZE_MAX;

			return RDESC_NOMATCH;

		case READY:
			return RDESC_READY;

		default: unreachable();  // GCOVR_EXCL_LINE
		}
	}
}
/* ------------------------------------------------------------------------- */

struct rdesc_node *rdesc_root(struct rdesc *p)
{
	if (rdesc_stack_len(p->cst_stack) == 0)
		return NULL;

	return rdesc_stack_at(p->cst_stack, 0);
}

struct rdesc_node *_rdesc_priv_cst_illegal_access(const struct rdesc *p,
						  size_t index)
{
	return index == SIZE_MAX ?
		NULL : rdesc_stack_at(p->cst_stack, index);
}

/* Makes the connection between parent and child, by adding `child_index` to
 * parent's children index list. */
static inline void push_child(struct rdesc *p, size_t parent_idx, size_t child_idx)
{
	node_t *parent = rdesc_stack_at(p->cst_stack, parent_idx);

	_rdesc_priv_child_idx(parent, rchild_count(parent)) = child_idx;

	rchild_count(parent)++;
}

/* Removes the last child from node. */
static inline void pop_child(struct rdesc *p, size_t node_idx)
{
	node_t *parent = rdesc_stack_at(p->cst_stack, node_idx);

	rchild_count(parent)--;
}

/* Pushes a new nonterminal to parser's CST stack and reserves space for its
 * children. */
static int new_nt_node(struct rdesc *p, uint16_t nt_id)
{
	/* allocate node pointer */
	node_t *n = rdesc_stack_push(&p->cst_stack, NULL);

	if (n == NULL)
		return 1;  /* node allocation failed */

	/* the new node will be the p->cur, so that we need to hold parent_idx
	 * in order to add it to its parent */
	size_t parent_idx = p->cur;
	p->cur = rdesc_stack_len(p->cst_stack) - 1;  /* index of the new node */

	_rdesc_priv_parent_idx(n) = parent_idx;
	runwind_size(n) = p->top_unwind;
	rtype(n) = RDESC_NONTERMINAL;

	rid(n) = nt_id;
	rvariant(n) = 0;
	rchild_count(n) = 0;

	uint16_t child_list_cap = rchild_list_cap(*p, nt_id);
	if (rdesc_stack_multipush(&p->cst_stack, NULL, child_list_cap) == NULL) {
		/* Rollback changes if nonterminal is partially constructed. */

		rdesc_stack_pop(&p->cst_stack);  /* Pop the node. */
		p->cur = parent_idx;  /* Rollback parent. */

		return 1;  /* child list allocation failed */
	} else {
		p->top_unwind = 1 + child_list_cap;

		if (parent_idx != SIZE_MAX)
			push_child(p, parent_idx, p->cur);

		return 0;
	}


}

/* Creates a new node in parser's CST stack and copies `seminfo` into it. */
static int new_tk_node(struct rdesc *p, uint16_t tk_id, const void *seminfo)
{
	node_t *n = rdesc_stack_push(&p->cst_stack, NULL);

	if (n == NULL)
		return 1;  /* node allocation failed */

	size_t node_id = rdesc_stack_len(p->cst_stack) - 1;

	push_child(p, p->cur, node_id);

	_rdesc_priv_parent_idx(n) = p->cur;
	runwind_size(n) = p->top_unwind;
	rtype(n) = RDESC_TOKEN;

	rid(n) = tk_id;

	if (seminfo)
		memcpy(rseminfo(n), seminfo, p->seminfo_size);

	p->top_unwind = 1;

	return 0;
}
