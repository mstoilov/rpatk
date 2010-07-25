#ifndef _RARRAY_H_
#define _RARRAY_H_

#include "rtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rarray_s rarray_t;

struct rarray_s {
	rpointer *data;
	ruint len;
	ruint alloc_len;
	ruint elt_size;
};


rarray_t *r_array_create(ruint elt_size);
rarray_t *r_array_init(rarray_t *array, ruint elt_size);
void r_array_destroy(rarray_t *array);
void r_array_cleanup(rarray_t *array);


#ifdef __cplusplus
}
#endif

#endif
