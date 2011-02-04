#include "rastnode.h"
#include "rgc.h"


robject_t *r_astnode_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy)
{
	rastnode_t *node = (rastnode_t*)obj;

	r_object_init(obj, type, cleanup, copy);
	node->props = r_harray_create(sizeof(rastval_t));
	return obj;
}


rastnode_t *r_astnode_create()
{
	rastnode_t *node = (rastnode_t*)r_object_create(sizeof(*node));
	r_astnode_init((robject_t*)node, R_OBJECT_ASTNODE, r_astnode_cleanup, r_astnode_copy);
	return node;
}


void r_astnode_addchild(rastnode_t *node, rastnode_t *child)
{
	if (node->val.type != R_ASTVAL_ARRAY) {
		rcarray_t *arr = r_carray_create(sizeof(rastval_t));
		if (((robject_t*)child)->gc)
			r_gc_add((rgc_t*)((robject_t*)child)->gc, (robject_t*)arr);
		R_ASTVAL_SET_ARRAY(&node->val, arr);
	}
	r_carray_add(node->val.v.arr, &child);
}


void r_astnode_cleanup(robject_t *obj)
{
	rastnode_t *node = (rastnode_t*)obj;

	r_object_destroy((robject_t*)node->props);
	r_object_cleanup(obj);
}


robject_t *r_astnode_copy(const robject_t *obj)
{

	return NULL;
}
