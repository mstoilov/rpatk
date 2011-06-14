#include <stdio.h>
#include <stdlib.h>
#include "rcarray.h"
#include "rmem.h"


static void r_carray_replaceint(rcarray_t *carray, ruinteger index, rinteger elt)
{
	r_carray_replace(carray, index, &elt);
}


int main(int argc, char *argv[])
{
	int i;
	rcarray_t *a, *b;

	a = r_carray_create(sizeof(int));
//	r_carray_setlength(a, 28);
	for (i = 0; i < 10; i++) {
		r_carray_replaceint(a, i, i);
	}
	i = 100;
	r_carray_replaceint(a, 5, i);
	r_carray_replaceint(a, 4, i);
	r_carray_replaceint(a, 20, i);
	r_carray_replaceint(a, 19, i);
	i = 99;
	r_carray_replaceint(a, 19, i);
	r_carray_setlength(a, 20);

	b = (rcarray_t*)r_object_v_copy((robject_t*)a);
	for (i = 0; i < r_carray_length(b); i++) {
		fprintf(stdout, "index %d: %d\n", i, *((int*)r_carray_slot(b, i)));
	}

	r_object_destroy((robject_t*)a);
	r_object_destroy((robject_t*)b);


	fprintf(stdout, "Max alloc mem: %ld\n", r_debug_get_maxmem());
	fprintf(stdout, "Leaked mem: %ld\n", r_debug_get_allocmem());
	return 0;
}
