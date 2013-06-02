/******************************************************************************
 * Copyright (C) 2013 Matthew Dellinger <matthew@mdelling.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *****************************************************************************/

#include "charset.h"

/* Initialize the character set */
int charset_init(struct charset *c, int length)
{
	/* Populate the next string */
	for (int i = 0; i < length; i++)
		c->current.string[i] = c->characters[0];
	for (int i = length; i < 16; i++)
		c->current.string[i] = 0;

	c->current.length = length;

	return 0;
}

/* Increment a guess according to an associated charset */
static void _charset_next(struct guess *g, struct charset *c)
{
	char last = c->characters[c->number - 1];
	for (int i = g->length - 1; i >= 0; i--) {
		if (g->string[i] < last) {
			char *curr = strchr(c->characters, g->string[i]);
			if (curr == NULL)
				continue;

			g->string[i] = curr[1];
			return;
		} else
			g->string[i] = c->characters[0];
	}

	g->string[g->length] = c->characters[0];
	g->length++;
}

/* Return the next guess from a character set */
struct guess *charset_next(struct charset *c)
{
	/* Allocate the first guess */
	size_t size = sizeof(struct guess);
	struct guess *g = (struct guess *)malloc(size);
	if (g == NULL)
		return NULL;

	/* Lock the character set */
	pthread_mutex_lock(&c->lock);

	/* Copy the last one and update it*/
	memcpy(g, &c->current, size);
	_charset_next(&c->current, c);

	/* Update the character set and return the old one */
	pthread_mutex_unlock(&c->lock);

	return g;
}

/* Return the next n guesses from a character set */
struct guess *charset_next_block(struct charset *c, int count)
{
	/* Allocate the first guess */
	size_t size = sizeof(struct guess) * count;
	struct guess *g = (struct guess *)malloc(size);
	struct guess *orig = g;
	if (g == NULL)
		return NULL;

	/* Lock the character set */
	pthread_mutex_lock(&c->lock);

	/* Copy the last one and update it*/
	for (int i = 0; i < count; i++) {
		memcpy(g, &c->current, sizeof(struct guess));
		_charset_next(&c->current, c);
		g++;
	}

	/* Update the character set and return the old one */
	pthread_mutex_unlock(&c->lock);

	return orig;
}

/* Destroy the character set */
void charset_destroy(struct charset *c)
{
	pthread_mutex_destroy(&c->lock);
}
