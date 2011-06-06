#ifndef _RHASHARRAY_H_
#define _RHASHARRAY_H_

#include "rcarray.h"
#include "rhash.h"
#include "rstring.h"
#include "robject.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct rhasharray_s {
	robject_t obj;
	rcarray_t *members;
	rarray_t *names;
	rhash_t *hash;
} rhasharray_t;


rhasharray_t *r_hasharray_create(ruint elt_size);
void r_hasharray_destroy(rhasharray_t *array);


#ifdef __cplusplus
}
#endif

#endif /* _RHASHARRAY_H_ */
