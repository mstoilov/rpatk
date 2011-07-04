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

#ifndef _RVMRELOCMAP_H_
#define _RVMRELOCMAP_H_

#include "rtypes.h"
#include "rlib/rarray.h"
#include "rlib/rhash.h"
#include "rlib/rstring.h"
#include "rvm/rvmcpu.h"
#include "rvm/rvmcodemap.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	RVM_RELOC_DEFAULT = 0,
	RVM_RELOC_JUMP,
	RVM_RELOC_BRANCH,
	RVM_RELOC_STRING,
	RVM_RELOC_BLOB,
} rvm_reloctype_t;


typedef enum {
	RVM_RELOC_CODE = 0,
	RVM_RELOC_DATA,
} rvm_reloctarget_t;


typedef struct rvm_relocrecord_s {
	rvm_reloctarget_t target;
	rvm_reloctype_t type;
	rulong offset;
	rulong label;
} rvm_relocrecord_t;


typedef struct rvm_relocmap_s {
	rarray_t *records;
} rvm_relocmap_t;


rvm_relocmap_t *rvm_relocmap_create();
void rvm_relocmap_destroy(rvm_relocmap_t *relocmap);
void rvm_relocmap_clear(rvm_relocmap_t *relocmap);
rlong rvm_relocmap_add(rvm_relocmap_t *relocmap, rvm_reloctarget_t target, rvm_reloctype_t type, rulong offset, rulong label);
rvm_relocrecord_t *rvm_relocmap_get(rvm_relocmap_t *relocmap, rulong index);
rulong rvm_relocmap_length(rvm_relocmap_t *relocmap);
rinteger rvm_relocmap_relocate(rvm_relocmap_t *relocmap, rvm_codemap_t *codemap, rvm_asmins_t *code, rvm_codelabel_t **err);


#ifdef __cplusplus
}
#endif

#endif
