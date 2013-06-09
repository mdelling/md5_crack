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

#include "charset_numeric.h"

static char numeric_characters[] = { "0123456789" };
static struct charset *c = &charset_numeric;

/* Initialize the character set */
int numeric_charset_init(int length)
{
	return charset_init(c, length);
}

/* Return the next guess */
struct guess *numeric_charset_next(struct guess *s)
{
	return charset_next_block(c, s, 1);
}

/* Return the next guess */
struct guess *numeric_charset_next_block(struct guess *s, int count)
{
	return charset_next_block(c, s, count);
}

/* Destroy the character set */
void numeric_charset_destroy()
{
	charset_destroy(c);
}

struct charset charset_numeric = {
	.characters = numeric_characters,
	.number = 10,
	.lock = PTHREAD_MUTEX_INITIALIZER,
	.current = GUESS_INITIALIZER,
	.positions = NULL,
	.init = numeric_charset_init,
	.next = numeric_charset_next,
	.next_block = numeric_charset_next_block,
	.destroy = numeric_charset_destroy
};
