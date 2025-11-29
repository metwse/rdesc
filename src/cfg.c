#include "../include/cfg.h"
#include "../include/bnf_dsl.h"
#include "detail.h"

#include <stdlib.h>


void rdesc_cfg_init(struct rdesc_cfg *cfg,
		    size_t nt_count,
		    size_t nt_variant_count,
		    size_t nt_body_length,
		    const struct rdesc_cfg_symbol *rules)
{
	cfg->rules = rules;

	cfg->nt_count = nt_count;
	cfg->nt_variant_count = nt_variant_count;
	cfg->nt_body_length = nt_body_length;

	assert_mem(cfg->child_caps = malloc(sizeof(size_t) * nt_count));

	for (size_t nt_id = 0; nt_id < nt_count; nt_id++) {
		cfg->child_caps[nt_id] = 0;

		for (size_t variant = 0; variant < nt_variant_count; variant++) {
			size_t len;
			struct rdesc_cfg_symbol sym;

			for (len = 0;
				(sym = productions(*cfg)[nt_id][variant][len]).ty !=
				CFG_SENTINEL;
				len++);

			if (len > cfg->child_caps[nt_id])
				cfg->child_caps[nt_id] = len;

			if (sym.id == EOC)
				break;
		}
	}
}

void rdesc_cfg_destroy(struct rdesc_cfg *cfg)
{
	free(cfg->child_caps);
}
