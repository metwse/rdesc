#include "test_instruments.h"


int malloc_fail_at = -1;
int realloc_fail_at = -1;
int multipush_fail_at = -1;

bool rdesc_test_instruments_check_failure(int *state, int count)
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
