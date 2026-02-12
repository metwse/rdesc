/**
 * @file cst_macros.h
 * @brief Macros for accessing fields of node structs.
 *
 * `rdesc` returns opaque pointers abstracting away underlying complexity of
 * memory layout. This header provides stable macros for dereferencing such
 * pointers to terminal/nonterminal fields that are tentative and unstructured
 * due to memory layout optimizations.
 */

#ifndef CST_MACROS_H
#define CST_MACROS_H


/** @cond */
/* rdesc node dereference */
#ifndef RDESC_ILLEGAL_ACCESS
#define rdesc_node_d(node) (*(struct _rdesc_priv_node *) (node));
#else
#define rdesc_node_d(node) (*(struct rdesc_node *) (node));
#endif
/** @endcond */


/**
 * @brief Returns index of parent of the given node.
 *
 * @note Returns `SIZE_MAX` if `node` is the root.
 */
#define rparent(node) rdesc_node_d(node).parent

/**
 * @brief Returns type of the node.
 *
 * @see enum rdesc_cfg_symbol_type
 */
#define rtype(node) rdesc_node_d(node).ty

/** @brief Returns the 31-bit identifier for underlying token/nonterminal. */
#define rid(node) rdesc_node_d(node).n.nt.id

/** @brief Returns a reference to token's seminfo field */
#define rseminfo(token_node) \
	((void *) &rdesc_node_d(token_node).n.tk.seminfo)

/** @brief Returns id of nonterminal variant that is matched. */
#define rvariant(nonterminal_node) \
	rdesc_node_d(nonterminal_node).n.nt.variant

/** @brief Returns number of child nodes. */
#define rchild_count(nonterminal_node) \
	rdesc_node_d(nonterminal_node).n.nt.child_count

/** @brief Returns the index of child in stack. */
#define rchildren(nonterminal_node, child_index) \
	(*(size_t *) (&rdesc_node_d(nonterminal_node)._[child_index * sizeof(size_t)]))


#undef rdesc_node_d


#endif
