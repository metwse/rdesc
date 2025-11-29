#include "../include/rdesc.h"
#include "../include/cfg.h"
#include "../src/detail.h"

#include "../examples/grammar/boolean-algebra.h"
#include "../examples/exutil.h"


int main()
{
	struct exblex lex;
	struct rdesc_cfg cfg;
	struct rdesc p;

	exblex_init(&lex,
		    "{ i, i = (i = 0) | 1 & !i(1, 0), i(); i(); {;;} }",
		    balg_tks, BALG_TK_COUNT);

	rdesc_cfg_init(&cfg, BALG_NT_COUNT, BALG_NT_VARIANT_COUNT,
		       BALG_NT_BODY_LENGTH, (struct rdesc_cfg_symbol *) balg);
	rdesc_init(&p, &cfg);
	rdesc_start(&p, NT_STMT);

	enum balg_tk id;
	struct rdesc_node *cst = NULL;
	while ((id = exblex_next(&lex)) != TK_NOTOKEN)
		rdesc_pump(&p, &cst, &(struct rdesc_cfg_token) { .id = id });

	assert(cst, "syntax tree could not be parsed");

	rdesc_destroy(&p);
	rdesc_cfg_destroy(&cfg);
	rdesc_node_destroy(cst);
}
