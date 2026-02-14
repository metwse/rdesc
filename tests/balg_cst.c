/* Test dump_cst utility via dumping a boolean algebra statement. */

#include "../include/cfg.h"
#include "../include/cst_macros.h"
#include "../include/util.h"
#include "../include/rdesc.h"

#include "../src/detail.h"

#include "../examples/grammar/boolean_algebra.h"

#include <stddef.h>


void balg_node_printer(const struct rdesc_node *node, FILE *out)
{
	if (rtype(node) == CFG_TOKEN)
		fprintf(out, "[shape=record,label=\"%s\"]", balg_tk_names[rid(node)]);
	else
		fprintf(out, "[label=\"%s\"]", balg_nt_names[rid(node)]);
}


int main(void)
{
	struct rdesc_cfg cfg;
	struct rdesc p;

	rdesc_cfg_init(&cfg,
		BALG_NT_COUNT, BALG_NT_VARIANT_COUNT, BALG_NT_BODY_LENGTH,
		cast(struct rdesc_cfg_symbol *, balg));
	rdesc_init(&p, sizeof(uint32_t), &cfg);

	rdesc_start(&p, NT_STMT);

	struct rdesc_node *out = NULL;
	rdesc_pump(&p, NULL, TK_IDENT, NULL);
	rdesc_pump(&p, NULL, TK_LPAREN, NULL);
	rdesc_pump(&p, NULL, TK_LPAREN, NULL);
	rdesc_pump(&p, NULL, TK_IDENT, NULL);
	rdesc_pump(&p, NULL, TK_EQ, NULL);
	rdesc_pump(&p, NULL, TK_IDENT, NULL);
	rdesc_pump(&p, NULL, TK_RPAREN, NULL);
	rdesc_pump(&p, NULL, TK_RPAREN, NULL);
	rdesc_pump(&p, &out, TK_SEMI, NULL);

	rdesc_assert(out, "coud not parse");
	rdesc_dump_cst(stdout, &p, balg_node_printer);

	rdesc_destroy(&p, NULL);
	rdesc_cfg_destroy(&cfg);
}
