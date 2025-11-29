// internal macros of rdesc

#ifndef RDESC_DETAIL_H
#define RDESC_DETAIL_H

#include <stdio.h> // IWYU pragma: begin_exports
#include <signal.h> // IWYU pragma: end_exports


#define assert_stringify_detail(a) #a
#define assert_stringify(a) assert_stringify_detail(a)

#define assert(c, fmt, ...) do { \
		if (!(c)) { \
			fprintf(stderr, "["__FILE__ ":" \
				assert_stringify(__LINE__) "] " \
				"Assertion failed for: " \
				   assert_stringify(c) \
				"\n> " fmt "\n" __VA_OPT__(,)__VA_ARGS__); \
			raise(SIGINT); \
		} \
	} while(0)

/* macro highlights memory allocation checks */
#define assert_mem(c) assert(c, "out of memory")

/* extra checks for flow of code. */
#define assert_logic(c, fmt, ...) \
	assert(c, "logic error: " fmt " is/are not meaningful" \
	       __VA_OPT__(,)__VA_ARGS__)

/* code reached unreachable branch */
#define unreachable() assert(0, "reached unreachable branch"); exit(2)

/* macro highlights type casts */
#define cast(t, exp) ((t) (exp))

#define productions(cfg) \
	(*cast(const struct rdesc_cfg_symbol (*) \
		[(cfg).nt_count][(cfg).nt_variant_count][(cfg).nt_body_length], \
	    (cfg).rules))


#endif
