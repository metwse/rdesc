/* Dump boolean algebra grammar to test dump_bnf feature. */

#include "../../include/grammar.h"
#include "../../include/util.h"

#include "../../examples/grammar/boolean_algebra.h"


int main(void)
{
	struct rdesc_grammar grammar;

	rdesc_grammar_init(&grammar,
			   BALG_NT_COUNT, BALG_NT_VARIANT_COUNT, BALG_NT_BODY_LENGTH,
			   (struct rdesc_grammar_symbol *) balg);

	rdesc_dump_bnf(stdout, &grammar, balg_tk_names, balg_nt_names);

	rdesc_grammar_destroy(&grammar);
}
