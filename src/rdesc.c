#include "../include/cfg.h"
#include "../include/cst_macros.h"
#include "../include/rdesc.h"
#include "../include/stack.h"
#include "detail.h"
#include "rdi.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>


void rdesc_init(struct rdesc *p,
		size_t seminfo_size,
		const struct rdesc_cfg *cfg)
{
	p->cfg = cfg;
	p->seminfo_size = seminfo_size;
	p->cur = 0;

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
	rdesc_assert(p->cur == 0, "cannot start during parse");

	new_nt_node(p, SIZE_MAX, start_symbol);
}

void rdesc_reset(struct rdesc *p /*,
		 rdesc_token_destroyer_func token_destroyer*/) {
	p->cur = 0;

	rdesc_stack_reset(&p->token_stack);
	rdesc_stack_reset(&p->cst_stack);
}

struct rdesc_node *_rdesc_priv_cst_illegal_access(struct rdesc *p, size_t index)
{
	return rdesc_stack_at(p->cst_stack, index);
}

RDI void push_child(struct rdesc *p,
		    size_t parent_idx,
		    size_t child_idx)
{
	node_t *parent = rdesc_stack_at(p->cst_stack, parent_idx);

	_rdesc_priv_child_idx(parent, rchild_count(parent)) = child_idx;

	rchild_count(parent)++;
}

RDI size_t new_nt_node(struct rdesc *p,
		       size_t parent_idx,
		       uint32_t nt_id)
{
	node_t *n = rdesc_stack_push(&p->cst_stack, NULL);
	size_t node_idx = rdesc_stack_len(p->cst_stack) - 1;

	if (parent_idx != SIZE_MAX)
		push_child(p, parent_idx, node_idx);

	rtype(n) = CFG_NONTERMINAL;

	_rdesc_priv_parent_idx(n) = parent_idx;
	rid(n) = nt_id;
	rvariant(n) = 0;
	rchild_count(n) = 0;

	size_t child_cap = p->cfg->child_caps[nt_id];
	size_t children_list_cap =
		(child_cap * sizeof(size_t) + sizeof_node(*p) - 1) / sizeof_node(*p);
	rdesc_stack_multipush(&p->cst_stack, NULL, children_list_cap);

	return node_idx;
}

RDI void new_tk_node(struct rdesc *p,
		     size_t parent_idx,
		     uint32_t tk_id,
		     const void *seminfo)
{
	node_t *n = rdesc_stack_push(&p->cst_stack, NULL);
	size_t node_id = rdesc_stack_len(p->cst_stack) - 1;

	push_child(p, parent_idx, node_id);

	rtype(n) = CFG_TOKEN;

	_rdesc_priv_parent_idx(n) = parent_idx;
	rid(n) = tk_id;

	if (seminfo)
		memcpy(rseminfo(n), seminfo, p->seminfo_size);
}
