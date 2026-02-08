#include "../include/rdesc.h"
#include "../include/cfg.h"
#include "../src/exblex.h"

#include "grammar/bc.h"
#include "lib/bc_interpreter.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>


void program(struct exblex *lex, struct rdesc *p)
{
	char buf[4096];

	struct rdesc_token tk;
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
			printf("< (%.2lf)\n", bc_interpreter(cst));
			rdesc_node_destroy(cst, bc_tk_destroyer);
		}
	}
}

int main(void)
{
	struct exblex lex;
	struct rdesc_cfg cfg;
	struct rdesc p;

	rdesc_cfg_init(&cfg, BC_NT_COUNT, BC_NT_VARIANT_COUNT,
		       BC_NT_BODY_LENGTH, (struct rdesc_cfg_symbol *) bc);
	rdesc_init(&p, 8 /* TODO: why 8? */, &cfg);

	printf("Basic Calculator, librdesc sample program\n");
	program(&lex, &p);

	rdesc_destroy(&p);
	rdesc_cfg_destroy(&cfg);
}
