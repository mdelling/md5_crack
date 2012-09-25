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
