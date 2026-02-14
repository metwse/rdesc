/**
 * @file exblex.h
 * @brief Basic lexer, used for testing.
 */

#ifndef EXBLEX_H
#define EXBLEX_H

#include <stddef.h>
#include <stdint.h>


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
 * 3. One-Character Punctuation: Matches single-character symbols provided in
 *    the lookup table (e.g., `+`, `*`, `;`).
 *
 * @note The lexer gives precedence to character classes (`w`, `d`). If a
 *       character does not fit into these classes, it is checked against the
 *       `tokens` table.
 *
 * @note First character of `tokens` will be ignored for matching, as
 *       identifier 0 is reserved in `rdesc`.
 */
struct exblex {
	/** @brief Underlying null-terminated input buffer. */
	const char *buf;

	/** @brief Array of token characters. */
	const char *tokens;

	/** @brief Number of tokens in provided array. */
	size_t token_count;

	/* --- Fields from now on should be zero-intialized. --- */

	/** @brief (current) Position in the buffer. */
	size_t cur /* (current) Index lexing continues on. */;

	/** @cond */
	char pushback_char /* A character that was part of the input stream but
			    * not consumed by the previous token (e.g., the `+`
			    * in `abc+def`). */;

	char *current_seminfo /* Holds semantic information pointer of the last
			       * token and returns it in
			       * `exblex_current_seminfo`. */;
	/** @endcond */
};


/**
 * @brief Initializes the basic lexer with null-terminated list of chars.
 *
 * @see `struct exblex` for details.
 */
void exblex_init(struct exblex *l,
		 const char *buf,
		 const char *tokens);

/**
 * @brief Fetches the next token.
 *
 * @return Token ID:
 *         - 0 for EOF/end of input
 *         - Index into tokens[] array for the matched character or class
 *
 * @note For 'w' (word) and 'd' (digit) tokens, retrieve the matched text
 *       using @ref exblex_current_seminfo.
 */
uint16_t exblex_next(struct exblex *l);

/**
 * @brief Retrieves the semantic information for the last token.
 *
 * @return Pointer to token text (for 'w' and 'd' tokens), or NULL for
 *         punctuation tokens.
 *
 * @note Caller takes ownership of returned pointer, and must `free()` after
 *       use.
 */
char *exblex_current_seminfo(struct exblex *l);


#endif
