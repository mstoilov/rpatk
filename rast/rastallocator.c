#include "rastallocator.h"


robject_t *r_astallocator_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy)
{
	r_astallocator_t *allocator = (r_astallocator_t*)obj;

	r_object_init(obj, type, cleanup, copy);
	r_list_init(&allocator->head);
	return obj;
}


r_astallocator_t *r_astallocator_create()
{
	r_astallocator_t *allocator = (r_astallocator_t*)r_object_create(sizeof(*allocator));
	r_astallocator_init((robject_t*)allocator, R_OBJECT_ASTALLOCATOR, r_astallocator_cleanup, r_astallocator_copy);
	return allocator;
}


void r_astallocator_cleanup(robject_t *obj)
{
	r_astallocator_t *allocator = (r_astallocator_t*)obj;
	r_astallocator_deallocateall(allocator);
	r_object_cleanup(obj);
}


robject_t *r_astallocator_copy(const robject_t *obj)
{

	return NULL;
}


void r_astallocator_deallocateall(r_astallocator_t *allocator)
{

}
