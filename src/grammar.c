#include "../include/grammar.h"
#include "../include/bnf_macros.h"
#include "detail.h"

#include <stdint.h>
#include <stdlib.h>


void rdesc_grammar_init(struct rdesc_grammar *grammar,
			uint32_t nt_count,
			uint16_t nt_variant_count,
			uint16_t nt_body_length,
			const struct rdesc_grammar_symbol *rules)
{
	grammar->rules = rules;

	grammar->nt_count = nt_count;
	grammar->nt_variant_count = nt_variant_count;
	grammar->nt_body_length = nt_body_length;

	grammar->child_caps = malloc(sizeof(size_t) * nt_count);
	rdesc_assert(grammar->child_caps,
	      "pre-computed child capacity array could not be allocated");

	for (size_t nt_id = 0; nt_id < nt_count; nt_id++) {
		grammar->child_caps[nt_id] = 0;

		for (size_t variant = 0; variant < nt_variant_count; variant++) {
			size_t len;
			struct rdesc_grammar_symbol sym;

			for (len = 0;
				(sym = productions(*grammar)[nt_id][variant][len]).ty !=
				RDESC_SENTINEL;
				len++);

			if (len > grammar->child_caps[nt_id])
				grammar->child_caps[nt_id] = len;

			if (sym.id == EOC)
				break;
		}
	}
}

void rdesc_grammar_destroy(struct rdesc_grammar *grammar)
{
	free(grammar->child_caps);
}
