#include "charsets.h"
#include "cpus.h"

struct charset *c = &charset_all;
int blocksize = 0;
long per_thread = 0;

static void *do_work(void *p)
{
	struct guess *g = NULL;
	size_t size = sizeof(struct guess);

	/* Allocation block */
	g = (struct guess *)malloc(size * blocksize);
	if (g == NULL) {
		printf("Failed to allocate\n");
		return NULL;
	}

	/* Get the data */
	for (long long i = 0; i < per_thread/blocksize; i++)
		c->next_block(g, blocksize);

	/* Cleanup */
	free(g);
}

int main(int argc, char *argv[])
{
	if (argc != 2 && argc != 3) {
		printf("Wrong arguments\n");
		exit(1);
	}

	/* Basic initialization */
	blocksize = atoi(argv[1]);
	c->init(3);
	int cpus = num_cpus();
	if (argc == 3)
		cpus = atoi(argv[2]);
	per_thread = 1000000000/cpus;

	/* Do the calculations */
	pthread_t *t;
	printf("Running through 1,000,000,000 strings with blocksize %d\n", blocksize);
	printf("Running on %d threads\n", cpus);

	/* Allocate the pthread structures */
	t = (pthread_t *)malloc(cpus * sizeof(pthread_t));
	if (!t) {
		printf("Failed to allocate memory\n");
		return 1;
	}

	/* Do some actual work */
	for (int i = 0; i < cpus; i++)
		pthread_create(&t[i], NULL, do_work, (void *)(unsigned long)i);

	/* CLenaup threads */
	for (int i = 0; i < cpus; i++)
		pthread_join(t[i], NULL);

	/* Cleanup */
	c->destroy();
	return 0;
}
