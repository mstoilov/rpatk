#ifndef _RREF_H_
#define _RREF_H_


#include "rtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RREF_TYPE_NONE 0

typedef struct rref_s rref_t;
typedef void (*r_ref_destroyptr)(rref_t *ptr);

struct rref_s {
	ruint32 count;
	ruint32 type;
	r_ref_destroyptr destroy;
};

ruint32 r_ref_inc(rref_t *ref);
ruint32 r_ref_dec(rref_t *ref);
ruint32 r_ref_get(rref_t *ref);
void r_ref_init(rref_t *ref, ruint32 count, ruint32 type, r_ref_destroyptr destroy);


#ifdef __cplusplus
}
#endif

#endif
