#include "../include/rdesc.h"
#include "../include/stack.h"
#include "../src/detail.h" // IWYU pragma: keep

#include <stdlib.h>
#include <string.h>
#include <time.h>


int main(void)
{
	srand(time(NULL));

	size_t seminfo_size = 8 + rand() % 16;

	int arr[128];
	int seminfo[128][seminfo_size];

	char incoming_tk_buf[sizeof(struct rdesc_token) + seminfo_size];
	struct rdesc_token *incoming_tk = cast(void *, &incoming_tk_buf);

	struct rdesc_stack *s;
	rdesc_stack_init(&s, seminfo_size);

	for (int _fuzz = 0; _fuzz < 16; _fuzz++) {
		for (int i = 0; i < 128; i++) {
			for (size_t j = 0; j < seminfo_size; j++)
				incoming_tk->seminfo[j] = seminfo[i][j] = rand() % 256;

			incoming_tk->id = arr[i] = rand() % 1024;

			rdesc_stack_push(&s, incoming_tk);
		}

		struct rdesc_token *top;

		for (int i = 0; i < 64; i++) {
			top = rdesc_stack_pop(&s);

			rdesc_assert(top->id == arr[127 - i],
				     "stack order not reserved");
			rdesc_assert(memcmp(&s, seminfo[127 - i], seminfo_size) != 0,
				     "seminfo corrupted");
		}

		rdesc_stack_reset(&s, NULL);
	}

	rdesc_stack_destroy(s, NULL);
}
