#include "rrefreg.h"
#include "rmem.h"

rrefreg_t *r_refreg_create()
{
	rrefreg_t *refreg = (rrefreg_t*)r_malloc(sizeof(*refreg));
	if (!refreg)
		return NULL;
	return r_refreg_init(refreg);
}


rrefreg_t *r_refreg_copy(const rrefreg_t* src)
{
	rrefreg_t *refreg = r_refreg_create();
	if (!refreg)
		return NULL;
	rvm_reg_copy(&refreg->reg, &refreg->reg);
	return refreg;
}


void r_refreg_cleanup(rrefreg_t *refreg)
{
	rvm_reg_cleanup(&refreg->reg);
}


void r_refreg_destroy(rrefreg_t *refreg)
{
	if (refreg) {
		r_refreg_cleanup(refreg);
		r_free(refreg);
	}
}


static void r_object_destroy_stub(robject_t *ptr)
{
	if (ptr)
		r_refreg_destroy((rrefreg_t*)ptr);
}


static robject_t *r_object_copy_stub(const robject_t *ptr)
{
	return (robject_t*) r_refreg_copy((const rrefreg_t*)ptr);
}


rrefreg_t *r_refreg_init(rrefreg_t *refreg)
{
	r_memset(refreg, 0, sizeof(*refreg));
	r_ref_init(&refreg->ref, 1, R_OBJECT_REFREG, RREF_TYPE_COW, r_object_destroy_stub, r_object_copy_stub);
	return refreg;
}

