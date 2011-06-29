#include <stdio.h>
#include <stdlib.h>
#include "rlib/rstring.h"
#include "rlib/rhash.h"
#include "rlib/rmem.h"
#include "rvm/rvmcodemap.h"


void codelabel_print_info(rvm_codemap_t *codemap, rchar* name)
{
	rvm_codelabel_t *label = rvm_codemap_label(codemap, rvm_codemap_lookup_s(codemap, name));

	if (!label)
		fprintf(stdout, "%s (not found)\n", name);
	else
		fprintf(stdout, "%s, asmins: 0x%lx\n", label->name->str, (rulong)label->value);
}


int main(int argc, char *argv[])
{
	rvm_codemap_t *codemap = rvm_codemap_create();

	rvm_codemap_addoffset_s(codemap, "add2", rvm_codemap_lookupadd_s(codemap, ".code"), 0);
	rvm_codemap_addoffset_s(codemap, "add3", rvm_codemap_lookupadd_s(codemap, ".code"), 3);
	rvm_codemap_addoffset_s(codemap, "sub2", rvm_codemap_lookupadd_s(codemap, ".code"), 7);

	codelabel_print_info(codemap, "add2");
	codelabel_print_info(codemap, "add7");
	codelabel_print_info(codemap, "sub2");

	rvm_codemap_destroy(codemap);

	fprintf(stdout, "Max alloc mem: %ld\n", r_debug_get_maxmem());
	fprintf(stdout, "Leaked mem: %ld\n", r_debug_get_allocmem());
	return 0;
}
