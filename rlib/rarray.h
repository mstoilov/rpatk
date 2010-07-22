#ifndef _RARRAY_H_
#define _RARRAY_H_

#include "rtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _RArray RArray;

struct _RArray {
	rpointer *data;
	ruint len;
	ruint alloc_len;
	ruint elt_size;
};

RArray *r_array_create(ruint elt_size);
RArray *r_array_init(RArray *array, ruint elt_size);
void r_array_destroy(RArray *array);
void r_array_cleanup(RArray *array);


#ifdef __cplusplus
}
#endif

#endif
