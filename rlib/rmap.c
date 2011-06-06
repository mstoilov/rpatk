#include "rmap.h"
#include "rstring.h"
#include "rmem.h"


typedef struct rmap_node_s {
	rstr_t key;
	rpointer value;
	rlink_t active;
	rlink_t hash;
} rmap_node_t;


robject_t *r_map_copy(const robject_t *obj)
{
	return (robject_t*)NULL;
}


void r_map_cleanup(robject_t *obj)
{
	rmap_t *map = (rmap_t*)obj;
	r_carray_destroy(map->members);
	r_hash_destroy(map->hash);
	r_object_cleanup(&map->obj);
}


robject_t *r_map_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy, ruint elt_size)
{
	rmap_t *map = (rmap_t*)obj;
	r_object_init(obj, type, cleanup, copy);
	map->hash = r_hash_create(5, r_hash_rstrequal, r_hash_rstrhash);
	map->members = r_carray_create(elt_size);
	r_list_init(&map->active);
	r_list_init(&map->inactive);
	return obj;
}

rmap_t *r_map_create(ruint elt_size)
{
	return NULL;
}


void r_map_destroy(rmap_t *map)
{
	r_object_destroy((robject_t*)map);
}

