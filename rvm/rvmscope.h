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

#ifndef _RVMSCOPE_H_
#define _RVMSCOPE_H_

#include "rtypes.h"
#include "rlib/rarray.h"
#include "rlib/rhash.h"


#ifdef __cplusplus
extern "C" {
#endif

#define VARMAP_DATATYPE_OFFSET 0
#define VARMAP_DATATYPE_PTR 1


typedef struct rvm_varmap_s {
	const char *name;
	union {
		rpointer ptr;
		ruint32 offset;
	} data;
	unsigned char datatype;
} rvm_varmap_t;


typedef struct rvm_scope_s {
	rarray_t *names;
	rhash_t *nameshash;
	rarray_t *varstack;
	rarray_t *scopestack;
} rvm_scope_t;


rvm_scope_t *rvm_scope_create();
void rvm_scope_destroy(rvm_scope_t *scope);
char *rvm_scope_addname(rvm_scope_t *scope, const char *name, unsigned int namesize);
char *rvm_scope_addstrname(rvm_scope_t *scope, const char *name);
void rvm_scope_addoffset(rvm_scope_t *scope, const char *name, unsigned int namesize, ruint32 off);
void rvm_scope_addpointer(rvm_scope_t *scope, const char *name, unsigned int namesize, rpointer ptr);
void rvm_scope_addoffset_s(rvm_scope_t *scope, const char *name, ruint32 off);
void rvm_scope_addpointer_s(rvm_scope_t *scope, const char *name, rpointer ptr);
void rvm_scope_push(rvm_scope_t* scope);
void rvm_scope_pop(rvm_scope_t* scope);
unsigned int rvm_scope_count(rvm_scope_t* scope);
unsigned int rvm_scope_numentries(rvm_scope_t *scope);
rvm_varmap_t *rvm_scope_lookup(rvm_scope_t *scope, const char *name, unsigned int namesize);
rvm_varmap_t *rvm_scope_lookup_s(rvm_scope_t *scope, const char *name);
rvm_varmap_t *rvm_scope_tiplookup(rvm_scope_t *scope, const char *name, unsigned int namesize);
rvm_varmap_t *rvm_scope_tiplookup_s(rvm_scope_t *scope, const char *name);


#ifdef __cplusplus
}
#endif

#endif
