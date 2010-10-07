#include <stdio.h>
#include <stdlib.h>
#include "rvmcodegen.h"
#include "rvmnamedarray.h"
#include "rstring.h"
#include "rmem.h"
#include "rvmcpu.h"



int main(int argc, char *argv[])
{
	rchar *hello = "Hello World";
	rchar *there = ", right there";
	rvm_namedarray_t *na, *nc;

	na = rvm_namedarray_create();
	rvm_namedarray_stradd(na, "again", NULL);
	rvm_namedarray_stradd(na, "hello", NULL);
	rvm_namedarray_stradd(na, "there", NULL);
	nc = (rvm_namedarray_t*)r_ref_copy(&na->ref);

	fprintf(stdout, "lookup 'again': %ld\n", rvm_namedarray_strlookup(nc, "again"));
	fprintf(stdout, "lookup 'hello': %ld\n", rvm_namedarray_strlookup(nc, "hello"));
	fprintf(stdout, "lookup 'there': %ld\n", rvm_namedarray_strlookup(nc, "there"));

	r_ref_dec((rref_t*)na);
	r_ref_dec((rref_t*)nc);
	fprintf(stdout, "Max alloc mem: %ld\n", r_debug_get_maxmem());
	fprintf(stdout, "Leaked mem: %ld\n", r_debug_get_allocmem());
	return 0;
}
