#ifndef MD5_SSE_H
#define MD5_SSE_H

#include "md5.h"
#include <stdint.h>
#include <string.h>
#include <nmmintrin.h>

/* Union wrapper for int and char access to m128i */
typedef union m128i {
	__m128i v;
	uint32_t i[4];
	unsigned char c[16];
} m128i_t;

/* MD5 data structure */
typedef union md5_raw {
	uint64_t r64[2];
	uint32_t r32[4];
	__m128i r128;
} md5_binary_t;

/* Four strings and their associated hashes */
typedef ALIGNED struct rainbow {
	m128i_t strings[4];
	md5_binary_t hashes[4];
} rainbow_t;

/* Per run data structures */
typedef struct md5_calc {
	ALIGNED m128i_t a[3];
	ALIGNED m128i_t b[3];
	ALIGNED m128i_t c[3];
	ALIGNED m128i_t d[3];
	ALIGNED m128i_t vbuffer[3][5];
	ALIGNED m128i_t iv;
	ALIGNED int size_i, size_j;
} md5_calc_t;

extern void MD5_init_once(md5_calc_t *calc);
extern void MD5_init(md5_calc_t *calc, rainbow_t *rainbow, unsigned long size);
extern void MD5_quad(md5_calc_t *calc, rainbow_t *rainbow, unsigned long size);

#endif
