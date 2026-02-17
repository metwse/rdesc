/* Generate a random grammar and parse several statements with the same parser
 * instance. */

#include "../../include/rdesc.h"
#include "../../src/detail.h"

#include "../../examples/grammar/bc.h"

#include "../lib/bc_fuzzer.c"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


void test_complete_parse(struct rdesc *p)
{
	struct bc_grammar_generator g = BC_DEFAULT_GENERATOR;

	uint16_t tk;

	rdesc_start(p, NT_STMT);
	while ((tk = bc_fuzzer_next_tk(&g)) != TK_ENDSYM) {
		g.group_start_p *= 0.9;

		rdesc_pump(p, tk, NULL);
	};

	tk = TK_ENDSYM;
	rdesc_assert(rdesc_pump(p, tk, NULL) == RDESC_READY,
		     "could not match grammar");

	rdesc_reset(p);
}

void test_interruption(struct rdesc *p)
{
	struct bc_grammar_generator g = BC_DEFAULT_GENERATOR;

	uint16_t tk;

	bool broken = false;

	rdesc_start(p, NT_STMT);
	while ((tk = bc_fuzzer_next_tk(&g)) != TK_ENDSYM) {
		g.group_start_p *= 0.9;

		if (rand() < RAND_MAX / 8) {
			broken = true;
			break;
		}

		rdesc_pump(p, tk, NULL);
	};

	if (broken && rand() < RAND_MAX / 2) {
		tk = TK_DUMMY_AMBIGUITY_TRIGGER;
		rdesc_pump(p, tk, NULL);
	}

	rdesc_reset(p);
}

void token_destroyer_for_test(uint16_t id, void *seminfo_)
{
	size_t *seminfo = seminfo_;

	size_t *seminfo_size;
	size_t *id_in_seminfo;

	memcpy(&seminfo_size, &seminfo[0], sizeof(void *));
	memcpy(&id_in_seminfo, &seminfo[1], sizeof(void *));

	rdesc_assert(*id_in_seminfo == id, "seminfo not preserved");

	size_t *elem;
	for (size_t i = 2; i < *seminfo_size; i++) {
		memcpy(&elem, &seminfo[i], sizeof(void *));
		rdesc_assert(*elem == i, "seminfo not preserved");
		free(elem);
	}

	free(seminfo_size);
	free(id_in_seminfo);
}

void test_with_seminfo(const struct rdesc_cfg *cfg)
{
	struct bc_grammar_generator g = BC_DEFAULT_GENERATOR;

	struct rdesc p;
	uint16_t tk;

	size_t seminfo_size = (2 + rand() % 8);

	size_t *seminfo[seminfo_size];

	rdesc_init(&p, cfg, seminfo_size * sizeof(size_t *), token_destroyer_for_test);

	rdesc_start(&p, NT_STMT);
	while ((tk = bc_fuzzer_next_tk(&g)) != TK_ENDSYM) {
		g.group_start_p *= 0.9;

		for (size_t i = 0; i < seminfo_size; i++) {
			seminfo[i] = malloc(sizeof(size_t));

			*seminfo[i] = i;
		}
		*seminfo[0] = seminfo_size;
		*seminfo[1] = tk;

		rdesc_pump(&p, tk, seminfo);
	};

	rdesc_destroy(&p);
}


int main(void)
{
	srand(time(NULL));

	struct rdesc_cfg cfg;
	struct rdesc p;

	rdesc_cfg_init(&cfg, BC_NT_COUNT, BC_NT_VARIANT_COUNT,
		       BC_NT_BODY_LENGTH, (struct rdesc_cfg_symbol *) bc);


	/* test interruption & complete parse in the same parser */
	for (int _fuzz = 0; _fuzz < 16; _fuzz++) {
		rdesc_init(&p, &cfg, rand() % 8, NULL);

		test_interruption(&p);
		test_complete_parse(&p);

		rdesc_destroy(&p);
	}

	/* test destroying the parser during a parse */
	rdesc_init(&p, &cfg, rand() % 8, NULL);

	rdesc_start(&p, NT_STMT);
	uint16_t id = TK_NUM;
	rdesc_pump(&p, id, NULL);

	rdesc_destroy(&p);

	for (int _fuzz = 0; _fuzz < 16; _fuzz++) {
		test_with_seminfo(&cfg);
	}

	rdesc_cfg_destroy(&cfg);

}
