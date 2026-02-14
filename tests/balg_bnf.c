/* Dump boolean algebra grammar to test dump_bnf feature. */

#include "../include/cfg.h"
#include "../include/util.h"

#include "../examples/grammar/boolean_algebra.h"


int main(void)
{
	struct rdesc_cfg cfg;

	rdesc_cfg_init(&cfg, BALG_NT_COUNT, BALG_NT_VARIANT_COUNT,
		       BALG_NT_BODY_LENGTH, (struct rdesc_cfg_symbol *) balg);

	rdesc_dump_bnf(stdout, &cfg, balg_tk_names, balg_nt_names);

	rdesc_cfg_destroy(&cfg);
}
