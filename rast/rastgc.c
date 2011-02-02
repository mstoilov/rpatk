#include "rastgc.h"


robject_t *r_astgc_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy)
{
	r_astgc_t *gc = (r_astgc_t*)obj;

	r_object_init(obj, type, cleanup, copy);
	r_list_init(&gc->head);
	return obj;
}


r_astgc_t *r_astgc_create()
{
	r_astgc_t *gc = (r_astgc_t*)r_object_create(sizeof(*gc));
	r_astgc_init((robject_t*)gc, R_OBJECT_ASTALLOCATOR, r_astgc_cleanup, r_astgc_copy);
	return gc;
}


void r_astgc_cleanup(robject_t *obj)
{
	r_astgc_t *gc = (r_astgc_t*)obj;
	r_astgc_deallocateall(gc);
	r_object_cleanup(obj);
}


robject_t *r_astgc_copy(const robject_t *obj)
{

	return NULL;
}


void r_astgc_deallocateall(r_astgc_t *gc)
{

}
