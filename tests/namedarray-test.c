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
	rvm_namedarray_t *na;

	na = rvm_namedarray_create();
	rvm_namedarray_add_str(na, "again");
	rvm_namedarray_add_str(na, "hello");
	rvm_namedarray_add_str(na, "there");

	fprintf(stdout, "lookup 'again': %ld\n", rvm_namedarray_lookup_str(na, "again"));
	fprintf(stdout, "lookup 'hello': %ld\n", rvm_namedarray_lookup_str(na, "hello"));
	fprintf(stdout, "lookup 'there': %ld\n", rvm_namedarray_lookup_str(na, "there"));



	rvm_namedarray_destroy(na);
	fprintf(stdout, "Max alloc mem: %ld\n", r_debug_get_maxmem());
	fprintf(stdout, "Leaked mem: %ld\n", r_debug_get_allocmem());
	return 0;
}
