#include "../include/rdesc.h"
#include "../include/stack.h"
#include "../src/detail.h" // IWYU pragma: keep

#include <stdlib.h>
#include <time.h>


int main(void)
{
	srand(time(NULL));

	struct rdesc_stack *s;
	int arr[128];
	size_t tk_size = sizeof(struct rdesc_token) + rand() % 16;

	rdesc_stack_init(&s, tk_size);

	for (int _fuzz = 0; _fuzz < 16; _fuzz++) {
		for (int i = 0; i < 128; i++)
			rdesc_stack_push(&s,
					 &(struct rdesc_token)
					 { .id = (arr[i] = rand() % 1024) });

		for (int i = 0; i < 64; i++)
			rdesc_assert(rdesc_stack_pop(&s)->id == arr[127 - i],
				     "stack order not reserved");

		rdesc_stack_reset(&s, NULL);
	}

	rdesc_stack_destroy(s, NULL);
}
