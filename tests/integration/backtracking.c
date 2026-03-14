/* Pass an invalid syntax and expect the tokens are pushed back to token
 * stack. */

#include "../../include/grammar.h"
#include "../../include/rdesc.h"
#include "../../include/stack.h"
#include "../../src/common.h"

#include "../../examples/grammar/boolean_algebra.h"

#include <stddef.h>
#include <stdint.h>


int main(void)
{
	struct rdesc_grammar grammar;
	struct rdesc p;

	unwrap(rdesc_grammar_init(&grammar,
				  BALG_NT_COUNT, BALG_NT_VARIANT_COUNT, BALG_NT_BODY_LENGTH,
				  cast(struct rdesc_grammar_symbol *, balg)));
	unwrap(rdesc_init(&p, &grammar, sizeof(uint32_t), NULL));

	unwrap(rdesc_start(&p, NT_STMT));

	rdesc_assert(rdesc_pump(&p, TK_IDENT, NULL) == RDESC_CONTINUE,);
	rdesc_assert(rdesc_pump(&p, TK_LPAREN, NULL) == RDESC_CONTINUE,);
	rdesc_assert(rdesc_pump(&p, TK_LPAREN, NULL) == RDESC_CONTINUE,);
	rdesc_assert(rdesc_pump(&p, TK_IDENT, NULL) == RDESC_CONTINUE,);
	rdesc_assert(rdesc_pump(&p, TK_EQ, NULL) == RDESC_CONTINUE,);
	rdesc_assert(rdesc_pump(&p, TK_IDENT, NULL) == RDESC_CONTINUE,);
	rdesc_assert(rdesc_pump(&p, TK_RPAREN, NULL) == RDESC_CONTINUE,);
	rdesc_assert(rdesc_pump(&p, TK_RPAREN, NULL) == RDESC_CONTINUE,);
	rdesc_assert(rdesc_stack_len(p.cst_stack) != 0,
	      "grammar is valid so the CST expected to be not empty");

	rdesc_assert(rdesc_pump(&p, TK_RPAREN, NULL) == RDESC_NOMATCH,);
	rdesc_assert(rdesc_stack_len(p.cst_stack) == 0,
	      "nomatch should teardown the CST");

	rdesc_assert(rdesc_stack_len(p.token_stack) == 9,
	      "tokens should be pushed back to stack due to teardown ");

	rdesc_destroy(&p);
	rdesc_grammar_destroy(&grammar);
}
