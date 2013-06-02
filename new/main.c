#include "charset_numeric.h"
#include "charset_lowercase.h"

int main()
{
	struct charset *c = &charset_numeric;
	struct guess *g = NULL;

	c->init(3);

	for (int i = 0; i < 10000; i++) {
		g = c->next();
		printf("String: %s (%d)\n", g->string, g->length);
		free(g);
	}

	c->destroy();
	return 0;
}
