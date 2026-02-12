#include "../include/cfg.h"
#include "../include/rdesc.h"

#include "../src/internal.h"

#include "../examples/grammar/bc.h"


int main(void)
{
	struct rdesc p;
	struct rdesc_cfg cfg;

	rdesc_cfg_init(&cfg,
		BC_NT_COUNT, BC_NT_VARIANT_COUNT, BC_NT_BODY_LENGTH,
		cast(struct rdesc_cfg_symbol *, bc));
	rdesc_init(&p, sizeof(uint32_t), &cfg);

	rdesc_start(&p, NT_STMT);
	new_nt_node(&p, 0, NT_EXPR);
	new_tk_node(&p, 0, TK_ENDSYM, "1");

	rdesc_destroy(&p);
	rdesc_cfg_destroy(&cfg);
}
