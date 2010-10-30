#include <stdio.h>
#include <stdlib.h>
#include "rstring.h"
#include "rhash.h"
#include "rmem.h"
#include "rvmscope.h"


void print_var_info(rvm_scope_t *scope, rchar* varname)
{
	rvm_varmap_t *vmap;

	vmap = rvm_scope_lookuptip_s(scope, varname);
	if (vmap) {
		if (vmap && vmap->datatype == VARMAP_DATATYPE_OFFSET)
			fprintf(stdout, "tip: %s, offset: %d\n", vmap->name, vmap->data.offset);
		else if (vmap && vmap->datatype == VARMAP_DATATYPE_PTR)
			fprintf(stdout, "tip: %s, ptr: 0x%p\n", vmap->name, vmap->data.ptr);
		return;
	}

	vmap = rvm_scope_lookup_s(scope, varname);
	if (!vmap)
		fprintf(stdout, "%s (not found)\n", varname);
	else if (vmap && vmap->datatype == VARMAP_DATATYPE_OFFSET)
		fprintf(stdout, "%s, offset: %d\n", vmap->name, vmap->data.offset);
	else if (vmap && vmap->datatype == VARMAP_DATATYPE_PTR)
		fprintf(stdout, "%s, ptr: 0x%p\n", vmap->name, vmap->data.ptr);
	else
		fprintf(stdout, "unkown\n");
}


int main(int argc, char *argv[])
{
	int a[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	rvm_scope_t *scope = rvm_scope_create();


	rvm_scope_addstrname(scope, "a1");
	rvm_scope_addstrname(scope, "a");
	rvm_scope_addstrname(scope, "a");
	rvm_scope_addstrname(scope, "ab");
	rvm_scope_addstrname(scope, "abcd");
	rvm_scope_addstrname(scope, "abce");
	rvm_scope_addstrname(scope, "abcf");

	rvm_scope_addoffset(scope, "a0", r_strlen("a0"), 0);
	rvm_scope_addoffset(scope, "a1", r_strlen("a1"), 1);
	rvm_scope_addoffset(scope, "a2", r_strlen("a2"), 2);
	rvm_scope_addpointer(scope, "a5", r_strlen("a5"), &a[5]);
	print_var_info(scope, "a0");
	print_var_info(scope, "a2");
	print_var_info(scope, "a5");


	rvm_scope_push(scope);
	rvm_scope_addoffset(scope, "a0", r_strlen("a0"), 10);
	rvm_scope_addoffset(scope, "a1", r_strlen("a1"), 11);
	rvm_scope_addoffset(scope, "a2", r_strlen("a2"), 12);

	rvm_scope_push(scope);
	rvm_scope_addoffset(scope, "a0", r_strlen("a0"), 101);
	rvm_scope_addoffset(scope, "a1", r_strlen("a1"), 112);
	rvm_scope_addoffset(scope, "a2", r_strlen("a2"), 223);

	print_var_info(scope, "a0");
	print_var_info(scope, "a2");
	print_var_info(scope, "a5");

	rvm_scope_pop(scope);
	print_var_info(scope, "a0");
	print_var_info(scope, "a2");
	print_var_info(scope, "a5");

	rvm_scope_pop(scope);
	print_var_info(scope, "a0");
	print_var_info(scope, "a2");
	print_var_info(scope, "a5");


	rvm_scope_destroy(scope);



	fprintf(stdout, "Max alloc mem: %ld\n", r_debug_get_maxmem());
	fprintf(stdout, "Leaked mem: %ld\n", r_debug_get_allocmem());
	return 0;
}
