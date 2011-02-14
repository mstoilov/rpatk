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
	codemap->blocks = r_array_create(sizeof(rvm_loopblock_t));
	codemap->labels = r_array_create(sizeof(rvm_codelabel_t*));
	codemap->hash = r_hash_create(5, r_hash_rstrequal, r_hash_rstrhash);
	return codemap;
}


void rvm_codemap_destroy(rvm_codemap_t *codemap)
{
	int i;
	rvm_codelabel_t *label;
	int len = r_array_length(codemap->labels);

	for (i = 0; i < len; i++) {
		label = r_array_index(codemap->labels, i, rvm_codelabel_t*);
		r_free(label->name);
		r_free(label);
	}

	r_object_destroy((robject_t*)codemap->blocks);
	r_object_destroy((robject_t*)codemap->labels);
	r_object_destroy((robject_t*)codemap->hash);
	r_free(codemap);
}


void rvm_codemap_pushloopblock(rvm_codemap_t *codemap, rulong begin, rulong size)
{
	rvm_loopblock_t loop;
	loop.begin = begin;
	loop.size = size;
	r_array_add(codemap->blocks, &loop);
}


void rvm_codemap_poploopblock(rvm_codemap_t *codemap)
{
	ruint len;
	if ((len = r_array_length(codemap->blocks)) > 0) {
		r_array_setlength(codemap->blocks, len - 1);
	}
}


rvm_loopblock_t *rvm_codemap_currentloopblock(rvm_codemap_t *codemap)
{
	ruint len;
	if ((len = r_array_length(codemap->blocks)) > 0) {
		return (rvm_loopblock_t*)r_array_slot(codemap->blocks, len - 1);
	}
	return NULL;
}


void rvm_codemap_clear(rvm_codemap_t *codemap)
{
	r_hash_removeall(codemap->hash);
	r_array_setlength(codemap->labels, 0);
}


static rvm_codelabel_t *rvm_codemap_dolookup(rvm_codemap_t *codemap, const rchar *name, ruint namesize)
{
	rstr_t lookupstr = {(char*)name, namesize};
	return (rvm_codelabel_t *)r_hash_lookup(codemap->hash, &lookupstr);
}



rvm_codelabel_t *rvm_codemap_add(rvm_codemap_t *codemap, const rchar *name, ruint namesize, rulong index)
{
	rvm_codelabel_t *label;

	label = rvm_codemap_dolookup(codemap, name, namesize);
	if (!label) {
		label = r_malloc(sizeof(*label));
		label->name = r_rstrdup(name, namesize);
		r_hash_insert(codemap->hash, label->name, label);
		r_array_add(codemap->labels, &label);
	}
	label->index = index;
	return label;
}


rvm_codelabel_t *rvm_codemap_add_s(rvm_codemap_t *codemap, const rchar *name, rulong index)
{
	return rvm_codemap_add(codemap, name, r_strlen(name), index);
}


rvm_codelabel_t *rvm_codemap_invalid_add(rvm_codemap_t *codemap, const rchar *name, ruint namesize)
{
	return rvm_codemap_add(codemap, name, namesize, RVM_CODELABEL_INVALID);
}


rvm_codelabel_t *rvm_codemap_invalid_add_s(rvm_codemap_t *codemap, const rchar *name)
{
	return rvm_codemap_invalid_add(codemap, name, r_strlen(name));
}


rvm_codelabel_t *rvm_codemap_lastlabel(rvm_codemap_t *codemap)
{
	if (r_array_size(codemap->labels)) {
		return (rvm_codelabel_t*)r_array_last(codemap->labels, rvm_codelabel_t*);
	}
	return NULL;
}


rvm_codelabel_t *rvm_codemap_lookup(rvm_codemap_t *codemap, const rchar *name, ruint namesize)
{
	rstr_t lookupstr = {(char*)name, namesize};
	rvm_codelabel_t *label = (rvm_codelabel_t *)r_hash_lookup(codemap->hash, &lookupstr);

	if (!label)
		label = rvm_codemap_invalid_add(codemap, name, namesize);
	return label;
}



rvm_codelabel_t *rvm_codemap_lookup_s(rvm_codemap_t *codemap, const rchar *name)
{
	return rvm_codemap_lookup(codemap, name, r_strlen(name));
}


