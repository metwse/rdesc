#include "../include/rdesc.h"
#include "../include/cfg.h"
#include "../include/stack.h"
#include "detail.h"

#include <stdlib.h>
#include <stddef.h>


#define STACK_INITIAL_CAP 8

#define RESIZE_STACK \
	assert_mem( \
		s->tokens = \
		realloc(s->tokens, s->cap * sizeof(struct rdesc_cfg_token)) \
	)


void rdesc_stack_init(struct rdesc_stack *s)
{
	s->len = 0;
	s->cap = STACK_INITIAL_CAP;
	assert_mem(
		s->tokens =
		malloc(STACK_INITIAL_CAP * sizeof(struct rdesc_cfg_token))
	);
}

void rdesc_stack_push(struct rdesc_stack *s, struct rdesc_cfg_token tk)
{
	if (s->cap == s->len) {
		s->cap *= 2;
		RESIZE_STACK;
	}

	s->tokens[s->len++] = tk;
}

void rdesc_stack_destroy(struct rdesc_stack *s)
{
	free(s->tokens);
}

struct rdesc_cfg_token rdesc_stack_pop(struct rdesc_stack *s)
{
	struct rdesc_cfg_token top = s->tokens[--s->len];

	if (s->len * 2 < s->cap && s->cap >= STACK_INITIAL_CAP * 2) {
		s->cap /= 2;
		RESIZE_STACK;
	}

	return top;
}

void *rdesc_stack_into_inner(struct rdesc_stack *s)
{
	return s->tokens;
}

size_t rdesc_stack_len(const struct rdesc_stack *s)
{
	return s->len;
}
