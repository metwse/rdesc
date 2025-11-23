/**
 * @file bnf_dsl.h
 * @brief Macros for context-free grammar definitions
 *
 * Provides macros (r, rrr, ropt) to define these production rules in a
 * readable way.
 *
 * You may define `PREFIX_TK` and `PREFIX_NT` macros before including this file
 * to automatically add prefixes in grammar definitions.
 */

#ifndef BNF_DSL_H
#define BNF_DSL_H

#include <stddef.h>

#ifndef PREFIX_TK
#define PREFIX_TK(tk) tk
#endif

#ifndef PREFIX_NT
#define PREFIX_NT(nt) nt
#endif

/** integer representing EOB (end of body) */
#define EOB -1
/** integer representing EOC (end of construct) */
#define EOC -2

/** @brief Sentinel struct for the end of a rule's body (EOB) */
#define SEOB (const struct bnf_symbol) { .ty = BNF_SENTINEL, .id = EOB }
/** @brief Sentinel struct for the end of a construct's variants (EOC) */
#define SEOC { (const struct bnf_symbol) { .ty = BNF_SENTINEL, .id = EOC } }

/** @brief Macro to create a terminal (Token) production symbol. */
#define TK(tk) { .ty = BNF_TOKEN, .id = PREFIX_TK(tk) }
/** @brief Macro to create a non-terminal production symbol. */
#define NT(nt) { .ty = BNF_NONTERMINAL, .id = PREFIX_NT(nt) }


/** @brief Macro to define a grammar rule. */
#define r(...) { { __VA_ARGS__ SEOB }, SEOC }

/**
 * @brief Macro to define a pair of rules for a right-recursive list.
 *
 * This is the standard pattern for lists and operator precedence. It
 * automatically defines the rule for 'head' and 'head_REST'.
 *
 * For example, `rrr(EXPR_LS, EXPR, COMMA)` defines:
 * 1. `<expr_ls> ::= <expr> <expr_ls_rest>`
 * 2. `<expr_ls_rest> ::= "," <expr_ls> <expr_ls_rest> | E`
 *
 * @param head The base non-terminal.
 * @param listelem The non-terminal for the list element.
 * @param delim The token used as a separator.
 */
#define rrr(head, listelem, delim) \
	{ { NT(listelem), NT(head ## _REST), SEOB }, SEOC }, \
	{ { TK(delim), NT(head), NT(head ## _REST), SEOB }, { SEOB }, SEOC }

/**
 * @brief Macro to define an optional grammar rule (epsilon production).
 *
 * This is a shortcut for a rule with two variants:
 * 1. The symbols provided in `__VA_ARGS__`
 * 2. An empty (epsilon) production
 */
#define ropt(...) { { __VA_ARGS__, SEOB }, { SEOB }, SEOC }

/**
 * @brief Macro for syntactic sugar to separate non-terminal alternatives.
 *
 * Expands to the end-of-body sentinel and new variant syntax.
 */
#define alt SEOB, }, {


#endif
