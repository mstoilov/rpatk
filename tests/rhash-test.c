#include <stdio.h>
#include <stdlib.h>
#include "rhash.h"
#include "rmem.h"


int main(int argc, char *argv[])
{
	rhash_t *h;
	rhash_node_t *node = NULL;
	ruint idig[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	rchar *sdig[] = {"zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"};

	h = r_hash_create(5, r_hash_strequal, r_hash_strhash);
	r_hash_insert(h, sdig[0], &idig[0]);
	r_hash_insert(h, sdig[7], &idig[8]);
	r_hash_insert(h, sdig[7], &idig[7]);
	r_hash_insert(h, sdig[5], &idig[5]);
	fprintf(stdout, "key: %s, value: %d\n", "seven", *((ruint*)r_hash_lookup(h, "seven")));
	r_hash_remove(h, sdig[7]);
	if (!r_hash_lookup(h, "seven")) {
		r_hash_insert(h, sdig[7], &idig[7]);
		r_hash_insert(h, sdig[7], &idig[8]);
		r_hash_insert(h, sdig[7], &idig[9]);
	}
	for (node = r_hash_nodelookup(h, node, "seven"); node; node = r_hash_nodelookup(h, node, "seven"))
		fprintf(stdout, "key: %s, value: %d\n", "seven", *((ruint*)r_hash_value(node)));
	fprintf(stdout, "key: %s, value: %d\n", sdig[5], *((ruint*)r_hash_lookup(h, sdig[5])));
	fprintf(stdout, "key: %s, value: %d\n", sdig[0], *((ruint*)r_hash_lookup(h, sdig[0])));

	r_hash_destroy(h);

	fprintf(stdout, "Max alloc mem: %ld\n", r_debug_get_maxmem());
	fprintf(stdout, "Leaked mem: %ld\n", r_debug_get_allocmem());
	return 0;
}
