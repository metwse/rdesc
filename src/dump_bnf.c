#include "../include/cfg.h"
#include "../include/bnf_dsl.h"
#include "../include/util.h"
#include "detail.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>


static void print_rule(const struct rdesc_cfg *cfg,
		       const struct rdesc_cfg_symbol *rule,
		       const char *const nt_names[],
		       const char *const tk_names[],
		       FILE *out)
{
	for (uint16_t i = 0; i < cfg->nt_body_length; i++) {
		if (rule[i].id == EOB) {
			if (i == 0)
				putc('E', out);

			break;
		}

		if (i)
			putc(' ', out);

		const char *name = (
			rule[i].ty == CFG_TOKEN ? tk_names : nt_names
		)[rule[i].id];
		const char *fstr = rule[i].ty == CFG_TOKEN ? (
			name[0] == '@' ? "%s" : "\"%s\""
		) : "<%s>";

		if (name[0] == '@' && rule[i].ty == CFG_TOKEN)
			name++;

		fprintf(out, fstr, name);
	}
}

void rdesc_dump_bnf(const struct rdesc_cfg *cfg,
		    const char *const tk_names[],
		    const char *const nt_names[],
		    FILE *out)
{
	for (uint32_t nt_id = 0 /* head of the rule*/;
	     nt_id < cfg->nt_count; nt_id++) {
		if (nt_id != 0)
			fputc('\n', out);

		fprintf(out, "<%s> ::= ", nt_names[nt_id]);
		int padding = strlen(nt_names[nt_id]);

		for (int variant_id = 0;
		     productions(*cfg)[nt_id][variant_id][0].id != EOC;
		     variant_id++) {
			if (variant_id != 0)
				printf("\n %*s    | ", padding, "");

			print_rule(cfg, productions(*cfg)[nt_id][variant_id],
				   nt_names, tk_names, out);
		}

		putc('\n', out);
		if (nt_id != cfg->nt_count - 1)
			putc('\n', out);
	}
}
