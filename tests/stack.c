#include "../bnf.h"
#include "../detail.h"

#include <stdlib.h>
#include <time.h>


int main()
{
	srand(time(NULL));

	struct rdesc_token_stack s;
	int arr[128];

	rdesc_token_stack_init(&s);

	for (int i = 0; i < 128; i++)
		rdesc_token_stack_push(
			&s, (struct bnf_token) { .id = (arr[i] = rand()) }
		);

	for (int i = 0; i < 128; i++)
		assert(rdesc_token_stack_top(&s).id == arr[127 - i] &&
		       rdesc_token_stack_pop(&s).id == arr[127 - i],
		       "stack order not reserved");

	rdesc_token_stack_destroy(&s);
}
