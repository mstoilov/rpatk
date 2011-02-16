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
	codemap->labels = r_array_create(sizeof(rvm_codelabel_t));
	codemap->hash = r_hash_create(5, r_hash_rstrequal, r_hash_rstrhash);
	return codemap;
}


void rvm_codemap_destroy(rvm_codemap_t *codemap)
{
	int i;
	rvm_codelabel_t *label;
	int len = r_array_length(codemap->labels);

	for (i = 0; i < len; i++) {
		label = (rvm_codelabel_t*)r_array_slot(codemap->labels, i);
		r_free(label->name.str);
	}
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
	return (rvm_codelabel_t*)r_array_slot(codemap->labels, index);
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
		r_memset(&label, 0, sizeof(label));
		labelidx = r_array_add(codemap->labels, NULL);
		label = rvm_codemap_label(codemap, labelidx);
		if (name) {
			label->name.str = r_strndup(name, namesize);
			label->name.size = namesize;
			r_hash_insert_indexval(codemap->hash, &label->name, labelidx);
		}
	}
	return labelidx;
}



rlong rvm_codemap_addoffset(rvm_codemap_t *codemap, const rchar *name, ruint namesize, rulong base, rulong offset)
{
	rlong labelidx = rvm_codemap_add(codemap, name, namesize);
	rvm_codelabel_t *label = rvm_codemap_label(codemap, labelidx);

	if (label) {
		label->base = base;
		label->value = offset;
		label->type = RVM_CODELABEL_OFFSET;
	}
	return labelidx;
}


rlong rvm_codemap_addoffset_s(rvm_codemap_t *codemap, const rchar *name, rulong base, rulong offset)
{
	return rvm_codemap_addoffset(codemap, name, r_strlen(name), base, offset);
}


rlong rvm_codemap_addpointer(rvm_codemap_t *codemap, const rchar *name, ruint namesize, rpointer ptr)
{
	rlong labelidx = rvm_codemap_add(codemap, name, namesize);
	rvm_codelabel_t *label = rvm_codemap_label(codemap, labelidx);

	if (label) {
		label->value = (rword)ptr;
		label->type = RVM_CODELABEL_POINTER;
	}
	return labelidx;
}


rlong rvm_codemap_addpointer_s(rvm_codemap_t *codemap, const rchar *name, rpointer ptr)
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


rword rvm_codemap_resolve(rvm_codemap_t *codemap, rlong index, rvm_codelabel_t **err)
{
	rvm_codelabel_t *label = rvm_codemap_label(codemap, index);
	rword value;

	if (!label)
		return 0;
	if (label->type == RVM_CODELABEL_POINTER) {
		return label->value;
	} else if (label->type == RVM_CODELABEL_OFFSET) {
		value = rvm_codemap_resolve(codemap, label->base, err);
		if (value == (rword)-1)
			return (rword)-1;
		return value + label->value;
	}
	if (err)
		*err = label;
	return (rword)-1;
}
