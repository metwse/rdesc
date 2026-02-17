/**
 * @file grammar.h
 * @brief Grammar data structures.
 *
 * This header defines the core data structures used to represent a grammar in
 * `librdesc`.
 */

#ifndef RDESC_GRAMMAR_H
#define RDESC_GRAMMAR_H

#include <stdint.h>


/**
 * @brief Grammar definition.
 *
 * The production rules are dimensioned as a 3D array:
 * - [nt_count][nt_variant_count][nt_body_length]
 */
struct rdesc_grammar {
	/** @brief Grammar production rules. */
	const struct rdesc_grammar_symbol *rules;

	/** @brief Total number of nonterminals. */
	uint32_t nt_count;

	/**
	 * @brief Maximum number of variants, used to segment the production
	 * rules array into a 3D array.
	 */
	uint16_t nt_variant_count;

	/** @brief Maximum number of symbols in a variant. */
	uint16_t nt_body_length;

	/** @brief Maximum number of children of nonterminal variants. */
	uint16_t *child_caps;
};

/** @brief The type of `rdesc_grammar_symbol` (the union's tag). */
enum rdesc_grammar_symbol_type {
	RDESC_TOKEN,
	RDESC_NONTERMINAL,
	/**
	 * @brief Sentinel for terminating a production body or the variants
	 * of a nonterminal.
	 */
	RDESC_SENTINEL,
};

/**
 * @brief A terminal or nonterminal representing the body (right side) of a
 * production rule.
 */
struct rdesc_grammar_symbol {
	enum rdesc_grammar_symbol_type ty  /** @brief Type of the symbol. */;

	int id  /** @brief Terminal, nonterminal, or sentinel identifier. */;
};


#ifdef __cplusplus
extern "C" {
#endif

/** @brief Initializes a grammar struct. */
void rdesc_grammar_init(struct rdesc_grammar *grammar,
			uint32_t nonterminal_count,
			uint16_t nonterminal_variant_count,
			uint16_t nonterminal_body_length,
			const struct rdesc_grammar_symbol *production_rules);

/** @brief Frees the resource allocated by grammar struct. */
void rdesc_grammar_destroy(struct rdesc_grammar *grammar);

#ifdef __cplusplus
}
#endif


#endif
