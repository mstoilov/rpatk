#include "rmem.h"
#include "robject.h"


robject_t *r_object_create(rsize_t size)
{
	robject_t *object;

	if ((object = (robject_t*)r_zmalloc(size)) == NULL)
		return NULL;
	return object;
}

void r_object_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy)
{
	r_list_init(&obj->lnk);
	obj->gc = NULL;
	obj->type = type;
	obj->cleanup = cleanup;
	obj->copy = copy;
}


robject_t *r_object_copy(const robject_t *obj)
{
	/*
	 * Nothing to do
	 */
	return NULL;
}


void r_object_cleanup(robject_t *obj)
{
	/*
	 * if on a list, remove it
	 */
	if (!r_list_empty(&obj->lnk))
		r_list_del(&obj->lnk);
	/*
	 * Nothing to do here, but for now lets wipe out the structure
	 */
	r_memset(obj, 0, sizeof(*obj));
}


void r_object_destroy(robject_t *obj)
{
	if (obj) {
		r_object_v_cleanup(obj);
		r_free(obj);
	}
}


robject_t *r_object_v_copy(const robject_t *obj)
{
	if (obj->copy)
		return obj->copy(obj);
	return NULL;
}


void r_object_v_cleanup(robject_t *obj)
{
	r_object_cleanupfun cleanup = obj->cleanup;
	if (cleanup)
		cleanup(obj);
}



void r_object_typeset(robject_t *obj, ruint32 type)
{
	obj->type = type;
}


ruint32 r_object_typeget(robject_t *obj)
{
	return obj->type;
}
