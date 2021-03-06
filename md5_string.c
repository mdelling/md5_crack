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

static uint32_t *array = NULL;
static uint8_t *array_count = NULL;
int buckets = 0, buckets_empty = 0, buckets_count = 0, match = 0, bucket_bits = 0;
static int count = 0, count_bits = 0, bucket_shift = 0, inv_top_bits = 0;
static uint32_t get_mask = 0, remove_mask = 0;

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

static void md5_binary_reverse(md5_binary_t *value)
{
	value->r32[0] += 0x67452301;
	value->r32[1] += 0xefcdab89;
	value->r32[2] += 0x98badcfe;
	value->r32[3] += 0x10325476;
	value->r64[0] = bswap64(value->r64[0]);
	value->r64[1] = bswap64(value->r64[1]);
}

static void print_found(rainbow_t *rainbow, int i, string_table_t *table)
{
	char string[16];
	md5_binary_t temp = rainbow->hashes[i];
	md5_binary_reverse(&temp);
	memcpy(string, &rainbow->prefixes.c[4 * i], 4);
	memcpy(string + 4, table->suffix.c, 12);
	fprintf(found, "%016" PRIx64 "%016" PRIx64 ":%s\n", temp.r64[0], temp.r64[1], string);
}

static void print_remaining(md5_binary_t *hash)
{
	md5_binary_t temp = *hash;
	md5_binary_reverse(&temp);
	fprintf(remaining, "%016" PRIx64 "%016" PRIx64 "\n", temp.r64[0], temp.r64[1]);
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

static inline uint32_t calc_top_binary(md5_binary_t *hash)
{
	return hash->r32[1] << inv_top_bits;
}

static inline uint32_t get_top(uint32_t value)
{
	return value & get_mask;
}

static inline uint32_t remove_top(uint32_t value)
{
	return value & remove_mask;
}

void check_md5(rainbow_t *r, string_table_t *table)
{
	/* Check for an existing bucket */
	for (int i = 0; i < ENTRY_SIZE; i++) {
		md5_binary_t *binary = &r->hashes[i], *start;
		unsigned int bucket = calc_bucket_binary(binary);
		uint32_t s = array[bucket];

		/* If its an empty bucket, just bail */
		if (likely(s == UINT32_MAX))
			continue;

		/* If we have a stored top and it doesn't match, continue */
		if (get_top(s) && (calc_top_binary(binary) != get_top(s)))
			continue;

		/* Search for a match */
		for (start = hash_buff + remove_top(s); start->r64[0] < binary->r64[0]; start++) { }
		for (; start->r64[0] == binary->r64[0]; start++) {
			if (start->r64[1] == binary->r64[1]) {
				print_found(r, i, table);
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
	size_t buff_size = (size + 1) * sizeof(md5_binary_t);;
	md5_binary_t *buff = (md5_binary_t *)malloc(buff_size);
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
	/* Figure out all the shift sizes */
	count_bits = (int)log2(count);
	bucket_bits = count_bits + BUCKETS_MORE;
	buckets = 1 << bucket_bits;
	bucket_shift =  64 - bucket_bits;
	uint32_t top_bits = 32 - count_bits - 1;
	inv_top_bits = 32 - top_bits;
	get_mask = UINT32_MAX << inv_top_bits;
	remove_mask = UINT32_MAX >> top_bits;

	array = (uint32_t *)malloc(buckets * sizeof(uint32_t));
	if (!array) {
		printf("%s failed to allocate memory: %d\n", __func__, errno);
		return;
	}

	array_count = (uint8_t *)malloc(buckets * sizeof(uint8_t));
	if (!array_count) {
		printf("%s failed to allocate memory: %d\n", __func__, errno);
		return;
	}

	memset(array_count, 0, buckets * sizeof(uint8_t));

	for (int i = 0; i < buckets; i++)
		array[i] = UINT32_MAX;
}

static void build_buckets(int size)
{
	/* Sort the buffer */
	for (int temp = 0; temp < size; temp++) {
		unsigned int bucket = calc_bucket_binary(&hash_buff[temp]);
		array_count[bucket]++;
		if (array[bucket] == UINT32_MAX) {
			array[bucket] = (&hash_buff[temp] - &hash_buff[0]);
		}
	}

	/* Generate the top data */
	for (int temp = 0; temp < size; temp++) {
		unsigned int bucket = calc_bucket_binary(&hash_buff[temp]);
		uint32_t top = calc_top_binary(&hash_buff[temp]);
		if (array_count[bucket] == 1)
			array[bucket] |= top;
	}

	/* Compute stats about the buckets */
	for (int i = 0; i < buckets; i++) {
		if (array[i] == UINT32_MAX)
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
		if (hash_buff[i].r64[1] != 0)
			print_remaining(&hash_buff[i]);
	}

	fclose(found);
	fclose(remaining);
	free(hash_buff);
	free(array);
}
