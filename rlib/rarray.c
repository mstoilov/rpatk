#include "rarray.h"
#include "rmem.h"


#define MIN_ARRAY_LEN 2

static void r_array_checkexpand(rarray_t *array, ruint index);


void r_array_cleanup(robject_t *obj)
{
	rarray_t *array = (rarray_t *)obj;
	if (array->oncleanup)
		array->oncleanup(array);
	r_free(array->data);
	r_object_cleanup((robject_t*)array);
}


robject_t *r_array_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy, ruint elt_size)
{
	rarray_t *array = (rarray_t*)obj;

	r_object_init(obj, type, cleanup, copy);
	array->elt_size = elt_size;
	array->data = r_zmalloc(MIN_ARRAY_LEN * array->elt_size);
	array->alloc_size = MIN_ARRAY_LEN;
	return obj;
}


rarray_t *r_array_create(ruint elt_size)
{
	rarray_t *array;
	array = (rarray_t*)r_object_create(sizeof(*array));
	r_array_init((robject_t*)array, R_OBJECT_ARRAY, r_array_cleanup, r_array_copy, elt_size);
	return array;
}


robject_t *r_array_copy(const robject_t *obj)
{
	ruint i;
	rarray_t *dst;
	const rarray_t *array = (const rarray_t *)obj;

	if (!array)
		return NULL;
	dst = r_array_create(array->elt_size);
	if (!dst)
		return NULL;
	for (i = 0; i < r_array_length(array); i++)
		r_array_replace(dst, i, r_array_slot(array, i));
	dst->oncopy = array->oncopy;
	dst->oncleanup = array->oncleanup;
	if (dst->oncopy)
		dst->oncopy(dst);
	return (robject_t *)dst;
}


static void r_array_exist_replace(rarray_t *array, ruint index, rconstpointer data)
{
	if (data)
		r_memcpy(r_array_slot(array, index), data, array->elt_size);
	else
		r_memset(r_array_slot(array, index), 0, array->elt_size);
}


rint r_array_add(rarray_t *array, rconstpointer data)
{
	rint index = r_array_length(array);

	r_array_setlength(array, index + 1);
	r_array_exist_replace(array, index, data);
	return index;
}


rint r_array_removelast(rarray_t *array)
{
	if (!r_array_empty(array))
		r_array_setlength(array, r_array_length(array) - 1);
	return r_array_length(array);
}


rint r_array_insert(rarray_t *array, ruint index, rconstpointer data)
{
	r_array_checkexpand(array, index + 1);
	if (index < r_array_length(array)) {
		ruint curlen = r_array_length(array);
		r_array_setlength(array, r_array_length(array) + 1);
		r_memmove(r_array_slot(array, index + 1), r_array_slot(array, index), (curlen - index) * array->elt_size);
	} else {
		r_array_setlength(array, index + 1);
	}
	r_array_exist_replace(array, index, data);
	return index;
}


rint r_array_replace(rarray_t *array, ruint index, rconstpointer data)
{
	if (index >= r_array_length(array))
		return r_array_insert(array, index, data);
	r_array_exist_replace(array, index, data);
	return index;
}


void r_array_setlength(rarray_t *array, ruint len)
{
	r_array_checkexpand(array, len);
	array->len = len;
}


static void r_array_checkexpand(rarray_t *array, ruint len)
{
	ruint nalloc_size;
	rpointer data;

	if (array->alloc_size < len) {
		for (nalloc_size = array->alloc_size; nalloc_size < len;)
			nalloc_size = 2 * nalloc_size + 1;
		data = r_realloc(array->data, nalloc_size * array->elt_size);
		if (data) {
			ruint old_len = array->alloc_size;
			array->data = data;
			array->alloc_size = nalloc_size;

			/*
			 * Zero the newly allocated memory - only the extension (above the alloc_size).
			 */
			r_memset((void*)r_array_slot(array, old_len), 0, (array->alloc_size - old_len) * array->elt_size);
		}
	}
}


void *r_array_slot_expand(rarray_t *array, ruint index)
{
	r_array_checkexpand(array, index+1);
	return (void*) r_array_slot(array, index);
}
