#include "rref.h"


void r_ref_init(rref_t *ref, ruint32 count, ruint32 type, r_ref_destroyfun destroy)
{
	ref->count = count;
	ref->type = type;
	ref->destroy = destroy;
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
