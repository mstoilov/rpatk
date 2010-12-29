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
	r_array_push(a, 20, int);
	r_array_push(a, 21, int);
	r_array_push(a, 22, int);
	r_array_push(a, 23, int);
	r_array_push(a, 24, int);

	for (i = 0; i < 5; i++) {
		fprintf(stdout, "r_array_pop %d: %d\n", i, r_array_pop(a, int));
	}


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



	for (i = 0; i < r_array_length(a); i++) {
		fprintf(stdout, "index %d: %d\n", i, *((int*)r_array_slot(a, i)));
	}

	r_object_destroy((robject_t*)a);

	fprintf(stdout, "Max alloc mem: %ld\n", r_debug_get_maxmem());
	fprintf(stdout, "Leaked mem: %ld\n", r_debug_get_allocmem());
	return 0;
}
