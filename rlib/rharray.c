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

	for (index = 0; index < r_array_length(array); index++) {
		src = r_array_index(array, index, rstr_t*);
		if (src)
			dst = r_rstrdup(src->str, src->size);
		r_array_replace(array, index, &dst);
	}
}

/*
 * Destroy the names in the hashed array.
 */
static void r_array_oncleanup_rstr(rarray_t *array)
{
	ruint index;
	rstr_t *src;

	for (index = 0; index < r_array_length(array); index++) {
		src = r_array_index(array, index, rstr_t*);
		if (src)
			r_free(src);
	}
}


robject_t *r_harray_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy, ruint elt_size)
{
	rharray_t *harray = (rharray_t*)obj;
	r_object_init(obj, type, cleanup, copy);
	harray->hash = r_hash_create(5, r_hash_rstrequal, r_hash_rstrhash);
	harray->members = r_carray_create(elt_size);
	harray->names = r_array_create(sizeof(rstr_t*));
	harray->names->oncleanup = r_array_oncleanup_rstr;
	harray->names->oncopy = r_array_oncopy_rstr;
	return obj;
}

rharray_t *r_harray_create(ruint elt_size)
{
	rharray_t *harray;

	harray = (rharray_t*)r_object_create(sizeof(*harray));
	r_harray_init((robject_t*)harray, R_OBJECT_HARRAY,r_harray_cleanup, r_harray_copy, elt_size);
	return harray;
}


robject_t *r_harray_copy(const robject_t *obj)
{
	rharray_t *harray;
	int i;
	rstr_t *n;
	const rharray_t *src = (const rharray_t *)obj;

	harray = (rharray_t*)r_object_create(sizeof(*harray));
	r_object_init(&harray->obj, R_OBJECT_HARRAY, r_harray_cleanup, r_harray_copy);
	harray->names = (rarray_t*)r_array_copy((robject_t*)src->names);
	harray->members = (rcarray_t*)r_carray_copy((robject_t*)src->members);
	harray->hash = r_hash_create(5, r_hash_rstrequal, r_hash_rstrhash);
	for (i = 0; i < r_carray_length(src->members); i++) {
		n = r_array_index(harray->names, i, rstr_t*);
		r_hash_insert_indexval(harray->hash, (rconstpointer)n, i);
	}
	return (robject_t*)harray;
}


void r_harray_cleanup(robject_t *obj)
{
	rharray_t *harray = (rharray_t *)obj;
	r_object_destroy((robject_t*)harray->members);
	r_object_destroy((robject_t*)harray->names);
	r_object_destroy((robject_t*)harray->hash);
	r_object_cleanup(&harray->obj);
}


rlong r_harray_add(rharray_t *harray, const rchar *name, rsize_t namesize, rconstpointer pval)
{
	rstr_t *membrName;
	rlong index, nameindex;

	membrName = r_rstrdup(name, namesize);
	index = r_carray_add(harray->members, pval);
	nameindex = r_array_add(harray->names, &membrName);
	/*
	 * Lets try to keep the name index and the data index in sync,
	 * if they are not, that might be a problem - we will have to
	 * think of some sort reverse lookup mechanism.
	 */
	R_ASSERT(index == nameindex);
	r_hash_insert_indexval(harray->hash, (rconstpointer)membrName, index);
	return index;
}


rlong r_harray_add_s(rharray_t *harray, const rchar *name, rconstpointer pval)
{
	return r_harray_add(harray, name, r_strlen(name), pval);
}


rlong r_harray_replace(rharray_t *harray, const rchar *name, rsize_t namesize, rconstpointer pval)
{
	rlong index = r_harray_lookup(harray, name, namesize);

	if (index < 0)
		return r_harray_add(harray, name, namesize, pval);
	index = r_carray_replace(harray->members, index,  pval);
	return index;
}

rlong r_harray_replace_s(rharray_t *harray, const rchar *name, rconstpointer pval)
{
	return r_harray_replace(harray, name, r_strlen(name), pval);
}


rlong r_harray_lookup(rharray_t *harray, const rchar *name, rsize_t namesize)
{
	rsize_t found;

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


rlong r_harray_taillookup(rharray_t *harray, const rchar *name, rsize_t namesize)
{
	rsize_t found;

	rstr_t lookupstr = {(char*)name, namesize};
	found = r_hash_taillookup_indexval(harray->hash, &lookupstr);
	if (found == R_HASH_INVALID_INDEXVAL)
		return -1;
	return (rlong)found;
}


rlong r_harray_taillookup_s(rharray_t *harray, const rchar *name)
{
	return r_harray_lookup(harray, name, r_strlen(name));
}



rhash_node_t* r_harray_nodelookup(rharray_t *harray, rhash_node_t *cur, const rchar *name, rsize_t namesize)
{
	rstr_t lookupstr = {(char*)name, namesize};
	return r_hash_nodelookup(harray->hash, cur, &lookupstr);
}


rhash_node_t* r_harray_nodelookup_s(rharray_t *harray, rhash_node_t *cur, const rchar *name)
{
	return r_harray_nodelookup(harray, cur, name, r_strlen(name));
}


rhash_node_t* r_harray_nodetaillookup(rharray_t *harray, rhash_node_t *cur, const rchar *name, rsize_t namesize)
{
	rstr_t lookupstr = {(char*)name, namesize};
	return r_hash_nodetaillookup(harray->hash, cur, &lookupstr);
}


rhash_node_t* r_harray_nodetaillookup_s(rharray_t *harray, rhash_node_t *cur, const rchar *name)
{
	return r_harray_nodetaillookup(harray, cur, name, r_strlen(name));
}


rint r_harray_set(rharray_t *harray, rlong index, rconstpointer pval)
{
	if (index < 0)
		return -1;
	r_carray_replace(harray->members, index, pval);
	return 0;
}


rpointer r_harray_get(rharray_t *harray, rsize_t index)
{
	if (index >= r_carray_length(harray->members) || index < 0)
		return NULL;
	return r_carray_slot(harray->members, index);
}


