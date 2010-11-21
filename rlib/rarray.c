#include "rarray.h"
#include "rmem.h"

#define MIN_ARRAY_LEN 2

#if 0

/*
 * Returns the smallest power of 2 greater than n
 */
static ruint r_nearest_pow (ruint num)
{
	ruint n = 1;

	while (n < num && n > 0)
		n <<= 1;
	return n ? n : num;
}
#endif


static void r_array_checkexpand(rarray_t *array, ruint index);


void r_array_cleanup(rarray_t *array)
{
	if (array->ondestroy)
		array->ondestroy(array);
	r_free(array->data);
}


void r_array_destroy(rarray_t *array)
{
	r_array_cleanup(array);
	r_free(array);
}


static void r_objectstub_destroy(robject_t *ptr)
{
	r_array_destroy((rarray_t*)ptr);
}


static robject_t *r_objectstub_copy(const robject_t *ptr)
{
	return (robject_t*) r_array_copy((const rarray_t*)ptr);
}


rarray_t *r_array_init(rarray_t *array, ruint elt_size)
{
	r_memset(array, 0, sizeof(*array));
	array->elt_size = elt_size;
	array->data = r_zmalloc(MIN_ARRAY_LEN * array->elt_size);
	array->alloc_len = MIN_ARRAY_LEN;
	if (!array->data)
		return NULL;
	return array;
}


rarray_t *r_array_create(ruint elt_size)
{
	rarray_t *array;
	if ((array = (rarray_t*)r_malloc(sizeof(*array))) == NULL)
		return NULL;
	if (!r_array_init(array, elt_size)) {
		r_array_destroy(array);
		return NULL;
	}
	r_object_init(&array->obj, R_OBJECT_ARRAY, r_objectstub_destroy, r_objectstub_copy);
	return array;
}


rarray_t *r_array_copy(const rarray_t *array)
{
	ruint i;
	rarray_t *dst;

	if (!array)
		return NULL;
	dst = r_array_create(array->elt_size);
	if (!dst)
		return NULL;
	for (i = 0; i < array->len; i++)
		r_array_replace(dst, i, r_array_slot(array, i));
	dst->oncopy = array->oncopy;
	dst->ondestroy = array->ondestroy;
	if (dst->oncopy)
		dst->oncopy(dst);
	return dst;
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
	rint index = array->len;

	r_array_setsize(array, index + 1);
	r_array_exist_replace(array, index, data);
	return index;
}


rint r_array_insert(rarray_t *array, ruint index, rconstpointer data)
{
	r_array_checkexpand(array, index + 1);
	if (index < array->len) {
		ruint curlen = r_array_size(array);
		r_array_setsize(array, array->len + 1);
		r_memmove(r_array_slot(array, index + 1), r_array_slot(array, index), (curlen - index) * array->elt_size);
	} else {
		r_array_setsize(array, index + 1);
	}
	r_array_exist_replace(array, index, data);
	return index;
}


rint r_array_replace(rarray_t *array, ruint index, rconstpointer data)
{
	if (index >= array->len)
		return r_array_insert(array, index, data);
	r_array_exist_replace(array, index, data);
	return index;
}


void r_array_setsize(rarray_t *array, ruint size)
{
	r_array_checkexpand(array, size);
	array->len = size;
}


static void r_array_checkexpand(rarray_t *array, ruint size)
{
	ruint nalloc_len;
	rpointer data;

	while (size > array->alloc_len) {
		nalloc_len = 2 * array->alloc_len;
		data = r_realloc(array->data, nalloc_len * array->elt_size);
		if (data) {
			ruint old_len = array->alloc_len;
			array->data = data;
			array->alloc_len = nalloc_len;

			/*
			 * Zero the newly allocated memory - only the extension (above the alloc_len).
			 */
			r_memset((void*)r_array_slot(array, old_len), 0, (array->alloc_len - old_len) * array->elt_size);
		}
	}
}
