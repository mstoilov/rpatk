#include "rref.h"


void r_ref_init(rref_t *ref, ruint32 count, ruint32 objtype, rref_type_t type, r_object_destroyfun destroy, r_object_copyfun copy)
{
	r_object_init(&ref->obj, objtype, destroy, copy);
	ref->count = count;
	ref->type = type;
	r_spinlock_init(&ref->lock);
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
