#include "../include/rdesc.h"
#include "../include/stack.h"
#include "detail.h"

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>


#define STACK_INITIAL_CAP 8


/**
 * @brief Default implementation of the stack.
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
	size_t len /** current number of elements in the stack */;
	size_t cap /** allocated capacity of the buffer */;
	size_t element_size /** size of a element in chars */;
	char elements[] /** pointer to the dynamic array buffer */;
};


static inline void *elem_at(struct rdesc_stack *s, size_t i)
{
	return cast(void *, &s->elements[i * s->element_size]);
}

static inline void resize_stack(struct rdesc_stack **s, size_t cap)
{
	*s = realloc(*s, sizeof(struct rdesc_stack) + cap * (*s)->element_size);
	assert_mem(*s);
	(*s)->cap = cap;
}

void rdesc_stack_init(struct rdesc_stack **s, size_t element_size)
{
	*s = malloc(sizeof(struct rdesc_stack) + STACK_INITIAL_CAP * element_size);
	assert_mem(s);

	(*s)->element_size = element_size;
	(*s)->len = 0;
	(*s)->cap = STACK_INITIAL_CAP;
}

void rdesc_stack_destroy(struct rdesc_stack *s)
{
	free(s);
}

void rdesc_stack_reset(struct rdesc_stack **s)
{
	if ((*s)->cap > STACK_INITIAL_CAP)
		resize_stack(s, STACK_INITIAL_CAP);

	(*s)->len = 0;
	(*s)->cap = STACK_INITIAL_CAP;
}

void rdesc_stack_foreach(struct rdesc_stack *stack, void fn(void *))
{
	for (size_t i = 0; i < stack->len; i++)
		fn(elem_at(stack, i));
}

void rdesc_stack_reserve(struct rdesc_stack **s, size_t reserved_space)
{
	size_t increased_cap = (*s)->cap;

	while (increased_cap <= (*s)->len - 1 + reserved_space) {
		rdesc_assert(increased_cap < SIZE_MAX / 2,
			     "go fix your grammar!");
		increased_cap *= 2;
	}

	if (increased_cap != (*s)->cap)
		resize_stack(s, increased_cap);

	(*s)->cap = increased_cap;
}

void *rdesc_stack_push(struct rdesc_stack **s, void *element)
{
	rdesc_stack_reserve(s, 1);

	void *top = elem_at(*s, (*s)->len);
	memcpy(top, element, (*s)->element_size);
	(*s)->len++;

	return top;
}

void *rdesc_stack_multipop(struct rdesc_stack **s, size_t count)
{
	size_t decreased_cap = (*s)->cap;

	while ((*s)->len * 4 <= decreased_cap &&
	       decreased_cap >= STACK_INITIAL_CAP * 2)
		decreased_cap /= 2;

	if (decreased_cap != (*s)->cap)
		resize_stack(s, decreased_cap);

	(*s)->len -= count;

	return rdesc_stack_top(*s);
}

void *rdesc_stack_top(struct rdesc_stack *s)
{
	return elem_at(s, s->len);
}

void *rdesc_stack_pop(struct rdesc_stack **s)
{
	return rdesc_stack_multipop(s, 1);
}

size_t rdesc_stack_len(const struct rdesc_stack *s)
{
	return s->len;
}
