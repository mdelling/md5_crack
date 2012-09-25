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

#ifndef MD5_STRING_H
#define MD5_STRING_H

#include "md5.h"
#include <math.h>

#define HASH_LENGTH 64

/* The buckets for the password list */
extern int buckets, buckets_empty, buckets_count, match;

/* Compare a md5_binary_t with the list */
void check_md5(rainbow_t *r);

/* Get the number of lines in a file */
int read_md5_file(const char *c);

/* Cleanup */
void cleanup(void);

#endif
