#include <stdio.h>
#include <stdlib.h>
#include "rarray.h"
#include "rmem.h"


static void r_array_addint(rarray_t *array, int elt)
{
	r_array_add(array, &elt);
}


int main(int argc, char *argv[])
{
	int i;
	rarray_t *a;

	a = r_array_create(sizeof(int));
	for (i = 0; i < 10; i++) {
		r_array_addint(a, i);
	}
	i = 100;
	r_array_insert(a, 5, &i);
	r_array_insert(a, 4, &i);
	r_array_insert(a, 20, &i);
	r_array_replace(a, 19, &i);
	i = 99;
	r_array_replace(a, 19, &i);

	for (i = 0; i < a->len; i++) {
		fprintf(stdout, "index %d: %d\n", i, *((int*)r_array_slot(a, i)));
	}

	r_array_destroy(a);

	fprintf(stdout, "Max alloc mem: %ld\n", r_debug_get_maxmem());
	fprintf(stdout, "Leaked mem: %ld\n", r_debug_get_allocmem());
	return 0;
}
