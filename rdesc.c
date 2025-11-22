#include "rdesc.h"
#include "bnf.h"
#include "bnf_dsl.h"
#include "detail.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>


#define PRODUCTIONS \
	(*(const struct bnf_symbol (*)[p->nt_count][p->nt_variant_count][p->nt_body_length]) p->rules)

#define get_body(sym) PRODUCTIONS[sym->nt.id][sym->nt.variant]
#define get_current_rule(sym) get_body(sym)[sym->nt.child_count]

#define is_grammar_complete(sym) \
	(get_current_rule(sym).id == EOB && \
	get_current_rule(sym).ty == BNF_SENTINEL)

#define is_construct_end(sym) \
	(get_body(sym)[0].id == EOC && \
	get_body(sym)[0].ty == BNF_SENTINEL)


enum match_result {
	READY,
	CONTINUE,
	RETRY,
	NOMATCH,
};


void rdesc_init(struct rdesc *p,
		size_t nt_count,
		size_t nt_variant_count,
		size_t nt_body_length,
		const struct bnf_symbol *rules)
{
	p->rules = rules;

	p->nt_count = nt_count;
	p->nt_variant_count = nt_variant_count;
	p->nt_body_length = nt_body_length;

	assert_mem(p->child_caps = malloc(sizeof(size_t) * nt_count));

	for (size_t nt_id = 0; nt_id < nt_count; nt_id++) {
		p->child_caps[nt_id] = 0;

		for (size_t variant = 0; variant < nt_variant_count; variant++) {
			size_t len;
			struct bnf_symbol sym;

			for (len = 0;
			     (sym = PRODUCTIONS[nt_id][variant][len]).ty !=
				BNF_SENTINEL;
			     len++);

			if (len > p->child_caps[nt_id])
				p->child_caps[nt_id] = len;

			if (sym.id == EOC)
				break;
		}
	}

	rdesc_token_stack_init(&p->tokens);

	p->root = p->cur = NULL;
}

static void push_child(struct rdesc_cst *parent, struct rdesc_cst *child)
{
	assert_logic(parent->ty == BNF_NONTERMINAL,
		     "token as a parent of another symbol");

	parent->nt.children[parent->nt.child_count++] = child;
}

static struct rdesc_cst *new_nt_node(struct rdesc *p, struct rdesc_cst *parent,
				     int id)
{
	struct rdesc_cst *n = malloc(sizeof(struct rdesc_cst));
	assert_mem(n);

	n->parent = parent;
	if (parent)
		push_child(parent, n);

	n->ty = BNF_NONTERMINAL;

	size_t child_cap = p->child_caps[id];
	assert_logic(child_cap, "a nonterminal with no child");

	n->nt.id = id;
	n->nt.child_count = 0;
	n->nt.children = malloc(sizeof(struct rdesc_cst *) * child_cap);
	assert_mem(n->nt.children);
	n->nt.variant = 0;

	return n;
}

void rdesc_start(struct rdesc *p, int start_symbol)
{
	assert_logic(p->root == NULL, "setting start symbol during parsing");

	p->cur = p->root = new_nt_node(p, NULL, start_symbol);
}

struct rdesc_cst *new_tk_node(struct rdesc_cst *parent, int id)
{
	struct rdesc_cst *n = malloc(sizeof(struct rdesc_cst));
	assert_mem(n);

	n->parent = parent;
	if (parent)
		push_child(parent, n);

	n->ty = BNF_TOKEN;

	n->tk.id = id;

	return n;
}

static void restore_token(struct rdesc *p, struct bnf_token tk)
{
	rdesc_token_stack_push(&p->tokens, tk);
}

static void next_variant(struct rdesc *p)
{
	while (p->cur->nt.child_count) {
		struct rdesc_cst *next_cur = NULL, *child;

		for (size_t i = p->cur->nt.child_count; i > 0; i--) {
			child = p->cur->nt.children[i - 1];

			if (child->ty == BNF_NONTERMINAL) {
				next_cur = child;

				break;
			} else {
				p->cur->nt.child_count--;

				restore_token(p, child->tk);
				free(child);
			}
		}

		if (next_cur)
			p->cur = next_cur;
		else
			break;
	}

	p->cur->nt.variant++;
	p->cur->nt.child_count = 0;
}

void rdesc_destroy_cst(struct rdesc_cst *sym)
{
	if (sym->ty == BNF_NONTERMINAL) {
		for (size_t i = sym->nt.child_count; i > 0; i--)
			rdesc_destroy_cst(sym->nt.children[i - 1]);

		free(sym->nt.children);
	}

	free(sym);
}

static void backtrace(struct rdesc *p)
{
	struct rdesc_cst *parent = p->cur->parent, *child = NULL;

	while (child != p->cur)
		rdesc_destroy_cst(
			child = parent->nt.children[--parent->nt.child_count]);
	p->cur = parent;

	next_variant(p);

	if (is_construct_end(p->cur) && p->cur->parent)
		backtrace(p);
}

static enum match_result match(struct rdesc *p, struct bnf_token tk)
{
	if (is_grammar_complete(p->cur)) {
		p->cur = p->cur->parent;

		if (p->cur == NULL)
			return READY;
		else
			return RETRY;
	}

	if (is_construct_end(p->cur)) {
		restore_token(p, tk);

		if (p->cur->parent) {
			backtrace(p);

			return CONTINUE;
		} else {
			return NOMATCH;
		}
	}

	struct bnf_symbol rule = get_current_rule(p->cur);

	switch (rule.ty) {
	case BNF_TOKEN:
		if (rule.id == tk.id) {
			new_tk_node(p->cur, rule.id)->tk.seminfo = tk.seminfo;

			return CONTINUE;
		} else {
			restore_token(p, tk);
			next_variant(p);

			return CONTINUE;
		}

	case BNF_NONTERMINAL:
		p->cur = new_nt_node(p, p->cur, rule.id);

		return RETRY;

	default: unreachable(); // GCOV_EXCL_LINE
	}
}

enum rdesc_result rdesc_continue_from_stack(struct rdesc *p,
					    struct rdesc_cst **out)
{
	assert_logic(p->root,
		     "continuing an incremental parse with no nonterminal");

	struct bnf_token tk;

	while (p->tokens.len) {
		tk = rdesc_token_stack_pop(&p->tokens);

		enum rdesc_result res = rdesc_continue(p, out, tk);

		if (res == RDESC_CONTINUE)
			continue;
		else
			return res;
	}

	return RDESC_CONTINUE;
}

enum rdesc_result rdesc_continue(struct rdesc *p,
				 struct rdesc_cst **out,
				 struct bnf_token tk)
{
	assert_logic(p->root,
		     "continuing an incremental parse with no nonterminal");

	enum match_result res;

	do {
		res = match(p, tk);
	} while (res == RETRY);

	switch (res) {
	case CONTINUE:
		return RDESC_CONTINUE;

	case NOMATCH:
		rdesc_destroy_cst(p->root);

		p->root = p->cur = NULL;
		return RDESC_NOMATCH;

	case READY:
		*out = p->root;

		p->cur = p->root = NULL;
		return RDESC_READY;

	default: unreachable(); // GCOV_EXCL_LINE
	}
}
