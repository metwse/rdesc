/* Basic tests to validate stack implementation. */

#include "../../include/stack.h"
#include "../../src/detail.h"

#define STACK_MAX_CAP STACK_INITIAL_CAP * 64

#include "lib/instrumented_stack.c"


int main(void)
{
	struct rdesc_stack *s1, *s2, *s3;

	malloc_fail_at = 2;

	rdesc_stack_init(&s1, sizeof(int));
	rdesc_stack_init(&s2, 1);
	rdesc_stack_init(&s3, 1);

	rdesc_assert(s1 && s3, "stack 1 and 3 expected to be allocated");
	rdesc_assert(!s2, "stack 2 expected to be failed to allocate");

	realloc_fail_at = 2;

	rdesc_assert(rdesc_stack_multipush(&s1, NULL, STACK_INITIAL_CAP),
		     "expected bottom of new pushed elements");

	rdesc_assert(rdesc_stack_multipush(&s1, NULL, STACK_INITIAL_CAP * 2) == NULL,
		     "expected realloc failure");

	rdesc_assert(rdesc_stack_multipush(&s1, NULL, STACK_MAX_CAP) == NULL,
		     "expected size max hit");

	rdesc_stack_reset(&s1);

	multipush_fail_at = 11;
	for (int i = 0; i < 10; i++)
		rdesc_stack_push(&s1, &i);

	rdesc_assert(rdesc_stack_multipush(&s1, NULL, 2) == NULL,
		     "expected push to be failed");

	rdesc_stack_destroy(s1);

	rdesc_assert(rdesc_stack_multipush(&s3, NULL, STACK_INITIAL_CAP * 64 - 1),
		     "expected multipush to be succeeded");

	realloc_fail_at = 1;

	rdesc_stack_reset(&s3);
	rdesc_assert(s3 == NULL, "expect reset to be failed");
}
