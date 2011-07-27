/*
 *  Regular Pattern Analyzer (RPA)
 *  Copyright (c) 2009-2010 Martin Stoilov
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
	unsigned long index;
} rhash_value_t;

struct rhash_node_s {
	rlink_t lnk;
	rconstpointer key;
	rhash_value_t value;
};


static rhash_node_t *r_hash_node_create()
{
	rhash_node_t *node;

	return (rhash_node_t *)r_malloc(sizeof(*node));
}


static void r_hash_node_destroy(rhash_node_t *node)
{
	r_free(node);
}


rhash_node_t *r_hash_nodetaillookup(rhash_t* hash, rhash_node_t *cur, rconstpointer key)
{
	unsigned int nbucket = hash->hfunc(key) & r_hash_mask(hash);
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


rhash_node_t *r_hash_nodelookup(rhash_t* hash, rhash_node_t *cur, rconstpointer key)
{
	unsigned int nbucket = hash->hfunc(key) & r_hash_mask(hash);
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


unsigned int r_hash_strhash(rconstpointer key)
{
	const char *str = (const char*) key;
	unsigned int hash = 0;
	int c;

	while ((c = *str++))
		hash = c + (hash << 6) + (hash << 16) - hash;
	return hash;
}


rboolean r_hash_strequal(rconstpointer key1, rconstpointer key2)
{
	return r_strcmp((const char*)key1, (const char*)key2) ? FALSE : TRUE;
}


unsigned int r_hash_rstrhash(rconstpointer key)
{
	const rstr_t *k = (const rstr_t *)key;
	const char *str = (const char*) k->str;
	unsigned int i;
	unsigned int size = k->size;
	unsigned int hash = 0;

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


rhash_t *r_hash_create(unsigned int nbits, r_hash_equalfunc eqfunc, r_hash_hashfun hfunc)
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
						unsigned int nbits, r_hash_equalfunc eqfunc, r_hash_hashfun hfunc)
{
	unsigned long i;
	unsigned long size;
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
}


void r_hash_insert(rhash_t* hash, rconstpointer key, rpointer value)
{
	unsigned int nbucket = hash->hfunc(key) & r_hash_mask(hash);
	rhash_node_t *node = r_hash_node_create();
	rhead_t *buckethead = &hash->buckets[nbucket];
	if (node) {
		r_list_init(&node->lnk);
		node->key = key;
		node->value.ptr = value;
	}
	r_list_addt(buckethead, &node->lnk);
}


void r_hash_remove(rhash_t* hash, rconstpointer key)
{
	rhash_node_t *node;

	while ((node = r_hash_nodelookup(hash, NULL, key)) != NULL) {
		r_list_del(&node->lnk);
		r_hash_node_destroy(node);
	}
}


void r_hash_removeall(rhash_t* hash)
{
	unsigned long nbucket;
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


rpointer r_hash_lookup(rhash_t* hash, rconstpointer key)
{
	rhash_node_t *node = r_hash_nodelookup(hash, NULL, key);
	if (node)
		return node->value.ptr;
	return NULL;
}


void r_hash_insert_indexval(rhash_t* hash, rconstpointer key, unsigned long index)
{
	unsigned int nbucket = hash->hfunc(key) & r_hash_mask(hash);
	rhash_node_t *node = r_hash_node_create();
	rhead_t *buckethead = &hash->buckets[nbucket];
	if (node) {
		r_list_init(&node->lnk);
		node->key = key;
		node->value.index = index;
	}
	r_list_addt(buckethead, &node->lnk);
}


unsigned long r_hash_lookup_indexval(rhash_t* hash, rconstpointer key)
{
	rhash_node_t *node = r_hash_nodelookup(hash, NULL, key);
	if (node)
		return node->value.index;
	return R_HASH_INVALID_INDEXVAL;
}


unsigned long r_hash_taillookup_indexval(rhash_t* hash, rconstpointer key)
{
	rhash_node_t *node = r_hash_nodetaillookup(hash, NULL, key);
	if (node)
		return node->value.index;
	return R_HASH_INVALID_INDEXVAL;
}


rpointer r_hash_value(rhash_node_t *node)
{
	return node->value.ptr;
}


unsigned long r_hash_indexval(rhash_node_t *node)
{
	return node->value.index;
}
