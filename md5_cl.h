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

#ifndef MD5_CL_H
#define MD5_CL_H

#include <assert.h>
#include "common.h"
#include <stdint.h>
#include <string.h>
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

/* OpenCL Stuff */
#define MEM_SIZE (128)
#define MAX_SOURCE_SIZE (0x100000)

/* Entry and step sizes */
#define ENTRY_SIZE 4
#define STEP_SIZE 3

/* Union wrapper for int and char access to m128i */
typedef union m128i {
	__m128i v;
	uint32_t i[4];
	unsigned char c[16];
} m128i_t;

/* Union wrapper for int and char access to m128i */
typedef union m384i {
	__m128i v[3];
	uint32_t i[12];
	unsigned char c[48];
} m384i_t;

/* MD5 data structure */
typedef union md5_raw {
	uint64_t r64[2];
	uint32_t r32[4];
	__m128i r128;
} md5_binary_t;

/* Four strings and their associated hashes */
typedef ALIGNED struct rainbow {
	md5_binary_t hashes[ENTRY_SIZE];
	m128i_t prefixes; /* Four four-letter prefixes */
} rainbow_t;

/* Per run data structures */
typedef struct md5_calc {
	uint32_t prefixes[ENTRY_SIZE];
	uint32_t suffix[15];
	int size_i, size_j, iv;
} md5_calc_t;

extern void MD5_init_once(md5_calc_t *calc);
extern void MD5_init(md5_calc_t *calc, m128i_t *suffix, unsigned long size);
extern void MD5_quad(md5_calc_t *calc, rainbow_t *rainbow, unsigned long size);

#endif
