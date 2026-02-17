#include "../../examples/grammar/bc.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>


#define BC_DEFAULT_GENERATOR (struct bc_grammar_generator) { \
		.group_start_p = 1, \
		.group_end_p = 0.5, \
		.group_depth = 0, \
		.state = GENERATE_LPAREN, \
	}


struct bc_grammar_generator {
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


enum bc_tk bc_fuzzer_next_tk(struct bc_grammar_generator *g)
{
	int r = rand();
	int r_ambiguity = rand();

	if (g->trigger_ambiguity) {
		g->trigger_ambiguity = false;
		return TK_DUMMY_AMBIGUITY_TRIGGER;
	}

	enum bc_tk out = 0;

	switch (g->state) {
	case GENERATE_SIGN:
		if (r < RAND_MAX / 6 * 4) {
			g->state += 1;
			return bc_fuzzer_next_tk(g);  // skip sign
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
			return bc_fuzzer_next_tk(g);  // skip parenthesis
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
			return bc_fuzzer_next_tk(g);  // skip parenthesis
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
