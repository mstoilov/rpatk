#ifndef _RREF_H_
#define _RREF_H_


#include "rtypes.h"
#include "rlib/robject.h"
#include "rlib/rspinlock.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	RREF_TYPE_SHARED = 0,
	RREF_TYPE_COW,
} rref_type_t;

typedef struct rref_s rref_t;

struct rref_s {
	robject_t obj;
	ruint32 count;
	rref_type_t type;
	rspinlock_t lock;
};

rref_t *r_ref_create(rref_type_t type);
robject_t *r_ref_init(robject_t *obj, ruint32 objtype, r_object_cleanupfun cleanup, r_object_copyfun copy, ruint32 count, rref_type_t type);

ruint32 r_ref_inc(rref_t *ref);
ruint32 r_ref_dec(rref_t *ref);
ruint32 r_ref_get(rref_t *ref);
void r_ref_typeset(rref_t *ref, rref_type_t type);
rref_type_t r_ref_typeget(rref_t *ref);

/*
 * Virtual methods implementation
 */
void r_ref_cleanup(robject_t *obj);
robject_t *r_ref_copy(const robject_t *obj);

#ifdef __cplusplus
}
#endif

#endif
