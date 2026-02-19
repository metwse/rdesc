/* Test dump_cst utility via dumping a boolean algebra statement. */

#include "../../include/grammar.h"
#include "../../include/cst_macros.h"
#include "../../include/util.h"
#include "../../include/rdesc.h"
#include "../../src/detail.h"

#include "../../examples/grammar/boolean_algebra.h"

#include <stddef.h>
#include <stdint.h>


void balg_node_printer(const struct rdesc_node *node, FILE *out)
{
	if (rtype(node) == RDESC_TOKEN)
		fprintf(out, "[shape=record,label=\"%s\"]", balg_tk_names[rid(node)]);
	else
		fprintf(out, "[label=\"%s\"]", balg_nt_names[rid(node)]);
}


int main(void)
{
	struct rdesc_grammar grammar;
	struct rdesc p;

	rdesc_grammar_init(&grammar,
			   BALG_NT_COUNT, BALG_NT_VARIANT_COUNT, BALG_NT_BODY_LENGTH,
			   cast(struct rdesc_grammar_symbol *, balg));
	rdesc_init(&p, &grammar, sizeof(uint32_t), NULL);

	rdesc_assert(rdesc_root(&p) == NULL,
		     "no root expected");

	rdesc_start(&p, NT_STMT);

	rdesc_pump(&p, TK_IDENT, NULL);
	rdesc_pump(&p, TK_LPAREN, NULL);
	rdesc_pump(&p, TK_LPAREN, NULL);
	rdesc_pump(&p, TK_IDENT, NULL);
	rdesc_pump(&p, TK_EQ, NULL);
	rdesc_pump(&p, TK_IDENT, NULL);
	rdesc_pump(&p, TK_RPAREN, NULL);
	rdesc_pump(&p, TK_RPAREN, NULL);

	rdesc_assert(rdesc_pump(&p, TK_SEMI, NULL) == RDESC_READY,
		     "coud not parse");
	rdesc_dump_cst(stdout, &p, balg_node_printer);

	rdesc_destroy(&p);
	rdesc_grammar_destroy(&grammar);
}
