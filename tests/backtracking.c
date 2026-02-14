#include "../include/cfg.h"
#include "../include/rdesc.h"
#include "../include/stack.h"

#include "../src/detail.h"

#include "../examples/grammar/boolean_algebra.h"

#include <stddef.h>


int main(void)
{
	struct rdesc_cfg cfg;
	struct rdesc p;

	rdesc_cfg_init(&cfg,
		BALG_NT_COUNT, BALG_NT_VARIANT_COUNT, BALG_NT_BODY_LENGTH,
		cast(struct rdesc_cfg_symbol *, balg));
	rdesc_init(&p, sizeof(uint32_t), &cfg);

	rdesc_start(&p, NT_STMT);

	rdesc_pump(&p, NULL, TK_IDENT, NULL);
	rdesc_pump(&p, NULL, TK_LPAREN, NULL);
	rdesc_pump(&p, NULL, TK_LPAREN, NULL);
	rdesc_pump(&p, NULL, TK_IDENT, NULL);
	rdesc_pump(&p, NULL, TK_EQ, NULL);
	rdesc_pump(&p, NULL, TK_IDENT, NULL);
	rdesc_pump(&p, NULL, TK_RPAREN, NULL);
	rdesc_pump(&p, NULL, TK_RPAREN, NULL);
	rdesc_pump(&p, NULL, TK_RPAREN, NULL);

	rdesc_assert(rdesc_stack_len(p.cst_stack) == 0,
	      "nomatch should teardown the CST");

	rdesc_assert(rdesc_stack_len(p.token_stack) == 9,
	      "tokens should be pushed back to stack due to teardown ");

	rdesc_destroy(&p);
	rdesc_cfg_destroy(&cfg);
}
