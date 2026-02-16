/* Instrument stack with failure injections. */

#include <stdbool.h>
#include <stdlib.h>


bool check_failure(int *state, int count)
{
	if (*state > 0) {
		*state -= count;
		if (*state < 0)
			*state = 0;
	}

	if (*state == 0) {
		(*state)--;
		return true;
	}

	return false;
}


static int malloc_fail_at = -1;
#define xmalloc(...) (check_failure(&malloc_fail_at, 1) ? \
	NULL : malloc(__VA_ARGS__))

static int realloc_fail_at = -1;
#define xrealloc(...) (check_failure(&realloc_fail_at, 1) ? \
	NULL : realloc(__VA_ARGS__))

static int multipush_fail_at = -1;
#define inject_multipush_failure(c) check_failure(&multipush_fail_at, c)


#include "../../src/stack.c"
