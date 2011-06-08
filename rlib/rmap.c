#include "rtypes.h"
#include "rmap.h"
#include "rstring.h"
#include "rmem.h"


typedef struct r_mapnode_s {
	rlink_t active;
	rlink_t hash;
	rsize_t index;
	rstr_t *key;
	union {
		rpointer p;
		rchar data[0];
	} value;
} r_mapnode_t;


static rboolean r_map_rstrequal(rstr_t *key1, rstr_t *key2)
{
	return (key1->size == key2->size && r_strncmp((const rchar*)key1->str, (const rchar*)key2->str, key1->size) == 0) ? TRUE : FALSE;
}


static rulong r_map_rstrhash(const rstr_t *key)
{
	const rchar *str = (const rchar*) key->str;
	rulong i;
	rulong size = key->size;
	rulong hash = 0;

	for (i = 0; i < size; i++, str++) {
		hash = *str + (hash << 6) + (hash << 16) - hash;
	}
	return hash;
}

void r_mapnode_init(r_mapnode_t *node, const rchar *key, rsize_t size)
{
	node->key = r_rstrdup(key, size);
	r_list_init(&node->active);
	r_list_init(&node->hash);
}


r_mapnode_t *r_map_getfreenode(rmap_t *map, const rchar *key, rsize_t size)
{
	r_mapnode_t *node = NULL;

	if (r_list_empty(&map->inactive)) {
		rlong index = r_carray_add(map->data, NULL);
		node = (r_mapnode_t*)r_carray_slot(map->data, index);
		r_mapnode_init(node, key, size);
		node->index = index;
	} else {
		node = r_list_entry(r_list_first(&map->inactive), r_mapnode_t, active);
		r_list_del(&node->active);
		r_mapnode_init(node, key, size);
	}
	return node;
}


robject_t *r_map_copy(const robject_t *obj)
{
	return (robject_t*)NULL;
}


void r_map_cleanup(robject_t *obj)
{
	r_mapnode_t *node;
	rmap_t *map = (rmap_t*)obj;

	while (!r_list_empty(&map->active)) {
		node = r_list_entry(r_list_first(&map->active), r_mapnode_t, active);
		r_list_del(&node->active);
	}
	while (!r_list_empty(&map->inactive)) {
		node = r_list_entry(r_list_first(&map->inactive), r_mapnode_t, active);
		r_list_del(&node->active);
	}

	r_carray_destroy(map->data);
	r_free(map->hash);
	r_object_cleanup(&map->obj);
}


robject_t *r_map_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy, ruint elt_size, ruint nbits)
{
	rsize_t elt_realsize = R_SIZE_ALIGN(elt_size + sizeof(r_mapnode_t), sizeof(rword));
	rsize_t hashsize, i;
	rmap_t *map = (rmap_t*)obj;
	if (nbits == 0 || nbits > 16) {
		R_ASSERT(0);
		return NULL;
	}
	r_object_init(obj, type, cleanup, copy);
	map->data = r_carray_create(elt_realsize);
	map->nbits = nbits;
	map->elt_size = elt_size;
	r_list_init(&map->active);
	r_list_init(&map->inactive);
	hashsize = r_map_hashsize(map);
	map->hash = (rlist_t*)r_malloc(sizeof(rlist_t) * hashsize);
	R_ASSERT(map->hash);
	for (i = 0; i < hashsize; i++) {
		r_list_init(&map->hash[i]);
	}

	return obj;
}

rmap_t *r_map_create(ruint elt_size, ruint nbits)
{
	rmap_t *map;
	map = (rmap_t*)r_object_create(sizeof(*map));
	r_map_init((robject_t*)map, R_OBJECT_MAP, r_map_cleanup, r_map_copy, elt_size, nbits);
	return map;

}


void r_map_destroy(rmap_t *map)
{
	r_object_destroy((robject_t*)map);
}


rlong r_map_add(rmap_t *map, const rchar *name, rsize_t namesize, rconstpointer pval)
{
	r_mapnode_t *node;
	rulong nbucket;

	node = r_map_getfreenode(map, name, namesize);
	r_memcpy(node->value.data, pval, map->elt_size);
	nbucket = (r_map_rstrhash(node->key) & r_map_hashmask(map));
	r_list_addt(&map->hash[nbucket], &node->hash);
	r_list_addt(&map->active, &node->active);
	return node->index;
}


rlong r_map_add_s(rmap_t *map, const rchar *name, rconstpointer pval)
{
	return r_map_add(map, name, r_strlen(name), pval);
}


r_mapnode_t *r_map_node(rmap_t *map, rsize_t index)
{
	r_mapnode_t *node;
	node = (r_mapnode_t*)r_carray_slot(map->data, index);
	return node;
}


const rchar *r_map_key(rmap_t *map, rsize_t index)
{
	r_mapnode_t *node = r_map_node(map, index);
	if (!node)
		return NULL;
	return node->key->str;
}


rpointer r_map_value(rmap_t *map, rsize_t index)
{
	r_mapnode_t *node = r_map_node(map, index);
	if (!node)
		return NULL;
	return (rpointer)node->value.data;
}


rint r_map_delete(rmap_t *map, rsize_t index)
{
	r_mapnode_t *node = r_map_node(map, index);
	if (!node)
		return -1;
	r_free(node->key);
	node->key = NULL;
	r_list_del(&node->hash);
	r_list_del(&node->active);
	r_list_addt(&map->inactive, &node->active);
	return 0;
}
