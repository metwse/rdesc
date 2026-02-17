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

#define PREFIX_TK(tk) TK_ ## tk
#define PREFIX_NT(nt) NT_ ## nt

#include "../../include/bnf_macros.h"


#define BC_TK_COUNT 11

#define BC_NT_COUNT 9
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
	NT_UNSIGNED, NT_OPTSIGN, NT_SIGNED,

	NT_EXPR, NT_EXPR_REST,
	NT_TERM, NT_TERM_REST,
	NT_FACTOR,

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

	"expr", "expr_rest",
	"term", "term_rest",
	"factor",

	"stmt",
};

static const struct rdesc_grammar_symbol
bc[BC_NT_COUNT][BC_NT_VARIANT_COUNT][BC_NT_BODY_LENGTH] = {
	/* <unsigned> ::= */ r(
		TK(NUM),
	alt	TK(DOT), TK(NUM),
	alt	TK(NUM), TK(DOT), TK(NUM),
	),
	/* <optsign> ::= */ r(
		TK(MINUS),
	alt	TK(PLUS),
	alt	EPSILON,
	),
	/* <signed> ::= */ r(
		NT(OPTSIGN), NT(UNSIGNED),
	),


	/* <expr> ::= */ r(
		NT(TERM), NT(EXPR_REST),
	),
	/* <expr_rest> ::= */ r(
		TK(PLUS), NT(EXPR),
	alt	TK(MINUS), NT(EXPR),
	alt	EPSILON,
	),

	/* <term> ::= */ r(
		NT(FACTOR), NT(TERM_REST),
	),
	/* <term_rest> ::= */ r(
		TK(MULT), NT(TERM),
	alt	TK(DIV), NT(TERM),
	alt	EPSILON,
	),

	/* <factor> ::= */ r(
		NT(SIGNED),
	alt	TK(LPAREN), NT(EXPR), TK(RPAREN),
	alt	TK(LPAREN), NT(EXPR), TK(RPAREN), TK(DUMMY_AMBIGUITY_TRIGGER),
	),


	/* <stmt> ::= */ r(
		NT(EXPR), TK(ENDSYM),
	),
};


#endif
/** @endcond */
