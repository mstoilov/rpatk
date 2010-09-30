#include "rmem.h"
#include "rstring.h"
#include "rhash.h"


typedef struct rhash_node_s {
	rlink_t lnk;
	rconstpointer key;
	rpointer value;
} rhash_node_t;


static rhash_node_t *r_hash_node_create()
{
	rhash_node_t *node;

	return (rhash_node_t *)r_malloc(sizeof(*node));
}


static void r_hash_node_destroy(rhash_node_t *node)
{
	r_free(node);
}


static rhash_node_t *r_hash_node_lookup(rhash_t* hash, rconstpointer key)
{
	ruint nbucket = hash->hfunc(key) & r_hash_mask(hash);
	rhash_node_t *node;
	rlink_t *pos;

	r_list_foreach(pos, &hash->buckets[nbucket]) {
		node = r_list_entry(pos, rhash_node_t, lnk);
		if (hash->eqfunc(node->key, key)) {
			return node;
		}
	}
	return NULL;
}


ruint r_hash_strhash(rconstpointer key)
{
	const rchar *str = (const rchar*) key;
	ruint hash = 0;
	int c;

	while ((c = *str++))
		hash = c + (hash << 6) + (hash << 16) - hash;
	return hash;
}


rboolean r_hash_strequal(rconstpointer key1, rconstpointer key2)
{
	return r_strcmp((const rchar*)key1, (const rchar*)key2) ? FALSE : TRUE;
}


ruint r_hash_strnhash(rconstpointer key)
{
	const rstr_t *k = (const rstr_t *)key;
	const rchar *str = (const rchar*) k->str;
	ruint i;
	ruint size = k->size;
	ruint hash = 0;

	for (i = 0; i < size; i++, str++) {
		hash = *str + (hash << 6) + (hash << 16) - hash;
	}
	return hash;
}


rboolean r_hash_strnequal(rconstpointer key1, rconstpointer key2)
{
	const rstr_t *k1 = (const rstr_t *)key1;
	const rstr_t *k2 = (const rstr_t *)key2;

	return (k1->size == k2->size && r_strncmp((const rchar*)k1->str, (const rchar*)k2->str, k1->size) == 0) ? TRUE : FALSE;
}


static void r_ref_destroy(rref_t *ref)
{
	r_hash_destroy((rhash_t*)ref);
}


rhash_t *r_hash_create(ruint nbits, r_hash_equalfunc eqfunc, r_hash_hashfun hfunc)
{
	rhash_t *hash;

	hash = (rhash_t*)r_malloc(sizeof(*hash));
	if (!hash)
		return NULL;
	r_memset(hash, 0, sizeof(*hash));
	if (!r_hash_init(hash, nbits, eqfunc, hfunc)) {
		r_hash_destroy(hash);
		return NULL;
	}
	r_ref_init(&hash->ref, 1, RREF_TYPE_NONE, r_ref_destroy);
	return hash;
}


rhash_t *r_hash_init(rhash_t *hash, ruint nbits, r_hash_equalfunc eqfunc, r_hash_hashfun hfunc)
{
	ruint i;
	rsize_t size;

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
	return hash;
}


void r_hash_destroy(rhash_t *hash)
{
	r_hash_cleanup(hash);
	r_free(hash);
}


void r_hash_cleanup(rhash_t *hash)
{
	r_hash_removeall(hash);
	r_free(hash->buckets);
}


void r_hash_insert(rhash_t* hash, rconstpointer key, rpointer value)
{
	ruint nbucket = hash->hfunc(key) & r_hash_mask(hash);
	rhash_node_t *node = r_hash_node_create();
	rhead_t *buckethead = &hash->buckets[nbucket];
	if (node) {
		r_list_init(&node->lnk);
		node->key = key;
		node->value = value;
	}
	r_list_addt(buckethead, &node->lnk);

}


void r_hash_remove(rhash_t* hash, rconstpointer key)
{
	rhash_node_t *node;

	while ((node = r_hash_node_lookup(hash, key)) != NULL) {
		r_list_del(&node->lnk);
		r_hash_node_destroy(node);
	}
}


void r_hash_removeall(rhash_t* hash)
{
	ruint nbucket;
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
	rhash_node_t *node = r_hash_node_lookup(hash, key);
	if (node)
		return node->value;
	return NULL;
}
