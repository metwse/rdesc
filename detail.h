/**
 * @file detail.h
 * @brief Private API of rdesc.
 */

#ifndef RDESC_DETAIL_H
#define RDESC_DETAIL_H

#include "bnf.h"

#include <stddef.h>
#include <stdio.h>
#include <signal.h>


#define STACK_INITIAL_CAP 8

#define assert_stringify_detail(a) #a
#define assert_stringify(a) assert_stringify_detail(a)

#define assert(c, fmt, ...) do { \
		if (!(c)) { \
			fprintf(stderr, "["__FILE__ ":" \
				assert_stringify(__LINE__) "] " \
				"Assertion failed for: " \
				   assert_stringify(c) \
				"\n> " fmt "\n" __VA_OPT__(,)__VA_ARGS__); \
			raise(2); \
		} \
	} while(0)

/** @brief macro highlights memory allocation checks */
#define assert_mem(c) assert(c, "out of memory")


struct rdesc_token_stack {
	struct bnf_token *tokens;
	size_t len;
	size_t cap;
};


void rdesc_token_stack_init(struct rdesc_token_stack *);

void rdesc_token_stack_push(struct rdesc_token_stack *, struct bnf_token);

struct bnf_token rdesc_token_stack_pop(struct rdesc_token_stack *);

struct bnf_token rdesc_token_stack_top(struct rdesc_token_stack *);

void rdesc_token_stack_destroy(struct rdesc_token_stack *);

#endif
