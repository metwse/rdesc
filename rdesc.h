/**
 * @file rdesc.h
 * @brief The Right-Recursive Descent Parser
 */

#ifndef RDESC_H
#define RDESC_H

#include "bnf.h"

#include <stddef.h>


/** @brief parsing status */
enum rdesc_result {
	RDESC_READY /** a tree is ready for consumption */,
	RDESC_CONTINUE /** new tokens should be provided */,
	RDESC_NOMATCH /** provided tokens do not match with any rule */,
};

#ifndef RDESC_STACK
#define RDESC_STACK

/**
 * @brief Default implementation of the token backtracking stack.
 *
 * @note Portability / Custom Implementation:
 *       This definition is guarded by `#ifndef RDESC_STACK`. If you are
 *       porting `librdesc` to an environment that already has a preferred
 *       dynamic array or stack implementation (e.g., a project-specific
 *       vector type), you can define the `RDESC_STACK` macro externally. This
 *       allows you to suppress this default struct and provide your own
 *       definition of `struct rdesc_stack` compatible with your system.
 */
struct rdesc_stack {
    struct bnf_token *tokens /** pointer to the dynamic array buffer */;
    size_t len /** current number of tokens in the stack */;
    size_t cap /** allocated capacity of the buffer */;
};

#endif

/** @brief Right-recursive descent parser */
struct rdesc {
	/** context-free grammar production rules */
	const struct bnf_symbol *rules;
	/** total number of non-terminals */
	size_t nt_count;
	/** maximum number of variants, used for segmenting production rules
	 * array to an 3D array */
	size_t nt_variant_count;
	/** maximum number of symbols in a variant */
	size_t nt_body_length;
	/** maximum number of children of non-terminal variants */
	size_t *child_caps;

	/** The token stack for backtracking. When a parsing rule fails,
	 * consumed tokens are pushedrdesc back onto this stack */
	struct rdesc_stack tokens;

	struct rdesc_node *root /** root of the tree */;
	struct rdesc_node *cur /** (current) node that parsing continues on */;
};

/** @brief A node in the CST */
struct rdesc_node {
	enum bnf_symbol_type ty /** type of the symbol (token/nonterminal) */;

	union {
		struct bnf_token tk /** token */;
		struct bnf_nonterminal nt /** nonterminal */;
	};

	struct rdesc_node *parent /** parent node */;
};


/** @brief Initializes a new parser. */
void rdesc_init(struct rdesc *,
		size_t nonterminal_count,
		size_t nonterminal_variant_count,
		size_t nonterminal_body_length,
		const struct bnf_symbol *production_rules);

/**
 * @brief Frees memory allocated by the parser and destroys the parser instance.
 *
 * @warning The token stack must be empty and no parse in progress before
 *          calling this function to prevent memory leaks. Raises an assertion
 *          error if the token stack is not empty or the root is not null.
 */
void rdesc_destroy(struct rdesc *);

/** @brief Sets start symbol for the next match. */
void rdesc_start(struct rdesc *, int start_symbol);

/** @brief Clears tokens in the tokenstack and returns them. */
void rdesc_clearstack(struct rdesc *,
		      struct bnf_token **out,
		      size_t *out_len);

/** @brief Tries to match the next token */
enum rdesc_result rdesc_pump(struct rdesc *p,
			     struct rdesc_node **out,
			     struct bnf_token *incoming_tk);

/**
 * @brief Recursively destroys nodes and its children.
 *
 * @warning `seminfo` field in `struct bnf_token` is not freed, it is assumed
 * that the tokens used.
 */
void rdesc_destroy_node(struct rdesc_node *cst);


#endif
