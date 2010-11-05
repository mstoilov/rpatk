#include "robject.h"


void r_object_init(robject_t *obj, ruint32 type, r_object_destroyfun destroy, r_object_copyfun copy)
{
	obj->type = type;
	obj->destroy = destroy;
	obj->copy = copy;
}


robject_t *r_object_copy(const robject_t *obj)
{
	if (obj->copy)
		return obj->copy(obj);
	return NULL;
}


void r_object_destroy(robject_t *obj)
{
	r_object_destroyfun destroy = obj->destroy;
	if (destroy)
		destroy(obj);
}


void r_object_typeset(robject_t *obj, ruint32 type)
{
	obj->type = type;
}


ruint32 r_object_typeget(robject_t *obj)
{
	return obj->type;
}
