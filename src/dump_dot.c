#include "../include/rdesc.h"
#include "../include/cfg.h"
#include "../include/util.h"

#include <stddef.h>
#include <stdio.h>


static void dump_graph_recursive(const struct rdesc_node *n,
				 size_t parent_id,
				 size_t *id_counter,
				 rdesc_tk_printer_func tk_printer,
				 const char *const *nt_names,
				 FILE *out)
{
	size_t this;

	if (!parent_id) {
		this = 1;
	} else {
		this = ++(*id_counter);
		fprintf(out, "\t%zu -> %zu;\n", parent_id, this);
	}

	if (n->ty == CFG_TOKEN) {
		fprintf(out, "\t%zu [shape=record,label=\"", this);
		tk_printer(&n->tk, out);
		fprintf(out, "\"];\n");
	} else {
		fprintf(out, "\t%zu [label=\"<%s>\"];\n", this, nt_names[n->nt.id]);

		for (size_t i = 0; i < n->nt.child_count; i++)
			dump_graph_recursive(n->nt.children[i], this, id_counter,
					     tk_printer, nt_names, out);

		if (!n->nt.child_count) {
			size_t epsilon_child = ++(*id_counter);
			printf("\t%zu [shape=record,label=\"E\"];\n"
			       "\t%zu -> %zu;\n",
			       epsilon_child, this, epsilon_child);
		}
	}
}

void rdesc_dump_dot(const struct rdesc_node *n,
		    rdesc_tk_printer_func tk_printer,
		    const char *const *nt_names,
		    FILE *out)
{
	size_t id_counter = 1;
	fprintf(out, "digraph G {\n");
	dump_graph_recursive(n, 0, &id_counter, tk_printer, nt_names, out);
	fprintf(out, "}\n");
}
