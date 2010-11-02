#include "rharray.h"
#include "rstring.h"
#include "rmem.h"

/*
 * Hash array
 *
 * names array:
 * ----------------------------------------------
 * | name1  | name2  | name3  |        | nameN  |
 * ----------------------------------------------
 *
 * members array:
 * ----------------------------------------------
 * | memb1  | memb2  | memb3  |        | membN  |
 * ----------------------------------------------
 *
 * Hash Entry:
 * --------------------
 * | pointer to name1  |
 * --------------------
 * | index of memb1    |
 * --------------------
 *
 *
 */

static void r_refstub_destroy(rref_t *ref)
{
	r_harray_destroy((rharray_t*)ref);
}


static rref_t *r_refstub_copy(const rref_t *ptr)
{
	return (rref_t*) r_harray_copy((const rharray_t*)ptr);
}

/*
 * Copy the names in the hashed array.
 * Pointers to the names are already in the array, but they point
 * to the source array names. We duplicate these source names and
 * replace the entries.
 */
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

/*
 * Destroy the names in the hashed array.
 */
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
	harray->hash = r_hash_create(5, r_hash_rstrequal, r_hash_rstrhash);
	r_ref_init(&harray->ref, 1, RREF_TYPE_COW, r_refstub_destroy, r_refstub_copy);
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
	harray->hash = r_hash_create(5, r_hash_rstrequal, r_hash_rstrhash);
	for (i = 0; i < src->members->len; i++) {
		n = r_array_index(harray->names, i, rstr_t*);
		r_hash_insert_indexval(harray->hash, (rconstpointer)n, i);
	}
	r_ref_init(&harray->ref, 1, RREF_TYPE_COW, r_refstub_destroy, r_refstub_copy);
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


rint r_harray_add_s(rharray_t *harray, const rchar *name, rconstpointer pval)
{
	return r_harray_add(harray, name, r_strlen(name), pval);
}


rlong r_harray_lookup(rharray_t *harray, const rchar *name, ruint namesize)
{
	rulong found;

	rstr_t lookupstr = {(char*)name, namesize};
	found = r_hash_lookup_indexval(harray->hash, &lookupstr);
	if (found == R_HASH_INVALID_INDEXVAL)
		return -1;
	return (rlong)found;
}


rlong r_harray_lookup_s(rharray_t *harray, const rchar *name)
{
	return r_harray_lookup(harray, name, r_strlen(name));
}


rhash_node_t* r_harray_nodelookup(rharray_t *harray, rhash_node_t *cur, const rchar *name, ruint namesize)
{
	rstr_t lookupstr = {(char*)name, namesize};
	return r_hash_nodelookup(harray->hash, cur, &lookupstr);
}


rhash_node_t* r_harray_nodelookup_s(rharray_t *harray, rhash_node_t *cur, const rchar *name)
{
	return r_harray_nodelookup(harray, cur, name, r_strlen(name));
}


rint r_harray_set(rharray_t *harray, rlong index, rconstpointer pval)
{
	if (index < 0)
		return -1;
	r_array_replace(harray->members, index, pval);
	return 0;
}


rpointer r_harray_get(rharray_t *harray, rulong index)
{
	if (index >= r_array_size(harray->members))
		return NULL;
	return r_array_slot(harray->members, index);
}


