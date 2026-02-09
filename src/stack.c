#include "../include/rdesc.h"
#include "../include/stack.h"
#include "detail.h"

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>


#define STACK_INITIAL_CAP 8


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
	size_t tk_size /** size of a token in chars */;
	char tokens[] /** pointer to the dynamic array buffer */;
};


static inline struct rdesc_token *elem_at(struct rdesc_stack *s, size_t i)
{
	return cast(struct rdesc_token *, &s->tokens[i * s->tk_size]);
}

static inline void resize_stack(struct rdesc_stack **s, size_t cap)
{
	*s = realloc(*s, sizeof(struct rdesc_stack) + cap * (*s)->tk_size);
	assert_mem(*s);
	(*s)->cap = cap;
}

void rdesc_stack_init(struct rdesc_stack **s, size_t tk_size)
{
	*s = malloc(sizeof(struct rdesc_stack) + STACK_INITIAL_CAP * tk_size);
	assert_mem(s);

	(*s)->tk_size = tk_size;
	(*s)->len = 0;
	(*s)->cap = STACK_INITIAL_CAP;
}

void rdesc_stack_push(struct rdesc_stack **s,
		      struct rdesc_token *tk)
{
	rdesc_assert((*s)->cap < SIZE_MAX / 2, "go fix your grammar!");

	if ((*s)->cap == (*s)->len)
		resize_stack(s, (*s)->cap * 2);

	memcpy(elem_at(*s, (*s)->len), tk, (*s)->tk_size);
	(*s)->len++;
}

void rdesc_stack_destroy(struct rdesc_stack *s,
			 rdesc_token_destroyer_func tk_destroyer)
{
	if (tk_destroyer)
		for (size_t i = 0; i < s->len; i++)
			tk_destroyer(elem_at(s, i));

	free(s);
}

void rdesc_stack_reset(struct rdesc_stack **s,
		       rdesc_token_destroyer_func tk_destroyer)
{
	if (tk_destroyer)
		for (size_t i = 0; i < (*s)->len; i++)
			tk_destroyer(elem_at(*s, i));

	if ((*s)->cap > STACK_INITIAL_CAP)
		resize_stack(s, STACK_INITIAL_CAP);

	(*s)->len = 0;
	(*s)->cap = STACK_INITIAL_CAP;
}

struct rdesc_token *rdesc_stack_pop(struct rdesc_stack **s)
{
	if ((*s)->len * 4 <= (*s)->cap && (*s)->cap >= STACK_INITIAL_CAP * 2)
		resize_stack(s, (*s)->cap / 2);

	(*s)->len--;

	return elem_at(*s, (*s)->len);
}

size_t rdesc_stack_len(const struct rdesc_stack *s)
{
	return s->len;
}
