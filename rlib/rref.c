#include "rref.h"


void r_ref_init(rref_t *ref, ruint32 count, rref_type_t type, r_ref_destroyfun destroy, r_ref_copyfun copy)
{
	ref->count = count;
	ref->type = type;
	ref->destroy = destroy;
	ref->copy = copy;
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
		if (!ref->count && ref->destroy) {
			ref->destroy(ref);
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


rref_t *r_ref_copy(const rref_t *ref)
{
	if (ref->copy)
		return ref->copy(ref);
	return NULL;
}


void r_ref_typeset(rref_t *ref, rref_type_t type)
{
	ref->type = type;
}


rref_type_t r_ref_typeget(rref_t *ref)
{
	return ref->type;
}
