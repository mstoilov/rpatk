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
	scope->scopestack = r_array_create(sizeof(scope->varstack->len));
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


rchar *rvm_scope_addname(rvm_scope_t *scope, const rchar* name)
{
	rchar *dupname = r_hash_lookup(scope->nameshash, name);

	if (!dupname) {
		dupname = r_strdup(name);
		r_array_add(scope->names, (rconstpointer)&dupname);
		r_hash_insert(scope->nameshash, name, dupname);
	}
	return dupname;
}


void rvm_scope_push(rvm_scope_t* scope)
{
	ruint scopesize = scope->varstack->len;
	r_array_add(scope->scopestack, &scopesize);
}


void rvm_scope_pop(rvm_scope_t* scope)
{
	ruint scopesize = r_array_index(scope->scopestack, scope->scopestack->len - 1, ruint);
	r_array_setsize(scope->scopestack, scope->scopestack->len - 1);
	r_array_setsize(scope->varstack, scopesize);
}


void rvm_scope_addoffset(rvm_scope_t *scope, const rchar *name, ruint32 off)
{
	rvm_varmap_t vmap;

	vmap.name = rvm_scope_addname(scope, name);
	vmap.data.offset = off;
	vmap.datatype = VARMAP_DATATYPE_OFFSET;
	r_array_add(scope->varstack, &vmap);
}


void rvm_scope_addpointer(rvm_scope_t *scope, const rchar *name, rpointer ptr)
{
	rvm_varmap_t vmap;

	vmap.name = rvm_scope_addname(scope, name);
	vmap.data.ptr = ptr;
	vmap.datatype = VARMAP_DATATYPE_PTR;
	r_array_add(scope->varstack, &vmap);
}


rvm_varmap_t *rvm_scope_lookup(rvm_scope_t *scope, const rchar *name)
{
	ruint scopesize = scope->varstack->len;
	rvm_varmap_t *varmap;
	rint i;

	for (i = scopesize - 1; i >= 0; i--) {
		varmap = (rvm_varmap_t*)r_array_slot(scope->varstack, i);
		if (r_strcmp(varmap->name, name) == 0)
			return varmap;
	}
	return NULL;
}


