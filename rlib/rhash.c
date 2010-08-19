#include "rmem.h"
#include "rhash.h"



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
	return hash;
}


rhash_t *r_hash_init(rhash_t *hash, ruint nbits, r_hash_equalfunc eqfunc, r_hash_hashfun hfunc)
{
	hash->nbits = nbits;
	hash->eqfunc = eqfunc;
	hash->hfunc = hfunc;
	hash->buckets = (rlist_t*)r_malloc(sizeof(rlist_t) * r_hash_size(hash));
	return NULL;
}


void r_hash_destroy(rhash_t *hash)
{
	r_free(hash);
}


void r_hash_cleanup(rhash_t *hash)
{
	r_free(hash->buckets);
}
