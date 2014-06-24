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
	/* Check the minimum length */
	if (length < 3)
		return 1;

	/* Allocate the table */
	size_t size = sizeof(struct guess);
	c->table = (struct guess *)malloc(size * c->size);
	if (c->table == NULL)
		return 1;

	/* Populate the table */
	for (int i = 0; i < c->number; i++) {
		for (int j = 0; j < c->number; j++) {
			int offset = (i * c->number) + j;
			/* Populate the first and second character and length */
			c->table[offset].string[0] = c->characters[i];
			c->table[offset].string[1] = c->characters[j];
			c->table[offset].length = length;

			/* Populate the body of the string */
			for (int k = 2; k < length; k++) {
				c->table[offset].string[k] = c->characters[0];
				c->positions[k] = &c->characters[0];
			}

			/* Populate the rest of it with 0's */
			for (int k = length; k < STRING_LENGTH; k++) {
				c->table[offset].string[k] = 0;
				c->positions[k] = &c->characters[0];
			}
		}
	}

	c->table_index = 0;

	return 0;
}

/* Update a table */
static inline void charset_rebuild_table(struct charset *c)
{
	/* Cache the length */
	int length = c->table[0].length;

	/* Reset the table index */
	c->table_index = 0;

	/* Find the position to start updating */
	for (int i = length - 1; i >= 2; i--) {
		/* Calculate loop start and end points */
		char *start = &c->table[0].string[i];
		char *end = &c->table[c->size].string[i];

		/* We found the correct place */
		if (c->table[0].string[i] != c->characters[c->number - 1]) {
			/* Find the string position */
			char *curr = c->positions[i];

			/* Update the table */
			for (char *s = start; s < end; s += sizeof(struct guess))
				*s = curr[1];

			/* Update the position marker */
			c->positions[i] = &curr[1];
			return;
		} else {
			/* Reset trailing character and continue */
			for (char *s = start; s < end; s += sizeof(struct guess))
				*s = c->characters[0];

			/* Reset the position marker */
			c->positions[i] = &c->characters[0];
		}
	}

	/* Move to a longer string */
	for (int i = 0; i < c->size; i++) {
		c->table[i].string[length] = c->characters[0];
		c->table[i].length = length + 1;
	}
}

/* Return the next n guesses from a character set */
struct guess *charset_next_block(struct charset *c, struct guess *s, int count)
{
	/* Counter for remaining requests to fulfill */
	int remaining = count;
	size_t size = sizeof(struct guess);

	/* Stash buffer pointer */
	struct guess *g = s;

	/* Lock the character set */
	pthread_mutex_lock(&c->lock);

	/* Copy from the table, rebuilding as needed */
	while (remaining > 0) {
		int table_left = c->size - c->table_index;

		/* If we need less than the table has, copy what we need */
		if (table_left > remaining) {
			/* Copy entries */
			memcpy(g, &c->table[c->table_index], size * remaining);
			/* Update table index */
			c->table_index += remaining;
			/* End the loop */
			remaining = 0;
		} else { /* We need more than the table has */
			memcpy(g, &c->table[c->table_index], size * table_left);
			/* Update pointer */
			g = &g[table_left];
			/* Update remaining needed */
			remaining -= table_left;
			/* Rebuild the table */
			charset_rebuild_table(c);
		}
	}

	/* Update the character set and return the old one */
	pthread_mutex_unlock(&c->lock);

	return s;
}

/* Destroy the character set */
void charset_destroy(struct charset *c)
{
	free(c->table);
	pthread_mutex_destroy(&c->lock);
}
