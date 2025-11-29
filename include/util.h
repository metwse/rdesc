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
 * @param nt_names The raw name of the non-terminal (e.g., `expr`). The dumper
 *        handles the surrounding `<` and `>` characters automatically.
 * @param tk_printer Callback to print Token names.
 *        - DOT Context: This function handle seminfo fields and constructs a
 *          table accordingly.
 */
void rdesc_dump_dot(const struct rdesc_node *,
		    rdesc_tk_printer_func tk_printer,
		    const char *const nt_names[],
		    FILE *);

/**
 * @brief Dumps the Static Grammar Configuration in BNF format.
 *
 * Iterates over the production rules defined in the configuration and
 * prints them in a human-readable BNF format. (e.g.`A ::= B | C`)
 *
 * @param tk_names The token name or literal representation (e.g., `IDENT` or
 *        `+`). If you do not want to put double quotes around token name,
 *        put @ to beginning of the name.
 * @param nt_names The raw name of the non-terminal (e.g., `expr`). The dumper
 *        handles the surrounding `<` and `>` characters automatically.
 */
void rdesc_dump_bnf(const struct rdesc_cfg *,
		    const char *const tk_names[],
		    const char *const nt_names[],
		    FILE *);


#endif
