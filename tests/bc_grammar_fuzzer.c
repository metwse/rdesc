/* Generate random grammar and parse several statements with the same parser
 * instance. */

#include "../include/rdesc.h"
#include "../src/detail.h"

#include "../examples/grammar/bc.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


struct grammar_generator {
	double group_start_p;
	double group_end_p;
	size_t group_depth;

	bool trigger_ambiguity;

	enum state {
		GENERATE_LPAREN,
		GENERATE_SIGN,
		GENERATE_NUM,
		GENERATE_RPAREN,
		GENERATE_OP,
	} state;
};

#define DEFAULT_GENERATOR (struct grammar_generator) { \
		.group_start_p = 1, \
		.group_end_p = 0.5, \
		.group_depth = 0, \
		.state = GENERATE_LPAREN, \
	}


enum bc_tk next_tk(struct grammar_generator *g)
{
	int r = rand();
	int r_ambiguity = rand();

	if (g->trigger_ambiguity) {
		g->trigger_ambiguity = false;
		return TK_DUMMY_AMBIGUITY_TRIGGER;
	}

	enum bc_tk out;

	switch (g->state) {
	case GENERATE_SIGN:
		if (r < RAND_MAX / 6 * 4) {
			g->state += 1;
			return next_tk(g);  // skip sign
		} else if (r < RAND_MAX / 6 * 5) {
			out = TK_PLUS;
		} else {
			out = TK_MINUS;
		}
		break;
	case GENERATE_NUM:
		out = TK_NUM;
		break;
	case GENERATE_LPAREN:
		if (r < RAND_MAX * g->group_start_p) {
			g->group_depth += 1;
			out = TK_LPAREN;
		} else {
			g->state += 1;
			return next_tk(g);  // skip parenthesis
		}
		break;
	case GENERATE_RPAREN:
		if (r < RAND_MAX * g->group_end_p) {
			if (g->group_depth > 0) {
				if (r_ambiguity < RAND_MAX / 2)
					g->trigger_ambiguity = true;

				g->group_depth -= 1;
				out = TK_RPAREN;
			} else {
				return TK_ENDSYM;
			}
		} else {
			g->state += 1;
			return next_tk(g);  // skip parenthesis
		}
		break;
	case GENERATE_OP:
		if (r < RAND_MAX / 4)
			out = TK_PLUS;
		else if (r < RAND_MAX / 4 * 2)
			out = TK_MINUS;
		else if (r < RAND_MAX / 4 * 3)
			out = TK_MULT;
		else
			out = TK_DIV;
		break;
	}

	g->state += 1;
	g->state %= 5;

	return out;
}

void test_complete_parse(struct rdesc *p)
{
	struct grammar_generator g = DEFAULT_GENERATOR;

	enum bc_tk tk;
	struct rdesc_node *cst;
	uint64_t dummy_seminfo_buf = 0;

	rdesc_start(p, NT_STMT);
	while ((tk = next_tk(&g)) != TK_ENDSYM) {
		g.group_start_p *= 0.9;

		rdesc_pump(p, &cst, tk, &dummy_seminfo_buf);
	};

	rdesc_assert(rdesc_pump(p, &cst,  TK_ENDSYM, NULL) == RDESC_READY,
		     "could not match grammar");

	rdesc_reset(p, NULL);
}

void dummy_token_destroyer(uint16_t id, void *seminfo)
{
	((void) id);
	((void) seminfo);
}

void test_interruption(struct rdesc *p)
{
	struct grammar_generator g = DEFAULT_GENERATOR;

	enum bc_tk tk;
	struct rdesc_node *cst;
	uint64_t dummy_seminfo_buf = 0;

	bool broken = false;

	rdesc_start(p, NT_STMT);
	while ((tk = next_tk(&g)) != TK_ENDSYM) {
		g.group_start_p *= 0.9;

		if (rand() < RAND_MAX / 8) {
			broken = true;
			break;
		}

		rdesc_pump(p, &cst, tk, &dummy_seminfo_buf);
	};

	if (broken && rand() < RAND_MAX / 2)
		rdesc_pump(p, &cst, TK_DUMMY_AMBIGUITY_TRIGGER, &dummy_seminfo_buf);

	rdesc_reset(p, dummy_token_destroyer);
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
	struct grammar_generator g = DEFAULT_GENERATOR;

	struct rdesc p;
	enum bc_tk tk;
	struct rdesc_node *cst;

	size_t seminfo_size = (2 + rand() % 8);

	size_t *seminfo[seminfo_size];

	rdesc_init(&p, seminfo_size * sizeof(size_t *), cfg);

	rdesc_start(&p, NT_STMT);
	while ((tk = next_tk(&g)) != TK_ENDSYM) {
		g.group_start_p *= 0.9;

		for (size_t i = 0; i < seminfo_size; i++) {
			seminfo[i] = malloc(sizeof(size_t));

			*seminfo[i] = i;
		}
		*seminfo[0] = seminfo_size;
		*seminfo[1] = tk;

		rdesc_pump(&p, &cst, tk, seminfo);
	};

	rdesc_destroy(&p, token_destroyer_for_test);
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
		rdesc_init(&p, rand() % 8, &cfg);

		test_interruption(&p);
		test_complete_parse(&p);

		rdesc_destroy(&p, NULL);
	}

	/* test destroying the parser during a parse */
	rdesc_init(&p, rand() % 8, &cfg);

	rdesc_start(&p, NT_STMT);
	rdesc_pump(&p, NULL, TK_NUM, NULL);

	rdesc_destroy(&p, NULL);

	for (int _fuzz = 0; _fuzz < 16; _fuzz++) {
		test_with_seminfo(&cfg);
	}

	rdesc_cfg_destroy(&cfg);

}
