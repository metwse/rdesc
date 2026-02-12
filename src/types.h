/**
 * @file types.h
 * @brief
 */

#ifndef TYPES_H
#define TYPES_H

#include "../include/rdesc.h"

#include <stddef.h>
#include <stdint.h>


/** @brief size of a token node (including its seminfo field) */
#define sizeof_tk(p) (sizeof(tk_t) - sizeof(uint32_t) + (p).seminfo_size)
/** @brief size of a nonterminal node (including size of its child pointer list) */
#define sizeof_nt(child_cap) (sizeof(nt_t) + sizeof(size_t) * child_cap)

/** @brief minimum size of a node that can be used interchangeably as either a
 * token (with seminfo) or a nonterminal (without child list) */
#define sizeof_node(p) (sizeof_tk(p) > sizeof_nt(0) ? sizeof_tk(p) : sizeof_nt(0))

/** @brief token type */
typedef struct _rdesc_priv_tk tk_t;
/** @brief nonterminal type */
typedef struct _rdesc_priv_nt nt_t;
/** @brief node (token or nonterminal union) type */
#ifndef RDESC_ILLEGAL_ACCESS
typedef struct _rdesc_priv_node node_t;
#else
typedef struct rdesc_node rdesc_node_t;
#endif


#endif
