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

#ifndef CHARSET_H
#define CHARSET_H

#include <string.h>
#include "common.h"
#include "lock.h"

#define STRING_LENGTH 16

/* A 16-character string and associated data */
struct guess {
	/* The string */
	char string[STRING_LENGTH];
	/* The length of the string */
	unsigned char length;
};

/* A basic initializer for it */
#define GUESS_INITIALIZER { '\0', 0 }

/* A character set */
struct charset {
	/* Character in this character set */
	char *characters;
	/* Number of characters in this character set */
	int number;
	/* The number of strings in the table */
	int size;
	/* Lock for the most recent string */
	lock_t lock;
	/* The latest strings */
	struct guess *table;
	/* Our current position in the table */
	int table_index;
	/* Positions */
	char *positions[STRING_LENGTH];
	/* Initialize the struct */
	int (*init) (int length);
	/* Return the next guess from this charset */
	struct guess* (*next) (struct guess *s);
	/* Return the next guess from this charset */
	struct guess* (*next_block) (struct guess *s, int count);
	/* Destroy this character set */
	void (*destroy) (void);
};

/* Generic methods */
int charset_init(struct charset *c, int length);
struct guess *charset_next_block(struct charset *c, struct guess *s, int count);
void charset_destroy(struct charset *c);

#endif
