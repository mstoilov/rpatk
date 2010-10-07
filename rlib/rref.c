#include "rref.h"


void r_ref_init(rref_t *ref, ruint32 count, ruint32 type, r_ref_destroyfun destroy, r_ref_copyfun copy)
{
	ref->count = count;
	ref->type = type;
	ref->destroy = destroy;
	ref->copy = copy;
}


ruint32 r_ref_inc(rref_t *ref)
{
	ref->count += 1;
	return ref->count;
}


ruint32 r_ref_dec(rref_t *ref)
{
	if (ref->count) {
		ref->count -= 1;
		if (!ref->count && ref->destroy) {
			ref->destroy(ref);
		}
	}
	return ref->count;
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
