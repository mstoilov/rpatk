#include "rvmcodemap.h"
#include "rstring.h"
#include "rmem.h"


rvm_codemap_t *rvm_codemap_create()
{
	rvm_codemap_t *codemap;

	codemap = (rvm_codemap_t*)r_malloc(sizeof(*codemap));
	if (!codemap)
		return NULL;
	r_memset(codemap, 0, sizeof(*codemap));
	codemap->labels = r_array_create(sizeof(rvm_label_t*));
	codemap->hash = r_hash_create(5, r_hash_strnequal, r_hash_strnhash);
	return codemap;
}


void rvm_codemap_destroy(rvm_codemap_t *codemap)
{
	int i;
	int len = codemap->labels->len;

	for (i = 0; i < len; i++)
		r_free(r_array_index(codemap->labels, i, rvm_label_t*));
	r_array_destroy(codemap->labels);
	r_hash_destroy(codemap->hash);
	r_free(codemap);
}


void rvm_codemap_add(rvm_codemap_t *codemap, const rchar *name, ruint namesize, rvm_asmins_t *ins)
{
	rvm_label_t *label = r_malloc(sizeof(*label));

	label->name = r_stringdup(name, namesize);
	label->ins = ins;
	r_hash_insert(codemap->hash, label->name, label);
}



rvm_label_t *rvm_codemap_lookup(rvm_codemap_t *codemap, const rchar *name, ruint namesize)
{
	rstring_t lookupstr = {(char*)name, namesize};
	return (rvm_label_t*) r_hash_lookup(codemap->hash, &lookupstr);
}



rvm_label_t *rvm_codemap_lookupstr(rvm_codemap_t *codemap, const rchar *name)
{
	return rvm_codemap_lookup(codemap, name, r_strlen(name));
}


