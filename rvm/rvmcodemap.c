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

#include "rvm/rvmcodemap.h"
#include "rlib/rstring.h"
#include "rlib/rmem.h"


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
	long i;
	rvm_codelabel_t *label;
	long len = r_array_length(codemap->labels);

	for (i = 0; i < len; i++) {
		label = (rvm_codelabel_t*)r_array_slot(codemap->labels, i);
		r_free(label->name);
	}
	r_object_destroy((robject_t*)codemap->labels);
	r_object_destroy((robject_t*)codemap->hash);
	r_free(codemap);
}


void rvm_codemap_clear(rvm_codemap_t *codemap)
{
	rsize_t i;
	rvm_codelabel_t *label;

	for (i = 0; i < r_array_length(codemap->labels); i++) {
		label = (rvm_codelabel_t*)r_array_slot(codemap->labels, i);
		r_free(label->name);
	}
	r_array_setlength(codemap->labels, 0);
	r_hash_removeall(codemap->hash);
}


rvm_codelabel_t *rvm_codemap_label(rvm_codemap_t *codemap, long index)
{
	if (index < 0 || index >= (long)r_array_length(codemap->labels))
		return NULL;
	return (rvm_codelabel_t*)r_array_slot(codemap->labels, index);
}


static long rvm_codemap_dolookup(rvm_codemap_t *codemap, const char *name, unsigned int namesize)
{
	rstr_t lookupstr = {(char*)name, namesize};
	return r_hash_lookup_indexval(codemap->hash, &lookupstr);
}


static long rvm_codemap_add(rvm_codemap_t *codemap, const char *name, unsigned int namesize)
{
	rvm_codelabel_t *label = NULL;
	long labelidx = -1;

	labelidx = rvm_codemap_dolookup(codemap, name, namesize);
	if (labelidx < 0) {
		labelidx = r_array_add(codemap->labels, NULL);
		label = rvm_codemap_label(codemap, labelidx);
		if (name) {
			label->name = r_rstrdup(name, namesize);
			r_hash_insert_indexval(codemap->hash, label->name, labelidx);
		}
	}
	return labelidx;
}


void rvm_codelabel_setoffset(rvm_codelabel_t *label, unsigned long base, unsigned long offset)
{
	label->base = base;
	label->value = offset;
	label->type = RVM_CODELABEL_OFFSET;
}


void rvm_codelabel_setpointer(rvm_codelabel_t *label, rpointer ptr)
{
	label->base = 0;
	label->value = (ruword)ptr;
	label->type = RVM_CODELABEL_POINTER;
}


void rvm_codelabel_setinvalid(rvm_codelabel_t *label)
{
	label->base = 0;
	label->value = 0;
	label->type = RVM_CODELABEL_INVALID;
}


long rvm_codemap_addoffset(rvm_codemap_t *codemap, const char *name, unsigned int namesize, unsigned long base, unsigned long offset)
{
	long labelidx = rvm_codemap_add(codemap, name, namesize);
	rvm_codelabel_t *label = rvm_codemap_label(codemap, labelidx);
	if (label)
		rvm_codelabel_setoffset(label, base, offset);
	return labelidx;
}


long rvm_codemap_addoffset_s(rvm_codemap_t *codemap, const char *name, unsigned long base, unsigned long offset)
{
	return rvm_codemap_addoffset(codemap, name, r_strlen(name), base, offset);
}


long rvm_codemap_addpointer(rvm_codemap_t *codemap, const char *name, unsigned int namesize, rpointer ptr)
{
	long labelidx = rvm_codemap_add(codemap, name, namesize);
	rvm_codelabel_t *label = rvm_codemap_label(codemap, labelidx);

	if (label) {
		rvm_codelabel_setpointer(label, ptr);
	}
	return labelidx;
}


long rvm_codemap_addpointer_s(rvm_codemap_t *codemap, const char *name, rpointer ptr)
{
	return rvm_codemap_addpointer(codemap, name, r_strlen(name), ptr);
}


long rvm_codemap_invalid_add(rvm_codemap_t *codemap, const char *name, unsigned int namesize)
{
	long labelidx = rvm_codemap_add(codemap, name, namesize);
	rvm_codelabel_t *label = rvm_codemap_label(codemap, labelidx);

	if (label) {
		rvm_codelabel_setinvalid(label);
	}
	return labelidx;
}


long rvm_codemap_validindex(rvm_codemap_t *codemap, long labelidx)
{
	rvm_codelabel_t *label = rvm_codemap_label(codemap, labelidx);

	if (label && label->type != RVM_CODELABEL_INVALID)
		return 1;
	return 0;
}


long rvm_codemap_invalid_add_s(rvm_codemap_t *codemap, const char *name)
{
	return rvm_codemap_invalid_add(codemap, name, r_strlen(name));
}


long rvm_codemap_lastlabel(rvm_codemap_t *codemap)
{
	return r_array_length(codemap->labels) - 1;
}


long rvm_codemap_lookupadd(rvm_codemap_t *codemap, const char *name, unsigned int namesize)
{
	rstr_t lookupstr = {(char*)name, namesize};
	long labelidx = r_hash_lookup_indexval(codemap->hash, &lookupstr);

	if (labelidx < 0)
		labelidx = rvm_codemap_invalid_add(codemap, name, namesize);
	return labelidx;
}


long rvm_codemap_lookupadd_s(rvm_codemap_t *codemap, const char *name)
{
	return rvm_codemap_lookupadd(codemap, name, r_strlen(name));
}


long rvm_codemap_lookup(rvm_codemap_t *codemap, const char *name, unsigned int namesize)
{
	rstr_t lookupstr = {(char*)name, namesize};
	long labelidx = r_hash_lookup_indexval(codemap->hash, &lookupstr);

	return labelidx;
}


long rvm_codemap_lookup_s(rvm_codemap_t *codemap, const char *name)
{
	return rvm_codemap_lookup(codemap, name, r_strlen(name));
}


ruword rvm_codemap_resolve(rvm_codemap_t *codemap, long index, rvm_codelabel_t **err)
{
	rvm_codelabel_t *label = rvm_codemap_label(codemap, index);
	ruword value;

	if (!label)
		return 0;
	if (label->type == RVM_CODELABEL_POINTER) {
		return label->value;
	} else if (label->type == RVM_CODELABEL_OFFSET) {
		value = rvm_codemap_resolve(codemap, label->base, err);
		if (value == (ruword)-1)
			return (ruword)-1;
		return value + label->value;
	}
	if (err)
		*err = label;
	return (ruword)-1;
}


void rvm_codemap_dump(rvm_codemap_t *codemap)
{
	rsize_t i = 0;

	for (i = 0; i < r_array_length(codemap->labels); i++) {
		rvm_codelabel_t *label = rvm_codemap_label(codemap, i);
		r_printf("%d: %s(%d), type: %d, base: %ld, value: %ld\n", i, label->name->str, label->name->size, label->type, label->base, label->value);
	}
}

