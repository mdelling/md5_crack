#include "string_table.h"

void string_table_init(string_table_t *table, charset_t *charset, int length)
{
	memset(table, 0, sizeof(string_table_t));
	table->length = length;
	table->charset = charset;
}

void string_table_set_prefix(string_table_t *table, char (*prefix)[PREFIX_TABLE_SIZE][PREFIX_LENGTH])
{
	table->prefix = prefix;
}

/* Initialize a buffer */
static void charset_init_string_table(string_table_t *table, const char start)
{
	char c_start = table->charset->start, c_end = table->charset->end;
	char temp1 = c_start, temp2 = c_start;

	for (int i = 0; i < table->entries; i++) {
		for (int j = 0; j < ENTRY_SIZE; j++) {
			table->rainbow[i].prefixes.c[j * 4 + 0] = temp1;
			table->rainbow[i].prefixes.c[j * 4 + 1] = temp2;
			table->rainbow[i].prefixes.c[j * 4 + 2] = start;
			if (table->length > 3)
				table->rainbow[i].prefixes.c[j * 4 + 3] = c_start;

			if (temp2 == c_end) {
				temp2 = c_start;
				temp1++;
			} else
				temp2++;
		}
	}
}

static void prefix_init_string_table(string_table_t *table, const char start)
{
	for (int i = 0; i < table->entries; i++) {
		for (int j = 0; j < ENTRY_SIZE; j++) {
			table->rainbow[i].prefixes.c[j * 4 + 0] = (*table->prefix)[i * ENTRY_SIZE + j][0];
			table->rainbow[i].prefixes.c[j * 4 + 1] = (*table->prefix)[i * ENTRY_SIZE + j][1];
			table->rainbow[i].prefixes.c[j * 4 + 2] = (*table->prefix)[i * ENTRY_SIZE + j][2];
			table->rainbow[i].prefixes.c[j * 4 + 3] = start;
		}
	}
}

void string_table_fill(string_table_t *table, const char start)
{
	if (table->prefix) {
		table->entries = PREFIX_TABLE_SIZE / ENTRY_SIZE;
		table->prefix_length = PREFIX_LENGTH;
		prefix_init_string_table(table, start);
	} else {
		table->entries = charset->table_size / ENTRY_SIZE;
		table->prefix_length = MIN_LENGTH - 1;
		charset_init_string_table(table, start);
	}

	for (int k = 4; k < table->length; k++)
		table->suffix.c[k - 4] = table->charset->start;
}

/* Increment a string */
void string_table_increment(string_table_t *table, const char start, const char end)
{
	for (int i = table->length - 1; i > 1; i--) {
		char temp = charset->start;
		char curr = (i < 4) ? table->rainbow[0].prefixes.c[i] : table->suffix.c[i - 4];
		if (i == table->prefix_length && curr == end)
			temp = start;
		else if (curr < charset->end)
			temp = curr + 1;

		if (i < 4) {
			for (int j = 0; j < table->entries; j++) {
				table->rainbow[j].prefixes.c[i] = temp;
				table->rainbow[j].prefixes.c[i + 4] = temp;
				table->rainbow[j].prefixes.c[i + 8] = temp;
				table->rainbow[j].prefixes.c[i + 12] = temp;
			}
		} else
			table->suffix.c[i - 4] = temp;

		if (i == table->prefix_length && temp == start)
			break;
		else if (temp != charset->start)
			return;
	}

	table->length++;
	if (table->length <= 4) {
		for (int j = 0; j < table->entries; j++) {
			table->rainbow[j].prefixes.c[table->length - 1] = charset->start;
			table->rainbow[j].prefixes.c[table->length + 3] = charset->start;
			table->rainbow[j].prefixes.c[table->length + 7] = charset->start;
			table->rainbow[j].prefixes.c[table->length + 11] = charset->start;
		}
	} else
		table->suffix.c[table->length - 5] = charset->start;
}

