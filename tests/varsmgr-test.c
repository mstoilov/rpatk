#include <stdio.h>
#include <stdlib.h>
#include "rhash.h"
#include "rmem.h"
#include "rvmvarsmgr.h"


int main(int argc, char *argv[])
{
	rvm_varsmgr_t *vmgr = rvm_varsmgr_create();

	rvm_varsmgr_addvar(vmgr, "a");
	rvm_varsmgr_addvar(vmgr, "ab");
	rvm_varsmgr_addvar(vmgr, "abc");

	fprintf(stdout, "key: %s, value: %s\n", "a", ((rchar*)r_hash_lookup(vmgr->nameshash, "a")));
	fprintf(stdout, "key: %s, value: %s\n", "abc", ((rchar*)r_hash_lookup(vmgr->nameshash, "abc")));
	fprintf(stdout, "key: %s, value: %s\n", "ddd", ((rchar*)r_hash_lookup(vmgr->nameshash, "ddd")));

	rvm_varsmgr_destroy(vmgr);



	fprintf(stdout, "Max alloc mem: %ld\n", r_debug_get_maxmem());
	fprintf(stdout, "Leaked mem: %ld\n", r_debug_get_allocmem());
	return 0;
}
