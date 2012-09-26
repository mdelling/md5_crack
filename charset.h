/******************************************************************************
 * Copyright (C) 2012 Matthew Dellinger <matthew@mdelling.com>
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

#include "md5.h"

#define MAX_TABLE_SIZE 9024
#define MAX_ENTRIES (MAX_TABLE_SIZE/ENTRY_SIZE)
#define MIN_CHAR 0x20
#define MAX_CHAR 0x7E
#define MAX_CHARS (MAX_CHAR - MIN_CHAR)

typedef struct charset {
	char start;
	char end;
	int table_size;
	int count;
	char *name;
} charset_t;

static charset_t lowercase_charset = { 0x61, 0x7A, 696, 26, "lowercase" };
static charset_t uppercase_charset = { 0x41, 0x5A, 696, 26, "uppercase" };
static charset_t numeric_charset = { 0x30, 0x39, 120, 10, "numeric" };
static charset_t all_charset = { MIN_CHAR, MAX_CHAR, 9024, 95, "all" };
static charset_t *charset = &all_charset;

#endif
