#include "../include/rdesc.h"
#include "detail.h"

#include <stddef.h>
#include <stdint.h>


#ifndef INTERNAL_H
#define INTERNAL_H


/* helper functions for allocating symbol types */
RDI void new_nt_node(struct rdesc *p,
		     size_t parent,
		     uint32_t id);

RDI void new_tk_node(struct rdesc *p,
		     size_t parent,
		     uint32_t id,
		     const void *seminfo);

RDI void push_child(struct rdesc *p,
		    size_t parent_index,
		    size_t child_index);


#endif
