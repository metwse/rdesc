/**
 * @file exblex.h
 * @brief Basic lexer, used for testing.
 */

#ifndef EXBLEX_H
#define EXBLEX_H

#include <stddef.h>


/**
 * @brief EXtremely Basic LEXer
 *
 * A lightweight lexer designed for internal tests. It ignores whitespace and
 * tokenizes the input based on three categories:
 *
 * 1. Words (`w`): Matches identifiers consisting of alphanumeric characters
 *    and underscores (`[a-zA-Z0-9_]`).
 * 2. Digits (`d`): Matches integer literals consisting of numeric digits
 *    (`[0-9]`).
 * 3. Punctuation: Matches single-character symbols provided in the lookup
 *    table (e.g., `+`, `*`, `;`).
 *
 * @note The lexer gives precedence to character classes (`w`, `d`). If a
 *       character does not fit into these classes, it is checked against the
 *       `tokens` table.
 */
struct exblex {
	const char *buf /** underlying null-terminated input buffer */;
	size_t cur /** current position in the buffer */;
	const char *tokens /** array of valid token characters */;
	size_t token_count /** number of tokens in the array */;
	/** A character that was part of the input stream but not
	consumed by the previous token. (e.g., the `+` in `abc+def`). */
	char peek;
};


/** @brief Initializes the basic lexer. */
void exblex_init(struct exblex *l,
		 const char *buf,
		 const char *tokens,
		 size_t token_count);

/**
 * @brief Fetches the next token.
 *
 * Skips whitespace. 0 is reserved for NO_TOKEN/EOF.
 */
struct rdesc_token exblex_next(struct exblex *l);


#endif
