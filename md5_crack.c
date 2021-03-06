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
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>

#include "cpus.h"
#include "md5_string.h"

/* Counters and quit flag */
static int cpus, min_length;
static int start_length = MIN_LENGTH, end_length = MAX_LENGTH;
static unsigned long calculated = 0, stopped = 0, quit = 0;
static char chars[MAX_CHARS + 1];
char (*prefix)[PREFIX_TABLE_SIZE][PREFIX_LENGTH] = NULL;

/* Signal handler */
static void terminate(int signum) { quit = 1; }

/* Actually do work */
static void *do_work(void *p)
{
	int char_count = charset->count - 1;
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
	string_table_init(table, charset, start_length);
	string_table_fill(table, start);
	string_table_set_prefix(table, prefix);
	rainbow = table->rainbow;
	MD5_init_once(calc);

	do {
		/* Initialize the data structures */
		MD5_init(calc, &table->suffix, table->length);

		/* Actually compute MD5 */
		for (int i = 0; i < table->entries; i += STEP_SIZE) {
			__builtin_prefetch(&rainbow[i + STEP_SIZE]);
			MD5_quad(calc, &rainbow[i], table->length);
			for (int j = 0; j < STEP_SIZE; j++)
				check_md5(&rainbow[i + j], table);
		}

		/* Increment the string */
		string_table_increment(table, start, end);
		__sync_fetch_and_add(&calculated, 1);
	} while(table->length < end_length && !quit);

	stopped++;
	return NULL;
}

void parse_charset(char *optarg)
{
	if (strcmp(optarg, "all") == 0)
		charset = &all_charset;
	else if (strcmp(optarg, "numeric") == 0)
		charset = &numeric_charset;
	else if (strcmp(optarg, "lowercase") == 0)
		charset = &lowercase_charset;
	else if (strcmp(optarg, "uppercase") == 0)
		charset = &uppercase_charset;
}

void parse_length(char *optarg)
{
	char *delim = strchr(optarg, ':');
	if (!delim)
		return;

	*delim = '\0';
	start_length = atoi(optarg);
	end_length = atoi(delim + 1) + 1;
}

void parse_prefix(char *optarg)
{
	if (strcmp(optarg, "lowercase") == 0)
		prefix = &lowercase_prefixes;
	else if (strcmp(optarg, "numeric") == 0)
		prefix = &numeric_prefixes;
	else if (strcmp(optarg, "camel") == 0)
		prefix = &camel_prefixes;
	if (prefix)
		printf("Using %s prefixes\n", optarg);
}

void parse_bad_char(char optopt)
{
	if (optopt == 'c' || optopt == 'f')
		fprintf(stderr, "Option -%c requires an argument\n", optopt);
	else if (isprint(optopt))
		fprintf(stderr, "Unknown option `-%c'\n", optopt);
	else
		fprintf(stderr, "Unknown option character `\\x%x'\n", optopt);
}

int main(int argc, char *argv[])
{
	pthread_t *t;
	int count = 0, c, calc_size;
	char *file = NULL;
	struct timeval start_time, end_time, total_time;

	printf("Sizeof md5_calc_t: %lu\n", sizeof(md5_calc_t));
	printf("Sizeof rainbow_t: %lu\n", sizeof(rainbow_t));

        /* Check arguments */
	if (argc < 3) {
		printf("Usage: md5_crack -f [list] -l [min_length]:[max_length]\n");
		return 1;
	}

	/* Parse arguments */
	opterr = 0;
	start_length = MIN_LENGTH;
	end_length = MAX_LENGTH;

	while ((c = getopt(argc, argv, "c:f:l:p:")) != -1) {
		switch (c) {
			case 'c':
				parse_charset(optarg);
				break;
			case 'f':
				file = optarg;
				break;
			case 'l':
				parse_length(optarg);
				break;
			case 'p':
				parse_prefix(optarg);
				break;
			case '?':
				parse_bad_char(optopt);
				return 1;
			default:
				abort();
		}
	}

	/* Check the file */
	if (!file) {
		fprintf(stderr, "File required\n");
		return 1;
	} else if (access(file, R_OK) == -1) {
		fprintf(stderr, "Unable to open file %s\n", file);
		return 1;
	}

	/* Switch values based on prefix mode */
	if (prefix) {
		calc_size = PREFIX_TABLE_SIZE;
		min_length = MIN_PREFIX_LENGTH;
	} else {
		calc_size = charset->table_size;
		min_length = MIN_LENGTH;
	}

	/* Check the start and end lengths */
	if (end_length <= MIN_LENGTH || end_length > MAX_LENGTH)
		end_length = MAX_LENGTH;
	if (start_length < min_length || start_length >= end_length)
		start_length == min_length;

	/* Initialize the signal handler */
	struct sigaction act = { .sa_handler = &terminate, .sa_flags = 0 };
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGINT);
	sigaction(SIGINT, &act, NULL);

	/* Initialize the character array */
	for (int i = 0; i < charset->count; i++)
		chars[i] = charset->start + i;

	/* Initialize the CPU count */
#ifdef OPENCL
	cpus = 1;
#else
	cpus = num_cpus();
#endif

	/* Get the number of lines */
	count = read_md5_file(file);
	if (count == 0) {
		printf("We didn't find any lines!\n");
		return 1;
	}

	/* Allocate the pthread structures */
	t = (pthread_t *)malloc(cpus * sizeof(pthread_t));
	if (!t) {
		printf("Failed to allocate memory\n");
		return 1;
	}

	/* Print number of cores, lock memory space, and start benchmarking */
	printf("Checking %d to %d characters using charset %s (%d)\n",
		start_length, end_length - 1, charset->name, charset->table_size);
	printf("Found %d hashes\n", count);
	printf("Found %d CPU cores\n", cpus);
	mlockall(MCL_CURRENT | MCL_FUTURE);
	gettimeofday(&start_time, NULL);

	/* Do some actual work */
	for (int i = 0; i < cpus; i++)
		pthread_create(&t[i], NULL, do_work, (void *)(unsigned long)i);

	while(stopped == 0 && sleep(1) == 0) {
		printf("Calculated %lu, %d matches\r", calculated * calc_size, match);
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
	printf("Calculated %lu, %d matches\n", calculated * calc_size, match);
	printf("Total time: %ld.%06d seconds\n", total_time.tv_sec, total_time.tv_usec);
	printf("%d populated buckets (%d empty, %d total, %d bits)\n",
		buckets_count, buckets_empty, buckets, bucket_bits);

	/* Cleanup the ingested hashes */
	cleanup();

	return 0;
}
