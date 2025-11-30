/**
 * @file cfg.h
 * @brief Context-Free Grammar (CFG) data structures
 *
 * This header defines the core data structures used to represent a CFG in
 * `librdesc`.
 */

#ifndef RDESC_CFG_H
#define RDESC_CFG_H

#include <stddef.h>


/**
 * @brief Context-free grammar definition.
 */
struct rdesc_cfg {
	/** context-free grammar production rules */
	const struct rdesc_cfg_symbol *rules;
	/** total number of non-terminals */
	size_t nt_count;
	/** maximum number of variants, used for segmenting production rules
	 * array to an 3D array */
	size_t nt_variant_count;
	/** maximum number of symbols in a variant */
	size_t nt_body_length;
	/** maximum number of children of non-terminal variants */
	size_t *child_caps;
};

/** @brief The type of `rdesc_cfg_symbol` (the union's tag) */
enum rdesc_cfg_symbol_type {
	CFG_TOKEN,
	CFG_NONTERMINAL,
	/** @brief sentinel for terminating production body or variants of
	 * a non-terminal */
	CFG_SENTINEL,
};

/**
 * @brief A terminal/non-terminal to describe body (right side) of a production
 * rule.
 */
struct rdesc_cfg_symbol {
	enum rdesc_cfg_symbol_type ty /** type of the symbol */;
	int id /** terminal, non-terminal, or sentinel identifier */;
};

/** @brief terminal (token) object for context-free grammar */
struct rdesc_cfg_token {
	int id /** token identifier */;
	void *seminfo /** additional semantic information */;
};

/** @brief nonterminal (syntatic variable) object for context-free grammar */
struct rdesc_cfg_nonterminal {
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


/** @brief Function pointer type for freeing tokens. */
typedef void (*rdesc_tk_destroyer_func)(struct rdesc_cfg_token *);


/** @brief Initializes a context-free grammar object. */
void rdesc_cfg_init(struct rdesc_cfg *cfg,
		    size_t nonterminal_count,
		    size_t nonterminal_variant_count,
		    size_t nonterminal_body_length,
		    const struct rdesc_cfg_symbol *production_rules);

/** @brief Frees the context-free grammar struct. */
void rdesc_cfg_destroy(struct rdesc_cfg *cfg);


#endif
