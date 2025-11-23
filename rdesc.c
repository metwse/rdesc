#include "rdesc.h"
#include "bnf.h"
#include "bnf_dsl.h"
#include "detail.h"
#include "stack.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>


#define PRODUCTIONS \
	(*(const struct bnf_symbol (*)[p->nt_count][p->nt_variant_count][p->nt_body_length]) p->rules)

#define get_body(node) PRODUCTIONS[node->nt.id][node->nt.variant]
#define get_current_rule(node) get_body(node)[node->nt.child_count]

#define is_grammar_complete(node) \
	(get_current_rule(node).id == EOB && \
	get_current_rule(node).ty == BNF_SENTINEL)

#define is_construct_end(node) \
	(get_body(node)[0].id == EOC && \
	get_body(node)[0].ty == BNF_SENTINEL)


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

	rdesc_stack_init(&p->tokens);

	p->root = p->cur = NULL;
}

static void push_child(struct rdesc_node *parent, struct rdesc_node *child)
{
	assert_logic(parent->ty == BNF_NONTERMINAL,
		     "a token node cannot be a parent of another node");

	parent->nt.children[parent->nt.child_count++] = child;
}

static struct rdesc_node *new_nt_node(struct rdesc *p,
				      struct rdesc_node *parent,
				      int id)
{
	struct rdesc_node *n = malloc(sizeof(struct rdesc_node));
	assert_mem(n);

	n->parent = parent;
	if (parent)
		push_child(parent, n);

	n->ty = BNF_NONTERMINAL;

	size_t child_cap = p->child_caps[id];
	assert_logic(child_cap, "a nonterminal with no child");

	n->nt.id = id;
	n->nt.child_count = 0;
	n->nt.children = malloc(sizeof(struct rdesc_node *) * child_cap);
	assert_mem(n->nt.children);
	n->nt.variant = 0;

	return n;
}

void rdesc_start(struct rdesc *p, int start_symbol)
{
	assert_logic(p->root == NULL, "setting start symbol during parsing");

	p->cur = p->root = new_nt_node(p, NULL, start_symbol);
}

struct rdesc_node *new_tk_node(struct rdesc_node *parent, int id)
{
	struct rdesc_node *n = malloc(sizeof(struct rdesc_node));
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
	rdesc_stack_push(&p->tokens, tk);
}

static void next_variant(struct rdesc *p)
{
	while (p->cur->nt.child_count) {
		struct rdesc_node *next_cur = NULL, *child;

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

void rdesc_destroy_node(struct rdesc_node *n)
{
	assert_logic(n->ty == BNF_NONTERMINAL,
		     "token nodes should not be destroyed, memory leak "
		     "otherwise");

	for (size_t i = n->nt.child_count; i > 0; i--)
		rdesc_destroy_node(n->nt.children[i - 1]);

	free(n->nt.children);

	free(n);
}

static void backtrace(struct rdesc *p)
{
	struct rdesc_node *parent = p->cur->parent, *child = NULL;

	while (child != p->cur)
		rdesc_destroy_node(
			child = parent->nt.children[--parent->nt.child_count]
		);
	p->cur = parent;

	next_variant(p);

	if (is_construct_end(p->cur) && p->cur->parent)
		backtrace(p);
}

static enum match_result match(struct rdesc *p, struct bnf_token tk)
{
	if (is_grammar_complete(p->cur)) {
		p->cur = p->cur->parent;

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

			while (is_grammar_complete(p->cur)) {
				p->cur = p->cur->parent;

				if (p->cur == NULL)
					return READY;
			}

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

enum rdesc_result rdesc_pump(struct rdesc *p,
			     struct rdesc_node **out,
			     struct bnf_token *incoming_tk)
{
	assert_logic(p->root,
		     "continuing an incremental parse with no root");

	enum match_result res;
	struct bnf_token tk = *incoming_tk;

	bool has_token = incoming_tk != NULL;

	while (true) {
		if (!has_token && rdesc_stack_len(&p->tokens)) {
			tk = rdesc_stack_pop(&p->tokens);
			has_token = true;
		}

		if (!has_token)
			return RDESC_CONTINUE;

		do {
			res = match(p, tk);
		} while (res == RETRY);

		switch (res) {
		case CONTINUE:
			has_token = false;
			break;

		case NOMATCH:
			free(p->root->nt.children);
			free(p->root);

			p->root = p->cur = NULL;
			return RDESC_NOMATCH;

		case READY:
			*out = p->root;

			p->cur = p->root = NULL;
			return RDESC_READY;

		default: unreachable(); // GCOV_EXCL_LINE
		}
	}
}
