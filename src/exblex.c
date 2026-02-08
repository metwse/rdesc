#include "../include/rdesc.h"
#include "exblex.h"
#include "detail.h"

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void exblex_init(struct exblex *l,
		 const char *buf, const char *tokens, size_t token_count)
{
	*l = (struct exblex) {
		.buf = buf, .cur = 0,
		.tokens = tokens, .token_count = token_count,
		.peek = '\0',
	};
}

static uint32_t tokenid(const struct exblex *l, char tk)
{
	for (uint32_t i = 1; i < l->token_count; i++)
		if (l->tokens[i] == tk)
			return cast(uint32_t, i);

	return 0;
}

struct rdesc_token exblex_next(struct exblex *l)
{
	if (l->peek) {
		int id = tokenid(l, l->peek);
		l->peek = '\0';

		if (id)
			return (struct rdesc_token) { .id = id };
	}

	while (l->buf[l->cur] && isspace(l->buf[l->cur]))
		l->cur++;

	char c = l->buf[l->cur++];
	char *seminfo = NULL;
	size_t seminfo_len = 0;
	bool is_num = true;

	while (isalnum(c) || c == '_') {
		if (seminfo == NULL) {
			assert_mem(seminfo = malloc(sizeof(char) * 2));
			seminfo_len++;
		} else {
			assert_mem(seminfo = realloc(seminfo,
						     sizeof(char) * (++seminfo_len + 1)));
		}

		if (!isdigit(c))
			is_num = false;

		seminfo[seminfo_len - 1] = c;

		c = l->buf[l->cur++];
	}

	if (seminfo) {
		seminfo[seminfo_len] = '\0';

		l->peek = c;
		int id = 0;

		if (is_num)
			id = tokenid(l, 'd');
		else if (!isdigit(seminfo[0]))
			id = tokenid(l, 'w');

		if (id) {
			struct rdesc_token tk = { .id = id };
			memcpy(tk.seminfo, &seminfo, sizeof(char *));

			return tk;
		} else {
			if (seminfo_len > 1)
				c = '\0';
			else
				c = seminfo[0];

			free(seminfo);
		}
	}

	return (struct rdesc_token) { .id = tokenid(l, c) };
}
