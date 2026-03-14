/**
 * @file cst_macros.h
 * @brief Macros for accessing fields of node structs.
 *
 * The parser returns opaque CST node pointers that hide memory layout details.
 * This header provides macros to safely access node fields despite the
 * optimized, compact memory representation.
 *
 * This header undefines its own macros if it is included a second time. You
 * can re-include this header at the end of your file to keep the macro
 * namespace clean and prevent polluting other headers.
 */

#ifndef RDESC_CST_MACROS_H
#define RDESC_CST_MACROS_H

#include "detail.h"  // IWYU pragma: export (for _rdesc_priv_node_deref)

#include <stddef.h>

struct rdesc;  /* defined in rdesc.h */


/** @cond */
#define _rdesc_priv_node_deref(node) (*(struct _rdesc_priv_node *) (node))

/* Returns index of parent of the node, or `SIZE_MAX` if the node is root. */
#define _rdesc_priv_parent_idx(node) _rdesc_priv_node_deref(node).parent

/* Returns index of the child in stack. */
#define _rdesc_priv_child_idx(nt_node, child_index) \
	(*(size_t *) (&((uint8_t *) ((struct _rdesc_priv_node *) nt_node + 1)) \
		[(child_index) * sizeof(size_t)]))

#ifdef __cplusplus
extern "C"
#endif
struct rdesc_node *_rdesc_priv_cst_illegal_access(const struct rdesc *parser,
						  size_t index);
/** @endcond */


#endif


#ifndef RDESC_CST_MACROS
/** @cond */
#define RDESC_CST_MACROS
/** @endcond */

/** @brief Returns parent of the node, or `NULL` if the node is root. */
#define rparent(p, node) \
	_rdesc_priv_cst_illegal_access(p, _rdesc_priv_parent_idx(node))

/** @brief Returns node type (RDESC_TOKEN or RDESC_NONTERMINAL).
 * @see enum rdesc_grammar_symbol_type */
#define rtype(node) _rdesc_priv_node_deref(node).n.ty

/** @brief Returns the 15-bit identifier for underlying token/nonterminal. */
#define rid(node) _rdesc_priv_node_deref(node).n.nt.id

/** @brief Returns a reference to token's seminfo field */
#define rseminfo(tk_node) \
	((void *) &_rdesc_priv_node_deref(tk_node).n.tk.seminfo)

/** @brief Returns id of nonterminal variant that is matched. */
#define rvariant(nt_node) \
	_rdesc_priv_node_deref(nt_node).n.nt.variant

/** @brief Returns number of child nodes. */
#define rchild_count(nt_node) \
	_rdesc_priv_node_deref(nt_node).n.nt.child_count

/** @brief Returns child of the node by its index. */
#define rchild(p, nt_node, child_idx) \
	_rdesc_priv_cst_illegal_access(p, _rdesc_priv_child_idx(nt_node, child_idx))

#else
#undef RDESC_CST_MACROS

#undef rparent

#undef rtype

#undef rid

#undef rseminfo

#undef rvariant

#undef rchild_count

#undef rchild

#endif
