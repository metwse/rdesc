/**
 * @file bnf.h
 * @brief Basic types for context-free grammar.
 */

#ifndef BNF_H
#define BNF_H

#include <stddef.h>


/** @brief terminal (token) object for context-free grammar */
struct bnf_token {
	int id /** token identifier */;
	void *seminfo /** additional semantic information */;
};

/** @brief nonterminal (syntatic variable) object for context-free grammar */
struct bnf_nonterminal {
	int id /** nonterminal identifier */;

	struct rdesc_node **children /** child nodes */;
	size_t child_count /** number of child ndes */;

	/** @brief The production rule variant being parsed
	 *
	 * This field is for the internal use of the `rdesc_parser`. When the
	 * backtracking parser tries different production rules for a
	 * non-terminal, it increments this index to track which variant it
	 * is currently attempting. This is purely parse-time data. */
	size_t variant;
};

/** @brief The type of `bnf_symbol` (the union's tag) */
enum bnf_symbol_type {
	BNF_TOKEN,
	BNF_NONTERMINAL,
	/** @brief sentinel for terminating production body or variants of
	 * a non-terminal */
	BNF_SENTINEL,
};

/**
 * @brief A terminal/non-terminal to describe body (right side) of a production
 * rule.
 */
struct bnf_symbol {
	enum bnf_symbol_type ty /** type of the symbol */;
	int id /** terminal, non-terminal, or sentinel identifier */;
};


#endif
