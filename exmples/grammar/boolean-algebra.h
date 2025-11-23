/**
 * @file boolean-algebra.h
 * @brief Boolean Algebra Grammar Example for librdesc.
 *
 * This file defines a sample Context-Free Grammar (CFG) for a simple
 * Boolean Algebra language. It serves two main purposes:
 * 1. To demonstrate how to use the BNF DSL (`bnf_dsl.h`) to define grammars.
 * 2. To act as a stress-test for the parser's backtracking engine by
 *    introducing intentional ambiguities.
 *
 * Language Features:
 * - Boolean literals (0, 1) and Identifiers.
 * - Operators with precedence: NOT (!), AND (&), OR (|).
 * - Function calls and Assignments.
 * - Right-recursive list structures.
 */

#ifndef BALG_H
#define BALG_H

#include "../../bnf.h"

#include <stddef.h>

/* These are required by `bnf_dsl.h` to map the concise `TK()` and `NT()`
 * macros to the actual enum values defined below. */
#define PREFIX_TK(tk) TK_ ## tk
#define PREFIX_NT(nt) NT_ ## nt

#include "../../bnf_dsl.h"


#define BALG_TK_COUNT 14

#define BALG_NT_COUNT 17
#define BALG_NT_VARIANT_COUNT 6
#define BALG_NT_BODY_LENGTH 5


/** @brief terminal symbols (tokens) */
enum balg_tk {
	TK_NOTOKEN,
	/* Literals */
	TK_TRUE, TK_FALSE, TK_IDENT,
	/* Operators */
	TK_PIPE, TK_AMP, TK_EXCL,
	/* Punctuation */
	TK_LPAREN, TK_RPAREN, TK_LCURLY, TK_RCURLY,
	TK_EQ, TK_COMMA, TK_SEMI,
};

/** @brief non-terminal Symbols */
enum balg_nt {
	NT_BIT, NT_IDENT, NT_CALL,
	NT_CALL_OPTPARAMS,

	/* Expression Hierarchy (Precedence) */
	NT_EXPR, NT_EXPR_REST,	// level 1: OR
	NT_TERM, NT_TERM_REST,	// level 2: AND
	NT_FACTOR, NT_ATOM,	// level 3: NOT and Atoms

	/* statements */
	NT_STMT, NT_STMTS, NT_ASGN,

	/* list helpers */
	NT_IDENT_LS, NT_IDENT_LS_REST,
	NT_EXPR_LS, NT_EXPR_LS_REST,
};

/** @brief token character mapping (for simple lexing) */
const char balg_tks[BALG_TK_COUNT] = {
	'\0',
	'1', '0', 'i',
	'|', '&', '!',
	'(', ')', '{', '}',
	'=', ',', ';',
};

/** @brief non-terminal names (for debugging/printing CST) */
const char *balg_nts[BALG_NT_COUNT] = {
	"bit", "ident", "call",
	"call_optparams",

	"expr", "expr_rest",
	"term", "term_rest",
	"factor", "atom",

	"stmt", "stmts", "asgn",

	"ident_ls", "ident_ls_rest",
	"expr_ls", "expr_ls_rest",
};


const struct bnf_symbol
balg[BALG_NT_COUNT][BALG_NT_VARIANT_COUNT][BALG_NT_BODY_LENGTH] = {
	/* <bit> ::= */ r(
		TK(TRUE),
	alt	TK(FALSE),
	),
	/* <ident> ::= */ r(
		TK(IDENT),
	),
	/* <call> ::= */ r(
		TK(IDENT), TK(LPAREN), NT(CALL_OPTPARAMS), TK(RPAREN),
	),
	/* <call_optparams> ::= */
		ropt(NT(EXPR_LS)),

	/* <expr> ::= */
		rrr(EXPR, TERM, PIPE),

	/* <term> ::= */
		rrr(TERM, FACTOR, AMP),
	/* <factor> ::= */ r(
		TK(EXCL), NT(ATOM),
	alt	NT(ATOM),
	),

	/* Intentional ambiguity for stress testing. The grammar has two rules
	 * starting with '(':
	 *  "(" <expr> ")" Parenthesized expression
	 *  "(" asgn ")" Parenthesized assignment
	 *
	 * The parser must consume "(", try to parse <expr>, and if it fails
	 * (e.g., it encounters an "="), it must backtrack, restore the "(",
	 * and try parsing <asgn>. */
	/* <atom> ::= */ r(
		TK(LPAREN), NT(EXPR), TK(RPAREN),
	alt	TK(LPAREN), NT(ASGN), TK(RPAREN), // <-- Ambiguity trigger
	alt	NT(BIT),
	alt	NT(IDENT),
	alt	NT(CALL),
	),

	/* <stmt> ::= */ r(
		TK(SEMI),
	alt	NT(CALL), TK(SEMI),
	alt	NT(ASGN), TK(SEMI),
	alt	TK(LCURLY), NT(STMTS), TK(RCURLY),
	),

	/* <stmts> ::= */ ropt(NT(STMT), NT(STMTS)),

	/* <asgn> */ r(
		NT(IDENT_LS), TK(EQ), NT(EXPR_LS),
	),

	/* <ident_ls> ::= */
		rrr(IDENT_LS, IDENT, COMMA),

	/* <expr_ls> ::= */
		rrr(EXPR_LS, EXPR, COMMA),
};


#endif
