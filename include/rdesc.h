/**
 * @file rdesc.h
 * @brief The Right-Recursive Descent Parser
 */

#ifndef RDESC_H
#define RDESC_H

#include "cfg.h"

#include <stddef.h>

/** @brief major version */
#define RDESC_VERSION_MAJOR 0
/** @brief minor version */
#define RDESC_VERSION_MINOR 1
/** @brief patch version */
#define RDESC_VERSION_PATCH 0
/** @brief prerelase identifier */
#define RDESC_VERSION_PRE_RELEASE "rc.1"


/** @brief parsing status */
enum rdesc_result {
	RDESC_READY /** a tree is ready for consumption */,
	RDESC_CONTINUE /** new tokens should be provided */,
	RDESC_NOMATCH /** provided tokens do not match with any rule */,
};

#ifndef RDESC_STACK
/** @brief guard macro controlling external token stack implementation */
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
    struct rdesc_cfg_token *tokens /** pointer to the dynamic array buffer */;
    size_t len /** current number of tokens in the stack */;
    size_t cap /** allocated capacity of the buffer */;
};

#endif

/** @brief Right-recursive descent parser */
struct rdesc {
	/** context-free grammar production rules */
	const struct rdesc_cfg *cfg;

	/** The token stack for backtracking. When a parsing rule fails,
	 * consumed tokens are pushedrdesc back onto this stack */
	struct rdesc_stack stack;

	struct rdesc_node *root /** root of the tree */;
	struct rdesc_node *cur /** (current) node that parsing continues on */;
};

/** @brief A node in the CST */
struct rdesc_node {
	enum rdesc_cfg_symbol_type ty /** type of the symbol (token/nonterminal) */;

	union {
		struct rdesc_cfg_token tk /** token */;
		struct rdesc_cfg_nonterminal nt /** nonterminal */;
	};

	struct rdesc_node *parent /** parent node */;
};


/** @brief Initializes a new parser. */
void rdesc_init(struct rdesc *p,
		const struct rdesc_cfg *cfg);

/**
 * @brief Frees memory allocated by the parser and destroys the parser instance.
 *
 * @warning The token stack must be empty and no parse in progress before
 *          calling this function to prevent memory leaks. Raises an assertion
 *          error if the token stack is not empty or the root is not null.
 */
void rdesc_destroy(struct rdesc *p);

/** @brief Sets start symbol for the next match. */
void rdesc_start(struct rdesc *p, int start_symbol);

/**
 * @brief Resets parser and its state.
 *
 * @note `seminfo` field in `struct rdesc_cfg_token` are not freed unless
 *        `free_tk` is set.
 */
void rdesc_reset(struct rdesc *p, rdesc_tk_destroyer_func free_tk);

/**
 * @brief Drives the parsing process, The Pump.
 *
 * As the central engine of the parser, it consumes tokens from either the
 * internal backtracking stack or the provided `incoming_tk`.
 *
 * @param p Pointer to the parser instance.
 * @param out Output pointer for the resulting CST node (if READY).
 * @param incoming_tk Pointer to the next token to consume.
 *        - Optional: Pass `NULL` to resume parsing using only the tokens
 *          currently in the backtracking stack.
 *        - Storage: The pointer is used solely to make the argument nullable
 *          (optional). It does not imply that the token must be heap-allocated.
 *          Passing a pointer to a stack-allocated (automatic) variable is
 *          valid and expected, as the parser copies the token data internally.
 *
 * @return The current status of the parse operation.
 */
enum rdesc_result rdesc_pump(struct rdesc *p,
			     struct rdesc_node **out,
			     struct rdesc_cfg_token *incoming_tk);

/**
 * @brief Recursively destroys the node and its children.
 *
 * @note `seminfo` field in `struct rdesc_cfg_token` are not freed unless
 *        `free_tk` is set.
 */
void rdesc_node_destroy(struct rdesc_node *n,
			rdesc_tk_destroyer_func free_tk);


#endif
