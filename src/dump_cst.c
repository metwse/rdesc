#include "../include/rdesc.h"
#include "../include/cst_macros.h"
#include "../include/grammar.h"
#include "../include/util.h"
#include "../include/stack.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>


static void dump_graph_recursive(const struct rdesc *p,
				 struct rdesc_node *n,
				 size_t parent_id,
				 size_t *id_counter,
				 void (*node_printer)(const struct rdesc_node *, FILE *),
				 FILE *out)
{
	size_t this;

	if (!parent_id) {
		this = 1;
	} else {
		this = ++(*id_counter);
		fprintf(out, "\t%zu -> %zu;\n", parent_id, this);
	}

	fprintf(out, "\t%zu ", this);
	node_printer(n, out);
	fprintf(out, ";\n");

	if (rtype(n) == RDESC_NONTERMINAL) {
		for (uint16_t i = 0; i < rchild_count(n); i++)
			dump_graph_recursive(p, rchild(p, n, i), this,
					     id_counter, node_printer, out);

		if (!rchild_count(n)) {
			size_t epsilon_child = ++(*id_counter);
			fprintf(out, "\t%zu [shape=record,label=\"ε\"];\n"
			        "\t%zu -> %zu;\n",
			        epsilon_child, this, epsilon_child);
		}
	}
}

void rdesc_dump_cst(FILE *out,
		    const struct rdesc *p,
		    void (*node_printer)(const struct rdesc_node *, FILE *))
{
	size_t id_counter = 1;
	fprintf(out, "digraph G {\n");
	if (rdesc_stack_len(p->cst_stack) != 0)
		dump_graph_recursive(
			p, rdesc_stack_at(p->cst_stack, 0), 0,
			&id_counter, node_printer, out);
	fprintf(out, "}\n");
}
