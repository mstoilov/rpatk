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

#ifndef _RVMCODEGEN_H_
#define _RVMCODEGEN_H_

#include "rtypes.h"
#include "rvm/rvmerror.h"
#include "rlib/rarray.h"
#include "rlib/rhash.h"
#include "rvm/rvmcpu.h"
#include "rvm/rvmcodemap.h"
#include "rvm/rvmrelocmap.h"

#ifdef __cplusplus
extern "C" {
#endif


#define RVM_CODEGEN_FUNCINITOFFSET 3
#define RVM_CODEGEN_E_NONE 0


typedef struct rvm_codegen_s {
	rarray_t *code;
	rarray_t *data;
	unsigned int codeoff;
	rarray_t *sourceidx;
	rvm_codemap_t *codemap;
	rvm_relocmap_t *relocmap;
	unsigned long cursrcidx;
	unsigned long userdata;
} rvm_codegen_t;


rvm_codegen_t *rvm_codegen_create();
void rvm_codegen_destroy(rvm_codegen_t *cg);
void rvm_codegen_setsource(rvm_codegen_t *cg, unsigned long srcidx);
long rvm_codegen_getsource(rvm_codegen_t *cg, unsigned long codeidx);
unsigned int rvm_codegen_funcstart(rvm_codegen_t *cg, const char* name, unsigned int namesize, unsigned int args);
unsigned int rvm_codegen_funcstart_s(rvm_codegen_t *cg, const char* name, unsigned int args);
unsigned int rvm_codegen_vargs_funcstart(rvm_codegen_t *cg, const char* name, unsigned int namesize);
unsigned int rvm_codegen_vargs_funcstart_s(rvm_codegen_t *cg, const char* name);
void rvm_codegen_funcend(rvm_codegen_t *cg);
unsigned long rvm_codegen_addins(rvm_codegen_t *cg, rvm_asmins_t ins);
unsigned long rvm_codegen_addlabelins(rvm_codegen_t *cg, const char* name, unsigned int namesize, rvm_asmins_t ins);
unsigned long rvm_codegen_addlabelins_s(rvm_codegen_t *cg, const char* name, rvm_asmins_t ins);
unsigned long rvm_codegen_index_addrelocins(rvm_codegen_t *cg, rvm_reloctype_t type, unsigned long index, rvm_asmins_t ins);
unsigned long rvm_codegen_addrelocins(rvm_codegen_t *cg, rvm_reloctype_t type, const char* name, unsigned int namesize, rvm_asmins_t ins);
unsigned long rvm_codegen_addrelocins_s(rvm_codegen_t *cg, rvm_reloctype_t type, const char* name, rvm_asmins_t ins);
unsigned long rvm_codegen_insertins(rvm_codegen_t *cg, unsigned int index, rvm_asmins_t ins);
unsigned long rvm_codegen_replaceins(rvm_codegen_t *cg, unsigned int index, rvm_asmins_t ins);
rvm_asmins_t *rvm_codegen_getcode(rvm_codegen_t *cg, unsigned int index);
unsigned long rvm_codegen_getcodesize(rvm_codegen_t *cg);
void rvm_codegen_setcodesize(rvm_codegen_t *cg, unsigned int size);
void rvm_codegen_clear(rvm_codegen_t *cg);
int rvm_codegen_relocate(rvm_codegen_t *cg, rvm_codelabel_t **err);
long rvm_codegen_validlabel(rvm_codegen_t *cg, long index);
long rvm_codegen_redefinelabel(rvm_codegen_t *cg, long index, unsigned long offset);
long rvm_codegen_redefinelabel_default(rvm_codegen_t *cg, long index);
long rvm_codegen_redefinepointer(rvm_codegen_t *cg, long index, rpointer data);
long rvm_codegen_addlabel(rvm_codegen_t *cg, const char* name, unsigned int namesize, unsigned long offset);
long rvm_codegen_addlabel_s(rvm_codegen_t *cg, const char* name, unsigned long offset);
long rvm_codegen_addlabel_default(rvm_codegen_t *cg, const char* name, unsigned int namesize);
long rvm_codegen_addlabel_default_s(rvm_codegen_t *cg, const char* name);
long rvm_codegen_invalid_addlabel(rvm_codegen_t *cg, const char* name, unsigned int namesize);
long rvm_codegen_invalid_addlabel_s(rvm_codegen_t *cg, const char* name);
long rvm_codegen_adddata(rvm_codegen_t *cg, const char *name, unsigned int namesize, rconstpointer data, unsigned long size);
long rvm_codegen_adddata_s(rvm_codegen_t *cg, const char *name, rconstpointer data, unsigned long size);
long rvm_codegen_addstring(rvm_codegen_t *cg, const char *name, unsigned int namesize, const char* data);
long rvm_codegen_addstring_s(rvm_codegen_t *cg, const char *name, const char* data);
long rvm_codegen_add_numlabel_s(rvm_codegen_t *cg, const char *alphaname, long numname);
long rvm_codegen_invalid_add_numlabel_s(rvm_codegen_t *cg, const char *alphaname, long numname);

#ifdef __cplusplus
}
#endif

#endif

