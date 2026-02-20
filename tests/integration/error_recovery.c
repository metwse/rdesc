/* Test rdesc pumping mechanism under OOM stress. */

#include "../../include/grammar.h"
#include "../../include/rdesc.h"
#include "../../src/detail.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define TEST_INSTRUMENTS

#include "../../src/test_instruments.h"
#include "../lib/bc_fuzzer.c"


#define INJECT_FAILURE do { \
	if (multipush_fail_at < 0) \
		multipush_fail_at = rand() % 32; \
	if (realloc_fail_at < 0) \
		realloc_fail_at = rand() % 32; \
	if (malloc_fail_at < 0) \
		malloc_fail_at = rand() % 8; \
	} while (0)


static const char *const enum_str[] = {
	"OK",
	"INIT_FAILED",
	"START_FAILED",
};

static size_t global_seminfo_counter;

void token_destroyer(uint16_t id, void *seminfo_)
{
	((void) id);
	size_t seminfo;

	memcpy(&seminfo, seminfo_, sizeof(size_t));

	rdesc_assert(seminfo == global_seminfo_counter,
		     "seminfo not preserved");

	global_seminfo_counter--;
}

enum {
	OK,
	INIT_FAILED,
	START_FAILED,
} test_complete_parse(struct rdesc_grammar *grammar)
{
	struct rdesc p;
	struct bc_grammar_generator g = BC_DEFAULT_GENERATOR;
	enum rdesc_result res;
	uint16_t tk;
	size_t seminfo_size = (rand() % 4) * sizeof(size_t);

	INJECT_FAILURE;

	if (rdesc_init(&p,
		       grammar,
		       seminfo_size,
		       seminfo_size ? token_destroyer : NULL))
		return INIT_FAILED;

	if (rdesc_start(&p, NT_STMT)) {
		rdesc_destroy(&p);

		return START_FAILED;
	}

	bool retry_on_enomem = rand() > RAND_MAX / 4;

	bool token_may_repeat = rand() < RAND_MAX / 4;
	bool token_repeated = false;

	size_t seminfo_counter = 0;
	global_seminfo_counter = 0;

	while ((tk = bc_fuzzer_next_tk(&g)) != TK_ENDSYM) {
		g.group_start_p *= 0.95;

		INJECT_FAILURE;

		do {
			seminfo_counter++;
			global_seminfo_counter++;
			res = rdesc_pump(&p, tk, &seminfo_counter);

			if (!retry_on_enomem && res == RDESC_ENOMEM)
				break;

			while (res == RDESC_ENOMEM) {
				INJECT_FAILURE;

				res = rdesc_resume(&p);
			}

			if (res != RDESC_CONTINUE)
				break;
		} while (token_may_repeat
			 && rand() % 16 == 0
			 && (token_repeated = true));

		if (res != RDESC_CONTINUE)
			break;
	}

	if (res == RDESC_CONTINUE) {
		multipush_fail_at = -1;
		realloc_fail_at = -1;

		seminfo_counter++;
		global_seminfo_counter++;

		rdesc_assert(rdesc_pump(&p, TK_ENDSYM, &seminfo_counter) == RDESC_READY
			     || token_repeated /* allow match to be failed if a token is repeated */,
			     "could not match grammar");
	}

	rdesc_destroy(&p);

	return OK;
}


int main(void)
{
	srand(time(NULL));

	struct rdesc_grammar grammar;

	rdesc_grammar_init(&grammar,
			   BC_NT_COUNT, BC_NT_VARIANT_COUNT, BC_NT_BODY_LENGTH,
			   (struct rdesc_grammar_symbol *) bc);

	int failure_stats[3] = { 0, 0, 0 };

	/* test interruption & complete parse in the same parser */
	for (int _fuzz = 0; _fuzz < 1024; _fuzz++)
		failure_stats[test_complete_parse(&grammar)]++;

	for (int i = 0; i < 3; i++)
		printf("%s: %d\n", enum_str[i], failure_stats[i]);

	rdesc_grammar_destroy(&grammar);
}
