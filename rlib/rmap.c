/*
 *  Regular Pattern Analyzer Toolkit (RPA/Tk)
 *  Copyright (c) 2009-2012 Martin Stoilov
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Martin Stoilov <martin@rpasearch.com>
 */

#include "rtypes.h"
#include "rlib/rmap.h"
#include "rlib/rstring.h"
#include "rlib/rmem.h"


typedef struct r_mapnode_s {
	rlink_t active;
	rlink_t hash;
	rstring_t *key;
	size_t index;
	size_t nbucket;
	union {
		rpointer p;
		char data[0];
	} value;
} r_mapnode_t;


static rboolean r_map_str_equal(const char *str1, size_t len1, const char *str2, size_t len2)
{
	return (len1 == len2 && r_strncmp(str1, str2, len1) == 0) ? TRUE : FALSE;
}


static size_t r_map_str_hash(const char *str, size_t size)
{
	size_t i;
	size_t hash = 0;

	for (i = 0; i < size; i++, str++) {
		hash = *str + (hash << 6) + (hash << 16) - hash;
	}
	return hash;
}


static rboolean r_map_rstring_equal(rstring_t *key1, rstring_t *key2)
{
	return r_map_str_equal(key1->str, key1->size, key2->str, key2->size);
}


static size_t r_map_rstring_hash(const rstring_t *key)
{
	return r_map_str_hash(key->str, key->size);
}


void r_mapnode_init(r_mapnode_t *node, const char *key, size_t size)
{
	node->key = r_string_create_from_str_len(key, size);
	r_list_init(&node->active);
	r_list_init(&node->hash);
}

/*
 * Get a free node from the inactive list. The map collection uses
 * the rcarray_t to store the nodes, once a node is removed from the
 * collection it is put on the inactive list so it can be reused
 * later. If the inactive list is empty the node will be allocated.
 */
r_mapnode_t *r_map_getfreenode(rmap_t *map, const char *key, size_t size)
{
	r_mapnode_t *node = NULL;

	if (r_list_empty(&map->inactive)) {
		long index = r_carray_add(map->data, NULL);
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

		/*
		 * Destroy the key only if it doesn't bellow to
		 * garbage collection.
		 */
		if (!r_object_gcget((robject_t*)node->key))
			r_string_destroy(node->key);

		/*
		 * Remove the node from the active list
		 */
		r_list_del(&node->active);
	}
	while (!r_list_empty(&map->inactive)) {
		node = r_list_entry(r_list_first(&map->inactive), r_mapnode_t, active);
		/*
		 * Remove the node from the inactive list
		 */
		r_list_del(&node->active);
	}

	/*
	 * Destroy the data chunk array
	 */
	r_carray_destroy(map->data);
	r_free(map->hash);
	r_object_cleanup(&map->obj);
}


robject_t *r_map_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy, size_t elt_size, size_t nbits)
{
	size_t elt_realsize = R_SIZE_ALIGN(elt_size + sizeof(r_mapnode_t), sizeof(ruword));
	size_t hashsize, i;
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

rmap_t *r_map_create(size_t elt_size, size_t nbits)
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


size_t r_map_add(rmap_t *map, const char *name, size_t namesize, rconstpointer pval)
{
	r_mapnode_t *node;

	node = r_map_getfreenode(map, name, namesize);
	if (pval)
		r_memcpy(node->value.data, pval, map->elt_size);
	node->nbucket = (r_map_rstring_hash(node->key) & r_map_hashmask(map));
	r_list_addt(&map->hash[node->nbucket], &node->hash);
	r_list_addt(&map->active, &node->active);
	return node->index;
}


size_t r_map_add_s(rmap_t *map, const char *name, rconstpointer pval)
{
	return r_map_add(map, name, r_strlen(name), pval);
}


size_t r_map_add_l(rmap_t *map, long name, rconstpointer pval)
{
	char key[128];
	r_memset(key, 0, sizeof(key));
	r_snprintf(key, sizeof(key) - 1, "%ld", name);
	return r_map_add_s(map, key, pval);
}


size_t r_map_add_d(rmap_t *map, double name, rconstpointer pval)
{
	char key[128];
	r_memset(key, 0, sizeof(key));
	r_snprintf(key, sizeof(key) - 1, "%.15f", name);
	return r_map_add_s(map, key, pval);
}


r_mapnode_t *r_map_node(rmap_t *map, size_t index)
{
	r_mapnode_t *node;
	if (index >= r_carray_length(map->data))
		return NULL;
	node = (r_mapnode_t*)r_carray_slot(map->data, index);
	if (r_list_empty(&node->hash))
		return NULL;
	return node;
}


rstring_t *r_map_key(rmap_t *map, size_t index)
{
	r_mapnode_t *node = r_map_node(map, index);
	if (!node)
		return NULL;
	return node->key;
}


rpointer r_map_value(rmap_t *map, size_t index)
{
	r_mapnode_t *node = r_map_node(map, index);
	if (!node)
		return NULL;
	return (rpointer)node->value.data;
}


size_t r_map_setvalue(rmap_t *map, size_t index, rconstpointer pval)
{
	r_mapnode_t *node = r_map_node(map, index);
	if (!node)
		return -1;
	r_memcpy(node->value.data, pval, map->elt_size);
	return index;
}


rboolean r_map_delete(rmap_t *map, size_t index)
{
	r_mapnode_t *node = r_map_node(map, index);
	if (!node)
		return FALSE;
	if (!r_object_gcget((robject_t*)node->key))
		r_string_destroy(node->key);
	node->key = NULL;
	r_list_del(&node->hash);
	r_list_init(&node->hash);
	r_list_del(&node->active);
	r_list_addt(&map->inactive, &node->active);
	return TRUE;
}


size_t r_map_lookup(rmap_t *map, const char *name, size_t namesize)
{
	size_t found = (size_t)-1;
	r_mapnode_t *node;
	size_t nbucket;
	rhead_t *bucket;
	rlink_t *pos;

	nbucket = (r_map_str_hash(name, namesize) & r_map_hashmask(map));
	bucket = &map->hash[nbucket];
	pos = r_list_first(bucket);
	for ( ; pos ; pos = r_list_next(bucket, pos)) {
		node = r_list_entry(pos, r_mapnode_t, hash);
		if (r_map_str_equal(node->key->str, node->key->size, name, namesize)) {
			found = node->index;
			break;
		}
	}
	return found;
}


size_t r_map_lookup_s(rmap_t *map, const char *name)
{
	return r_map_lookup(map, name, r_strlen(name));
}


size_t r_map_lookup_l(rmap_t *map, long name)
{
	char key[128];
	r_memset(key, 0, sizeof(key));
	r_snprintf(key, sizeof(key) - 1, "%ld", name);
	return r_map_lookup_s(map, key);
}


size_t r_map_lookup_d(rmap_t *map, double name)
{
	char key[128];
	r_memset(key, 0, sizeof(key));
	r_snprintf(key, sizeof(key) - 1, "%.15f", name);
	return r_map_lookup_s(map, key);
}


size_t r_map_taillookup(rmap_t *map, const char *name, size_t namesize)
{
	size_t found = (size_t)-1;
	r_mapnode_t *node;
	size_t nbucket;
	rhead_t *bucket;
	rlink_t *pos;

	nbucket = (r_map_str_hash(name, namesize) & r_map_hashmask(map));
	bucket = &map->hash[nbucket];
	pos = r_list_last(bucket);
	for ( ; pos ; pos = r_list_prev(bucket, pos)) {
		node = r_list_entry(pos, r_mapnode_t, hash);
		if (r_map_str_equal(node->key->str, node->key->size, name, namesize)) {
			found = node->index;
			break;
		}
	}

	return (long)found;
}


size_t r_map_taillookup_s(rmap_t *map, const char *name)
{
	return r_map_taillookup(map, name, r_strlen(name));
}


size_t r_map_first(rmap_t *map)
{
	r_mapnode_t *node;
	rlink_t *pos = r_list_first(&map->active);

	if (!pos)
		return -1;
	node = r_list_entry(pos, r_mapnode_t, active);
	return node->index;
}


size_t r_map_last(rmap_t *map)
{
	r_mapnode_t *node;
	rlink_t *pos = r_list_last(&map->active);

	if (!pos)
		return -1;
	node = r_list_entry(pos, r_mapnode_t, active);
	return node->index;
}


size_t r_map_next(rmap_t *map, size_t current)
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


size_t r_map_prev(rmap_t *map, size_t current)
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
