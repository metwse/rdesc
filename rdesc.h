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
	RDESC_PARSER_READY /** a tree is ready for consumption */,
	RDESC_PARSER_CONTINUE /** new tokens should be provided */,
	RDESC_PARSER_NOMATCH /** provided tokens do not match with any rule */,
};

/** @brief Right-recursive descent parser configuration */
struct rdesc_parser {
	/** context-free grammar production rules */
	struct bnf_symbol **rules;
	/** total number of nonterminals */
	size_t nt_count;
	/** maximum number of variants, used for segmenting production rules */
	size_t max_variant_count;

	/** The token stack for backtracking. When a parsing rule fails,
	 * consumed tokens are pushedrdesc back onto this stack */
	struct rdesc_token_stack *tokens;

	struct rdesc_cst *root /** root of the tree */;
	struct rdesc_cst *cur /** (current) node that parsing continues on */;

};

/** @brief A node in the CST */
struct rdesc_cst {
	enum bnf_symbol_type ty /** type of the symbol (token/nonterminal) */;

	union {
		struct bnf_token tk /** token */;
		struct bnf_nonterminal nt /** nonterminal */;
	};

	struct rdesc_cst *parent /** parent node */;
};


/** @brief Initializes a new parser. */
void rdesc_init(struct rdesc_parser *, struct bnf_symbol **production_rules);

/** @brief Sets start symbol for the next match. */
void rdesc_start(struct rdesc_parser *, int start_symbol);

/** @brief Clears tokens in the tokenstack and returns them. */
void rdesc_clearstack(struct rdesc_parser *,
		      struct bnf_token **out,
		      size_t *out_len);

/**
 * @brief Continues the parsing process.
 *
 * This function acts like a coroutine, consumes tokens until it either
 * completes the start symbol set by `rdesc_start` or requires more tokens
 * from the lexer.
 *
 * @param p Pointer to the parser.
 * @param out If the result is `RDESC_PARSER_READY`, this pointer is set to the
 *            root of the completed CST (`struct rdesc_cst *`).
 * @param token Identifier of next token.
 * @returns The status of the parse operation.
 */
enum rdesc_result rdesc_continue(struct rdesc_parser *p,
				 struct rdesc_cst **out,
				 struct bnf_token token);

/**
 * @brief Recursively destroys nodes and its children.
 *
 * @warning `seminfo` field in `struct bnf_token` is not freed, it is assumed
 * that the tokens used.
 */
void rdesc_destroy_cst(struct rdesc_cst **cst);


#endif
