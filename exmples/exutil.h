/**
 * @file exutil.h
 * @brief Basic utilities for examples.
 *
 * Contains a lightweight, whitespace-skipping lexer designed for
 * simple single-character token grammars (like Boolean Algebra).
 */

#ifndef EXUTL_H
#define EXUTL_H

#include <stddef.h>
#include <ctype.h>

/**
 * @brief EXtremely Basic LEXer
 *
 * Ignores any whitespace and matches character-wise lexemes against a
 * provided lookup table.
 */
struct exblex {
	const char *buf /** underlying null-terminated input buffer */;
	size_t cur /** current position in the buffer */;
	const char *tokens /** array of valid token characters */;
	size_t token_count /** number of tokens in the array */;
};

/** @brief Initializes the basic lexer. */
static inline void exblex_init(struct exblex *l,
			       const char *buf,
			       const char *tokens,
			       size_t token_count)
{
	*l = (struct exblex) {
		.buf = buf, .cur = 0,
		.tokens = tokens, .token_count = token_count,
	};
}

/**
 * @brief Fetches the next token ID.
 * Skips whitespace. If a character matches one in the `tokens` array, returns
 * its index, 0 is reserved for NO_TOKEN/EOF.
 *
 * @param l Pointer to the lexer instance.
 *
 * @return int The token ID index, or 0 if EOF/Unknown.
 */
static inline int exblex_next(struct exblex *l)
{
	while (l->buf[l->cur] && isspace(l->buf[l->cur]))
		l->cur++;

	char c = l->buf[l->cur];

	if (c == '\0')
		return 0;


	for (size_t i = 1; i < l->token_count; i++)
		if (l->tokens[i] == c) {
			l->cur++;
			return (int) i;
		}

	return 0;
}

#endif
