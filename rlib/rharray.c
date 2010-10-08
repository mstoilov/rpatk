#include "rharray.h"
#include "rstring.h"
#include "rmem.h"


static void r_refstub_destroy(rref_t *ref)
{
	r_harray_destroy((rharray_t*)ref);
}


static rref_t *r_refstub_copy(const rref_t *ptr)
{
	return (rref_t*) r_harray_copy((const rharray_t*)ptr);
}


rharray_t *r_harray_create(ruint elt_size, r_array_destroyelt_fun destroy, r_array_copyelt_fun copy)
{
	rharray_t *harray;

	harray = (rharray_t*)r_malloc(sizeof(*harray));
	if (!harray)
		return NULL;
	r_memset(harray, 0, sizeof(*harray));
	harray->members = r_array_create(elt_size, destroy, copy);
	harray->names = r_array_create(sizeof(rstr_t*), NULL, NULL);
	harray->hash = r_hash_create(5, r_hash_strnequal, r_hash_strnhash);
	r_ref_init(&harray->ref, 1, RREF_TYPE_NONE, r_refstub_destroy, r_refstub_copy);
	return harray;
}


rharray_t *r_harray_copy(const rharray_t *src)
{
	int i;
	rpointer m;
	rstr_t *n;
	rharray_t *dst = r_harray_create(src->members->elt_size, src->members->destroy, src->members->copy);

	if (!dst)
		return NULL;
	for (i = 0; i < src->members->len; i++) {
		m = r_array_slot(src->members, i);
		n = r_array_index(src->names, i, rstr_t*);
		r_harray_add(dst, n->str, n->size, m);
	}
	return dst;
}


void r_harray_destroy(rharray_t *harray)
{
	r_array_destroy(harray->members);
	r_array_destroy(harray->names);
	r_hash_destroy(harray->hash);
	r_free(harray);
}


rint r_harray_add(rharray_t *harray, const rchar *name, ruint namesize, rconstpointer pval)
{
	rstr_t *membrName;
	rint index;

	membrName = r_rstrdup(name, namesize);
	index = r_array_add(harray->members, pval);
	r_array_add(harray->names, &membrName);
	r_hash_insert_indexval(harray->hash, (rconstpointer)membrName, index);
	return index;
}


rint r_harray_stradd(rharray_t *harray, const rchar *name, rconstpointer pval)
{
	return r_harray_add(harray, name, r_strlen(name), pval);
}


rint r_harray_set(rharray_t *harray, const rchar *name, ruint namesize, rconstpointer pval)
{
	rlong index = r_harray_lookup_index(harray, name, namesize);
	if (index < 0)
		return -1;
	r_array_insert(harray->members, index, pval);
	return 0;
}


rint r_harray_strset(rharray_t *harray, const rchar *name, rconstpointer pval)
{
	return r_harray_set(harray, name, r_strlen(name), pval);
}


rlong r_harray_lookup_index(rharray_t *harray, const rchar *name, ruint namesize)
{
	rulong found;

	rstr_t lookupstr = {(char*)name, namesize};
	found = r_hash_lookup_indexval(harray->hash, &lookupstr);
	if (found == R_HASH_INVALID_INDEXVAL)
		return -1;
	return (rlong)found;
}



rlong r_harray_strlookup_index(rharray_t *harray, const rchar *name)
{
	return r_harray_lookup_index(harray, name, r_strlen(name));
}


rpointer r_harray_lookup(rharray_t *harray, const rchar *name, ruint namesize)
{
	rulong found = r_harray_lookup_index(harray, name, namesize);
	if (found == R_HASH_INVALID_INDEXVAL)
		return NULL;
	return r_array_slot(harray->members, found);
}


rpointer r_harray_strlookup(rharray_t *harray, const rchar *name)
{
	return r_harray_lookup(harray, name, r_strlen(name));
}
