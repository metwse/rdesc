#include "../include/rdesc.h"
#include "../include/cfg.h"
#include "../src/exblex.h"
#include "../src/detail.h"

#include "grammar/bc.h"

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


double pow10(int i)
{
	double r = 1;

	while (i--)
		r *= 10;

	return r;
}

double interpret(struct rdesc_node *n)
{
	size_t v = n->nt.variant; /* variant */
	struct rdesc_node **c = n->nt.children; /* children */

	switch (n->nt.id) {
	case NT_UNSIGNED:
		switch (v) {
		case 0:
			return strtod(c[0]->tk.seminfo, NULL);
		case 1:
			return strtod(c[1]->tk.seminfo, NULL) /\
				pow10(strlen(c[1]->tk.seminfo));
		default:
			return strtod(c[0]->tk.seminfo, NULL) + \
				strtod(c[2]->tk.seminfo, NULL) / \
				pow10(strlen(c[2]->tk.seminfo));
		}

	case NT_OPTSIGN:
		return (v == 0) ? -1 : 1;

	case NT_SIGNED:
		return interpret(c[0]) * interpret(c[1]);

	case NT_EXPR:
		return interpret(c[0]) + interpret(c[1]);

	case NT_EXPR_REST:
		switch (v) {
		case 0:
			return interpret(c[1]);
		case 1:
			return -interpret(c[1]);
		default:
			return 0;
		}

	case NT_TERM:
		return interpret(c[0]) * interpret(c[1]);

	case NT_TERM_REST:
		switch (v) {
		case 0:
			return interpret(c[1]);
		case 1:
			return 1 / interpret(c[1]);
		default:
			return 1;
		}

	case NT_FACTOR:
		switch (v) {
		case 0:
			return interpret(c[0]);
		default:
			return interpret(c[1]);
		}

	case NT_STMT:
		return interpret(c[0]);
	}

	unreachable(); // GCOV_EXCL_LINE
}

void bc_tk_destroyer(struct rdesc_cfg_token *tk)
{
	if (tk->seminfo)
		free(tk->seminfo);
}

void program(struct exblex *lex, struct rdesc *p)
{
	char buf[4096];

	struct rdesc_cfg_token tk;
	struct rdesc_node *cst;

	int pump_res;

	while (true) {
		rdesc_start(p, NT_STMT);

		pump_res = -1;

		printf("> ");
		while (pump_res != RDESC_READY) {
			if (fgets(buf, 4096, stdin) == NULL) {
				rdesc_reset(p, bc_tk_destroyer);

				return;
			}

			exblex_init(lex, buf, bc_tks, BC_TK_COUNT);

			while ((tk = exblex_next(lex)).id != 0) {
				pump_res = rdesc_pump(p, &cst, &tk);

				if (pump_res == RDESC_CONTINUE)
					continue;

				break;
			}

			if (tk.id == 0 && lex->cur <= strlen(buf)) {
				printf("  %*s", (int) lex->cur, "^\n");
				printf("Invalid token, ignoring tokens after "
				       "index %zu!\n", lex->cur - 1);
			}

			if (pump_res == RDESC_NOMATCH) {
				printf("SYNTAX ERROR!\n");
				rdesc_reset(p, bc_tk_destroyer);

				break;
			}

			if (pump_res == RDESC_CONTINUE || pump_res == -1)
				printf("  ");
		}

		if (pump_res == RDESC_READY) {
			printf("< (%.2lf)\n", interpret(cst));
			rdesc_node_destroy(cst, bc_tk_destroyer);
		}
	}
}

int main()
{
	struct exblex lex;
	struct rdesc_cfg cfg;
	struct rdesc p;

	rdesc_cfg_init(&cfg, BC_NT_COUNT, BC_NT_VARIANT_COUNT,
		       BC_NT_BODY_LENGTH, (struct rdesc_cfg_symbol *) bc);
	rdesc_init(&p, &cfg);

	printf("Basic Calculator, librdesc sample program\n");
	program(&lex, &p);

	rdesc_destroy(&p);
	rdesc_cfg_destroy(&cfg);
}
