/* header exposing rdesc internal functions to tests suite */

#ifndef RDI_H
#define RDI_H

#include "../include/rdesc.h"
#include "detail.h"


/* Pushes a new nonterminal to parser's CST stack and reserves space for its
 * children. */
RDI size_t new_nt_node(struct rdesc *p,
		     size_t parent,
		     uint32_t id);

/* Creates a new node in parser's CST stack and copies `seminfo` into it. */
RDI void new_tk_node(struct rdesc *p,
		     size_t parent,
		     uint32_t id,
		     const void *seminfo);

/* Makes the connection between parent and child, by adding `child_index` to
 * parent's children index list. */
RDI void push_child(struct rdesc *p,
		    size_t parent_index,
		    size_t child_index);


#endif
