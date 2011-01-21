#include "rjsobject.h"
#include "rmem.h"



void rjs_object_cleanup(robject_t *obj)
{
	rjs_object_t *jso = (rjs_object_t *)obj;
	r_object_destroy((robject_t*)jso->harray);
	r_object_destroy((robject_t*)jso->narray);
}


robject_t *rjs_object_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy, ruint elt_size)
{
	rjs_object_t *jso = (rjs_object_t *)obj;

	r_object_init(obj, type, cleanup, copy);
	jso->harray = r_harray_create(elt_size);
	jso->narray = r_carray_create(elt_size);
	return obj;
}


rjs_object_t *rjs_object_create(ruint elt_size)
{
	rjs_object_t *jso;
	jso = (rjs_object_t*)r_object_create(sizeof(*jso));
	rjs_object_init((robject_t*)jso, R_OBJECT_JSOBJECT, rjs_object_cleanup, rjs_object_copy, elt_size);
	return jso;
}


robject_t *rjs_object_copy(const robject_t *obj)
{
	rjs_object_t *jso;
	jso = (rjs_object_t*)r_object_create(sizeof(*jso));
	jso->harray = (rharray_t*)r_object_v_copy((robject_t *)((rjs_object_t *)obj)->harray);
	jso->narray = (rcarray_t*)r_object_v_copy((robject_t *)((rjs_object_t *)obj)->narray);
	return (robject_t*)jso;
}

