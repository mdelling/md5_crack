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

#include "md5_string.h"

/* The buckets for the password list, and a -1 filled md5 */
#define BUCKETS_MORE 3
#define bswap64(x) (uint64_t)__builtin_bswap64(x)
#define PRINT_FOUND(x, y)	fprintf(found, "%016llx%016llx:%s\n", \
				bswap64(x->r64[0] + 0xefcdab8967452301), \
				bswap64(x->r64[1] + 0x1032547698badcfe), y.c);
#define PRINT_REMAINING(x)	fprintf(remaining, "%016llx%016llx\n", x.r64[0], x.r64[1])
static uint32_t *array = NULL;
int buckets = 0, buckets_empty = 0, buckets_count = 0, match = 0;
static int bucket_shift = 0, count = 0;

/* The found and remaining files */
FILE *found, *remaining;

/* The actual hash buffer */
static md5_binary_t *hash_buff = NULL;

SCALIGNED unsigned char HEX_2_INT_TABLE[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	 9, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12,
	 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0};

static uint64_t md5_bytes(const char *val)
{
	uint64_t value = (uint64_t)HEX_2_INT_TABLE[*val] << 60;
	value += (uint64_t)HEX_2_INT_TABLE[*(val + 1)] << 56;
	value += (uint64_t)HEX_2_INT_TABLE[*(val + 2)] << 52;
	value += (uint64_t)HEX_2_INT_TABLE[*(val + 3)] << 48;
	value += (uint64_t)HEX_2_INT_TABLE[*(val + 4)] << 44;
	value += (uint64_t)HEX_2_INT_TABLE[*(val + 5)] << 40;
	value += (uint64_t)HEX_2_INT_TABLE[*(val + 6)] << 36;
	value += (uint64_t)HEX_2_INT_TABLE[*(val + 7)] << 32;
	value += (uint64_t)HEX_2_INT_TABLE[*(val + 8)] << 28;
	value += (uint64_t)HEX_2_INT_TABLE[*(val + 9)] << 24;
	value += (uint64_t)HEX_2_INT_TABLE[*(val + 10)] << 20;
	value += (uint64_t)HEX_2_INT_TABLE[*(val + 11)] << 16;
	value += (uint64_t)HEX_2_INT_TABLE[*(val + 12)] << 12;
	value += (uint64_t)HEX_2_INT_TABLE[*(val + 13)] << 8;
	value += (uint64_t)HEX_2_INT_TABLE[*(val + 14)] << 4;
	return value + (uint64_t)HEX_2_INT_TABLE[*(val + 15)];
}

static void md5_binary(const char *val, md5_binary_t *result)
{
	result->r64[0] = bswap64(md5_bytes(val));
	result->r64[1] = bswap64(md5_bytes(val + 16));
	result->r32[0] -= 0x67452301;
	result->r32[1] -= 0xefcdab89;
	result->r32[2] -= 0x98badcfe;
	result->r32[3] -= 0x10325476;
}

static void reverse_md5_binary(md5_binary_t *val)
{
	val->r32[3] += 0x10325476;
	val->r32[2] += 0x98badcfe;
	val->r32[1] += 0xefcdab89;
	val->r32[0] += 0x67452301;
	val->r64[1] = bswap64(val->r64[1]);
	val->r64[0] = bswap64(val->r64[0]);
}

static int compare_md5(const void *e1, const void *e2)
{
	md5_binary_t *b1 = (md5_binary_t *)e1, *b2 = (md5_binary_t *)e2;
	if (b1->r64[0] < b2->r64[0])
		return -1;
	else if (b1->r64[0] > b2->r64[0])
		return 1;
	else if (b1->r64[1] < b2->r64[1])
		return -1;
	else if (b1->r64[1] > b2->r64[1])
		return 1;
	else
		return 0;
}

static inline unsigned int calc_bucket_binary(md5_binary_t *hash)
{
	return hash->r64[0] >> bucket_shift;
}

void check_md5(rainbow_t *r)
{
	/* Check for an existing bucket */
	for (int i = 0; i < ENTRY_SIZE; i++) {
		md5_binary_t *binary = &r->hashes[i], *start;
		unsigned int bucket = calc_bucket_binary(binary);
		uint32_t s = array[bucket];

		/* If its an empty bucket, just bail */
		if (likely(s == 0))
			continue;

		/* Search for a match */
		for (start = hash_buff + s; start->r64[0] < binary->r64[0]; start++) { }
		for (; start->r64[0] == binary->r64[0]; start++) {
			if (start->r64[1] == binary->r64[1]) {
				PRINT_FOUND(binary, r->strings[i]);
				start->r64[1] = 0;
				__sync_fetch_and_add(&match, 1);
				break;
			}
		}
	}
}

/* Allocate the hash buffer */
static md5_binary_t *allocate_buffer(int size)
{
	size_t buff_size = size * sizeof(md5_binary_t);;
	md5_binary_t *buff = (md5_binary_t *)malloc(buff_size + 1);
	if (!buff) {
		printf("%s failed to allocate memory: %d\n", __func__, errno);
		return NULL;
	}

	memset(buff, 0, buff_size);
	buff[size].r64[0] = -1;
	buff[size].r64[1] = -1;

	return buff;
}

static void init_buckets(int count)
{
	buckets = 1 << ((int)log2(count) + BUCKETS_MORE);
	array = (uint32_t *)malloc(buckets * sizeof(uint32_t));
	if (!array) {
		printf("%s failed to allocate memory: %d\n", __func__, errno);
		return;
	}

	for (int i = 0; i < buckets; i++)
		array[i] = 0;

	bucket_shift =  64 - log2(buckets);
}

static void build_buckets(int size)
{
	/* Sort the buffer */
	for (int temp = 0; temp < size; temp++) {
		unsigned int bucket = calc_bucket_binary(&hash_buff[temp]);
		if (array[bucket] == 0)
			array[bucket] = &hash_buff[temp] - &hash_buff[0];
	}

	/* Compute stats about the buckets */
	for (int i = 0; i < buckets; i++) {
		if (array[i] == 0)
			buckets_empty++;
		else
			buckets_count++;
	}
}

int read_md5_file(const char *c)
{
	char buff[HASH_LENGTH], found_name[256], remaining_name[256];

	/* Create the found and remaining names */
	snprintf(found_name, sizeof(found_name), "%s%s", c, ".found");
	snprintf(remaining_name, sizeof(remaining_name), "%s%s", c, ".remaining");

	/* Open the file to get the number of lines */
	FILE *file = fopen(c, "r");
	if (!file) {
		printf("%s failed to open password file: %d\n", __func__, errno);
		return 0;
	}

	found = fopen(found_name, "a");
	remaining = fopen(remaining_name, "w");
	if (!found) {
		printf("%s failed to open found password file: %d\n", __func__, errno);
		return 0;
	} else if (!remaining) {
		printf("%s failed to open remaining password file: %d\n", __func__, errno);
		return 0;
	}

	/* Count the number of lines */
	while (fgets(buff, HASH_LENGTH, file))
		count++;

	/* Initialize the buckets */
	init_buckets(count);

        /* Allocate the buffer */
	hash_buff = allocate_buffer(count);
	if (hash_buff == NULL)
		return 1;

	/* Read in the buffer */
	fseek(file, 0, SEEK_SET);
	count = 0;

	while (fgets(buff, HASH_LENGTH, file)) {
		md5_binary(buff, &hash_buff[count]);
		count++;
	}

	qsort(hash_buff, count, sizeof(md5_binary_t), compare_md5);
	fclose(file);

	/* Build the buckets */
	build_buckets(count);

	return count;
}

void cleanup()
{
	/* Dump remaining passwords to the remaining file */
	for (int i = 0; i < count; i++) {
		if (hash_buff[i].r64[1] != 0) {
			reverse_md5_binary(&hash_buff[i]);
			PRINT_REMAINING(hash_buff[i]);
		}
	}

	fclose(found);
	fclose(remaining);
	free(hash_buff);
	free(array);
}
