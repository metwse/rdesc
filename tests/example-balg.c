#include "../rdesc.h"
#include "../detail.h"

#include "../exmples/grammar/boolean-algebra.h"
#include "../exmples/exutil.h"


int main()
{
	struct exblex lex;
	struct rdesc p;

	exblex_init(&lex,
		    "{ i, i = (i = 0) | 1 & !i(1, 0), i(); i(); {;;} }",
		    balg_tks, BALG_TK_COUNT);

	rdesc_init(&p, BALG_NT_COUNT, BALG_NT_VARIANT_COUNT,
		   BALG_NT_BODY_LENGTH, (struct bnf_symbol *) balg);
	rdesc_start(&p, NT_STMT);

	enum balg_tk id;
	struct rdesc_node *cst = NULL;
	while ((id = exblex_next(&lex)) != TK_NOTOKEN) {
		rdesc_pump(&p, &cst, &(struct bnf_token) { .id = id });
	}

	assert(cst, "syntax tree could not be parsed");
}
