/**
 * @file rdesc.h
 * @brief The right-recursive descent parser.
 */

#ifndef RDESC_H
#define RDESC_H

#include <stdint.h>
#include <stddef.h>

/** @brief major version */
#define RDESC_VERSION_MAJOR 0
/** @brief minor version */
#define RDESC_VERSION_MINOR 2
/** @brief patch version */
#define RDESC_VERSION_PATCH 0
/** @brief prerelease identifier */
#define RDESC_VERSION_PRE_RELEASE "alpha.0"


/** @brief Parsing status. */
enum rdesc_result {
	RDESC_READY /** A tree is ready for consumption. */,
	RDESC_CONTINUE /** New tokens should be provided. */,
	RDESC_NOMATCH /** Provided tokens do not match with any rule. */,
};

/** @brief The right-recursive descent parser. */
struct rdesc {
	/** @brief Context-free grammar production rules. */
	const struct rdesc_cfg *cfg;

	/** @brief Size in bytes allocated for each token's semantic
	 * information. */
	size_t seminfo_size;

	/** @brief Token stack used for backtracing and trying another start
	 * symbol. */
	struct rdesc_stack *token_stack;

	/** @brief Underlying concrete syntax tree, stored in a stack. */
	struct rdesc_stack *cst_stack;

	/** @brief (current) Index in CST that parsing continues on. */
	size_t cur;
};

/** @brief A node in the CST */
struct rdesc_node;


/** @brief Function pointer type for freeing tokens. */
typedef void (*rdesc_token_destroyer_func)(uint32_t id, void *seminfo);


#ifdef __cplusplus
extern "C" {
#endif

/** @brief Initializes a new parser. */
void rdesc_init(struct rdesc *parser,
		size_t seminfo_size,
		const struct rdesc_cfg *cfg);

/**
 * @brief Frees memory allocated by the parser and destroys the parser instance.
 *
 * @note `seminfo` field in `struct rdesc_token` is ignored, e.g. not freed,
 *        unless `token_destroyer` is set.
 */
void rdesc_destroy(struct rdesc *parser /*,
		   rdesc_token_destroyer_func token_destroyer*/);

/** @brief Sets start symbol for the next match. */
void rdesc_start(struct rdesc *parser, int start_symbol);

/**
 * @brief Resets parser and its state.
 *
 * @note `seminfo` field in `struct rdesc_token` is ignored, e.g. not freed,
 *        unless `token_destroyer` is set.
 */
void rdesc_reset(struct rdesc *parser /*,
		 rdesc_token_destroyer_func token_destroyer*/);

/**
 * @brief Drives the parsing process, the pump.
 *
 * As the central engine of the parser, it consumes tokens from either the
 * internal backtracking stack or the provided id.
 *
 * @param parser Pointer to the parser instance.
 * @param out Output pointer for the resulting CST node (IF READY).
 * @param id Identifier of the next token to consume.
 *        - **Zero is reserved** and used for resuming parsing using only the
 *          tokens currently in the backtracking stack. **Do not use zero as a
 *          token identifier in your grammar.**
 * @param seminfo Extra semantic information for the token.
 *        - The pointer is used solely to make the argument nullable
 *          (optional). It does not imply that the token must be heap-allocated.
 *          Passing a pointer to a stack-allocated (automatic) variable is
 *          valid and expected, as the parser copies the token data internally.
 *
 * @return The current status of the parse operation.
 */
enum rdesc_result rdesc_pump(struct rdesc *parser,
			     struct rdesc_node **out,
			     size_t id,
			     const void *seminfo);

/**
 * @brief Recursively destroys the node and its children.
 *
 * @note `seminfo` field in `struct rdesc_token` are not freed unless
 *        `token_destroyer` is set.
 */
void rdesc_node_destroy(struct rdesc_node *node,
			rdesc_token_destroyer_func token_destroyer);

#ifdef __cplusplus
}
#endif


#endif
