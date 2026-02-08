// internal macros of rdesc

#ifndef RDESC_DETAIL_H
#define RDESC_DETAIL_H


#if (defined(__unix__) || defined(__APPLE__) || defined(__linux__)) && \
    (defined(__GNUC__) || defined(__clang__))
#include <stdio.h>  // IWYU pragma: begin_exports
#include <signal.h>  // IWYU pragma: end_exports

#define assert_stringify_detail(a) #a
#define assert_stringify(a) assert_stringify_detail(a)

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

/* extra checks for flow of code. */
#define assert_logic(c, msg) \
	rdesc_assert(c, "logic error: " msg " is/are not meaningful")

/* macro highlights memory allocation checks */
#define assert_mem(c) rdesc_assert(c, "out of memory")

/* unreachable branch */
#if defined(__GNUC__) || defined(__clang__)
#define unreachable() __builtin_unreachable()
#elif defined(_MSC_VER)
#define unreachable() __assume(false)
#else
#define unreachable()
#endif

/* macro highlights type casts */
#define cast(t, exp) ((t) (exp))

#define productions(cfg) \
	(*cast(const struct rdesc_cfg_symbol (*) \
		[(cfg).nt_count][(cfg).nt_variant_count][(cfg).nt_body_length], \
	    (cfg).rules))

/* helper macros for accessing node fields */
#define tk_ n.tk
#define nt_ n.nt

#define tk_size(p) \
	RDESC_MIN_SEMINFO_SIZE < (p).seminfo_size ? \
		sizeof(struct rdesc_token) + (p).seminfo_size - RDESC_MIN_SEMINFO_SIZE : \
		sizeof(struct rdesc_token)


#endif
