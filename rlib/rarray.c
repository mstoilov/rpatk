#include "rarray.h"
#include "rmem.h"

#define MIN_ARRAY_LEN 8
static ruint r_nearest_pow (ruint num);


void r_array_cleanup(RArray *array)
{
	r_free(array->data);
}


void r_array_destroy(RArray *array)
{

}


RArray *r_array_init(RArray *array, ruint elt_size)
{
	r_memset(array, 0, sizeof(*array));
	array->elt_size = r_nearest_pow(elt_size);
	return array;
}


RArray *r_array_create(ruint elt_size)
{
	RArray *array;
	if ((array = (RArray*)r_malloc(sizeof(*array))) == NULL)
		return NULL;
	return r_array_init(array, elt_size);
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
