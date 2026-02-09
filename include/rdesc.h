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

	/** @brief Opaque pointer to the backtracking stack.
	 *
	 * The implementation details of the stack are hidden. The parser uses
	 * the interface defined in `stack.h` to manipulate this. */
	struct rdesc_stack *stack;

	/** @brief Root of the tree. */
	struct rdesc_node *root;
	/** @brief (current) node that parsing continues on. */
	struct rdesc_node *cur;
};

/** @brief Nonterminal (syntatic variable) object for context-free grammar. */
struct rdesc_nonterminal {
	uint32_t _pad /** Padding for symbol type in node struct. */ : 1;
	uint32_t id /** @brief Nonterminal identifier. */ : 31;

	uint16_t child_count /** @brief Number of child nodes. */;
	/** @brief The production rule variant being parsed.
	 *
	 * After the parsing stage, you can use this to identify which variant
	 * of the construct rule is matched for this non-terminal. */
	uint16_t variant;

	struct rdesc_node **children /** @brief child nodes */;
};

/**
 * @brief Minimum seminfo array size for union compatibility in rdesc_node
 *
 * This value ensures that `struct rdesc_token` is large enough to be used
 * interchangeably with `struct rdesc_nonterminal` in the union within
 * `struct rdesc_node`.
 *
 * @see rdesc_init
 * @see struct rdesc_node
 */
#define RDESC_BASE_SEMINFO_SIZE ( \
		(sizeof(struct rdesc_nonterminal) - sizeof(uint32_t)) \
		+ sizeof(char) - 1 \
	) / sizeof(char)

/** @brief terminal (token) object for context-free grammar */
struct rdesc_token {
	uint32_t _pad /** @brief padding for symbol type in node struct */ : 1;
	uint32_t id /** @brief token identifier */ : 31 ;

	/** @brief additional semantic information, as flexible array member */
	char seminfo[RDESC_BASE_SEMINFO_SIZE];
};

/**
 * @brief A node in the CST
 *
 * Uses a union to allow a single node type to represent both (tokens and
 * non-terminals. The `ty` field in the union determines which interpretation
 * to use (0 = token, 1 = nonterminal).
 *
 * The flexible array member `_[]` allows extending seminfo storage beyond
 * the base token size when seminfo_size > RDESC_BASE_SEMINFO_SIZE.
 *
 * @see rdesc_token
 * @see rdesc_nonterminal
 */
#pragma pack(push, 1)
struct rdesc_node {
	struct rdesc_node *parent /** @brief parent node */;

	union {
		/** @brief type of the symbol (token = 0 / nonterminal = 1) */
		uint32_t ty : 1;

		struct rdesc_token tk /** @brief token */;
		struct rdesc_nonterminal nt /** @brief nonterminal */;
	} n /** 'n' is a dummy union name, as anonymous unions are not supported
	      * in C99 */;

	/** Continuation of token seminfo when seminfo_size >
	 * RDESC_MIN_SEMINFO_SIZE */
	char _unsized_marker[];
};
#pragma pack(pop)


/** @brief Function pointer type for freeing tokens. */
typedef void (*rdesc_token_destroyer_func)(struct rdesc_token *);


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
void rdesc_destroy(struct rdesc *parser,
		   rdesc_token_destroyer_func token_destroyer);

/** @brief Sets start symbol for the next match. */
void rdesc_start(struct rdesc *parser, int start_symbol);

/**
 * @brief Resets parser and its state.
 *
 * @note `seminfo` field in `struct rdesc_token` is ignored, e.g. not freed,
 *        unless `token_destroyer` is set.
 */
void rdesc_reset(struct rdesc *parser,
		 rdesc_token_destroyer_func token_destroyer);

/**
 * @brief Drives the parsing process, The Pump.
 *
 * As the central engine of the parser, it consumes tokens from either the
 * internal backtracking stack or the provided `incoming_tk`.
 *
 * @param parser Pointer to the parser instance.
 * @param out Output pointer for the resulting CST node (IF READY).
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
enum rdesc_result rdesc_pump(struct rdesc *parser,
			     struct rdesc_node **out,
			     struct rdesc_token *incoming_tk);

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
