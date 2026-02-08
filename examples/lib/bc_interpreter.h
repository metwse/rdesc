/**
 * @file bc_interpreter.h
 * @brief Basic interpreter for bc.
 */

#include "../../include/rdesc.h"
#include "../../src/detail.h"

#include "../grammar/bc.h"

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
static inline double bc_interpreter(struct rdesc_node *n)
{
	size_t v = n->n.nt.variant; /* variant */
	struct rdesc_node **c = n->n.nt.children; /* children */

	switch (n->n.nt.id) {
	case NT_UNSIGNED:
		switch (v) {
		case 0:
			return strtod(*cast(char **, c[0]->n.tk.seminfo), NULL);
		case 1:
			return strtod(*cast(char **, c[1]->n.tk.seminfo), NULL) /\
				bc_pow10(strlen(c[1]->n.tk.seminfo));
		default:
			return strtod(*cast(char **, c[0]->n.tk.seminfo), NULL) + \
				strtod(*cast(char **, c[2]->n.tk.seminfo), NULL) / \
				bc_pow10(strlen(c[2]->n.tk.seminfo));
		}

	case NT_OPTSIGN:
		return (v == 0) ? -1 : 1;

	case NT_SIGNED:
		return bc_interpreter(c[0]) * bc_interpreter(c[1]);

	case NT_EXPR:
		return bc_interpreter(c[0]) + bc_interpreter(c[1]);

	case NT_EXPR_REST:
		switch (v) {
		case 0:
			return bc_interpreter(c[1]);
		case 1:
			return -bc_interpreter(c[1]);
		default:
			return 0;
		}

	case NT_TERM:
		return bc_interpreter(c[0]) * bc_interpreter(c[1]);

	case NT_TERM_REST:
		switch (v) {
		case 0:
			return bc_interpreter(c[1]);
		case 1:
			return 1 / bc_interpreter(c[1]);
		default:
			return 1;
		}

	case NT_FACTOR:
		switch (v) {
		case 0:
			return bc_interpreter(c[0]);
		default:
			return bc_interpreter(c[1]);
		}

	case NT_STMT:
		return bc_interpreter(c[0]);
	}

	unreachable(); // GCOV_EXCL_LINE
}

/** @brief Frees seminfo of a bc token */
static inline void bc_tk_destroyer(struct rdesc_token *tk)
{
	void *seminfo;
	memcpy(&seminfo, tk->seminfo, sizeof(void *));

	if (seminfo)
		free(seminfo);
}


#endif
