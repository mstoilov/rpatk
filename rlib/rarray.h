#ifndef _RARRAY_H_
#define _RARRAY_H_

#include "rtypes.h"
#include "rref.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rarray_s rarray_t;

struct rarray_s {
	rref_t ref;
	rpointer *data;
	ruint len;
	ruint alloc_len;
	ruint elt_size;
};

#define r_array_size(__array__) ((__array__)->len)
#define r_array_empty(__array__) ((r_array_size(__array__)) ? 0 : 1)
#define r_array_index(__array__, __index__, __type__) (((__type__*)(void*)(__array__)->data)[__index__])
#define r_array_last(__array__, __type__) (r_array_empty(__array__) ? (__type__)0 : r_array_index(__array__, (__array__)->len - 1, __type__))
#define r_array_slot(__array__, __index__) (((ruint8*)(__array__)->data) + (__array__)->elt_size * (__index__))

rarray_t *r_array_create(ruint elt_size);
rarray_t *r_array_init(rarray_t *array, ruint elt_size);
void r_array_destroy(rarray_t *array);
void r_array_cleanup(rarray_t *array);
ruint r_array_add(rarray_t *array, rconstpointer data);
void r_array_insert(rarray_t *array, ruint index, rconstpointer data);
void r_array_replace(rarray_t *array, ruint index, rconstpointer data);
void r_array_setsize(rarray_t *array, ruint size);


#ifdef __cplusplus
}
#endif

#endif
