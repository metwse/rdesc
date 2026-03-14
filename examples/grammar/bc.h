/**
 * @file bc.h
 * @brief Basic calculator.
 *
 * A lightweight arithmetic expression evaluator designed to demonstrate
 * the capabilities of librdesc in handling standard algebraic grammars.
 *
 * @note Disambiguation: This module is not related to the POSIX `bc`
 *       (arbitrary-precision calculator language). It is a minimal
 *       demonstration implementation named strictly for "Basic Calculator".
 *
 * Language features:
 * - Arithmetic operators (+, -, *, /)
 * - Operator precedence
 * - Grouping using parentheses ( ... ) to override standard precedence.
 * - Integer/floating point literals
 *
 * @see This header provides the exact same api with boolean_algebra.h. You may
 *      inspect source code of bc.h.
 */

/** @cond */
#ifndef BC_H
#define BC_H

#include "../../include/grammar.h"
#include "../../include/rule_macros.h"


#define BC_TK_COUNT 11

#define BC_NT_COUNT 12
#define BC_NT_VARIANT_COUNT 4
#define BC_NT_BODY_LENGTH 5

enum bc_tk {
	TK_NOTOKEN,
	TK_NUM, TK_DOT,
	TK_MINUS, TK_PLUS, TK_MULT, TK_DIV,
	TK_LPAREN, TK_RPAREN, TK_ENDSYM,

	TK_DUMMY_AMBIGUITY_TRIGGER,
};

enum bc_nt {
	NT_UNSIGNED_NUM, NT_SIGNED_NUM,

	NT_EXPR, NT_EXPR_REST, NT_EXPR_OP,
	NT_TERM, NT_TERM_REST, NT_TERM_OP,
	NT_FACTOR, NT_OPTSIGN,
	NT_ATOM,

	NT_STMT,
};

const char bc_tks[BC_TK_COUNT + 1 /* for null-terminator */] = {
	'\0',
	'd', '.',
	'-', '+', '*', '/',
	'(', ')', ';',
	'?',

	'\0' /* required for exblex */
};

const char *const bc_nt_names[BC_NT_COUNT] = {
	"unsigned", "optsign", "sign",

	"expr", "expr_rest", "expr_op",
	"term", "term_rest", "term_op",
	"factor",

	"stmt",
};

static const struct rdesc_grammar_symbol
bc[BC_NT_COUNT][BC_NT_VARIANT_COUNT][BC_NT_BODY_LENGTH] = {
	/* <unsigned_num> ::= */ r(
		TK(NUM)
	alt	TK(DOT), TK(NUM)
	alt	TK(NUM), TK(DOT), TK(NUM)
	),
	/* <signed_num> ::= */ r(
		NT(OPTSIGN), NT(UNSIGNED_NUM)
	),


	/* <expr> ::= */
		rrr(EXPR, (NT(TERM)), (NT(EXPR_OP), NT(TERM))),
	/* <expr_op> ::= */ r(
		TK(PLUS)
	alt	TK(MINUS)
	),

	/* <term> ::= */
		rrr(TERM, (NT(FACTOR)), (NT(TERM_OP), NT(FACTOR))),
	/* <term_op> ::= */ r(
		TK(MULT)
	alt	TK(DIV)
	),

	/* <factor> ::= */ r(
		NT(OPTSIGN), NT(ATOM)
	),
	/* <optsign> ::= */ r(
		TK(MINUS)
	alt	TK(PLUS)
	alt	EPSILON
	),

	/* <atom> ::= */ r(
		NT(SIGNED_NUM)
	alt	TK(LPAREN), NT(EXPR), TK(RPAREN)
	alt	TK(LPAREN), NT(EXPR), TK(RPAREN), TK(DUMMY_AMBIGUITY_TRIGGER)
	),


	/* <stmt> ::= */ r(
		NT(EXPR), TK(ENDSYM)
	)
};


#endif
/** @endond */
