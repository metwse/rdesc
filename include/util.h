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

struct rdesc_grammar;  /* defined in grammar.h */


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Dumps the concrete syntax tree (CST) as a graphviz DOT graph.
 *
 * @param out Output file stream
 * @param parser Parser to dump
 * @param node_printer Callback to print token and nonterminal names.
 */
void rdesc_dump_cst(FILE *out,
		    const struct rdesc *parser,
		    void (*node_printer)(const struct rdesc_node *, FILE *out));

/**
 * @brief Dumps the grammar in BNF format.
 *
 * Prints all production rules in human-readable BNF format. (e.g.`A ::= B | C`)
 *
 * @param out Output file stream
 * @param grammar Underlying grammar struct
 * @param tk_names The token name or literal representation (e.g., `IDENT` or
 *        `+`). Prefix with '@' to suppress automatic quote wrapping.
 * @param nt_names The raw name of the nonterminal (e.g., `expr`). Angle
 *        brackets `<>` are added automatically.
 */
void rdesc_dump_bnf(FILE *out,
		    const struct rdesc_grammar *grammar,
		    const char *const tk_names[],
                    const char *const nt_names[]);

#ifdef __cplusplus
}
#endif


#endif
