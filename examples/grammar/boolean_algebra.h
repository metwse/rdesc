/**
 * @file boolean_algebra.h
 * @brief Boolean algebra grammar example for librdesc.
 *
 * This file defines a sample CFG for a simple Boolean Algebra language. It
 * serves two main purposes:
 * 1. To demonstrate how to use the BNF macros (`bnf_macros.h`) to define
 *    grammars.
 * 2. To act as a stress-test for the parser's backtracking engine by
 *    introducing intentional ambiguities.
 *
 * Language features:
 * - Boolean literals (0, 1) and identifiers
 * - Operators with precedence: NOT (!), AND (&), OR (|)
 * - Function calls and assignments
 * - Right-recursive list structures
 * - Block statements
 */

#ifndef BALG_H
#define BALG_H

#include "../../include/cfg.h"

/** @brief add TK_ prefix in `TK` macro */
#define PREFIX_TK(tk) TK_ ## tk
/** @brief add NT_ prefix in `NT` macro */
#define PREFIX_NT(nt) NT_ ## nt

#include "../../include/bnf_macros.h"


/** @brief total count of terminal symbols */
#define BALG_TK_COUNT 14

/**
 * @brief Total count of non-terminal symbols defined in `enum balg_nt`.
 * Determines the size of the first dimension of the grammar table.
 */
#define BALG_NT_COUNT 17

/**
 * @brief Maximum number of production variants (alternatives) for a single
 * non-terminal and +1 for end-of-construct sentinel. Used to dimension the
 * static array (2nd dimension).
 */
#define BALG_NT_VARIANT_COUNT 6

/**
 * @brief Maximum number of symbols in a production body (Right-Hand Side) and
 * +1 for end-of-body sentinel. Used to dimension the static array (3rd
 * dimension).
 */
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

/** @brief token character mapping (for `exblex`) */
const char balg_tks[BALG_TK_COUNT] = {
	'\0',
	'1', '0', 'w',
	'|', '&', '!',
	'(', ')', '{', '}',
	'=', ',', ';',
};

/**
 * @brief names of tokens that used in BNF
 */
const char *const balg_tk_names[BALG_TK_COUNT] = {
	"\0",
	"1", "0", "@ident",
	"|", "&", "!",
	"(", ")", "{", "}",
	"=", ",", ";",
};

/**
 * @brief names of tokens that can be used in dotlang graph (special chars are
 * escaped)
 */
const char *const balg_tk_names_escaped[BALG_TK_COUNT] = {
	"\0",
	"1", "0", "@ident",
	"\\|", "&", "!",
	"(", ")", "\\{", "\\}",
	"=", ",", ";",
};

/** @brief non-terminal names (for debugging/printing CST) */
const char *const balg_nt_names[BALG_NT_COUNT] = {
	"bit", "ident", "call",
	"call_optparams",

	"expr", "expr_rest",
	"term", "term_rest",
	"factor", "atom",

	"stmt", "stmts", "asgn",

	"ident_ls", "ident_ls_rest",
	"expr_ls", "expr_ls_rest",
};

/** @brief example context-free grammar */
static const struct rdesc_cfg_symbol
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
		rrr(EXPR, NT(TERM), TK(PIPE)),

	/* <term> ::= */
		rrr(TERM, NT(FACTOR), TK(AMP)),
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
		rrr(IDENT_LS, NT(IDENT), TK(COMMA)),

	/* <expr_ls> ::= */
		rrr(EXPR_LS, NT(EXPR), TK(COMMA)),
};


#endif
