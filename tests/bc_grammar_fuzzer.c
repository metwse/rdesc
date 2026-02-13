#include "../include/rdesc.h"
#include "../src/detail.h"

#include "../examples/grammar/bc.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
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


enum bc_tk next_tk(struct grammar_generator *g) {
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

void test_complete_parse(struct rdesc *p) {
	struct grammar_generator g = DEFAULT_GENERATOR;

	enum bc_tk tk;
	struct rdesc_node *cst;

	rdesc_start(p, NT_STMT);
	while ((tk = next_tk(&g)) != TK_ENDSYM) {
		g.group_start_p *= 0.99;

		rdesc_pump(p, &cst, tk, NULL);
	};

	rdesc_assert(rdesc_pump(p, &cst,  TK_ENDSYM, NULL) == RDESC_READY,
		     "could not match grammar");

	rdesc_reset(p);
}

void test_interruption(struct rdesc *p) {
	struct grammar_generator g = DEFAULT_GENERATOR;

	enum bc_tk tk;
	struct rdesc_node *cst;

	rdesc_start(p, NT_STMT);
	while ((tk = next_tk(&g)) != TK_ENDSYM) {
		g.group_start_p *= 0.9;

		rdesc_pump(p, &cst, tk, NULL);
	};

	rdesc_reset(p);
}


int main(void)
{
	struct rdesc_cfg cfg;
	struct rdesc p;

	rdesc_cfg_init(&cfg, BC_NT_COUNT, BC_NT_VARIANT_COUNT,
		       BC_NT_BODY_LENGTH, (struct rdesc_cfg_symbol *) bc);
	rdesc_init(&p, 0, &cfg);

	srand(time(NULL));

	/* test interruption & complete parse in the same parser */
	for (int _fuzz = 0; _fuzz < 16; _fuzz++) {
		test_interruption(&p);
		test_complete_parse(&p);
	}

	/* test destroying the parser during a parse */
	rdesc_start(&p, NT_STMT);
	rdesc_pump(&p, NULL, TK_NUM, NULL);
	rdesc_destroy(&p);

	rdesc_cfg_destroy(&cfg);
}
