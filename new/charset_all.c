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

#include "charset_all.h"

static char all_characters[] = { "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz" };
static struct charset *c = &charset_all;

/* Initialize the character set */
int all_charset_init(int length)
{
	return charset_init(c, length);
}

/* Return the next guess */
struct guess *all_charset_next(struct guess *s)
{
	return charset_next_block(c, s, 1);
}

/* Return the next guess */
struct guess *all_charset_next_block(struct guess *s, int count)
{
	return charset_next_block(c, s, count);
}

/* Destroy the character set */
void all_charset_destroy()
{
	charset_destroy(c);
}

struct charset charset_all = {
	.characters = all_characters,
	.number = 90,
	.last = 0,
	.lock = PTHREAD_MUTEX_INITIALIZER,
	.table = NULL,
	.table_index = 0,
	.positions = NULL,
	.init = all_charset_init,
	.next = all_charset_next,
	.next_block = all_charset_next_block,
	.destroy = all_charset_destroy
};
