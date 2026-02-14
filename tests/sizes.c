/* Validate the sizes of node types to ensure packing does not break memory
 * layout. */

#include "../include/rdesc.h"
#include "../src/detail.h"

#include <stddef.h>
#include <stdint.h>


int main(void)
{
	struct rdesc p;

	rdesc_init(&p, NULL, 0);

	/* nonterminal without a child */
	rdesc_assert(sizeof_nt(0) == sizeof(nt_t),
			"nonterminal size mismatch");

	/* 0-sized seminfo field */
	rdesc_assert(sizeof_tk(p) == sizeof(tk_t)
			- sizeof(uint32_t),
			"token size mismatch");

	/* nonterminal size will be bigger than the token size, it will be
	 * used for union alignment */
	rdesc_assert(sizeof_node(p) == sizeof(nt_t)
			+ sizeof(uint16_t) /* plus size of offset to previous */
			+ sizeof(size_t) /* plus size of parent pointer */,
			"node size mismatch");

	rdesc_destroy(&p, NULL);

	rdesc_init(&p, NULL, 32);

	rdesc_assert(sizeof_tk(p) == sizeof(tk_t)
			- sizeof(uint32_t) /* minus dummy seminfo field */
			+ 32 /* plus user-specified seminfo size */,
			"token size mismatch");

	/* now the token size is greater than the nonterminal size */
	rdesc_assert(sizeof_node(p) == sizeof(tk_t)
			- sizeof(uint32_t) + 32
			+ sizeof(uint16_t) + sizeof(size_t),
			"node size mismatch");

	rdesc_destroy(&p, NULL);
}
