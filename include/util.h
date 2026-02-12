/**
 * @file util.h
 * @brief Development and debugging utilities.
 *
 * This header provides tools for visualizing data structures in `librdesc`.
 */

#ifndef RDESC_UTIL_H
#define RDESC_UTIL_H

#include <stdio.h>

struct rdesc; /* defined in rdesc.h */
struct rdesc_node;

struct rdesc_cfg;  /* defined in cfg.h */


/** @brief Function pointer type for printing nodes. */
typedef void (*rdesc_node_printer_func)(const struct rdesc_node *, FILE *out);


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Dumps the concrete syntax tree (CST) as a graphviz DOT graph.
 *
 * @param out Output file stream
 * @param p Parser to dump CST
 * @param node_printer Callback to print token and nonterminal names.
 */
void rdesc_dump_cst(FILE *out,
		    const struct rdesc *p,
                    rdesc_node_printer_func node_printer);

/**
 * @brief Dumps the Context-free grammar in BNF format.
 *
 * Iterates over the production rules defined in the configuration and prints
 * them in a human-readable BNF format. (e.g.`A ::= B | C`)
 *
 * @param out Output file stream
 * @param cfg Underlying CFG
 * @param tk_names The token name or literal representation (e.g., `IDENT` or
 *        `+`). If you do not want to put double quotes around token name,
 *        put @ at the beginning of the name.
 * @param nt_names The raw name of the nonterminal (e.g., `expr`). The dumper
 *        handles the surrounding `<` and `>` characters automatically.
 */
void rdesc_dump_bnf(FILE *out,
		    const struct rdesc_cfg *cfg,
		    const char *const tk_names[],
                    const char *const nt_names[]);

#ifdef __cplusplus
}
#endif

#endif
