/******************************************************************************
 * Copyright (C) 2012 Matthew Dellinger <matthew@mdelling.com>
 *
 * This program is free software; you can redistribute it and/or
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

#ifndef STRING_TABLE_H
#define STRING_TABLE_H

#include "charset.h"
#include "prefix_list.h"

#define MIN_LENGTH 3
#define MAX_LENGTH 16

/* Structure to store strings */
typedef ALIGNED struct string_table {
	/* State information */
	charset_t *charset;
	char (*prefix)[PREFIX_TABLE_SIZE][PREFIX_LENGTH];
	int length;
	int prefix_length;
	int entries;

	/* Storage */
	rainbow_t rainbow[MAX_ENTRIES];
	m128i_t suffix;
} string_table_t;

void string_table_init(string_table_t *table, charset_t *charset, int length);
void string_table_set_prefix(string_table_t *table, char (*prefix)[PREFIX_TABLE_SIZE][PREFIX_LENGTH]);
void string_table_fill(string_table_t *table, const char start);
void string_table_increment(string_table_t *table, const char start, const char end);

#endif
