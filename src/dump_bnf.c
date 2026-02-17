#include "../include/bnf_macros.h"
#include "../include/grammar.h"
#include "../include/util.h"
#include "detail.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>


static void print_rule(const struct rdesc_grammar *grammar,
		       const struct rdesc_grammar_symbol *rule,
		       const char *const nt_names[],
		       const char *const tk_names[],
		       FILE *out)
{
	for (uint16_t i = 0; i < grammar->nt_body_length; i++) {
		if (rule[i].id == EOB) {
			if (i == 0)
				putc('E', out);

			break;
		}

		if (i)
			putc(' ', out);

		const char *name = (
			rule[i].ty == RDESC_TOKEN ? tk_names : nt_names
		)[rule[i].id];
		const char *fstr = rule[i].ty == RDESC_TOKEN ? (
			name[0] == '@' ? "%s" : "\"%s\""
		) : "<%s>";

		if (name[0] == '@' && rule[i].ty == RDESC_TOKEN)
			name++;

		fprintf(out, fstr, name);
	}
}

void rdesc_dump_bnf(FILE *out,
		    const struct rdesc_grammar *grammar,
		    const char *const tk_names[],
		    const char *const nt_names[])
{
	for (uint32_t nt_id = 0 /* head of the rule*/;
	     nt_id < grammar->nt_count; nt_id++) {
		if (nt_id != 0)
			fputc('\n', out);

		fprintf(out, "<%s> ::= ", nt_names[nt_id]);
		int padding = strlen(nt_names[nt_id]);

		for (int variant_id = 0;
		     productions(*grammar)[nt_id][variant_id][0].id != EOC;
		     variant_id++) {
			if (variant_id != 0)
				printf("\n %*s    | ", padding, "");

			print_rule(grammar, productions(*grammar)[nt_id][variant_id],
				   nt_names, tk_names, out);
		}

		putc('\n', out);
		if (nt_id != grammar->nt_count - 1)
			putc('\n', out);
	}
}
