/**
 * @file bc_interpreter.h
 * @brief Basic interpreter for bc.
 */

#include "../../include/rdesc.h"
#include "../../include/cst_macros.h"
#include "../../src/detail.h"

#include "../grammar/bc.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>


#ifndef BC_INTERPRETER_H
#define BC_INTERPRETER_H


/** @brief Raises 10 to the power of i */
static inline double bc_pow10(int i)
{
	double r = 1;

	while (i--)
		r *= 10;

	return r;
}

/** @brief Interprets CST of bc */
static inline double bc_interpreter(struct rdesc *p, struct rdesc_node *n)
{
	size_t v = rvariant(n);

	/* for use in str->num serialization in NT_UNSIGNED */
	char **decimal_part, **floating_part;
	double converted;

	switch (rid(n)) {
	case NT_UNSIGNED:
		switch (v) {
		case 0:
			decimal_part = rseminfo(rchild(p, n, 0));

			converted = strtod(*decimal_part, NULL);

			free(*decimal_part);
			return converted;
		case 1:
			floating_part = rseminfo(rchild(p, n, 1));

			converted = strtod(*floating_part, NULL) /
					   bc_pow10(strlen(*floating_part));

			free(*floating_part);
			return converted;
		default:
			decimal_part = rseminfo(rchild(p, n, 0));
			floating_part = rseminfo(rchild(p, n, 2));

			converted = strtod(*decimal_part, NULL) +
					   strtod(*floating_part, NULL) /
					   bc_pow10(strlen(*floating_part));

			free(*decimal_part);
			free(*floating_part);
			return converted;
		}

	case NT_OPTSIGN:
		return (v == 0) ? -1 : 1;

	case NT_SIGNED:
		return bc_interpreter(p, rchild(p, n, 0)) * bc_interpreter(p, rchild(p, n, 1));

	case NT_EXPR:
		return bc_interpreter(p, rchild(p, n, 0)) + bc_interpreter(p, rchild(p, n, 1));

	case NT_EXPR_REST:
		switch (v) {
		case 0:
			return bc_interpreter(p, rchild(p, n, 1));
		case 1:
			return -bc_interpreter(p, rchild(p, n, 1));
		default:
			return 0;
		}

	case NT_TERM:
		return bc_interpreter(p, rchild(p, n, 0)) * bc_interpreter(p, rchild(p, n, 1));

	case NT_TERM_REST:
		switch (v) {
		case 0:
			return bc_interpreter(p, rchild(p, n, 1));
		case 1:
			return 1 / bc_interpreter(p, rchild(p, n, 1));
		default:
			return 1;
		}

	case NT_FACTOR:
		switch (v) {
		case 0:
			return bc_interpreter(p, rchild(p, n, 0));
		default:
			return bc_interpreter(p, rchild(p, n, 1));
		}

	case NT_STMT:
		return bc_interpreter(p, rchild(p, n, 0));
	}

	unreachable(); // GCOV_EXCL_LINE
}

/** @brief Frees seminfo of a bc token */
static inline void bc_tk_destroyer(uint16_t tk, void *seminfo_)
{
	if (tk == TK_NUM) {
		void *seminfo;
		memcpy(&seminfo, seminfo_, sizeof(void *));

		if (seminfo)
			free(seminfo);
	}
}


#endif
