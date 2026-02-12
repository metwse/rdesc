/**
 * @file bnf_macros.h
 * @brief Macros to facilitate defining context-free grammar in Backus-Naur
 *        form (BNF).
 *
 * Provides macros (r, rrr, ropt) to define these production rules in a
 * readable way.
 *
 * @section prefix_sec Name Mapping for Identifiers
 *
 * `PREFIX_TK` and `PREFIX_NT` macros determine how a raw token name passed to
 * `TK(...)`/`NT(...)` is expanded into a C identifier (typically an enum
 * member).
 *
 * - Default: Identity mapping. `TK(ID)`/`NT(ID)` expands to `ID`.
 * - Override: Define this macro before including the `bnf_macros.h` to add
 *   namespaces (e.g., `#define PREFIX_TK(x) MY_TK_ ## x`).
 */

#ifndef BNF_MACROS_H
#define BNF_MACROS_H


#ifndef PREFIX_TK
/** @brief See @ref prefix_sec  */
#define PREFIX_TK(tk) tk
#endif

#ifndef PREFIX_NT
/** @brief See @ref prefix_sec  */
#define PREFIX_NT(nt) nt
#endif

#ifndef POSTFIX_NT_REST
/** @brief Name mapping for nonterminal list to rest types. */
#define POSTFIX_NT_REST(nt) nt ## _REST
#endif

/** @brief Integer representing EOB (end-of-body). */
#define EOB -1
/** @brief Integer representing EOC (end-of-construct). */
#define EOC -2

/** @cond */
/** sentinel struct for the end of a rule's body */
#define SEOB { .ty = CFG_SENTINEL, .id = EOB }
/** sentinel struct for the end of a construct's variants */
#define SEOC { { .ty = CFG_SENTINEL, .id = EOC } }
/** @endcond */

/** @brief Macro to create a terminal (token) production symbol. */
#define TK(tk) { .ty = CFG_TOKEN, .id = PREFIX_TK(tk) }
/** @brief Macro to create a nonterminal production symbol. */
#define NT(nt) { .ty = CFG_NONTERMINAL, .id = PREFIX_NT(nt) }
/** @brief Macro to create an epsilon production symbol. Use this to represent
 * an empty production (ε). */
#define EPSILON SEOB


/**
 * @brief Macro to define a grammar rule. Adds end-of-body and construct
 * sentinels to grammar rules.
 */
#define r(...) { { __VA_ARGS__ SEOB }, SEOC }

/**
 * @brief Macro to define a pair of rules for a right-recursive list.
 *
 * This is the standard pattern for lists and operator precedence. It
 * automatically defines the rule for 'head' and 'head_REST'.
 *
 * For example, `rrr(EXPR_LS, NT(EXPR), TK(COMMA))` defines:
 * 1. `<expr_ls> ::= <expr> <expr_ls_rest>`
 * 2. `<expr_ls_rest> ::= "," <expr_ls> | E`
 *
 * @param head The base nonterminal.
 * @param listelem The nonterminal for the list element.
 * @param delim The token used as a separator.
 */
#define rrr(head, listelem, delim) \
	{ { listelem, NT(POSTFIX_NT_REST(head)), SEOB }, SEOC }, \
	{ { delim, NT(head), SEOB }, { SEOB }, SEOC }

/**
 * @brief Macro to define an optional grammar rule (epsilon production).
 *
 * This is a shortcut for a rule with two variants:
 * 1. The symbols provided in `__VA_ARGS__`
 * 2. An empty (epsilon) production
 */
#define ropt(...) { { __VA_ARGS__, SEOB }, { SEOB }, SEOC }

/**
 * @brief Macro for syntactic sugar to separate nonterminal alternatives.
 *
 * Expands to the end-of-body sentinel and new variant syntax.
 */
#define alt SEOB, }, {


#endif
