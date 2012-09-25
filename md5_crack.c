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

#include <sys/mman.h>
#include <sys/time.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>

#include "cpus.h"
#include "md5_string.h"
#include "md5.h"

#define CHAR_START 0x20
#define CHAR_END 0x7E
#define MIN_LENGTH 3
#define MAX_LENGTH 16
#define TABLE_SIZE 9024
#define ENTRY_SIZE 4
#define ENTRIES (TABLE_SIZE/ENTRY_SIZE)

/* Structure to store strings */
typedef __attribute__ ((aligned(16))) struct string_table {
	rainbow_t rainbow[ENTRIES];
	int length;
} string_table_t;

/* Counters and quit flag */
static int cpus, char_count = CHAR_END - CHAR_START;
static int start_length = MIN_LENGTH, end_length = MAX_LENGTH;
static unsigned long calculated = 0, stopped = 0, quit = 0;

static char chars[CHAR_END-CHAR_START + 1];

/* Signal handler */
static void terminate(int signum) { quit = 1; }

/* Initialize a buffer */
static void init_string_table(string_table_t *table, const char start, int length)
{
	char temp1 = CHAR_START, temp2 = CHAR_START;
	table->length = length;
	memset(&table->rainbow, 0, sizeof(rainbow_t) * ENTRIES);

	for (int i = 0; i < ENTRIES; i++) {
		for (int j = 0; j < 4; j++) {
			table->rainbow[i].strings[j].c[0] = temp1;
			table->rainbow[i].strings[j].c[1] = temp2;
			table->rainbow[i].strings[j].c[2] = start;
			for (int k = 3; k < length; k++)
				table->rainbow[i].strings[j].c[k] = CHAR_START;

			if (temp2 == CHAR_END) {
				temp2 = CHAR_START;
				temp1++;
			} else
				temp2++;
		}
	}
}

/* Increment a string */
static void inc_string(string_table_t *table, const char start, const char end)
{
	for (int i = table->length - 1; i > 1; i--) {
		char temp = CHAR_START;
		if (i == 2 && table->rainbow[0].strings[0].c[i] == end)
			temp = start;
		else if (table->rainbow[0].strings[0].c[i] < CHAR_END)
			temp = table->rainbow[0].strings[0].c[i] + 1;

		for (int j = 0; j < ENTRIES; j++) {
			table->rainbow[j].strings[0].c[i] = temp;
			table->rainbow[j].strings[1].c[i] = temp;
			table->rainbow[j].strings[2].c[i] = temp;
			table->rainbow[j].strings[3].c[i] = temp;
		}

		if (i == 2 && temp == start)
			break;
		else if (temp != CHAR_START)
			return;
	}

	table->length++;
	for (int j = 0; j < ENTRIES; j++) {
		table->rainbow[j].strings[0].c[table->length - 1] = CHAR_START;
		table->rainbow[j].strings[1].c[table->length - 1] = CHAR_START;
		table->rainbow[j].strings[2].c[table->length - 1] = CHAR_START;
		table->rainbow[j].strings[3].c[table->length - 1] = CHAR_START;
	}
}

/* Actually do work */
static void *do_work(void *p)
{
	unsigned long id = (unsigned long)p;
	unsigned long start_index = (id * char_count + cpus - 1)/cpus;
	unsigned long end_index = ((id + 1) * char_count) / cpus;
	unsigned char start = chars[start_index], end = chars[end_index];
	string_table_t *table = NULL;
	rainbow_t *rainbow = NULL;
	md5_calc_t *calc = NULL;

	/* Allocate an MD5 calculation structure  and string array */
	calc = (md5_calc_t *)malloc(sizeof(md5_calc_t));
	table = (string_table_t *)malloc(sizeof(string_table_t));

	if (!calc || !table) {
		printf("Failed to allocate memory\n");
		return NULL;
	}

	/* Initialize data structure */
	init_string_table(table, start, start_length);
	rainbow = table->rainbow;
	MD5_init_once(calc);

	do {
		/* Initialize the data structures */
		MD5_init(calc, &rainbow[0], table->length);

		/* Actually compute MD5 */
		for (int i = 0; i < ENTRIES; i += 3) {
			__builtin_prefetch(&rainbow[i + 3]);
			MD5_quad(calc, &rainbow[i], table->length);
			check_md5(&rainbow[i]);
			check_md5(&rainbow[i + 1]);
			check_md5(&rainbow[i + 2]);
		}

		/* Increment the string */
		inc_string(table, start, end);
		__sync_fetch_and_add(&calculated, 1);
	} while(table->length < end_length && !quit);

	stopped++;
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t *t;
	int count = 0;
	struct timeval start_time, end_time, total_time;

        /* Check arguments */
	if (argc < 4) {
		printf("Usage: md5_crack [list] [min_length] [max_length]\n");
		return 1;
	}

	/* Initialize the start length */
	start_length = atoi(argv[2]);
	end_length = atoi(argv[3]) + 1;
	if (end_length <= MIN_LENGTH || end_length > MAX_LENGTH)
		end_length = MAX_LENGTH;
	if (start_length < 3 || start_length >= end_length)
		start_length == 3;

	printf("Checking %d to %d characters\n", start_length, end_length - 1);

	/* Initialize the signal handler */
	struct sigaction act = { .sa_handler = &terminate, .sa_flags = 0 };
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGINT);
	sigaction(SIGINT, &act, NULL);

	/* Initialize the character array */
	for (int i = 0; i <= char_count; i++)
		chars[i] = CHAR_START + i;

	/* Initialize the CPU count */
	cpus = num_cpus();

	/* Get the number of lines */
	count = read_md5_file(argv[1]);
	if (count == 0) {
		printf("We didn't find any lines!\n");
		return 1;
	}

	printf("Found %d hashes\n", count);

	/* Allocate the pthread structures */
	t = (pthread_t *)malloc(cpus * sizeof(pthread_t));
	if (!t) {
		printf("Failed to allocate memory\n");
		return 1;
	}

	/* Print number of cores, lock memory space, and start benchmarking */
	printf("Found %d CPU cores\n", cpus);
	mlockall(MCL_CURRENT | MCL_FUTURE);
	gettimeofday(&start_time, NULL);

	/* Do some actual work */
	for (int i = 0; i < cpus; i++)
		pthread_create(&t[i], NULL, do_work, (void *)(unsigned long)i);

	while(stopped == 0 && sleep(1) == 0) {
		printf("Calculated %lu, %d matches\r", calculated * TABLE_SIZE, match);
		fflush(stdout);
	}

	for (int i = 0; i < cpus; i++)
		pthread_join(t[i], NULL);

	/* Compute time used */
	gettimeofday(&end_time, NULL);
	total_time.tv_sec = end_time.tv_sec - start_time.tv_sec;
	total_time.tv_usec = end_time.tv_usec - start_time.tv_usec;
	if (total_time.tv_usec < 0) {
		total_time.tv_usec += 1000000;
		total_time.tv_sec--;
	}

	/* Print info */
	printf("Calculated %lu, %d matches\n", calculated * TABLE_SIZE, match);
	printf("Total time: %ld.%06d seconds\n", total_time.tv_sec, total_time.tv_usec);
	printf("%d populated buckets (%d empty, %d total)\n", buckets_count, buckets_empty, buckets);

	/* Cleanup the ingested hashes */
	cleanup();

	return 0;
}
