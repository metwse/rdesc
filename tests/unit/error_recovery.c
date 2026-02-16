/* Test rdesc pumping mechanism under OOM stress. */

#include "../../include/cfg.h"
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
#include "../../src/cfg.c"


void test_complete_parse(struct rdesc_cfg *cfg)
{
	struct rdesc p;
	rdesc_init(&p, cfg, rand() % 8);

	struct bc_grammar_generator g = BC_DEFAULT_GENERATOR;

	uint16_t tk;
	struct rdesc_node *cst;

	rdesc_start(&p, NT_STMT);

	while ((tk = bc_fuzzer_next_tk(&g)) != TK_ENDSYM) {
		g.group_start_p *= 0.9;

		uint16_t *tk_ = &tk;
		while (true) {
			if (multipush_fail_at < 0)
				multipush_fail_at = rand() % 8;

			if (realloc_fail_at < 0)
				realloc_fail_at = rand() % 8;

			enum rdesc_result res = rdesc_pump(&p, NULL, tk_, NULL);

			if (res == RDESC_CONTINUE) {
				break;
			} else if (res == RDESC_ENOMEM) {
				tk = 0;
			} else if (res != RDESC_ENOMEM_SEMINFO_NOT_OWNED) {
				rdesc_assert(0, "unreachable");  // GCOVR_EXCL_LINE
			}
		}
	}

	multipush_fail_at = -1;
	realloc_fail_at = -1;
	tk = TK_ENDSYM;
	rdesc_assert(rdesc_pump(&p, &cst, &tk, NULL) == RDESC_READY,
		     "could not match grammar");

	rdesc_destroy(&p, NULL);
}


int main(void)
{
	srand(time(NULL));

	struct rdesc_cfg cfg;

	rdesc_cfg_init(&cfg, BC_NT_COUNT, BC_NT_VARIANT_COUNT,
		       BC_NT_BODY_LENGTH, (struct rdesc_cfg_symbol *) bc);

	/* test interruption & complete parse in the same parser */
	for (int _fuzz = 0; _fuzz < 32; _fuzz++) {
		test_complete_parse(&cfg);
	}

	rdesc_cfg_destroy(&cfg);
}
