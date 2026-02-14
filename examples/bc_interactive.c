#include "../include/rdesc.h"
#include "../include/cfg.h"
#include "../src/exblex.h"

#include "grammar/bc.h"
#include "lib/bc_interpreter.h"

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>


void program(struct exblex *lex, struct rdesc *p)
{
	char buf[4096];

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

			exblex_init(lex, buf, bc_tks);

			uint16_t tk;
			while ((tk = exblex_next(lex)) != 0) {
				char *seminfo = exblex_current_seminfo(lex);
				pump_res =
					rdesc_pump(p, &cst, tk,
					           &seminfo);

				if (pump_res == RDESC_CONTINUE)
					continue;

				break;
			}

			if (tk == 0 && lex->cur <= strlen(buf)) {
				printf("  %*s", (int) lex->cur, "^\n");
				printf("Invalid token, ignoring tokens after "
				       "index %zu!\n", lex->cur);
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
			printf("< (%.2lf)\n", bc_interpreter(p, cst));
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
	rdesc_init(&p, &cfg, sizeof(void *) /* semantic info holds char* */);

	printf("Basic Calculator, librdesc sample program\n");
	program(&lex, &p);

	rdesc_destroy(&p, NULL);
	rdesc_cfg_destroy(&cfg);
}
