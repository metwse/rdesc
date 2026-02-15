/* Test dump_cst utility via dumping a boolean algebra statement. */

#include "../../include/cfg.h"
#include "../../include/cst_macros.h"
#include "../../include/util.h"
#include "../../include/rdesc.h"
#include "../../src/detail.h"

#include "../../examples/grammar/boolean_algebra.h"

#include <stddef.h>
#include <stdint.h>


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
	rdesc_init(&p, &cfg, sizeof(uint32_t));

	rdesc_start(&p, NT_STMT);

	struct rdesc_node *out = NULL;
	uint16_t id;
	id = TK_IDENT; rdesc_pump(&p, NULL, &id, NULL);
	id = TK_LPAREN; rdesc_pump(&p, NULL, &id, NULL);
	id = TK_LPAREN; rdesc_pump(&p, NULL, &id, NULL);
	id = TK_IDENT; rdesc_pump(&p, NULL, &id, NULL);
	id = TK_EQ; rdesc_pump(&p, NULL, &id, NULL);
	id = TK_IDENT; rdesc_pump(&p, NULL, &id, NULL);
	id = TK_RPAREN; rdesc_pump(&p, NULL, &id, NULL);
	id = TK_RPAREN; rdesc_pump(&p, NULL, &id, NULL);
	id = TK_SEMI; rdesc_pump(&p, &out, &id, NULL);

	rdesc_assert(out, "coud not parse");
	rdesc_dump_cst(stdout, &p, balg_node_printer);

	rdesc_destroy(&p, NULL);
	rdesc_cfg_destroy(&cfg);
}
