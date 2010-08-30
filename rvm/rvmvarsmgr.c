#include "rvmvarsmgr.h"
#include "rstring.h"
#include "rmem.h"

rvm_varsmgr_t *rvm_varsmgr_create()
{
	rvm_varsmgr_t *varsmgr;

	varsmgr = (rvm_varsmgr_t*)r_malloc(sizeof(*varsmgr));
	if (!varsmgr)
		return NULL;
	r_memset(varsmgr, 0, sizeof(*varsmgr));
	varsmgr->names = r_array_create(sizeof(char*));
	varsmgr->nameshash = r_hash_create(5, r_hash_strequal, r_hash_strhash);
	return varsmgr;
}


void rvm_varsmgr_destroy(rvm_varsmgr_t *varsmgr)
{
	int i;
	int len = varsmgr->names->len;

	for (i = 0; i < len; i++)
		r_free(r_array_index(varsmgr->names, i, rchar*));
	r_array_destroy(varsmgr->names);
	r_hash_destroy(varsmgr->nameshash);
	r_free(varsmgr);
}


void rvm_varsmgr_addvar(rvm_varsmgr_t *varsmgr, const rchar* varname)
{
	rchar *dupname;

	if (!r_hash_lookup(varsmgr->nameshash, varname)) {
		dupname = r_strdup(varname);
		r_array_add(varsmgr->names, (rconstpointer)&dupname);
		r_hash_insert(varsmgr->nameshash, varname, dupname);
	}
}


