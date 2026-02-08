#include "../include/rdesc.h"
#include "../include/cfg.h"
#include "../include/bnf_dsl.h"
#include "../include/stack.h"
#include "detail.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#define get_body(node) productions(*p->cfg)[node->nt_.id][node->nt_.variant]
#define get_current_rule(node) get_body(node)[node->nt_.child_count]

#define is_grammar_complete(node) \
	(get_current_rule(node).id == EOB && \
	get_current_rule(node).ty == CFG_SENTINEL)

#define is_construct_end(node) \
	(get_body(node)[0].id == EOC && \
	get_body(node)[0].ty == CFG_SENTINEL)


enum match_result {
	READY,
	CONTINUE,
	RETRY,
	NOMATCH,
};

/* pushes the node child to children field in the parent */
static void push_child(struct rdesc_node *parent, struct rdesc_node *child);

/* helpers for allocating symbol types */
static struct rdesc_node *new_nt_node(struct rdesc *, struct rdesc_node *, uint32_t);
static struct rdesc_node *new_tk_node(struct rdesc_node *, uint32_t);

/* increment variant counter, trigger backgtracing if required */
static void next_variant(struct rdesc *);
static void backtrace(struct rdesc *);

/* internal token matching mechanism */
static enum match_result match(struct rdesc *, struct rdesc_token);


void rdesc_init(struct rdesc *p,
		size_t seminfo_size,
		const struct rdesc_cfg *cfg)
{
	p->cfg = cfg;
	p->seminfo_size = seminfo_size;

	rdesc_stack_init(&p->stack);

	p->root = p->cur = NULL;
}

void rdesc_start(struct rdesc *p, int start_symbol)
{
	assert_logic(p->root == NULL, "setting start symbol during parsing");

	p->cur = p->root = new_nt_node(p, NULL, start_symbol);
}

void rdesc_node_destroy(struct rdesc_node *n, rdesc_tk_destroyer_func free_tk)
{
	if (n->ty == CFG_NONTERMINAL) {
		for (uint16_t i = n->nt_.child_count; i > 0; i--)
			rdesc_node_destroy(n->nt_.children[i - 1], free_tk);

		free(n->nt_.children);
	} else if (free_tk) {
		free_tk(&n->tk_);
	}

	free(n);
}

void rdesc_reset(struct rdesc *p, rdesc_tk_destroyer_func free_tk)
{
	if (free_tk) {
		struct rdesc_token *tks = rdesc_stack_as_ref(p->stack);

		for (size_t tk_count = rdesc_stack_len(p->stack);
		     tk_count > 0; tk_count--)
			free_tk(&tks[tk_count - 1]);
	}

	if (p->root)
		rdesc_node_destroy(p->root, free_tk);

	rdesc_stack_destroy(p->stack);
	rdesc_stack_init(&p->stack);

	p->root = p->cur = NULL;
}

void rdesc_destroy(struct rdesc *p)
{
	assert_logic(p->root == NULL, "destroying parser during parsing");
	rdesc_assert(rdesc_stack_len(p->stack) == 0,
		     "cannot destroy parser if token stack is not "
		     "empty");

	rdesc_stack_destroy(p->stack);
}

enum rdesc_result rdesc_pump(struct rdesc *p,
			     struct rdesc_node **out,
			     struct rdesc_token *incoming_tk)
{
	assert_logic(p->root,
		     "continuing an incremental parse with no root");

	enum match_result res;
	struct rdesc_token tk;

	bool has_token = incoming_tk != NULL;
	if (has_token)
		tk = *incoming_tk;

	while (true) {
		if (!has_token && rdesc_stack_len(p->stack)) {
			tk = rdesc_stack_pop(&p->stack);
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
			free(p->root->nt_.children);
			free(p->root);

			p->root = p->cur = NULL;
			return RDESC_NOMATCH;

		case READY:
			*out = p->root;

			p->cur = p->root = NULL;
			return RDESC_READY;

		default: unreachable(); // GCOV_EXCL_LINE
		} // GCOV_EXCL_LINE
	}
}


static struct rdesc_node *new_nt_node(struct rdesc *p,
				      struct rdesc_node *parent,
				      uint32_t id)
{
	struct rdesc_node *n = malloc(sizeof(struct rdesc_node));
	assert_mem(n);

	n->parent = parent;
	if (parent)
		push_child(parent, n);

	n->ty = CFG_NONTERMINAL;

	uint16_t child_cap = p->cfg->child_caps[id];
	assert_logic(child_cap, "a nonterminal with no child");

	n->nt_.id = id;
	n->nt_.child_count = 0;
	n->nt_.children = malloc(sizeof(struct rdesc_node *) * child_cap);
	assert_mem(n->nt_.children);
	n->nt_.variant = 0;

	return n;
}

static struct rdesc_node *new_tk_node(struct rdesc_node *parent, uint32_t id)
{
	struct rdesc_node *n = malloc(sizeof(struct rdesc_node));
	assert_mem(n);

	n->parent = parent;
	if (parent)
		push_child(parent, n);

	n->ty = CFG_TOKEN;

	n->tk_.id = id;

	return n;
}

static void push_child(struct rdesc_node *parent, struct rdesc_node *child)
{
	assert_logic(parent->ty == CFG_NONTERMINAL,
		     "a token node cannot be a parent of another node");

	parent->nt_.children[parent->nt_.child_count++] = child;
}

static void next_variant(struct rdesc *p)
{
	while (p->cur->nt_.child_count) {
		struct rdesc_node *next_cur = NULL, *child;

		for (uint16_t i = p->cur->nt_.child_count; i > 0; i--) {
			child = p->cur->nt_.children[i - 1];

			if (child->ty == CFG_NONTERMINAL) {
				next_cur = child;

				break;
			} else {
				p->cur->nt_.child_count--;

				rdesc_stack_push(&p->stack, child->tk_);
				free(child);
			}
		}

		if (next_cur)
			p->cur = next_cur;
		else
			break;
	}

	p->cur->nt_.variant++;
	p->cur->nt_.child_count = 0;
}

static void backtrace(struct rdesc *p)
{
	struct rdesc_node *parent = p->cur->parent, *child = NULL;

	while (child != p->cur)
		rdesc_node_destroy(
			child = parent->nt_.children[--parent->nt_.child_count],
			NULL
		);
	p->cur = parent;

	next_variant(p);

	if (is_construct_end(p->cur) && p->cur->parent)
		backtrace(p);
}

static enum match_result match(struct rdesc *p, struct rdesc_token tk)
{
	if (is_grammar_complete(p->cur)) {
		p->cur = p->cur->parent;

		return RETRY;
	}

	if (is_construct_end(p->cur)) {
		rdesc_stack_push(&p->stack, tk);

		if (p->cur->parent) {
			backtrace(p);

			return CONTINUE;
		} else {
			return NOMATCH;
		}
	}

	struct rdesc_cfg_symbol rule = get_current_rule(p->cur);

	switch (rule.ty) {
	case CFG_TOKEN:
		if (rule.id == tk.id) {
			struct rdesc_node *n = new_tk_node(p->cur, rule.id);
			memcpy(&n->tk_, &tk, tk_size(*p));

			while (is_grammar_complete(p->cur)) {
				p->cur = p->cur->parent;

				if (p->cur == NULL)
					return READY;
			}

			return CONTINUE;
		} else {
			rdesc_stack_push(&p->stack, tk);
			next_variant(p);

			return CONTINUE;
		}

	case CFG_NONTERMINAL:
		p->cur = new_nt_node(p, p->cur, rule.id);

		return RETRY;

	default: unreachable(); // GCOV_EXCL_LINE
	} // GCOV_EXCL_LINE
}
