#include "rvmscope.h"
#include "rstring.h"
#include "rmem.h"

rvm_scope_t *rvm_scope_create()
{
	rvm_scope_t *scope;

	scope = (rvm_scope_t*)r_malloc(sizeof(*scope));
	if (!scope)
		return NULL;
	r_memset(scope, 0, sizeof(*scope));
	scope->names = r_array_create(sizeof(char*));
	scope->nameshash = r_hash_create(5, r_hash_strequal, r_hash_strhash);
	scope->varstack = r_array_create(sizeof(rvm_varmap_t));
	scope->scopestack = r_array_create(sizeof(ruint32));
	return scope;
}


void rvm_scope_destroy(rvm_scope_t *scope)
{
	int i;
	int len = scope->names->len;

	for (i = 0; i < len; i++)
		r_free(r_array_index(scope->names, i, rchar*));
	r_array_destroy(scope->names);
	r_array_destroy(scope->varstack);
	r_array_destroy(scope->scopestack);
	r_hash_destroy(scope->nameshash);
	r_free(scope);
}


void rvm_scope_addname(rvm_scope_t *scope, const rchar* name)
{
	rchar *dupname;

	if (!r_hash_lookup(scope->nameshash, name)) {
		dupname = r_strdup(name);
		r_array_add(scope->names, (rconstpointer)&dupname);
		r_hash_insert(scope->nameshash, name, dupname);
	}
}


void rvm_scope_push(rvm_scope_t* scope)
{

}


void rvm_scope_pop(rvm_scope_t* scope)
{

}


rvm_varmap_t *rvm_scope_lookup(rvm_scope_t *scope, const rchar *name)
{
	return NULL;
}


