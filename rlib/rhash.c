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

#include "rlib/rmem.h"
#include "rlib/rstring.h"
#include "rlib/rhash.h"


typedef union rhash_value_s {
	rpointer ptr;
	size_t index;
} rhash_value_t;

struct rhash_node_s {
	rlink_t lnk;
	rconstpointer key;
	rhash_value_t value;
};


static rhash_node_t *r_hash_node_create()
{
	rhash_node_t *node;

	node = (rhash_node_t *)r_zmalloc(sizeof(*node));
	r_list_init(&node->lnk);
	return node;
}


static void r_hash_node_destroy(rhash_node_t *node)
{
	r_free(node);
}

/*
 * Search for a node corresponding to the 'key' parameter. The search in the bucket
 * will be performed from the tail towards the head.
 */
rhash_node_t *r_hash_nodetaillookup(rhash_t* hash, rhash_node_t *cur, rconstpointer key)
{
	size_t nbucket = hash->hfunc(key) & r_hash_mask(hash);
	rhead_t *bucket = &hash->buckets[nbucket];
	rhash_node_t *node;
	rlink_t *pos;

	for (pos = cur ? r_list_prev(bucket, &cur->lnk) : r_list_last(bucket); pos ; pos = r_list_prev(bucket, pos)) {
		node = r_list_entry(pos, rhash_node_t, lnk);
		if (hash->eqfunc(node->key, key)) {
			return node;
		}
	}
	return NULL;
}

/*
 * Search for a node corresponding to the 'key' parameter. The search in the bucket
 * will be performed from the head towards the tail.
 */
rhash_node_t *r_hash_nodelookup(rhash_t* hash, rhash_node_t *cur, rconstpointer key)
{
	size_t nbucket = hash->hfunc(key) & r_hash_mask(hash);
	rhead_t *bucket = &hash->buckets[nbucket];
	rhash_node_t *node;
	rlink_t *pos;

	for (pos = cur ? r_list_next(bucket, &cur->lnk) : r_list_first(bucket); pos ; pos = r_list_next(bucket, pos)) {
		node = r_list_entry(pos, rhash_node_t, lnk);
		if (hash->eqfunc(node->key, key)) {
			return node;
		}
	}
	return NULL;
}

/*
 * Calculate a string hash.
 */
size_t r_hash_strhash(rconstpointer key)
{
	const char *str = (const char*) key;
	size_t hash = 0;
	int c;

	while ((c = *str++))
		hash = c + (hash << 6) + (hash << 16) - hash;
	return hash;
}

/*
 * Check if two strings match.
 */
rboolean r_hash_strequal(rconstpointer key1, rconstpointer key2)
{
	return r_strcmp((const char*)key1, (const char*)key2) ? FALSE : TRUE;
}


size_t r_hash_longhash(rconstpointer key)
{
	return (size_t)(*((long*)key));
}


rboolean r_hash_longequal(rconstpointer key1, rconstpointer key2)
{
	return (*((long*)key1) == *((long*)key2)) ? TRUE : FALSE;
}

size_t r_hash_rstrhash(rconstpointer key)
{
	const rstr_t *k = (const rstr_t *)key;
	const char *str = (const char*) k->str;
	size_t i;
	size_t size = k->size;
	size_t hash = 0;

	for (i = 0; i < size; i++, str++) {
		hash = *str + (hash << 6) + (hash << 16) - hash;
	}
	return hash;
}


rboolean r_hash_rstrequal(rconstpointer key1, rconstpointer key2)
{
	const rstr_t *k1 = (const rstr_t *)key1;
	const rstr_t *k2 = (const rstr_t *)key2;

	return (k1->size == k2->size && r_strncmp((const char*)k1->str, (const char*)k2->str, k1->size) == 0) ? TRUE : FALSE;
}


/*
 * Calculate a rstring_t hash.
 */
size_t r_hash_rstring_hash(rconstpointer key)
{
	const rstring_t *k = (const rstring_t *)key;
	const char *str = (const char*) k->str;
	size_t i;
	size_t size = k->size;
	size_t hash = 0;

	for (i = 0; i < size; i++, str++) {
		hash = *str + (hash << 6) + (hash << 16) - hash;
	}
	return hash;
}

/*
 * Check if two rstring_t objects match.
 */
rboolean r_hash_rstring_equal(rconstpointer key1, rconstpointer key2)
{
	const rstring_t *k1 = (const rstring_t *)key1;
	const rstring_t *k2 = (const rstring_t *)key2;

	return (k1->size == k2->size && r_strncmp((const char*)k1->str, (const char*)k2->str, k1->size) == 0) ? TRUE : FALSE;
}


rhash_t *r_hash_create(size_t nbits, r_hash_equalfunc eqfunc, r_hash_hashfun hfunc)
{
	rhash_t *hash;

	hash = (rhash_t*)r_object_create(sizeof(*hash));
	r_hash_init((robject_t *)hash, R_OBJECT_HASH, r_hash_cleanup, r_hash_copy, nbits, eqfunc, hfunc);
	return hash;
}


void r_hash_destroy(rhash_t* hash)
{
	r_object_destroy((robject_t*)hash);
}


robject_t *r_hash_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy,
						size_t nbits, r_hash_equalfunc eqfunc, r_hash_hashfun hfunc)
{
	size_t i;
	size_t size;
	rhash_t *hash = (rhash_t *)obj;

	hash->nbits = nbits;
	hash->eqfunc = eqfunc;
	hash->hfunc = hfunc;
	hash->buckets = (rlist_t*)r_malloc(sizeof(rlist_t) * r_hash_size(hash));
	if (!hash->buckets)
		return NULL;
	size = r_hash_size(hash);
	for (i = 0; i < size; i++) {
		r_list_init(&hash->buckets[i]);
	}
	r_object_init(obj, type, cleanup, copy);
	return obj;
}


robject_t *r_hash_copy(const robject_t *obj)
{
	return NULL;
}


void r_hash_cleanup(robject_t *obj)
{
	rhash_t *hash = (rhash_t *)obj;
	r_hash_removeall(hash);
	r_free(hash->buckets);
	r_object_cleanup(obj);
}

/*
 * Insert a pointer to an object in the hash
 */
void r_hash_insert_object(rhash_t* hash, rconstpointer key, rpointer object)
{
	size_t nbucket = hash->hfunc(key) & r_hash_mask(hash);
	rhash_node_t *node = r_hash_node_create();
	rhead_t *buckethead = &hash->buckets[nbucket];
	if (node) {
		r_list_init(&node->lnk);
		node->key = key;
		node->value.ptr = object;
	}
	r_list_addt(buckethead, &node->lnk);
}

/*
 * Insert an index in the hash
 */
void r_hash_insert_index(rhash_t* hash, rconstpointer key, size_t index)
{
	size_t nbucket = hash->hfunc(key) & r_hash_mask(hash);
	rhash_node_t *node = r_hash_node_create();
	rhead_t *buckethead = &hash->buckets[nbucket];
	if (node) {
		r_list_init(&node->lnk);
		node->key = key;
		node->value.index = index;
	}
	r_list_addt(buckethead, &node->lnk);
}

/*
 * Remove all objects for the specified key from the collection
 */
void r_hash_remove(rhash_t* hash, rconstpointer key)
{
	rhash_node_t *node;

	while ((node = r_hash_nodelookup(hash, NULL, key)) != NULL) {
		r_list_del(&node->lnk);
		r_hash_node_destroy(node);
	}
}

/*
 * Remove all nodes from the collection
 */
void r_hash_removeall(rhash_t* hash)
{
	size_t nbucket;
	rhead_t *head;
	rhash_node_t *node;

	for (nbucket = 0; nbucket < r_hash_size(hash); nbucket++) {
		head = &hash->buckets[nbucket];
		while (!r_list_empty(head)) {
			node = r_list_entry(r_list_first(head), rhash_node_t, lnk);
			r_list_del(&node->lnk);
			r_hash_node_destroy(node);
		}
	}
}

/*
 * Look-up and return a pointer to object in the collection.
 * If the specified key cannot be found NULL will be returned.
 */
rpointer r_hash_lookup_object(rhash_t* hash, rconstpointer key)
{
	rhash_node_t *node = r_hash_nodelookup(hash, NULL, key);
	if (node)
		return node->value.ptr;
	return NULL;
}

/*
 * Look-up and return a value. If the key cannot be found in the
 * collection the function will return R_HASH_INVALID_INDEXVAL.
 */
size_t r_hash_lookup_index(rhash_t* hash, rconstpointer key)
{
	rhash_node_t *node = r_hash_nodelookup(hash, NULL, key);
	if (node)
		return node->value.index;
	return R_HASH_INVALID_INDEXVAL;
}

/*
 * Look-up and return a value. The search will start from the tail of the bucket.
 * If the key cannot be found in the
 * collection the function will return R_HASH_INVALID_INDEXVAL.
 */
size_t r_hash_taillookup_index(rhash_t* hash, rconstpointer key)
{
	rhash_node_t *node = r_hash_nodetaillookup(hash, NULL, key);
	if (node)
		return node->value.index;
	return R_HASH_INVALID_INDEXVAL;
}

/*
 * Return pointer to the hashed object.
 */
rpointer r_hash_object(rhash_node_t *node)
{
	return node->value.ptr;
}

/*
 * Return hashed index (unsigned integer)
 */
size_t r_hash_index(rhash_node_t *node)
{
	return node->value.index;
}
