#include "rtypes.h"
#include "rmap.h"
#include "rstring.h"
#include "rmem.h"


typedef struct r_mapnode_s {
	rlink_t active;
	rlink_t hash;
	rsize_t index;
	rlong nbucket;
	rstring_t *key;
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
//	node->key = r_rstrdup(key, size);
	node->key = r_string_create_strsize(key, size);
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
		if (!r_object_gcget((robject_t*)node->key))
			r_string_destroy(node->key);
		r_list_del(&node->active);
	}
	while (!r_list_empty(&map->inactive)) {
		node = r_list_entry(r_list_first(&map->inactive), r_mapnode_t, active);
		if (!r_object_gcget((robject_t*)node->key))
			r_string_destroy(node->key);
		r_list_del(&node->active);
	}

	r_carray_destroy(map->data);
	r_free(map->hash);
	r_object_cleanup(&map->obj);
}


robject_t *r_map_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy, ruinteger elt_size, ruinteger nbits)
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

rmap_t *r_map_create(ruinteger elt_size, ruinteger nbits)
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


rlong r_map_gckey_add(rmap_t *map, rgc_t* gc, const rchar *name, rsize_t namesize, rconstpointer pval)
{
	r_mapnode_t *node;

	node = r_map_getfreenode(map, name, namesize);
	if (pval)
		r_memcpy(node->value.data, pval, map->elt_size);
	node->nbucket = (r_map_rstrhash(R_STRING2RSTR(node->key)) & r_map_hashmask(map));
	r_list_addt(&map->hash[node->nbucket], &node->hash);
	r_list_addt(&map->active, &node->active);
	if (gc)
		r_gc_add(gc, (robject_t*)node->key);
	return node->index;
}


rlong r_map_gckey_add_s(rmap_t *map, rgc_t* gc, const rchar *name, rconstpointer pval)
{
	return r_map_gckey_add(map, gc, name, r_strlen(name), pval);
}


rlong r_map_gckey_add_d(rmap_t *map, rgc_t* gc, double name, rconstpointer pval)
{
	rchar key[128];
	r_memset(key, 0, sizeof(key));
	r_snprintf(key, sizeof(key) - 1, "%.15f", name);
	return r_map_gckey_add_s(map, gc, key, pval);
}


rlong r_map_gckey_add_l(rmap_t *map, rgc_t* gc, long name, rconstpointer pval)
{
	rchar key[128];
	r_memset(key, 0, sizeof(key));
	r_snprintf(key, sizeof(key) - 1, "%ld", name);
	return r_map_gckey_add_s(map, gc, key, pval);
}


rlong r_map_add(rmap_t *map, const rchar *name, rsize_t namesize, rconstpointer pval)
{
	return r_map_gckey_add(map, NULL, name, namesize, pval);
}


rlong r_map_add_s(rmap_t *map, const rchar *name, rconstpointer pval)
{
	return r_map_gckey_add_s(map, NULL, name, pval);
}


rlong r_map_add_l(rmap_t *map, long name, rconstpointer pval)
{
	return r_map_gckey_add_l(map, NULL, name, pval);
}


rlong r_map_add_d(rmap_t *map, double name, rconstpointer pval)
{
	return r_map_gckey_add_d(map, NULL, name, pval);
}


r_mapnode_t *r_map_node(rmap_t *map, rulong index)
{
	r_mapnode_t *node;
	if (index >= r_carray_length(map->data))
		return NULL;
	node = (r_mapnode_t*)r_carray_slot(map->data, index);
	if (r_list_empty(&node->hash))
		return NULL;
	return node;
}


rstring_t *r_map_key(rmap_t *map, rulong index)
{
	r_mapnode_t *node = r_map_node(map, index);
	if (!node)
		return NULL;
	return node->key;
}


rpointer r_map_value(rmap_t *map, rulong index)
{
	r_mapnode_t *node = r_map_node(map, index);
	if (!node)
		return NULL;
	return (rpointer)node->value.data;
}


rlong r_map_setvalue(rmap_t *map, rlong index, rconstpointer pval)
{
	r_mapnode_t *node = r_map_node(map, index);
	if (!node)
		return -1;
	r_memcpy(node->value.data, pval, map->elt_size);
	return index;
}


rinteger r_map_delete(rmap_t *map, rulong index)
{
	r_mapnode_t *node = r_map_node(map, index);
	if (!node)
		return -1;
	if (!r_object_gcget((robject_t*)node->key))
		r_string_destroy(node->key);
	node->key = NULL;
	r_list_del(&node->hash);
	r_list_init(&node->hash);
	r_list_del(&node->active);
	r_list_addt(&map->inactive, &node->active);
	return 0;
}


rlong r_map_lookup(rmap_t *map, rlong current, const rchar *name, rsize_t namesize)
{
	rlong found = -1;
	r_mapnode_t *node;
	rstr_t lookupstr = {(char*)name, namesize};
	rulong nbucket;
	rhead_t *bucket;
	rlink_t *pos;

	if (current >= 0) {
		r_mapnode_t *curnode = r_map_node(map, current);
		if (!curnode)
			return -1;
		nbucket = curnode->nbucket;
		bucket = &map->hash[nbucket];
		pos = r_list_next(bucket, &curnode->hash);
	} else {
		nbucket = (r_map_rstrhash(&lookupstr) & r_map_hashmask(map));
		bucket = &map->hash[nbucket];
		pos = r_list_first(bucket);
	}
	for ( ; pos ; pos = r_list_next(bucket, pos)) {
		node = r_list_entry(pos, r_mapnode_t, hash);
		if (r_map_rstrequal(R_STRING2RSTR(node->key), &lookupstr)) {
			found = node->index;
			break;
		}
	}

	return (rlong)found;
}


rlong r_map_lookup_s(rmap_t *map, rlong current, const rchar *name)
{
	return r_map_lookup(map, current, name, r_strlen(name));
}


rlong r_map_lookup_l(rmap_t *map, rlong current, long name)
{
	rchar key[128];
	r_memset(key, 0, sizeof(key));
	r_snprintf(key, sizeof(key) - 1, "%ld", name);
	return r_map_lookup_s(map, current, key);
}


rlong r_map_lookup_d(rmap_t *map, rlong current, double name)
{
	rchar key[128];
	r_memset(key, 0, sizeof(key));
	r_snprintf(key, sizeof(key) - 1, "%.15f", name);
	return r_map_lookup_s(map, current, key);
}


rlong r_map_taillookup(rmap_t *map, rlong current, const rchar *name, rsize_t namesize)
{
	rlong found = -1;
	r_mapnode_t *node;
	rstr_t lookupstr = {(char*)name, namesize};
	rulong nbucket;
	rhead_t *bucket;
	rlink_t *pos;

	if (current >= 0) {
		r_mapnode_t *curnode = r_map_node(map, current);
		if (!curnode)
			return -1;
		nbucket = curnode->nbucket;
		bucket = &map->hash[nbucket];
		pos = r_list_prev(bucket, &curnode->hash);
	} else {
		nbucket = (r_map_rstrhash(&lookupstr) & r_map_hashmask(map));
		bucket = &map->hash[nbucket];
		pos = r_list_last(bucket);
	}
	for ( ; pos ; pos = r_list_prev(bucket, pos)) {
		node = r_list_entry(pos, r_mapnode_t, hash);
		if (r_map_rstrequal(R_STRING2RSTR(node->key), &lookupstr)) {
			found = node->index;
			break;
		}
	}

	return (rlong)found;
}


rlong r_map_taillookup_s(rmap_t *map, rlong current, const rchar *name)
{
	return r_map_taillookup(map, current, name, r_strlen(name));
}


rlong r_map_first(rmap_t *map)
{
	r_mapnode_t *node;
	rlink_t *pos = r_list_first(&map->active);

	if (!pos)
		return -1;
	node = r_list_entry(pos, r_mapnode_t, active);
	return node->index;
}


rlong r_map_last(rmap_t *map)
{
	r_mapnode_t *node;
	rlink_t *pos = r_list_last(&map->active);

	if (!pos)
		return -1;
	node = r_list_entry(pos, r_mapnode_t, active);
	return node->index;
}


rlong r_map_next(rmap_t *map, rlong current)
{
	r_mapnode_t *node = r_map_node(map, current);
	rlink_t *pos;

	if (!node)
		return -1;
	pos = r_list_next(&map->active, &node->active);
	if (!pos)
		return -1;
	node = r_list_entry(pos, r_mapnode_t, active);
	return node->index;
}


rlong r_map_prev(rmap_t *map, rlong current)
{
	r_mapnode_t *node = r_map_node(map, current);
	rlink_t *pos;

	if (!node)
		return -1;
	pos = r_list_prev(&map->active, &node->active);
	if (!pos)
		return -1;
	node = r_list_entry(pos, r_mapnode_t, active);
	return node->index;
}
