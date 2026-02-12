#include "../include/cfg.h"
#include "../include/cst_macros.h"
#include "../include/util.h"
#include "../include/rdesc.h"

#include "../src/detail.h"
#include "../src/rdi.h"

#include "../examples/grammar/boolean_algebra.h"

#include <stddef.h>


void balg_node_printer(const struct rdesc_node *node, FILE *out)
{
	if (rtype(node) == CFG_TOKEN)
		fprintf(out, "%s", balg_tk_names[rid(node)]);
	else
		fprintf(out, "<%s>", balg_nt_names[rid(node)]);
}

int main(void)
{
	struct rdesc p;
	struct rdesc_cfg cfg;

	rdesc_cfg_init(&cfg,
		BALG_NT_COUNT, BALG_NT_VARIANT_COUNT, BALG_NT_BODY_LENGTH,
		cast(struct rdesc_cfg_symbol *, balg));
	rdesc_init(&p, sizeof(uint32_t), &cfg);

	rdesc_start(&p, NT_STMT);
	size_t nt_call_idx = new_nt_node(&p, 0, NT_CALL);

	 new_tk_node(&p, nt_call_idx, TK_IDENT, "1");
	 new_tk_node(&p, nt_call_idx, TK_LPAREN, NULL);

	 size_t nt_calloptparams_idx = new_nt_node(&p, nt_call_idx, NT_CALL_OPTPARAMS);

	  size_t nt_expr_ls_idx = new_nt_node(&p, nt_calloptparams_idx, NT_EXPR_LS);

	   size_t nt_expr_idx = new_nt_node(&p, nt_expr_ls_idx, NT_EXPR);

	    size_t nt_term_idx = new_nt_node(&p, nt_expr_idx, NT_TERM);

	     size_t nt_factor_idx = new_nt_node(&p, nt_term_idx, NT_FACTOR);

	      size_t nt_atom_idx = new_nt_node(&p, nt_factor_idx, NT_ATOM);

	       size_t nt_bit_idx = new_nt_node(&p, nt_atom_idx, NT_BIT);

	         new_tk_node(&p, nt_bit_idx, TK_TRUE, NULL);

	     new_nt_node(&p, nt_term_idx, NT_TERM_REST);
	    new_nt_node(&p, nt_expr_idx, NT_EXPR_REST);
	   new_nt_node(&p, nt_expr_ls_idx, NT_EXPR_LS_REST);

	 new_tk_node(&p, nt_call_idx, TK_RPAREN, NULL);
	new_tk_node(&p, 0, TK_SEMI, NULL);

	rdesc_dump_cst(stdout, &p, balg_node_printer);

	rdesc_destroy(&p);
	rdesc_cfg_destroy(&cfg);
}
