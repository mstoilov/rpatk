#ifndef _RREF_H_
#define _RREF_H_


#include "rtypes.h"
#include "robject.h"
#include "rspinlock.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	RREF_TYPE_COW = 0,
	RREF_TYPE_SHARED,
} rref_type_t;

typedef struct rref_s rref_t;

struct rref_s {
	robject_t obj;
	ruint32 count;
	rref_type_t type;
	rspinlock_t lock;
};

ruint32 r_ref_inc(rref_t *ref);
ruint32 r_ref_dec(rref_t *ref);
ruint32 r_ref_get(rref_t *ref);
void r_ref_typeset(rref_t *ref, rref_type_t type);
rref_type_t r_ref_typeget(rref_t *ref);
void r_ref_init(rref_t *ref, ruint32 count, ruint32 objtype, rref_type_t type, r_object_destroyfun destroy, r_object_copyfun copy);


#ifdef __cplusplus
}
#endif

#endif
