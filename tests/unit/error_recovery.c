/* Test rdesc pumping mechanism under OOM stress. */

#include "../../include/grammar.h"
#include "../../include/rdesc.h"
#include "../../src/detail.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define STACK_INTIAL_CAP 1

#include "../lib/instrumented_stack.c"
#include "../lib/bc_fuzzer.c"

#include "../../src/rdesc.c"
#include "../../src/grammar.c"


void test_complete_parse(struct rdesc_grammar *grammar)
{
	struct rdesc p;
	rdesc_init(&p, grammar, rand() % 8, NULL);

	struct bc_grammar_generator g = BC_DEFAULT_GENERATOR;

	uint16_t tk;

	rdesc_start(&p, NT_STMT);

	while ((tk = bc_fuzzer_next_tk(&g)) != TK_ENDSYM) {
		g.group_start_p *= 0.9;

		while (true) {
			if (multipush_fail_at < 0)
				multipush_fail_at = rand() % 8;

			if (realloc_fail_at < 0)
				realloc_fail_at = rand() % 8;

			enum rdesc_result res = rdesc_pump(&p, tk, NULL);

			if (res == RDESC_CONTINUE) {
				break;
			} else if (res == RDESC_ENOMEM) {
				tk = 0;
			}
		}
	}

	multipush_fail_at = -1;
	realloc_fail_at = -1;
	rdesc_assert(rdesc_pump(&p, TK_ENDSYM, NULL) == RDESC_READY,
		     "could not match grammar");

	rdesc_destroy(&p);
}


int main(void)
{
	srand(time(NULL));

	struct rdesc_grammar grammar;

	rdesc_grammar_init(&grammar,
			   BC_NT_COUNT, BC_NT_VARIANT_COUNT, BC_NT_BODY_LENGTH,
			   (struct rdesc_grammar_symbol *) bc);

	/* test interruption & complete parse in the same parser */
	for (int _fuzz = 0; _fuzz < 32; _fuzz++) {
		test_complete_parse(&grammar);
	}

	rdesc_grammar_destroy(&grammar);
}
