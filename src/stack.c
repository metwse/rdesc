#include "../include/stack.h"
#include "test_instruments.h"
#include "detail.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>


#ifndef STACK_INITIAL_CAP
#define STACK_INITIAL_CAP 32
#endif

#ifndef STACK_MAX_CAP
#define STACK_MAX_CAP SIZE_MAX
#endif


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
	char elements[] /** the dynamic array buffer */;
};


static inline void *elem_at(struct rdesc_stack *s, size_t i)
{
	rdesc_assert(s->len >= i, "range overflow");

	return cast(void *, &s->elements[i * s->element_size]);
}

/* return non-zero value if reallocation failure */
static inline int resize_stack(struct rdesc_stack **s, size_t cap)
{
	struct rdesc_stack *new =
		xrealloc(*s, sizeof(struct rdesc_stack) + cap * (*s)->element_size);

	if (new != NULL) {
		*s = new;
		(*s)->cap = cap;

		return 0;
	} else {
		return 1;
	}
}

/* returns non-zero value if resize failure */
static int stack_reserve(struct rdesc_stack **s, size_t reserved_space)
{
	size_t increased_cap = (*s)->cap;

	while (increased_cap <= (*s)->len + reserved_space) {
		if (increased_cap > STACK_MAX_CAP / (2 * (*s)->element_size))
			return 1;

		increased_cap *= 2;
	}

	if (increased_cap != (*s)->cap)
		return resize_stack(s, increased_cap);

	return 0;
}

void rdesc_stack_init(struct rdesc_stack **s, size_t element_size)
{
	*s = xmalloc(sizeof(struct rdesc_stack) + STACK_INITIAL_CAP * element_size);

	if (*s == NULL)
		return;

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
	if ((*s)->cap > STACK_INITIAL_CAP) {
		resize_stack(s, STACK_INITIAL_CAP);
	}

	(*s)->len = 0;
}

void *rdesc_stack_at(struct rdesc_stack *s, size_t i)
{
	return elem_at(s, i);
}

void *rdesc_stack_multipush(struct rdesc_stack **s, void *element, size_t count)
{
	/* return null if grow failed */
	if (stack_reserve(s, count) || (xmultipush(count)))
		return NULL;

	void *top = elem_at(*s, (*s)->len);

	if (element)
		memcpy(top, element, (*s)->element_size * count);
	(*s)->len += count;

	return top;
}

void *rdesc_stack_push(struct rdesc_stack **s, void *element)
{
	return rdesc_stack_multipush(s, element, 1);
}

void *rdesc_stack_multipop(struct rdesc_stack **s, size_t count)
{
	rdesc_assert((*s)->len >= count, "stack underflow");

	size_t decreased_cap = (*s)->cap;

	while ((*s)->len * 4 <= decreased_cap &&
	       decreased_cap >= STACK_INITIAL_CAP * 2)
		decreased_cap /= 2;

	(*s)->len -= count;

	if (decreased_cap != (*s)->cap)
		resize_stack(s, decreased_cap);

	return elem_at(*s, (*s)->len);
}

void *rdesc_stack_top(struct rdesc_stack *s)
{
	return elem_at(s, s->len - 1);
}

void *rdesc_stack_pop(struct rdesc_stack **s)
{
	return rdesc_stack_multipop(s, 1);
}

size_t rdesc_stack_len(const struct rdesc_stack *s)
{
	return s->len;
}
