#include "../include/rdesc.h"
#include "../src/detail.h"

#include <stddef.h>
#include <stdint.h>


int main(void)
{
	struct rdesc p;

	rdesc_init(&p, 0, NULL);

	/* nonterminal without a child */
	rdesc_assert(sizeof_nt(0) == sizeof(struct _rdesc_priv_nt),
			"nonterminal size mismatch");

	/* 0-sized seminfo field */
	rdesc_assert(sizeof_tk(p) == sizeof(struct _rdesc_priv_tk)
			- sizeof(uint32_t),
			"token size mismatch");

	/* nonterminal size will be bigger than the token size, it will be
	 * used for union alignment */
	rdesc_assert(sizeof_node(p) == sizeof(struct _rdesc_priv_nt)
			+ sizeof(uint16_t) /* plus size of offset to previous */
			+ sizeof(size_t) /* plus size of parent pointer */,
			"node size mismatch");

	rdesc_destroy(&p);

	rdesc_init(&p, 32, NULL);

	rdesc_assert(sizeof_tk(p) == sizeof(struct _rdesc_priv_tk)
			- sizeof(uint32_t) /* minus dummy seminfo field */
			+ 32 /* plus user-specified seminfo size */,
			"token size mismatch");

	/* now the token size is greater than the nonterminal size */
	rdesc_assert(sizeof_node(p) == sizeof(struct _rdesc_priv_tk)
			- sizeof(uint32_t) + 32
			+ sizeof(uint16_t) + sizeof(size_t),
			"node size mismatch");

	rdesc_destroy(&p);
}
