#include "rdesc.h"
#include "bnf.h"
#include "detail.h"

#include <stdlib.h>


#define RESIZE_STACK \
	assert_mem( \
		s->tokens = \
		realloc(s->tokens, s->cap * sizeof(struct bnf_token)) \
	)


void rdesc_token_stack_init(struct rdesc_token_stack *s)
{
	s->len = 0;
	s->cap = STACK_INITIAL_CAP;
	assert_mem(
		s->tokens =
		malloc(STACK_INITIAL_CAP * sizeof(struct bnf_token))
	);
}

void rdesc_token_stack_push(struct rdesc_token_stack *s, struct bnf_token tk)
{
	if (s->cap == s->len) {
		s->cap *= 2;
		RESIZE_STACK;
	}

	s->tokens[s->len++] = tk;
}

struct bnf_token rdesc_token_stack_pop(struct rdesc_token_stack *s)
{
	struct bnf_token top = s->tokens[--s->len];

	if (s->len * 2 < s->cap && s->cap >= STACK_INITIAL_CAP * 2) {
		s->cap /= 2;
		RESIZE_STACK;
	}

	return top;
}

struct bnf_token rdesc_token_stack_top(struct rdesc_token_stack *s)
{
	return s->tokens[s->len - 1];
}


void rdesc_token_stack_destroy(struct rdesc_token_stack *s)
{
	free(s->tokens);
}
