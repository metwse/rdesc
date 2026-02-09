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

	for (int i = 0; i < 128; i++)
		rdesc_stack_push(
			&s,
			&(struct rdesc_token) { .id = (arr[i] = rand() % 1024) },
			tk_size
		);

	for (int i = 0; i < 64; i++)
		rdesc_assert(rdesc_stack_pop(&s, tk_size)->id == arr[127 - i],
			     "stack order not reserved");

	rdesc_stack_reset(&s, NULL, tk_size);

	rdesc_stack_destroy(s);
}
