/* Pass an invalid syntax and expect the tokens are pushed back to token
 * stack. */

#include "../../include/grammar.h"
#include "../../include/rdesc.h"
#include "../../include/stack.h"
#include "../../src/detail.h"

#include "../../examples/grammar/boolean_algebra.h"

#include <stddef.h>
#include <stdint.h>


int main(void)
{
	struct rdesc_grammar grammar;
	struct rdesc p;

	rdesc_grammar_init(&grammar,
			   BALG_NT_COUNT, BALG_NT_VARIANT_COUNT, BALG_NT_BODY_LENGTH,
			   cast(struct rdesc_grammar_symbol *, balg));
	rdesc_init(&p, &grammar, sizeof(uint32_t), NULL);

	rdesc_start(&p, NT_STMT);

	rdesc_pump(&p, TK_IDENT, NULL);
	rdesc_pump(&p, TK_LPAREN, NULL);
	rdesc_pump(&p, TK_LPAREN, NULL);
	rdesc_pump(&p, TK_IDENT, NULL);
	rdesc_pump(&p, TK_EQ, NULL);
	rdesc_pump(&p, TK_IDENT, NULL);
	rdesc_pump(&p, TK_RPAREN, NULL);
	rdesc_pump(&p, TK_RPAREN, NULL);
	rdesc_assert(rdesc_stack_len(p.cst_stack) != 0,
	      "grammar is valid so the CST expected to be not empty");

	rdesc_pump(&p, TK_RPAREN, NULL);
	rdesc_assert(rdesc_stack_len(p.cst_stack) == 0,
	      "nomatch should teardown the CST");

	rdesc_assert(rdesc_stack_len(p.token_stack) == 9,
	      "tokens should be pushed back to stack due to teardown ");

	rdesc_destroy(&p);
	rdesc_grammar_destroy(&grammar);
}
