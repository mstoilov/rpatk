#include "rvm/rvmscope.h"
#include "rlib/rstring.h"
#include "rlib/rmem.h"


rvm_scope_t *rvm_scope_create()
{
	rvm_scope_t *scope;

	scope = (rvm_scope_t*)r_malloc(sizeof(*scope));
	if (!scope)
		return NULL;
	r_memset(scope, 0, sizeof(*scope));
	scope->names = r_array_create(sizeof(rstr_t*));
	scope->nameshash = r_hash_create(5, r_hash_rstrequal, r_hash_rstrhash);
	scope->varstack = r_array_create(sizeof(rvm_varmap_t));
	scope->scopestack = r_array_create(sizeof(scope->varstack->len));
	return scope;
}


void rvm_scope_destroy(rvm_scope_t *scope)
{
	int i;
	int len = r_array_length(scope->names);

	for (i = 0; i < len; i++)
		r_free(r_array_index(scope->names, i, rchar*));
	r_object_destroy((robject_t*)scope->names);
	r_object_destroy((robject_t*)scope->varstack);
	r_object_destroy((robject_t*)scope->scopestack);
	r_object_destroy((robject_t*)scope->nameshash);
	r_free(scope);
}


rchar *rvm_scope_addname(rvm_scope_t *scope, const rchar* name, ruinteger namesize)
{
	rstr_t namestr = {(rchar*)name, namesize};
	rstr_t *dupname = r_hash_lookup(scope->nameshash, &namestr);

	if (!dupname) {
		dupname = r_rstrdup(name, namesize);
		r_array_add(scope->names, (rconstpointer)&dupname);
		r_hash_insert(scope->nameshash, dupname, dupname);
	}
	return dupname->str;
}


rchar *rvm_scope_addstrname(rvm_scope_t *scope, const rchar *name)
{
	return rvm_scope_addname(scope, name, r_strlen(name));
}


ruinteger rvm_scope_count(rvm_scope_t* scope)
{
	return r_array_length(scope->scopestack);
}


void rvm_scope_push(rvm_scope_t* scope)
{
	rsize_t scopelen = r_array_length(scope->varstack);
//	r_printf("SCOPE FRAME: %d, PUSH: %d\n", r_array_length(scope->scopestack), scopelen);
	r_array_add(scope->scopestack, &scopelen);
}


void rvm_scope_pop(rvm_scope_t* scope)
{
	rsize_t scopelen = r_array_index(scope->scopestack, r_array_length(scope->scopestack) - 1, rsize_t);
	r_array_removelast(scope->scopestack);
	r_array_setlength(scope->varstack, scopelen);
//	r_printf("SCOPE FRAME: %d, POP: %d\n", r_array_length(scope->scopestack), scopelen);
}


void rvm_scope_addoffset(rvm_scope_t *scope, const rchar *name, ruinteger namesize, ruint32 off)
{
	rvm_varmap_t vmap;

	vmap.name = rvm_scope_addname(scope, name, namesize);
	vmap.data.offset = off;
	vmap.datatype = VARMAP_DATATYPE_OFFSET;
	r_array_add(scope->varstack, &vmap);
}


ruinteger rvm_scope_numentries(rvm_scope_t *scope)
{
	return r_array_length(scope->varstack);
}


void rvm_scope_addpointer(rvm_scope_t *scope, const rchar *name, ruinteger namesize, rpointer ptr)
{
	rvm_varmap_t vmap;

	vmap.name = rvm_scope_addname(scope, name, namesize);
	vmap.data.ptr = ptr;
	vmap.datatype = VARMAP_DATATYPE_PTR;
	r_array_add(scope->varstack, &vmap);
}


void rvm_scope_addoffset_s(rvm_scope_t *scope, const rchar *name, ruint32 off)
{
	rvm_scope_addoffset(scope, name ,r_strlen(name), off);
}


void rvm_scope_addpointer_s(rvm_scope_t *scope, const rchar *name, rpointer ptr)
{
	rvm_scope_addpointer(scope, name ,r_strlen(name), ptr);
}


rvm_varmap_t *rvm_scope_lookup(rvm_scope_t *scope, const rchar *name, ruinteger namesize)
{
	rsize_t scopelen = r_array_length(scope->varstack);
	rvm_varmap_t *varmap;
	rinteger i;

	if (!scopelen)
		return NULL;

	for (i = scopelen - 1; i >= 0; i--) {
		varmap = (rvm_varmap_t*)r_array_slot(scope->varstack, i);
		if (r_strlen(varmap->name) == namesize && r_strncmp(varmap->name, name, namesize) == 0)
			return varmap;
	}
	return NULL;
}


rvm_varmap_t *rvm_scope_tiplookup(rvm_scope_t *scope, const rchar *name, ruinteger namesize)
{
	rsize_t scopelen = r_array_length(scope->varstack);
	ruinteger tipstart = r_array_empty(scope->scopestack) ? 0 : r_array_last(scope->scopestack, ruinteger);
	rvm_varmap_t *varmap;
	rinteger i;

	if (!scopelen)
		return NULL;
	for (i = scopelen - 1; i >= (rinteger)tipstart; i--) {
		varmap = (rvm_varmap_t*)r_array_slot(scope->varstack, i);
		if (r_strlen(varmap->name) == namesize && r_strncmp(varmap->name, name, namesize) == 0)
			return varmap;
	}
	return NULL;
}


rvm_varmap_t *rvm_scope_lookup_s(rvm_scope_t *scope, const rchar *name)
{
	return rvm_scope_lookup(scope, name, r_strlen(name));
}


rvm_varmap_t *rvm_scope_tiplookup_s(rvm_scope_t *scope, const rchar *name)
{
	return rvm_scope_tiplookup(scope, name, r_strlen(name));
}
