#include "../include/rdesc.h"
#include "../include/stack.h"
#include "detail.h"

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>


#define STACK_INITIAL_CAP 8

#define RESIZE_STACK \
	assert_mem(*s = realloc(*s, sizeof(struct rdesc_stack) + \
			(*s)->cap * tk_size))


/**
 * @brief Default implementation of the token backtracking stack.
 *
 * @note Portability / Custom Implementation:
 *       This definition is guarded by `stack` feature flag. If you are porting
 *       `librdesc` to an environment that already has a preferred dynamic
 *       array or stack implementation (e.g., a project-specific
 *       vector type), you can remove the `stack` feature flag. This
 *       allows you to suppress this default struct and provide your own
 *       definition of `struct rdesc_stack` compatible with your system.
 */
struct rdesc_stack {
	size_t len /** current number of tokens in the stack */;
	size_t cap /** allocated capacity of the buffer */;
	char tokens[] /** pointer to the dynamic array buffer */;
};

void rdesc_stack_init(struct rdesc_stack **s, size_t tk_size)
{
	*s = malloc(sizeof(struct rdesc_stack) +
		    STACK_INITIAL_CAP * tk_size);
	assert_mem(s);

	(*s)->len = 0;
	(*s)->cap = STACK_INITIAL_CAP;
}

void rdesc_stack_push(struct rdesc_stack **s,
		      struct rdesc_token *tk,
		      size_t tk_size)
{
	if ((*s)->cap == (*s)->len) {
		assert_mem((*s)->cap < SIZE_MAX / 2);
		(*s)->cap *= 2;
		RESIZE_STACK;
	}

	memcpy(&(*s)->tokens[(*s)->len * tk_size], tk, tk_size);
	(*s)->len++;
}

void rdesc_stack_destroy(struct rdesc_stack *s)
{
	free(s);
}

void rdesc_stack_reset(struct rdesc_stack **s,
		       rdesc_tk_destroyer_func tk_destroyer,
		       size_t tk_size)
{
	for (size_t i = 0; i < (*s)->len; i++) {
		void *top = &(*s)->tokens[i * tk_size];

		if (tk_destroyer)
			tk_destroyer(top);
	}

	if ((*s)->cap > STACK_INITIAL_CAP) {
		(*s)->cap = STACK_INITIAL_CAP;
		RESIZE_STACK;
	}

	(*s)->len = 0;
	(*s)->cap = STACK_INITIAL_CAP;
}

struct rdesc_token *rdesc_stack_pop(struct rdesc_stack **s, size_t tk_size)
{
	if ((*s)->len * 4 <= (*s)->cap && (*s)->cap >= STACK_INITIAL_CAP * 2) {
		(*s)->cap /= 2;
		RESIZE_STACK;
	}

	void *top = &(*s)->tokens[((*s)->len - 1) * tk_size];
	(*s)->len--;

	return top;
}

size_t rdesc_stack_len(const struct rdesc_stack *s)
{
	return s->len;
}
