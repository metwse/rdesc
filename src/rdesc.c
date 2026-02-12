#include "../include/cfg.h"
#include "../include/rdesc.h"
#include "../include/stack.h"

#include "detail.h"
#include "internal.h"
#include "types.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>


#define ty_ n.ty
#define nt_ n.nt
#define tk_ n.tk


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

RDI void push_child(struct rdesc *p,
		    size_t parent_index,
		    size_t child_index)
{
	node_t *parent = rdesc_stack_at(p->cst_stack, parent_index);

	memcpy(&parent->_[parent->nt_.child_count * sizeof(size_t)],
	       &child_index, sizeof(size_t));

	parent->nt_.child_count++;
}

RDI void new_nt_node(struct rdesc *p,
		     size_t parent,
		     uint32_t id)
{
	node_t *n = rdesc_stack_push(&p->cst_stack, NULL);

	if (parent != SIZE_MAX)
		push_child(p, parent, rdesc_stack_len(p->cst_stack));
	n->parent = SIZE_MAX;
	n->nt_.id = id;
	n->nt_.child_count = 0;

	size_t child_cap = p->cfg->child_caps[id];
	rdesc_stack_multipush(&p->cst_stack, NULL,
		(child_cap * sizeof(size_t) + sizeof_node(*p) - 1) / sizeof_node(*p));
}

RDI void new_tk_node(struct rdesc *p,
		     size_t parent,
		     uint32_t id,
		     const void *seminfo)
{
	node_t *n = rdesc_stack_push(&p->cst_stack, NULL);

	push_child(p, parent, rdesc_stack_len(p->cst_stack));
	n->parent = parent;
	n->tk_.id = id;
	memcpy(&n->tk_.seminfo, seminfo, p->seminfo_size);
}
