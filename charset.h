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
} charset_t;

static charset_t lowercase_charset = { 0x61, 0x7A, 696, 26 };
static charset_t uppercase_charset = { 0x41, 0x5A, 696, 26 };
static charset_t numeric_charset = { 0x30, 0x39, 120, 10 };
static charset_t all_charset = { MIN_CHAR, MAX_CHAR, 9024, 95 };
static charset_t *charset = &all_charset;

#endif
