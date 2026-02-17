/**
 * @file rdesc.h
 * @brief The recursive descent parser.
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
#define RDESC_VERSION_PRE_RELEASE "alpha+api-review"


/** @brief Pump status. */
enum rdesc_result {
	/** Memory allocation failed. */
	RDESC_ENOMEM = -1,
	/** A CST is ready for consumption. */
	RDESC_READY = 0,
	/** New tokens should be provided. */
	RDESC_CONTINUE = 1,
	/** Provided tokens do not match with any rule. */
	RDESC_NOMATCH = 2,
};

/** @brief The recursive descent parser. */
struct rdesc {
	/** @cond */

	/* Context-free grammar production rules. */
	const struct rdesc_cfg *cfg;

	/* Size in bytes allocated for each token's semantic information. */
	size_t seminfo_size;

	/* - Error Recovery -
	 *
	 * Extra space for holding a token in case of memory allocation error.
	 * Token will be copied to those fields for retry in next pump call.
	 */
	uint16_t saved_tk;
	void *saved_seminfo;

	/* - Navigation - */
	size_t cur  /* (currrent) Nonterminal being expaned, and might not be
		     * the top element. */;
	uint16_t top_unwind  /* Stack's top node's unwind distance. */;

	/* Destructor method for tokens the parser owns. */
	void (*token_destroyer)(uint16_t, void *);

	/* Token stack used to store tokens temporarily during nonterminal
	 * backtracing. */
	struct rdesc_stack *token_stack;

	/* Underlying concrete syntax tree. */
	struct rdesc_stack *cst_stack;

	/** @endcond */
};

/** @brief A node in the CST */
struct rdesc_node;


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes a new parser.
 *
 * @return Non-zero value if memory allocation is failed.
 */
int rdesc_init(struct rdesc *parser,
	       const struct rdesc_cfg *cfg,
	       size_t seminfo_size,
	       void (*token_destroyer)(uint16_t id, void *seminfo));

/**
 * @brief Frees memory allocated by the parser and destroys the parser instance.
 *
 * @note `seminfo` field in `struct rdesc_token` is ignored, i.e., not freed,
 *        unless `token_destroyer` is set.
 */
void rdesc_destroy(struct rdesc *parser);

/**
 * @brief Sets start symbol for the next match.
 *
 * @return Non-zero if memory allocation is failed.
 */
int rdesc_start(struct rdesc *parser, int start_symbol);

/**
 * @brief Resets parser and its state.
 *
 * @note `seminfo` field in `struct rdesc_token` is ignored, i.e., not freed,
 *        unless `token_destroyer` is set.
 *
 * @return Non-zero if memory allocation is failed.
 */
int rdesc_reset(struct rdesc *parser);

/**
 * @brief Drives the parsing process, the pump.
 *
 * As the central engine of the parser, it consumes tokens from either the
 * internal backtracking stack or the provided id.
 *
 * @param parser Pointer to the parser instance.
 * @param id Identifier of the next token to consume.
 *        - **ID 0 is reserved** for resuming from backtrack stack (for start
 *          symbol change or in retries due to memory allocation errors or )
 * @param seminfo Extra semantic information for the token.
 *        - Semantic information pointer. The parser copies this data
 *          internally, so passing a pointer to stack-allocated data is valid.
 *          NULL is acceptable.
 *
 * @return The current status of the parse operation.
 *
 * @warning Raises an error if parser is not started.
 */
enum rdesc_result rdesc_pump(struct rdesc *parser, uint16_t id, void *seminfo);

/**
 * @brief Resume parsing without providing a new token.
 *
 * Continues parsing using either:
 * - The saved token from a previous ENOMEM error, or
 * - A token from the backtrack stack
 *
 * This is equivalent to `rdesc_pump(parser, 0, NULL)`.
 */
static inline enum rdesc_result rdesc_resume(struct rdesc *parser)
{ return rdesc_pump(parser, 0, NULL); }

/**
 * @brief Returns the root of CST.
 *
 * @note Returns NULL if parser does not contain any tree.
 */
struct rdesc_node *rdesc_root(struct rdesc *parser);

/** @cond */
struct rdesc_node *_rdesc_priv_cst_illegal_access(const struct rdesc *parser,
						  size_t index);
/** @endcond */

#ifdef __cplusplus
}
#endif

/** @cond */
/* These structs are private that should only be used with the provided CST
 * macros. */

#pragma pack(push, 1)

struct _rdesc_priv_tk {
	uint16_t _pad : 1;
	uint16_t id : 15  /* Token identifier (0 reserved, 1-32767 valid). */;

	uint32_t seminfo  /* Semantic info starts here and extends into
			   * the flexible array member in _rdesc_priv_node. */;
};

struct _rdesc_priv_nt {
	uint16_t _pad : 1;
	uint16_t id : 15  /* 0 is NOT reserved unlike token ids. */;

	uint16_t variant;
	uint16_t child_count;
};

struct _rdesc_priv_node {
	/* ALSO CHANGE sizeof_node macro for any change in this struct. */
	size_t parent  /* Index of parent. */;
	uint16_t unwind_size  /* Previous node's unwind size (for backwards
			       * navigation on stack). */;

	union {
		uint16_t ty : 1  /* 0 for token and 1 for nonterminal. */;

		struct _rdesc_priv_tk tk;
		struct _rdesc_priv_nt nt;
	} n;

	uint8_t _[];
};

#pragma pack(pop)
/** @endcond */


#endif
