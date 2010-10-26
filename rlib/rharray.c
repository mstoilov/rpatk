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


static void r_array_oncopy_rstr(rarray_t *array)
{
	ruint index;
	rstr_t *src, *dst;


	for (index = 0; index < array->len; index++) {
		src = r_array_index(array, index, rstr_t*);
		if (src)
			dst = r_rstrdup(src->str, src->size);
		r_array_replace(array, index, &dst);
	}
}


static void r_array_ondestroy_rstr(rarray_t *array)
{
	ruint index;
	rstr_t *src;


	for (index = 0; index < array->len; index++) {
		src = r_array_index(array, index, rstr_t*);
		if (src)
			r_free(src);
	}
}


rharray_t *r_harray_create(ruint elt_size)
{
	rharray_t *harray;

	harray = (rharray_t*)r_malloc(sizeof(*harray));
	if (!harray)
		return NULL;
	r_memset(harray, 0, sizeof(*harray));
	harray->members = r_array_create(elt_size);
	harray->names = r_array_create(sizeof(rstr_t*));
	harray->names->ondestroy = r_array_ondestroy_rstr;
	harray->names->oncopy = r_array_oncopy_rstr;
	harray->hash = r_hash_create(5, r_hash_strnequal, r_hash_strnhash);
	r_ref_init(&harray->ref, 1, RREF_TYPE_NONE, r_refstub_destroy, r_refstub_copy);
	return harray;
}


rharray_t *r_harray_copy(const rharray_t *src)
{
	rharray_t *harray;
	int i;
	rstr_t *n;

	harray = (rharray_t*)r_malloc(sizeof(*harray));
	if (!harray)
		return NULL;
	harray->names = r_array_copy(src->names);
	harray->members = r_array_copy(src->members);
	harray->hash = r_hash_create(5, r_hash_strnequal, r_hash_strnhash);
	for (i = 0; i < src->members->len; i++) {
		n = r_array_index(harray->names, i, rstr_t*);
		r_hash_insert_indexval(harray->hash, (rconstpointer)n, i);
	}
	r_ref_init(&harray->ref, 1, RREF_TYPE_NONE, r_refstub_destroy, r_refstub_copy);
	return harray;
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
	r_array_replace(harray->members, index, pval);
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