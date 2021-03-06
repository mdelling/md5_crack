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

#include "md5_sse.h"

/* SSE-based implementations of the basic functions */
#define paddd(X, Y)     __builtin_ia32_paddd128(X, Y)
#define pshufd(X, s)    __builtin_ia32_pshufd(X, s)
#define pand(X, Y)      __builtin_ia32_pand128(X, Y)
#define pxor(X, Y)      __builtin_ia32_pxor128(X, Y)
#define por(X, Y)       __builtin_ia32_por128(X, Y)
#define pslld(X, s)     __builtin_ia32_pslldi128(X, s)
#define psrld(X, s)     __builtin_ia32_psrldi128(X, s)
#define pcmpeqd(X, Y)   __builtin_ia32_pcmpeqd128(X, Y)

/* Basic function wrappers */
#define F_SSE2(x, y, z) pxor(z, pand(x, pxor(y, z)))
#define G_SSE2(x, y, z) F_SSE2(z, x, y)
#define H_SSE2(x, y, z) pxor(x, pxor(y, z))
#define I_SSE2(x, y, z) pxor(y, por(x, pxor(pcmpeqd(z, z), z)))

/* Rotate functions */
#define R_SSE2(X, s) por(pslld(X, s), psrld(X, 32 - s))
#ifdef __SSSE3__
#define rot16_mask _mm_set_epi64x(0x0d0c0f0e09080b0aL, 0x0504070601000302UL)
#define R_16_SSE2(X, s) _mm_shuffle_epi8((X), rot16_mask)
#else
#define R_16_SSE2(X, s) _mm_shufflehi_epi16(_mm_shufflelo_epi16(X, 0xB1), 0xB1)
#endif

/* Data access */
#define COMMON(x, y) calc->common[y - 1].v
#define DATA(x, y) calc->vbuffer[x].v

/* The MD5 transformation for all four rounds */
#define SSE_STEP(f, a, b, c, d, x, t, s, R, V) \
	(calc->a[0].v) = paddd(paddd(f((calc->b[0].v), (calc->c[0].v), (calc->d[0].v)), (calc->a[0].v)), storage[t].v); \
	(calc->a[1].v) = paddd(paddd(f((calc->b[1].v), (calc->c[1].v), (calc->d[1].v)), (calc->a[1].v)), storage[t].v); \
	(calc->a[2].v) = paddd(paddd(f((calc->b[2].v), (calc->c[2].v), (calc->d[2].v)), (calc->a[2].v)), storage[t].v); \
	(calc->a[0].v) = paddd(R(paddd((calc->a[0].v), V(0, x)), s), (calc->b[0].v)); \
	(calc->a[1].v) = paddd(R(paddd((calc->a[1].v), V(1, x)), s), (calc->b[1].v)); \
	(calc->a[2].v) = paddd(R(paddd((calc->a[2].v), V(2, x)), s), (calc->b[2].v));

#define SSE_SHORT_STEP(f, a, b, c, d, x, t, s, R) \
	(calc->a[0].v) = paddd(paddd(f((calc->b[0].v), (calc->c[0].v), (calc->d[0].v)), (calc->a[0].v)), storage[t].v); \
	(calc->a[1].v) = paddd(paddd(f((calc->b[1].v), (calc->c[1].v), (calc->d[1].v)), (calc->a[1].v)), storage[t].v); \
	(calc->a[2].v) = paddd(paddd(f((calc->b[2].v), (calc->c[2].v), (calc->d[2].v)), (calc->a[2].v)), storage[t].v); \
	(calc->a[0].v) = paddd(R((calc->a[0].v), s), (calc->b[0].v)); \
	(calc->a[1].v) = paddd(R((calc->a[1].v), s), (calc->b[1].v)); \
	(calc->a[2].v) = paddd(R((calc->a[2].v), s), (calc->b[2].v));

#define SSE_FIRST_STEP(f, a, b, c, d, x, t, s, R, V) \
	(calc->a[0].v) = paddd(first.v, storage[t].v); \
	(calc->a[1].v) = paddd(first.v, storage[t].v); \
	(calc->a[2].v) = paddd(first.v, storage[t].v); \
	(calc->a[0].v) = paddd(R(paddd((calc->a[0].v), V(0, x)), s), (calc->b[0].v)); \
	(calc->a[1].v) = paddd(R(paddd((calc->a[1].v), V(1, x)), s), (calc->b[1].v)); \
	(calc->a[2].v) = paddd(R(paddd((calc->a[2].v), V(2, x)), s), (calc->b[2].v));

/* Constant data structures */
SCALIGNED m128i_t iva = { .i = { 0x67452301, 0x67452301, 0x67452301, 0x67452301 } };
SCALIGNED m128i_t ivb = { .i = { 0xefcdab89, 0xefcdab89, 0xefcdab89, 0xefcdab89 } };
SCALIGNED m128i_t ivc = { .i = { 0x98badcfe, 0x98badcfe, 0x98badcfe, 0x98badcfe } };
SCALIGNED m128i_t ivd = { .i = { 0x10325476, 0x10325476, 0x10325476, 0x10325476 } };
SCALIGNED m128i_t first = { .i = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF } };
SCALIGNED m128i_t end = { .i = { 0x80, 0x80, 0x80, 0x80 } };
SCALIGNED m128i_t storage[64] = { { .i = { 0xd76aa478, 0xd76aa478, 0xd76aa478, 0xd76aa478 } },
				  { .i = { 0xe8c7b756, 0xe8c7b756, 0xe8c7b756, 0xe8c7b756 } },
				  { .i = { 0x242070db, 0x242070db, 0x242070db, 0x242070db } },
				  { .i = { 0xc1bdceee, 0xc1bdceee, 0xc1bdceee, 0xc1bdceee } },
				  { .i = { 0xf57c0faf, 0xf57c0faf, 0xf57c0faf, 0xf57c0faf } },
				  { .i = { 0x4787c62a, 0x4787c62a, 0x4787c62a, 0x4787c62a } },
				  { .i = { 0xa8304613, 0xa8304613, 0xa8304613, 0xa8304613 } },
				  { .i = { 0xfd469501, 0xfd469501, 0xfd469501, 0xfd469501 } },
				  { .i = { 0x698098d8, 0x698098d8, 0x698098d8, 0x698098d8 } },
				  { .i = { 0x8b44f7af, 0x8b44f7af, 0x8b44f7af, 0x8b44f7af } },
				  { .i = { 0xffff5bb1, 0xffff5bb1, 0xffff5bb1, 0xffff5bb1 } },
				  { .i = { 0x895cd7be, 0x895cd7be, 0x895cd7be, 0x895cd7be } },
				  { .i = { 0x6b901122, 0x6b901122, 0x6b901122, 0x6b901122 } },
				  { .i = { 0xfd987193, 0xfd987193, 0xfd987193, 0xfd987193 } },
				  { .i = { 0xa679438e, 0xa679438e, 0xa679438e, 0xa679438e } },
				  { .i = { 0x49b40821, 0x49b40821, 0x49b40821, 0x49b40821 } },
				  { .i = { 0xf61e2562, 0xf61e2562, 0xf61e2562, 0xf61e2562 } },
				  { .i = { 0xc040b340, 0xc040b340, 0xc040b340, 0xc040b340 } },
				  { .i = { 0x265e5a51, 0x265e5a51, 0x265e5a51, 0x265e5a51 } },
				  { .i = { 0xe9b6c7aa, 0xe9b6c7aa, 0xe9b6c7aa, 0xe9b6c7aa } },
				  { .i = { 0xd62f105d, 0xd62f105d, 0xd62f105d, 0xd62f105d } },
				  { .i = { 0x02441453, 0x02441453, 0x02441453, 0x02441453 } },
				  { .i = { 0xd8a1e681, 0xd8a1e681, 0xd8a1e681, 0xd8a1e681 } },
				  { .i = { 0xe7d3fbc8, 0xe7d3fbc8, 0xe7d3fbc8, 0xe7d3fbc8 } },
				  { .i = { 0x21e1cde6, 0x21e1cde6, 0x21e1cde6, 0x21e1cde6 } },
				  { .i = { 0xc33707d6, 0xc33707d6, 0xc33707d6, 0xc33707d6 } },
				  { .i = { 0xf4d50d87, 0xf4d50d87, 0xf4d50d87, 0xf4d50d87 } },
				  { .i = { 0x455a14ed, 0x455a14ed, 0x455a14ed, 0x455a14ed } },
				  { .i = { 0xa9e3e905, 0xa9e3e905, 0xa9e3e905, 0xa9e3e905 } },
				  { .i = { 0xfcefa3f8, 0xfcefa3f8, 0xfcefa3f8, 0xfcefa3f8 } },
				  { .i = { 0x676f02d9, 0x676f02d9, 0x676f02d9, 0x676f02d9 } },
				  { .i = { 0x8d2a4c8a, 0x8d2a4c8a, 0x8d2a4c8a, 0x8d2a4c8a } },
				  { .i = { 0xfffa3942, 0xfffa3942, 0xfffa3942, 0xfffa3942 } },
				  { .i = { 0x8771f681, 0x8771f681, 0x8771f681, 0x8771f681 } },
				  { .i = { 0x6d9d6122, 0x6d9d6122, 0x6d9d6122, 0x6d9d6122 } },
				  { .i = { 0xfde5380c, 0xfde5380c, 0xfde5380c, 0xfde5380c } },
				  { .i = { 0xa4beea44, 0xa4beea44, 0xa4beea44, 0xa4beea44 } },
				  { .i = { 0x4bdecfa9, 0x4bdecfa9, 0x4bdecfa9, 0x4bdecfa9 } },
				  { .i = { 0xf6bb4b60, 0xf6bb4b60, 0xf6bb4b60, 0xf6bb4b60 } },
				  { .i = { 0xbebfbc70, 0xbebfbc70, 0xbebfbc70, 0xbebfbc70 } },
				  { .i = { 0x289b7ec6, 0x289b7ec6, 0x289b7ec6, 0x289b7ec6 } },
				  { .i = { 0xeaa127fa, 0xeaa127fa, 0xeaa127fa, 0xeaa127fa } },
				  { .i = { 0xd4ef3085, 0xd4ef3085, 0xd4ef3085, 0xd4ef3085 } },
				  { .i = { 0x04881d05, 0x04881d05, 0x04881d05, 0x04881d05 } },
				  { .i = { 0xd9d4d039, 0xd9d4d039, 0xd9d4d039, 0xd9d4d039 } },
				  { .i = { 0xe6db99e5, 0xe6db99e5, 0xe6db99e5, 0xe6db99e5 } },
				  { .i = { 0x1fa27cf8, 0x1fa27cf8, 0x1fa27cf8, 0x1fa27cf8 } },
				  { .i = { 0xc4ac5665, 0xc4ac5665, 0xc4ac5665, 0xc4ac5665 } },
				  { .i = { 0xf4292244, 0xf4292244, 0xf4292244, 0xf4292244 } },
				  { .i = { 0x432aff97, 0x432aff97, 0x432aff97, 0x432aff97 } },
				  { .i = { 0xab9423a7, 0xab9423a7, 0xab9423a7, 0xab9423a7 } },
				  { .i = { 0xfc93a039, 0xfc93a039, 0xfc93a039, 0xfc93a039 } },
				  { .i = { 0x655b59c3, 0x655b59c3, 0x655b59c3, 0x655b59c3 } },
				  { .i = { 0x8f0ccc92, 0x8f0ccc92, 0x8f0ccc92, 0x8f0ccc92 } },
				  { .i = { 0xffeff47d, 0xffeff47d, 0xffeff47d, 0xffeff47d } },
				  { .i = { 0x85845dd1, 0x85845dd1, 0x85845dd1, 0x85845dd1 } },
				  { .i = { 0x6fa87e4f, 0x6fa87e4f, 0x6fa87e4f, 0x6fa87e4f } },
				  { .i = { 0xfe2ce6e0, 0xfe2ce6e0, 0xfe2ce6e0, 0xfe2ce6e0 } },
				  { .i = { 0xa3014314, 0xa3014314, 0xa3014314, 0xa3014314 } },
				  { .i = { 0x4e0811a1, 0x4e0811a1, 0x4e0811a1, 0x4e0811a1 } },
				  { .i = { 0xf7537e82, 0xf7537e82, 0xf7537e82, 0xf7537e82 } },
				  { .i = { 0xbd3af235, 0xbd3af235, 0xbd3af235, 0xbd3af235 } },
				  { .i = { 0x2ad7d2bb, 0x2ad7d2bb, 0x2ad7d2bb, 0x2ad7d2bb } },
				  { .i = { 0xeb86d391, 0xeb86d391, 0xeb86d391, 0xeb86d391 } } };

/* Zero out the buffer */
void MD5_init_once(md5_calc_t *calc)
{
	memset(calc, 0, sizeof(calc));
}

/* Initialize for this size */
void MD5_init(md5_calc_t *calc, m128i_t *suffix, unsigned long size)
{
	uint32_t val = (size << 3);
	calc->size_i = size / 4;
	calc->size_j = size % 4;
	__builtin_prefetch(storage);
	calc->iv.v = pslld(end.v, 8 * calc->size_j);

	/* Fill byte 14 */
	calc->common[3].i[0] = val;
	calc->common[3].i[1] = val;
	calc->common[3].i[2] = val;
	calc->common[3].i[3] = val;

	/* Fill the rest of the common buffer */
	calc->common[0].v = _mm_shuffle_epi32(suffix->v, 0x00);
	calc->common[1].v = _mm_shuffle_epi32(suffix->v, 0x55);
	calc->common[2].v = _mm_shuffle_epi32(suffix->v, 0xAA);

	if (size >= 4)
		calc->common[calc->size_i - 1].v = por(calc->iv.v, calc->common[calc->size_i - 1].v);
}

/* Do 12 MD5 calculations */
void MD5_quad(md5_calc_t *calc, rainbow_t *rainbow, unsigned long size)
{
	/* Initialize the start vectors - a is set in the first pass */
	calc->b[0].v = calc->b[1].v = calc->b[2].v = ivb.v;
	calc->c[0].v = calc->c[1].v = calc->c[2].v = ivc.v;
	calc->d[0].v = calc->d[1].v = calc->d[2].v = ivd.v;

	/* Copy user data into vector buffer */
	calc->vbuffer[0] = rainbow[0].prefixes;
	calc->vbuffer[1] = rainbow[1].prefixes;
	calc->vbuffer[2] = rainbow[2].prefixes;

	if (size < 4) {
		calc->vbuffer[0].v = por(calc->iv.v, calc->vbuffer[0].v);
		calc->vbuffer[1].v = por(calc->iv.v, calc->vbuffer[1].v);
		calc->vbuffer[2].v = por(calc->iv.v, calc->vbuffer[2].v);
	}

	/* Compute round 1 blocks */
	SSE_FIRST_STEP(F_SSE2, a, b, c, d, 0, 0, 7, R_SSE2, DATA)
	SSE_STEP(F_SSE2, d, a, b, c, 1, 1, 12, R_SSE2, COMMON)
	SSE_STEP(F_SSE2, c, d, a, b, 2, 2, 17, R_SSE2, COMMON)
	SSE_STEP(F_SSE2, b, c, d, a, 3, 3, 22, R_SSE2, COMMON)
	SSE_SHORT_STEP(F_SSE2, a, b, c, d, 4, 4, 7, R_SSE2)
	SSE_SHORT_STEP(F_SSE2, d, a, b, c, 5, 5, 12, R_SSE2)
	SSE_SHORT_STEP(F_SSE2, c, d, a, b, 6, 6, 17, R_SSE2)
	SSE_SHORT_STEP(F_SSE2, b, c, d, a, 7, 7, 22, R_SSE2)
	SSE_SHORT_STEP(F_SSE2, a, b, c, d, 8, 8, 7, R_SSE2)
	SSE_SHORT_STEP(F_SSE2, d, a, b, c, 9, 9, 12, R_SSE2)
	SSE_SHORT_STEP(F_SSE2, c, d, a, b, 10, 10, 17, R_SSE2)
	SSE_SHORT_STEP(F_SSE2, b, c, d, a, 11, 11, 22, R_SSE2)
	SSE_SHORT_STEP(F_SSE2, a, b, c, d, 12, 12, 7, R_SSE2)
	SSE_SHORT_STEP(F_SSE2, d, a, b, c, 13, 13, 12, R_SSE2)
	SSE_STEP(F_SSE2, c, d, a, b, 4, 14, 17, R_SSE2, COMMON) /* 14 -> 4 */
	SSE_SHORT_STEP(F_SSE2, b, c, d, a, 15, 15, 22, R_SSE2)

	/* Compute round 2 blocks */
	SSE_STEP(G_SSE2, a, b, c, d, 1, 16, 5, R_SSE2, COMMON)
	SSE_SHORT_STEP(G_SSE2, d, a, b, c, 6, 17, 9, R_SSE2)
	SSE_SHORT_STEP(G_SSE2, c, d, a, b, 11, 18, 14, R_SSE2)
	SSE_STEP(G_SSE2, b, c, d, a, 0, 19, 20, R_SSE2, DATA)
	SSE_SHORT_STEP(G_SSE2, a, b, c, d, 5, 20, 5, R_SSE2)
	SSE_SHORT_STEP(G_SSE2, d, a, b, c, 10, 21, 9, R_SSE2)
	SSE_SHORT_STEP(G_SSE2, c, d, a, b, 15, 22, 14, R_SSE2)
	SSE_SHORT_STEP(G_SSE2, b, c, d, a, 4, 23, 20, R_SSE2)
	SSE_SHORT_STEP(G_SSE2, a, b, c, d, 9, 24, 5, R_SSE2)
	SSE_STEP(G_SSE2, d, a, b, c, 4, 25, 9, R_SSE2, COMMON) /* 14 -> 4 */
	SSE_STEP(G_SSE2, c, d, a, b, 3, 26, 14, R_SSE2, COMMON)
	SSE_SHORT_STEP(G_SSE2, b, c, d, a, 8, 27, 20, R_SSE2)
	SSE_SHORT_STEP(G_SSE2, a, b, c, d, 13, 28, 5, R_SSE2)
	SSE_STEP(G_SSE2, d, a, b, c, 2, 29, 9, R_SSE2, COMMON)
	SSE_SHORT_STEP(G_SSE2, c, d, a, b, 7, 30, 14, R_SSE2)
	SSE_SHORT_STEP(G_SSE2, b, c, d, a, 12, 31, 20, R_SSE2)

	/* Compute round 3 blocks */
	SSE_SHORT_STEP(H_SSE2, a, b, c, d, 5, 32, 4, R_SSE2)
	SSE_SHORT_STEP(H_SSE2, d, a, b, c, 8, 33, 11, R_SSE2)
	SSE_SHORT_STEP(H_SSE2, c, d, a, b, 11, 34, 16, R_16_SSE2)
	SSE_STEP(H_SSE2, b, c, d, a, 4, 35, 23, R_SSE2, COMMON) /* 14 -> 4 */
	SSE_STEP(H_SSE2, a, b, c, d, 1, 36, 4, R_SSE2, COMMON)
	SSE_SHORT_STEP(H_SSE2, d, a, b, c, 4, 37, 11, R_SSE2)
	SSE_SHORT_STEP(H_SSE2, c, d, a, b, 7, 38, 16, R_16_SSE2)
	SSE_SHORT_STEP(H_SSE2, b, c, d, a, 10, 39, 23, R_SSE2)
	SSE_SHORT_STEP(H_SSE2, a, b, c, d, 13, 40, 4, R_SSE2)
	SSE_STEP(H_SSE2, d, a, b, c, 0, 41, 11, R_SSE2, DATA)
	SSE_STEP(H_SSE2, c, d, a, b, 3, 42, 16, R_16_SSE2, COMMON)
	SSE_SHORT_STEP(H_SSE2, b, c, d, a, 6, 43, 23, R_SSE2)
	SSE_SHORT_STEP(H_SSE2, a, b, c, d, 9, 44, 4, R_SSE2)
	SSE_SHORT_STEP(H_SSE2, d, a, b, c, 12, 45, 11, R_SSE2)
	SSE_SHORT_STEP(H_SSE2, c, d, a, b, 15, 46, 16, R_16_SSE2)
	SSE_STEP(H_SSE2, b, c, d, a, 2, 47, 23, R_SSE2, COMMON)

	/* Compute round 4 blocks */
	SSE_STEP(I_SSE2, a, b, c, d, 0, 48, 6, R_SSE2, DATA)
	SSE_SHORT_STEP(I_SSE2, d, a, b, c, 7, 49, 10, R_SSE2)
	SSE_STEP(I_SSE2, c, d, a, b, 4, 50, 15, R_SSE2, COMMON) /* 14 -> 4 */
	SSE_SHORT_STEP(I_SSE2, b, c, d, a, 5, 51, 21, R_SSE2)
	SSE_SHORT_STEP(I_SSE2, a, b, c, d, 12, 52, 6, R_SSE2)
	SSE_STEP(I_SSE2, d, a, b, c, 3, 53, 10, R_SSE2, COMMON)
	SSE_SHORT_STEP(I_SSE2, c, d, a, b, 10, 54, 15, R_SSE2)
	SSE_STEP(I_SSE2, b, c, d, a, 1, 55, 21, R_SSE2, COMMON)
	SSE_SHORT_STEP(I_SSE2, a, b, c, d, 8, 56, 6, R_SSE2)
	SSE_SHORT_STEP(I_SSE2, d, a, b, c, 15, 57, 10, R_SSE2)
	SSE_SHORT_STEP(I_SSE2, c, d, a, b, 6, 58, 15, R_SSE2)
	SSE_SHORT_STEP(I_SSE2, b, c, d, a, 13, 59, 21, R_SSE2)
	SSE_SHORT_STEP(I_SSE2, a, b, c, d, 4, 60, 6, R_SSE2)
	SSE_SHORT_STEP(I_SSE2, d, a, b, c, 11, 61, 10, R_SSE2)
	SSE_STEP(I_SSE2, c, d, a, b, 2, 62, 15, R_SSE2, COMMON)
	SSE_SHORT_STEP(I_SSE2, b, c, d, a, 9, 63, 21, R_SSE2)

	/* Do final step */
	for (int i = 0; i < 3; i++) {
		/* Push back out to user storage */
		__m128i T0 = _mm_unpacklo_epi32(calc->a[i].v, calc->b[i].v);
		__m128i T1 = _mm_unpacklo_epi32(calc->c[i].v, calc->d[i].v);
		__m128i T2 = _mm_unpackhi_epi32(calc->a[i].v, calc->b[i].v);
		__m128i T3 = _mm_unpackhi_epi32(calc->c[i].v, calc->d[i].v);
		rainbow[i].hashes[0].r128 = _mm_unpacklo_epi64(T0, T1);
		rainbow[i].hashes[1].r128 = _mm_unpackhi_epi64(T0, T1);
		rainbow[i].hashes[2].r128 = _mm_unpacklo_epi64(T2, T3);
		rainbow[i].hashes[3].r128 = _mm_unpackhi_epi64(T2, T3);
	}
}
