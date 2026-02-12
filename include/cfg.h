/**
 * @file cfg.h
 * @brief Context-free grammar (CFG) data structures
 *
 * This header defines the core data structures used to represent a CFG in
 * `librdesc`.
 */

#ifndef RDESC_CFG_H
#define RDESC_CFG_H

#include <stdint.h>

/**
 * @brief Context-free grammar definition.
 *
 * The production rules are dimensioned as a 3D array:
 * - [nt_count][nt_variant_count][nt_body_length]
 */
struct rdesc_cfg {
  /** @brief Context-free grammar production rules. */
  const struct rdesc_cfg_symbol *rules;
  /** @brief Total number of nonterminals. */
  uint32_t nt_count;
  /** @brief Maximum number of variants, used for segmenting production rules
   * array to a 3D array. */
  uint16_t nt_variant_count;
  /** @brief Maximum number of symbols in a variant. */
  uint16_t nt_body_length;
  /** @brief Maximum number of children of nonterminal variants. */
  uint16_t *child_caps;
};

/** @brief The type of `rdesc_cfg_symbol` (the union's tag). */
enum rdesc_cfg_symbol_type {
  CFG_TOKEN,
  CFG_NONTERMINAL,
  /** @brief sentinel for terminating production body or variants of
   * a nonterminal. */
  CFG_SENTINEL,
};

/**
 * @brief A terminal/nonterminal to describe body (right side) of a production
 * rule.
 */
struct rdesc_cfg_symbol {
  enum rdesc_cfg_symbol_type ty /** Type of the symbol. */;
  int id /** Terminal, nonterminal, or sentinel identifier. */;
};

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Initializes a context-free grammar object. */
void rdesc_cfg_init(struct rdesc_cfg *cfg, uint32_t nonterminal_count,
                    uint16_t nonterminal_variant_count,
                    uint16_t nonterminal_body_length,
                    const struct rdesc_cfg_symbol *production_rules);

/** @brief Frees the context-free grammar struct. */
void rdesc_cfg_destroy(struct rdesc_cfg *cfg);

#ifdef __cplusplus
}
#endif

#endif
