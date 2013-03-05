#ifndef PREFIX_LIST
#define PREFIX_LIST

#define PREFIX_TABLE_SIZE 120
#define PREFIX_LENGTH 3
#define MIN_PREFIX_LENGTH 4

extern char lowercase_prefixes[PREFIX_TABLE_SIZE][PREFIX_LENGTH];
extern char numeric_prefixes[PREFIX_TABLE_SIZE][PREFIX_LENGTH];
extern char camel_prefixes[PREFIX_TABLE_SIZE][PREFIX_LENGTH];

#endif
