#include "../bnf.h"
#include "../rdesc.h"
#include "../stack.h"
#include "../detail.h"

#include <stdlib.h>
#include <time.h>


int main()
{
	srand(time(NULL));

	struct rdesc_stack s;
	int arr[128];

	rdesc_stack_init(&s);

	for (int i = 0; i < 128; i++)
		rdesc_stack_push(
			&s, (struct bnf_token) { .id = (arr[i] = rand()) }
		);

	for (int i = 0; i < 128; i++)
		assert(rdesc_stack_pop(&s).id == arr[127 - i],
		       "stack order not reserved");

	free(rdesc_stack_into_inner(&s));
}
