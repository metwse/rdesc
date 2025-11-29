#include "../include/rdesc.h"
#include "../include/cfg.h"
#include "../include/util.h"
#include "../src/detail.h" // IWYU pragma: keep
#include "../src/exblex.h"

#include "../examples/grammar/boolean-algebra.h"


int main()
{
	struct exblex lex;
	struct rdesc_cfg cfg;
	struct rdesc p;

	exblex_init(&lex,
		    "{ a, b = (c = 0) | 1 & !test(1, 0), _123(); X_Y(); {;;} }",
		    balg_tks, BALG_TK_COUNT);

	rdesc_cfg_init(&cfg, BALG_NT_COUNT, BALG_NT_VARIANT_COUNT,
		       BALG_NT_BODY_LENGTH, (struct rdesc_cfg_symbol *) balg);
	rdesc_init(&p, &cfg);
	rdesc_start(&p, NT_STMT);

	struct rdesc_cfg_token tk;
	struct rdesc_node *cst = NULL;

	while ((tk = exblex_next(&lex)).id != 0)
		rdesc_pump(&p, &cst, &tk);

	assert(cst, "syntax tree could not be parsed");

	rdesc_dump_dot(cst, balg_tk_printer_with_free, balg_nt_names, stdout);

	rdesc_destroy(&p);
	rdesc_cfg_destroy(&cfg);
	rdesc_node_destroy(cst);
}
