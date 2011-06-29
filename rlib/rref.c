#include "rlib/rref.h"


robject_t *r_ref_init(robject_t *obj, ruint32 objtype, r_object_cleanupfun cleanup, r_object_copyfun copy, ruint32 count, rref_type_t type)
{
	rref_t *ref = (rref_t *)obj;
	r_object_init(&ref->obj, objtype, cleanup, copy);
	ref->count = count;
	ref->type = type;
	r_spinlock_init(&ref->lock);
	return obj;
}


rref_t *r_ref_create(rref_type_t type)
{
	rref_t *ref;
	ref = (rref_t*)r_object_create(sizeof(*ref));
	r_ref_init((robject_t*)ref, R_OBJECT_REF, r_ref_cleanup, r_ref_copy, 1, RREF_TYPE_SHARED);
	return ref;
}

robject_t *r_ref_copy(const robject_t *obj)
{
	return (robject_t*) r_ref_create(((rref_t *)obj)->type);
}


void r_ref_cleanup(robject_t *obj)
{
	r_object_cleanup(obj);
}


ruint32 r_ref_inc(rref_t *ref)
{
	ruint32 count;

	r_spinlock_lock(&ref->lock);
	ref->count += 1;
	count = ref->count;
	r_spinlock_unlock(&ref->lock);
	return count;
}


ruint32 r_ref_dec(rref_t *ref)
{
	ruint32 count;

	r_spinlock_lock(&ref->lock);
	if (ref->count) {
		ref->count -= 1;
		if (!ref->count) {
			/*
			 * No body else should be waiting on the spinlock
			 * as this was apparently the last reference. There is
			 * no need to unlock, just destroy the object.
			 */
			r_object_destroy((robject_t*)ref);
			return 0;
		}
	}
	count = ref->count;
	r_spinlock_unlock(&ref->lock);
	return count;
}


ruint32 r_ref_get(rref_t *ref)
{
	return ref->count;
}


void r_ref_typeset(rref_t *ref, rref_type_t type)
{
	ref->type = type;
}


rref_type_t r_ref_typeget(rref_t *ref)
{
	return ref->type;
}
