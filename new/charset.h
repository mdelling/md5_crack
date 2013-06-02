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

#include <pthread.h>
#include <string.h>
#include "common.h"

/* A 16-character string and associated data */
struct guess {
	/* The string */
	char string[16];
	/* The length of the string */
	int length;
};

/* A basic initializer for it */
#define GUESS_INITIALIZER { '\0', 0 }

/* A character set */
struct charset {
	/* Character in this character set */
	char *characters;
	/* Number of characters in this character set */
	int number;
	/* Lock for the most recent string */
	pthread_mutex_t lock;
	/* The latest string */
	struct guess current;
	/* Initialize the struct */
	int (*init) (int length);
	/* Return the next guess from this charset */
	struct guess* (*next) (void);
	/* Return the next guess from this charset */
	struct guess* (*next_block) (int count);
	/* Destroy this character set */
	void (*destroy) (void);
};

/* Generic methods */
int charset_init(struct charset *c, int length);
struct guess *charset_next(struct charset *c);
struct guess *charset_next_block(struct charset *c, int count);
void charset_destroy(struct charset *c);

#endif
