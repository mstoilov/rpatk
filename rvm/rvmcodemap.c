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


void rvm_codemap_clear(rvm_codemap_t *codemap)
{
	r_hash_removeall(codemap->hash);
	r_array_setlength(codemap->labels, 0);
}


rvm_codelabel_t *rvm_codemap_label(rvm_codemap_t *codemap, rlong index)
{
	if (index < 0)
		return NULL;
	return r_array_index(codemap->labels, index, rvm_codelabel_t*);
}


static rlong rvm_codemap_dolookup(rvm_codemap_t *codemap, const rchar *name, ruint namesize)
{
	rstr_t lookupstr = {(char*)name, namesize};
	return r_hash_lookup_indexval(codemap->hash, &lookupstr);
}


static rlong rvm_codemap_add(rvm_codemap_t *codemap, const rchar *name, ruint namesize)
{
	rvm_codelabel_t *label;
	rlong labelidx = -1;

	labelidx = rvm_codemap_dolookup(codemap, name, namesize);
	if (labelidx < 0) {
		label = r_zmalloc(sizeof(*label));
		labelidx = r_array_add(codemap->labels, &label);
		if (name) {
			label->name = r_rstrdup(name, namesize);
			r_hash_insert_indexval(codemap->hash, label->name, labelidx);
		}
	}
	return labelidx;
}



rlong rvm_codemap_addoffset(rvm_codemap_t *codemap, rulong base, const rchar *name, ruint namesize, rulong offset)
{
	rlong labelidx = rvm_codemap_add(codemap, name, namesize);
	rvm_codelabel_t *label = rvm_codemap_label(codemap, labelidx);

	if (label) {
		label->base = base;
		label->loc.index = offset;
		label->type = RVM_CODELABEL_INDEX;
	}
	return labelidx;
}


rlong rvm_codemap_addoffset_s(rvm_codemap_t *codemap, rulong base, const rchar *name, rulong offset)
{
	return rvm_codemap_addoffset(codemap, base, name, r_strlen(name), offset);
}


rlong rvm_codemap_addpointer(rvm_codemap_t *codemap, const rchar *name, ruint namesize, rvm_asmins_t *ptr)
{
	rlong labelidx = rvm_codemap_add(codemap, name, namesize);
	rvm_codelabel_t *label = rvm_codemap_label(codemap, labelidx);

	if (label) {
		label->loc.ptr = ptr;
		label->type = RVM_CODELABEL_POINTER;
	}
	return labelidx;
}


rlong rvm_codemap_addpointer_s(rvm_codemap_t *codemap, const rchar *name, rvm_asmins_t *ptr)
{
	return rvm_codemap_addpointer(codemap, name, r_strlen(name), ptr);
}


rlong rvm_codemap_invalid_add(rvm_codemap_t *codemap, const rchar *name, ruint namesize)
{
	rlong labelidx = rvm_codemap_add(codemap, name, namesize);
	rvm_codelabel_t *label = rvm_codemap_label(codemap, labelidx);

	if (label) {
		label->type = RVM_CODELABEL_INVALID;
	}
	return labelidx;
}


rlong rvm_codemap_invalid_add_s(rvm_codemap_t *codemap, const rchar *name)
{
	return rvm_codemap_invalid_add(codemap, name, r_strlen(name));
}


rlong rvm_codemap_lastlabel(rvm_codemap_t *codemap)
{
	return r_array_length(codemap->labels) - 1;
}


rlong rvm_codemap_lookup(rvm_codemap_t *codemap, const rchar *name, ruint namesize)
{
	rstr_t lookupstr = {(char*)name, namesize};
	rlong labelidx = r_hash_lookup_indexval(codemap->hash, &lookupstr);

	if (labelidx < 0)
		labelidx = rvm_codemap_invalid_add(codemap, name, namesize);
	return labelidx;
}


rlong rvm_codemap_lookup_s(rvm_codemap_t *codemap, const rchar *name)
{
	return rvm_codemap_lookup(codemap, name, r_strlen(name));
}


