#include "../../src/detail.h"
#include "exblex.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>



void exblex_init(struct exblex *l,
		 const char *buf,
		 const char *tokens)
{
	*l = (struct exblex) {
		.buf = buf,
		.tokens = tokens,
		.token_count = strlen(tokens + 1),
		.cur = 0, .current_seminfo = NULL,
	};
}

static uint16_t tokenid(const struct exblex *l, char tk)
{
	for (uint16_t i = 1; i <= l->token_count; i++)
		if (l->tokens[i] == tk)
			return cast(uint16_t, i);

	return '\0';
}

uint16_t exblex_next(struct exblex *l)
{
	while (l->buf[l->cur] && isspace(l->buf[l->cur]))
		l->cur++;

	char c = l->buf[l->cur++];
	char *seminfo = NULL;
	size_t seminfo_len = 0;

	bool is_num = true;
	while (isalnum(c) || c == '_') {
		if (seminfo == NULL) {
			rdesc_assert(seminfo = malloc(sizeof(char) * 2),
				     "malloc failed");
			seminfo_len++;
		} else {
			rdesc_assert(seminfo = realloc(seminfo, sizeof(char) * (++seminfo_len + 1)),
				     "realloc failed");
		}

		if (!isdigit(c))
			is_num = false;

		seminfo[seminfo_len - 1] = c;

		c = l->buf[l->cur++];
	}

	if (seminfo) {
		seminfo[seminfo_len] = '\0';

		l->cur--;
		int id = 0;

		if (is_num)
			id = tokenid(l, 'd');
		else if (!isdigit(seminfo[0]))
			id = tokenid(l, 'w');

		if (id) {
			l->current_seminfo = seminfo;

			return id;
		} else {
			if (seminfo_len > 1)
				c = '\0';
			else
				c = seminfo[0];

			free(seminfo);
		}
	}

	return tokenid(l, c);
}

char *exblex_current_seminfo(struct exblex *l)
{
	return l->current_seminfo;
}
