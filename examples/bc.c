#include "../include/rdesc.h"
#include "../include/cfg.h"
#include "../src/exblex.h"

#include "grammar/bc.h"

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


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
				printf("Invalid token, ignoring token at "
				       "index %zu!\n", lex->cur - 1);
			}

			if (pump_res == RDESC_NOMATCH) {
				printf("SYNTAX ERROR!\n");
				rdesc_reset(p, bc_tk_destroyer);

				break;
			}

			if (pump_res == RDESC_CONTINUE)
				printf("  ");
		}

		if (pump_res == RDESC_READY) {
			// TODO: interpret result
			rdesc_node_destroy(cst, bc_tk_destroyer);
			printf("< (result)\n");
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
