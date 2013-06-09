#include "charsets.h"

int main()
{
	struct charset *c = &charset_numeric;
	struct guess *g = NULL;
	size_t size = sizeof(struct guess);

	/* Basic initialization */
	c->init(3);
	g = (struct guess *)malloc(size);
	if (g == NULL) {
		printf("Failed to allocate\n");
		return 1;
	}

	printf("Running through 10,000 strings with blocksize 1\n");

	for (long long i = 0; i < 10000; i++) {
		c->next(g);
		printf("String: %s (%d)\n", g->string, g->length);
	}

	free(g);
	c->destroy();
	return 0;
}
