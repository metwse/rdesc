/**
 * @file util.h
 * @brief Development and Debugging Utilities.
 *
 * This header provides tools for visualizing data structures in `librdesc`.
 */

#ifndef RDESC_UTIL_H
#define RDESC_UTIL_H

#include <stdio.h>

struct rdesc_node; /* defined in rdesc.h */

struct rdesc_cfg; /* defined in cfg.h */
struct rdesc_cfg_token;
struct rdesc_cfg_nonterminal;


/** @brief Function pointer type for printing tokens. */
typedef void (*rdesc_tk_printer_func)(const struct rdesc_cfg_token *, FILE *out);


/**
 * @brief Dumps the Concrete Syntax Tree (CST) as a Graphviz DOT graph.
 *
 * Traverses the CST and generates a `.dot` representation.
 *
 * @param cst CST node
 * @param nt_names The raw name of the non-terminal (e.g., `expr`). The dumper
 *        handles the surrounding `<` and `>` characters automatically.
 * @param tk_printer Callback to print token names.
 *        - DOT Context: This function handle seminfo fields and constructs a
 *          table accordingly.
 * @param out Output file stream
 */
void rdesc_dump_dot(const struct rdesc_node *cst,
		    rdesc_tk_printer_func tk_printer,
		    const char *const nt_names[],
		    FILE *out);

/**
 * @brief Dumps the Context-Free Grammar in BNF format.
 *
 * Iterates over the production rules defined in the configuration and prints
 * them in a human-readable BNF format. (e.g.`A ::= B | C`)
 *
 * @param cfg Underlying CFG
 * @param tk_names The token name or literal representation (e.g., `IDENT` or
 *        `+`). If you do not want to put double quotes around token name,
 *        put @ to beginning of the name.
 * @param nt_names The raw name of the non-terminal (e.g., `expr`). The dumper
 *        handles the surrounding `<` and `>` characters automatically.
 * @param out Output file stream
 */
void rdesc_dump_bnf(const struct rdesc_cfg *cfg,
		    const char *const tk_names[],
		    const char *const nt_names[],
		    FILE *out);


#endif
