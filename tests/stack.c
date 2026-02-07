#include "../include/cfg.h"
#include "../include/rdesc.h"
#include "../include/stack.h"
#include "../src/detail.h" // IWYU pragma: keep

#include <stdlib.h>
#include <time.h>


int main()
{
	srand(time(NULL));

	struct rdesc_stack *s;
	int arr[128];

	rdesc_stack_init(&s);

	for (int i = 0; i < 128; i++)
		rdesc_stack_push(
			&s, (struct rdesc_cfg_token) { .id = (arr[i] = rand()) }
		);

	for (int i = 0; i < 128; i++)
		rdesc_assert(rdesc_stack_pop(&s).id == arr[127 - i],
			     "stack order not reserved");

	rdesc_stack_destroy(s);
}
