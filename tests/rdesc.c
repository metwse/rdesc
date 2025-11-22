#include "../rdesc.h"
#include "../bnf_dsl.h"


enum token {
	TK_NUM, TK_IDENT,
	TK_PLUS, TK_STAR,
	TK_LPAREN, TK_RPAREN,
	TK_COMMA,
};

enum nonterminal {
	NT_NUM, NT_IDENT,
	NT_EXPR, NT_EXPR_REST,
	NT_TERM, NT_TERM_REST,
	NT_CALL, NT_CALL_OPTPARAMS,
	NT_CALL_PARAMS, NT_CALL_PARAMS_REST,
	NT_FACTOR,
};


const struct bnf_symbol productions[11][3][5] = {
	/* <num> ::= */ r(
		TK(TK_NUM),
	),
	/* <ident> ::= */ r(
		TK(TK_IDENT),
	),

	/* <expr> ::= */
		rrr(NT_EXPR, NT_TERM, TK_PLUS),
	/* <term> ::= */
		rrr(NT_TERM, NT_FACTOR, TK_STAR),

	/* <call> ::= */ r(
		TK(NT_IDENT), TK(TK_LPAREN), NT(NT_CALL_OPTPARAMS), TK(TK_RPAREN),
	),
	/* <call_optparams> ::= */
		ropt(NT(NT_CALL_PARAMS)),
	/* <call_params> ::= */
		rrr(NT_CALL_PARAMS, NT_EXPR, TK_COMMA),

	/* <factor> ::= */ r(
		NT(NT_IDENT),
	alt	NT(NT_NUM),
	),
};


int main()
{
	struct rdesc p;

	rdesc_init(&p, 11, 3, 5, (const struct bnf_symbol *) productions);

	rdesc_start(&p, NT_EXPR);
}
