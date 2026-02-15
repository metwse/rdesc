/* Stress test underlying stack implementation. */

#include "../../include/stack.h"
#include "../../src/detail.h"

#define STACK_INITIAL_CAP 2

#include "../../src/stack.c"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


void test_fuzz(void)
{
	size_t element_count = 1 << (6 + rand() % 4);
	size_t element_size = 8 + rand() % 16;
	size_t multipush_count = 1 << (rand() % 4);

	char elements[element_count][element_size];

	char buf[element_size * multipush_count];

	struct rdesc_stack *s;
	rdesc_stack_init(&s, element_size);

	stack_reserve(&s, 64);

	for (size_t i = 0; i < element_count; i += multipush_count) {
		for (size_t j = 0; j < element_size * multipush_count; j++)
			buf[j] = elements[i][j] = rand() % 256;

		rdesc_assert(rdesc_stack_len(s) == i,);

		if (i < element_count / 2) {
			rdesc_stack_multipush(&s, buf, multipush_count);
		} else {
			void *top = rdesc_stack_multipush(&s, NULL, multipush_count);
			memcpy(top, buf, element_size * multipush_count);
		}

		rdesc_stack_multipush(&s, NULL, 0);
	}

	char *top;
	for (size_t i = 0; i < (multipush_count << 2); i++) {
		top = rdesc_stack_pop(&s);

		rdesc_assert(memcmp(top, elements[element_count - 1 - i],
				    element_size) == 0,
			     "element corrupted");

		rdesc_stack_multipop(&s, 0);
	}

	if (rand() % 2) {
		for (size_t i = (multipush_count << 2); i < element_count; i += multipush_count) {
			top = rdesc_stack_multipop(&s, multipush_count);

			rdesc_assert(memcmp(top,
					    elements[element_count - multipush_count - i],
					    element_size * multipush_count) == 0,
				     "element corrupted");
		}
	} else {
		rdesc_stack_reset(&s);
	}

	rdesc_stack_destroy(s);
}

void test_basic(void)
{
	struct rdesc_stack *s;
	rdesc_stack_init(&s, 8);

	for (uint64_t i = 0; i < 2048; i++) {
		rdesc_stack_push(&s, &i);
		uint64_t top = *cast(uint64_t *, rdesc_stack_top(s));
		rdesc_assert(top == i,);
	}

	for (uint64_t i = 0; i < 2048; i++) {
		uint64_t elem = *cast(uint64_t *, rdesc_stack_at(s, i));
		rdesc_assert(elem == i,);
		rdesc_stack_multipop(&s, 0);
	}

	rdesc_stack_destroy(s);
}


int main(void)
{
	srand(time(NULL));

	test_basic();

	for (int _fuzz = 0; _fuzz < 16; _fuzz++)
		test_fuzz();
}
