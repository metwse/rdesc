#include "../include/rdesc.h"
#include "../include/cfg.h"
#include "../include/util.h"
#include "../src/detail.h" // IWYU pragma: keep
#include "../src/exblex.h"

#include "../examples/grammar/boolean_algebra.h"

#include <stddef.h>
#include <stdlib.h>


/* Print token as a dotlang node. */
void balg_tk_printer_with_free(const struct rdesc_token *tk, FILE *out)
{
	if (tk->id == TK_IDENT) {
		fprintf(out, "{{ident|%s}}", (char *) tk->seminfo);

		free(tk->seminfo);
	} else {
		fprintf(out, "%s", balg_tk_names_escaped[tk->id]);
	}
}

void balg_tk_destroyer(struct rdesc_token *tk)
{
	if (tk->id == TK_IDENT)
		free(tk->seminfo);
}

int main(void)
{
	struct exblex lex;
	struct rdesc_cfg cfg;
	struct rdesc p;

	struct rdesc_token tk;
	struct rdesc_node *cst = NULL;

	enum rdesc_result pump_res;

	rdesc_cfg_init(&cfg, BALG_NT_COUNT, BALG_NT_VARIANT_COUNT,
		       BALG_NT_BODY_LENGTH, (struct rdesc_cfg_symbol *) balg);
	rdesc_init(&p, &cfg);

	rdesc_start(&p, NT_STMT);
	exblex_init(&lex,
		    "{ a, b = (c = 0) | 1 & !test(1, 0), _123(); X_Y(); {;;} }",
		    balg_tks, BALG_TK_COUNT);
	while ((tk = exblex_next(&lex)).id != 0)
		pump_res = rdesc_pump(&p, &cst, &tk);

	rdesc_assert(pump_res == RDESC_READY, "READY result expected");
	rdesc_assert(cst, "syntax tree could not be parsed");

	rdesc_dump_dot(cst, balg_tk_printer_with_free, balg_nt_names, stdout);
	rdesc_node_destroy(cst, NULL);

	exblex_init(&lex,
		    "a =;",
		    balg_tks, BALG_TK_COUNT);
	rdesc_start(&p, NT_STMT);
	while ((tk = exblex_next(&lex)).id != 0)
		pump_res = rdesc_pump(&p, NULL, &tk);

	rdesc_assert(pump_res == RDESC_NOMATCH, "NOMATCH result expected");
	rdesc_reset(&p, balg_tk_destroyer);

	rdesc_destroy(&p);
	rdesc_cfg_destroy(&cfg);
}
