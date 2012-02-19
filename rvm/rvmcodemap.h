/*
 *  Regular Pattern Analyzer Toolkit (RPA/Tk)
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

#ifndef _RVMCODEMAP_H_
#define _RVMCODEMAP_H_

#include "rtypes.h"
#include "rvm/rvmcpu.h"
#include "rlib/rarray.h"
#include "rlib/rhash.h"
#include "rlib/rstring.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct rvm_codelabel_s {
	enum {
		RVM_CODELABEL_OFFSET = 0,
		RVM_CODELABEL_POINTER,
		RVM_CODELABEL_INVALID,
	} type;
	unsigned long base;
	ruword value;
	rstr_t *name;
	unsigned long size; // Optional, used for function declarations
} rvm_codelabel_t;


typedef struct rvm_codemap_s {
	rarray_t *labels;
	rhash_t *hash;
} rvm_codemap_t;


rvm_codemap_t *rvm_codemap_create();
void rvm_codemap_destroy(rvm_codemap_t *codemap);
void rvm_codemap_clear(rvm_codemap_t *codemap);
long rvm_codemap_invalid_add(rvm_codemap_t *codemap, const char *name, unsigned int namesize);
long rvm_codemap_invalid_add_s(rvm_codemap_t *codemap, const char *name);
long rvm_codemap_addoffset(rvm_codemap_t *codemap, const char *name, unsigned int namesize, unsigned long base, unsigned long offset);
long rvm_codemap_addoffset_s(rvm_codemap_t *codemap, const char *name, unsigned long base, unsigned long offset);
long rvm_codemap_addpointer(rvm_codemap_t *codemap, const char *name, unsigned int namesize, rpointer ptr);
long rvm_codemap_addpointer_s(rvm_codemap_t *codemap, const char *name, rpointer ptr);
long rvm_codemap_lookupadd(rvm_codemap_t *codemap, const char *name, unsigned int namesize);
long rvm_codemap_lookupadd_s(rvm_codemap_t *codemap, const char *name);
long rvm_codemap_lookup(rvm_codemap_t *codemap, const char *name, unsigned int namesize);
long rvm_codemap_lookup_s(rvm_codemap_t *codemap, const char *name);
long rvm_codemap_lastlabel(rvm_codemap_t *codemap);
rvm_codelabel_t *rvm_codemap_label(rvm_codemap_t *codemap, long index);
long rvm_codemap_validindex(rvm_codemap_t *codemap, long labelidx);
ruword rvm_codemap_resolve(rvm_codemap_t *codemap, long index, rvm_codelabel_t **err);
void rvm_codemap_dump(rvm_codemap_t *codemap);
void rvm_codelabel_setoffset(rvm_codelabel_t *label, unsigned long base, unsigned long offset);
void rvm_codelabel_setpointer(rvm_codelabel_t *label, rpointer ptr);
void rvm_codelabel_setinvalid(rvm_codelabel_t *label);

#ifdef __cplusplus
}
#endif

#endif
