#include "rarray.h"
#include "rmem.h"

#define MIN_ARRAY_LEN 4
static ruint r_nearest_pow(ruint num);
static void r_array_checkexpand(rarray_t *array);


void r_array_cleanup(rarray_t *array)
{
	r_free(array->data);
}


void r_array_destroy(rarray_t *array)
{
	r_array_cleanup(array);
	r_free(array);
}


rarray_t *r_array_init(rarray_t *array, ruint elt_size)
{
	r_memset(array, 0, sizeof(*array));
	array->elt_size = r_nearest_pow(elt_size);
	array->data = r_malloc(MIN_ARRAY_LEN * array->elt_size);
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
	return r_array_init(array, elt_size);
}


void r_array_add(rarray_t *array, rconstpointer data)
{
	r_array_checkexpand(array);
	r_memcpy(((rint8*)array->data) + array->len * array->elt_size, data, array->elt_size);
	array->len += 1;
}


static void r_array_checkexpand(rarray_t *array)
{
	ruint nalloc_len;
	rpointer data;

	if (array->len >= array->alloc_len) {
		nalloc_len = 2 * array->alloc_len;
		data = r_realloc(array->data, nalloc_len * array->elt_size);
		if (data) {
			/*
			 * Zero the newly allocated memory - only the extension (above the alloc_len).
			 */
			r_memset((void*)r_array_slot(array, array->alloc_len), 0, (nalloc_len - array->alloc_len) * array->elt_size);
			array->data = data;
			array->alloc_len = nalloc_len;
		}
	}
}


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
