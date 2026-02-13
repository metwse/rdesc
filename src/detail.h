/**
 * @file detail.h
 * @brief Internal rdesc utilities.
 *
 * Functions defined in this file used for exposing internal functions to test
 * suite.
 */

#ifndef RDESC_DETAIL_H
#define RDESC_DETAIL_H


/** @brief Assertion macro that prints formatted error message if failed. */
#if (defined(__unix__) || defined(__APPLE__) || defined(__linux__)) && \
    (defined(__GNUC__) || defined(__clang__))
#include <stdio.h>  // IWYU pragma: begin_exports
#include <signal.h>  // IWYU pragma: end_exports

/** @cond */
#define assert_stringify_detail(a) #a
#define assert_stringify(a) assert_stringify_detail(a)
/** @endcond */

#define rdesc_assert(c, ...) do { \
		if (!(c)) { \
			fprintf(stderr, "["  __FILE__ ":" \
				assert_stringify(__LINE__) "] " \
				"Assertion failed for: " \
				assert_stringify(c) \
				"\n> " __VA_ARGS__); \
			fputc('\n', stderr); \
			raise(SIGINT); \
		} \
	} while(0)

#else
#include <assert.h>  // IWYU pragma: export

#define rdesc_assert(c, ...) assert(c)
#endif

/** @brief Unreachable branch. */
#if defined(__GNUC__) || defined(__clang__)
#define unreachable() __builtin_unreachable()
#elif defined(_MSC_VER)
#define unreachable() __assume(false)
#else
#define unreachable()
#endif

/** @brief Macro highlights type casts. */
#define cast(t, exp) ((t) (exp))

/** @brief Internal macro for casting symbol table pointer to 3D array type. */
#define productions(cfg) \
	(*cast(const struct rdesc_cfg_symbol (*) \
		[(cfg).nt_count][(cfg).nt_variant_count][(cfg).nt_body_length], \
	      (cfg).rules))


/** @brief Size of a token node for parser (including its seminfo field). */
#define sizeof_tk(p) \
	(sizeof(struct _rdesc_priv_tk) - sizeof(uint32_t) + (p).seminfo_size)

/**
 * @brief Size of a nonterminal node (including size of its child pointer
 * list).
 */
#define sizeof_nt(child_cap) \
	(sizeof(struct _rdesc_priv_nt) + sizeof(size_t) * child_cap)

/**
 * @brief Minimum size of a node that can be used interchangeably as either a
 * token (with seminfo) or a nonterminal (without child list).
 */
#define sizeof_node(p) \
	((sizeof(size_t) + sizeof(uint32_t)) + (sizeof_tk(p) > sizeof_nt(0) ? \
		sizeof_tk(p) : \
		sizeof_nt(0)))

/** @cond */
#ifndef RDESC_ILLEGAL_ACCESS
typedef struct _rdesc_priv_node node_t;
#else
typedef struct rdesc_node rdesc_node_t;
#endif
/** @endcond */


#endif
