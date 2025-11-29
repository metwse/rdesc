#include "../include/rdesc.h"
#include "../include/cfg.h"
#include "../include/bnf_dsl.h"
#include "../include/stack.h"
#include "detail.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>


#define get_body(node) productions(*p->cfg)[node->nt.id][node->nt.variant]
#define get_current_rule(node) get_body(node)[node->nt.child_count]

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
static struct rdesc_node *new_nt_node(struct rdesc *, struct rdesc_node *, int);
static struct rdesc_node *new_tk_node(struct rdesc_node *, int);

/* increment variant counter, trigger backgtracing if required */
static void next_variant(struct rdesc *);
static void backtrace(struct rdesc *);

/* internal token matching mechanism */
static enum match_result match(struct rdesc *, struct rdesc_cfg_token);



void rdesc_init(struct rdesc *p,
		const struct rdesc_cfg *cfg)
{
	p->cfg = cfg;

	rdesc_stack_init(&p->stack);

	p->root = p->cur = NULL;
}

void rdesc_start(struct rdesc *p, int start_symbol)
{
	assert_logic(p->root == NULL, "setting start symbol during parsing");

	p->cur = p->root = new_nt_node(p, NULL, start_symbol);
}

void rdesc_node_destroy(struct rdesc_node *n)
{
	if (n->ty == CFG_NONTERMINAL) {
		for (size_t i = n->nt.child_count; i > 0; i--)
			rdesc_node_destroy(n->nt.children[i - 1]);

		free(n->nt.children);
	}

	free(n);
}

void rdesc_clearstack(struct rdesc *p, struct rdesc_cfg_token **out, size_t *out_len)
{
	assert_logic(p->root == NULL, "clearing symbol stack during parsing");

	*out = rdesc_stack_into_inner(&p->stack);
	*out_len = rdesc_stack_len(&p->stack);

	rdesc_stack_init(&p->stack);
}

void rdesc_destroy(struct rdesc *p)
{
	assert_logic(p->root == NULL, "destroying parser during parsing");
	assert(rdesc_stack_len(&p->stack) == 0,
			       "cannot destroy parser if token stack is not "
			       "empty");

	rdesc_stack_destroy(&p->stack);
}

enum rdesc_result rdesc_pump(struct rdesc *p,
			     struct rdesc_node **out,
			     struct rdesc_cfg_token *incoming_tk)
{
	assert_logic(p->root,
		     "continuing an incremental parse with no root");

	enum match_result res;
	struct rdesc_cfg_token tk = *incoming_tk;

	bool has_token = incoming_tk != NULL;

	while (true) {
		if (!has_token && rdesc_stack_len(&p->stack)) {
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


static struct rdesc_node *new_nt_node(struct rdesc *p,
				      struct rdesc_node *parent,
				      int id)
{
	struct rdesc_node *n = malloc(sizeof(struct rdesc_node));
	assert_mem(n);

	n->parent = parent;
	if (parent)
		push_child(parent, n);

	n->ty = CFG_NONTERMINAL;

	size_t child_cap = p->cfg->child_caps[id];
	assert_logic(child_cap, "a nonterminal with no child");

	n->nt.id = id;
	n->nt.child_count = 0;
	n->nt.children = malloc(sizeof(struct rdesc_node *) * child_cap);
	assert_mem(n->nt.children);
	n->nt.variant = 0;

	return n;
}

static struct rdesc_node *new_tk_node(struct rdesc_node *parent, int id)
{
	struct rdesc_node *n = malloc(sizeof(struct rdesc_node));
	assert_mem(n);

	n->parent = parent;
	if (parent)
		push_child(parent, n);

	n->ty = CFG_TOKEN;

	n->tk.id = id;

	return n;
}

static void push_child(struct rdesc_node *parent, struct rdesc_node *child)
{
	assert_logic(parent->ty == CFG_NONTERMINAL,
		     "a token node cannot be a parent of another node");

	parent->nt.children[parent->nt.child_count++] = child;
}

static void next_variant(struct rdesc *p)
{
	while (p->cur->nt.child_count) {
		struct rdesc_node *next_cur = NULL, *child;

		for (size_t i = p->cur->nt.child_count; i > 0; i--) {
			child = p->cur->nt.children[i - 1];

			if (child->ty == CFG_NONTERMINAL) {
				next_cur = child;

				break;
			} else {
				p->cur->nt.child_count--;

				rdesc_stack_push(&p->stack, child->tk);
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

static void backtrace(struct rdesc *p)
{
	struct rdesc_node *parent = p->cur->parent, *child = NULL;

	while (child != p->cur)
		rdesc_node_destroy(
			child = parent->nt.children[--parent->nt.child_count]
		);
	p->cur = parent;

	next_variant(p);

	if (is_construct_end(p->cur) && p->cur->parent)
		backtrace(p);
}

static enum match_result match(struct rdesc *p, struct rdesc_cfg_token tk)
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
			new_tk_node(p->cur, rule.id)->tk.seminfo = tk.seminfo;

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
	}
}
