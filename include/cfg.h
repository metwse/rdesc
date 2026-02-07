/**
 * @file cfg.h
 * @brief Context-Free Grammar (CFG) data structures
 *
 * This header defines the core data structures used to represent a CFG in
 * `librdesc`.
 */

#ifndef RDESC_CFG_H
#define RDESC_CFG_H

#include <stdint.h>


/**
 * @brief Context-free grammar definition.
 */
struct rdesc_cfg {
	/** context-free grammar production rules */
	const struct rdesc_cfg_symbol *rules;
	/** total number of non-terminals */
	uint32_t nt_count;
	/** maximum number of variants, used for segmenting production rules
	 * array to an 3D array */
	uint16_t nt_variant_count;
	/** maximum number of symbols in a variant */
	uint16_t nt_body_length;
	/** maximum number of children of non-terminal variants */
	uint16_t *child_caps;
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


#ifdef __cplusplus
extern "C" {
#endif

/** @brief Initializes a context-free grammar object. */
void rdesc_cfg_init(struct rdesc_cfg *cfg,
		    uint32_t nonterminal_count,
		    uint16_t nonterminal_variant_count,
		    uint16_t nonterminal_body_length,
		    const struct rdesc_cfg_symbol *production_rules);

/** @brief Frees the context-free grammar struct. */
void rdesc_cfg_destroy(struct rdesc_cfg *cfg);

#ifdef __cplusplus
}
#endif


#endif
