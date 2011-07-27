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

#include "rlib/rmem.h"
#include "rvm/rvmcodegen.h"
#include "rvm/rvmcpu.h"


rvm_codegen_t *rvm_codegen_create()
{
	rvm_codegen_t *cg;

	cg = (rvm_codegen_t *)r_malloc(sizeof(*cg));
	if (!cg)
		return (NULL);
	r_memset(cg, 0, sizeof(*cg));
	cg->code = r_array_create(sizeof(rvm_asmins_t));
	cg->data = r_array_create(sizeof(ruint8));
	cg->codemap = rvm_codemap_create();
	cg->relocmap = rvm_relocmap_create();
	cg->sourceidx = r_array_create(sizeof(rsize_t));
	return cg;
}


void rvm_codegen_destroy(rvm_codegen_t *cg)
{
	rvm_codemap_destroy(cg->codemap);
	rvm_relocmap_destroy(cg->relocmap);
	r_array_destroy(cg->data);
	r_array_destroy(cg->code);
	r_array_destroy(cg->sourceidx);
	r_free(cg);
}


void rvm_codegen_clear(rvm_codegen_t *cg)
{
	r_array_setlength(cg->code, 0);
	r_array_setlength(cg->data, 0);
	rvm_codemap_clear(cg->codemap);
	rvm_relocmap_clear(cg->relocmap);
}


void rvm_codegen_setsource(rvm_codegen_t *cg, rsize_t srcidx)
{
	cg->cursrcidx = srcidx;
}


long rvm_codegen_getsource(rvm_codegen_t *cg, rsize_t codeidx)
{
	if (codeidx >= r_array_length(cg->sourceidx))
		return -1;
	return r_array_index(cg->sourceidx, codeidx, long);
}


rvm_asmins_t *rvm_codegen_getcode(rvm_codegen_t *cg, unsigned int index)
{
	return (rvm_asmins_t *)r_array_slot(cg->code, index);
}


rsize_t rvm_codegen_getcodesize(rvm_codegen_t *cg)
{
	return r_array_length(cg->code);
}

void rvm_codegen_setcodesize(rvm_codegen_t *cg, unsigned int size)
{
	r_array_setlength(cg->code, size);
}


rsize_t rvm_codegen_addins(rvm_codegen_t *cg, rvm_asmins_t ins)
{
	rsize_t codeidx;

	codeidx = r_array_add(cg->code, &ins);
	r_array_replace(cg->sourceidx, codeidx, &cg->cursrcidx);
	return codeidx;
}


long rvm_codegen_redefinelabel(rvm_codegen_t *cg, long index, unsigned long offset)
{
	long codeidx = rvm_codemap_lookupadd_s(cg->codemap, ".code");
	rvm_codelabel_t *label = rvm_codemap_label(cg->codemap, index);

	if (!label)
		return -1;
	rvm_codelabel_setoffset(label, codeidx, RVM_CODE2BYTE_OFFSET(offset));
	return index;
}


long rvm_codegen_redefinelabel_default(rvm_codegen_t *cg, long index)
{
	return rvm_codegen_redefinelabel(cg, index, rvm_codegen_getcodesize(cg));
}


long rvm_codegen_validlabel(rvm_codegen_t *cg, long index)
{
	return rvm_codemap_validindex(cg->codemap, index);
}


long rvm_codegen_redefinepointer(rvm_codegen_t *cg, long index, rpointer data)
{
	rvm_codelabel_t *label = rvm_codemap_label(cg->codemap, index);

	if (!label)
		return -1;
//	return rvm_codemap_addpointer(cg->codemap, label->name->str, label->name->size, data);
	rvm_codelabel_setpointer(label, data);
	return index;
}


long rvm_codegen_addlabel(rvm_codegen_t *cg, const char* name, unsigned int namesize, unsigned long offset)
{
	return rvm_codemap_addoffset(cg->codemap, name, namesize, rvm_codemap_lookupadd_s(cg->codemap, ".code"), RVM_CODE2BYTE_OFFSET(offset));
}


long rvm_codegen_addlabel_s(rvm_codegen_t *cg, const char* name, unsigned long offset)
{
	return rvm_codegen_addlabel(cg, name, r_strlen(name), offset);
}


long rvm_codegen_addlabel_default(rvm_codegen_t *cg, const char* name, unsigned int namesize)
{
	return rvm_codegen_addlabel(cg, name, namesize, rvm_codegen_getcodesize(cg));
}


long rvm_codegen_addlabel_default_s(rvm_codegen_t *cg, const char* name)
{
	return rvm_codegen_addlabel_default(cg, name, r_strlen(name));
}


long rvm_codegen_invalid_addlabel(rvm_codegen_t *cg, const char* name, unsigned int namesize)
{
	return rvm_codemap_invalid_add(cg->codemap, name, namesize);
}


long rvm_codegen_invalid_addlabel_s(rvm_codegen_t *cg, const char* name)
{
	return rvm_codegen_invalid_addlabel(cg, name, r_strlen(name));
}


rsize_t rvm_codegen_addlabelins(rvm_codegen_t *cg, const char* name, unsigned int namesize, rvm_asmins_t ins)
{
	rvm_codemap_addoffset(cg->codemap, name, namesize, rvm_codemap_lookupadd_s(cg->codemap, ".code"), RVM_CODE2BYTE_OFFSET(rvm_codegen_getcodesize(cg)));
	return rvm_codegen_addins(cg, ins);
}


rsize_t rvm_codegen_addlabelins_s(rvm_codegen_t *cg, const char* name, rvm_asmins_t ins)
{
	return rvm_codegen_addlabelins(cg, name, r_strlen(name), ins);
}


rsize_t rvm_codegen_index_addrelocins(rvm_codegen_t *cg, rvm_reloctype_t type, unsigned long index, rvm_asmins_t ins)
{
	rvm_relocmap_add(cg->relocmap, RVM_RELOC_CODE, type, rvm_codegen_getcodesize(cg), index);
	return rvm_codegen_addins(cg, ins);
}


rsize_t rvm_codegen_addrelocins(rvm_codegen_t *cg, rvm_reloctype_t type, const char* name, unsigned int namesize, rvm_asmins_t ins)
{
	return rvm_codegen_index_addrelocins(cg, type, rvm_codemap_lookupadd(cg->codemap, name, namesize), ins);
}


rsize_t rvm_codegen_addrelocins_s(rvm_codegen_t *cg, rvm_reloctype_t type, const char* name, rvm_asmins_t ins)
{
	return rvm_codegen_addrelocins(cg, type, name, r_strlen(name), ins);
}


int rvm_codegen_relocate(rvm_codegen_t *cg, rvm_codelabel_t **err)
{
	rvm_codemap_addpointer_s(cg->codemap, ".code", r_array_slot(cg->code, 0));
	rvm_codemap_addpointer_s(cg->codemap, ".data", r_array_slot(cg->data, 0));
	return rvm_relocmap_relocate(cg->relocmap, cg->codemap, (rvm_asmins_t *)r_array_slot(cg->code, 0), err);
}


rsize_t rvm_codegen_insertins(rvm_codegen_t *cg, unsigned int index, rvm_asmins_t ins)
{
	return r_array_insert(cg->code, index, &ins);
}


rsize_t rvm_codegen_replaceins(rvm_codegen_t *cg, unsigned int index, rvm_asmins_t ins)
{
	return r_array_replace(cg->code, index, &ins);

}


unsigned int rvm_codegen_funcstart(rvm_codegen_t *cg, const char* name, unsigned int namesize, unsigned int args)
{
	unsigned int start;
	rvm_codegen_addins(cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));
	start = rvm_codegen_addlabelins(cg, name, namesize, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(FP)|BIT(SP)|BIT(LR)));
	rvm_codegen_addins(cg, rvm_asm(RVM_MOV, FP, SP, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_ADD, SP, SP, DA, args));
//	rvm_codemap_addoffset(cg->codemap, name, namesize, rvm_codemap_lookupadd_s(cg->codemap, ".code"), start);
	return start;
}


unsigned int rvm_codegen_funcstart_s(rvm_codegen_t *cg, const char* name, unsigned int args)
{
	return rvm_codegen_funcstart(cg, name, r_strlen(name), args);
}


unsigned int rvm_codegen_vargs_funcstart(rvm_codegen_t *cg, const char* name, unsigned int namesize)
{
	unsigned int start;
	rvm_codegen_addins(cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));
	start = rvm_codegen_addlabelins(cg, name, namesize, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(FP)|BIT(SP)|BIT(LR)));
	rvm_codegen_addins(cg, rvm_asm(RVM_MOV, FP, SP, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_ADD, SP, SP, R0, 0));
//	rvm_codemap_addoffset(cg->codemap, name, namesize, rvm_codemap_lookupadd_s(cg->codemap, ".code"), start);
	return start;
}


unsigned int rvm_codegen_vargs_funcstart_s(rvm_codegen_t *cg, const char* name)
{
	return rvm_codegen_vargs_funcstart(cg, name, r_strlen(name));
}


void rvm_codegen_funcend(rvm_codegen_t *cg)
{
	rvm_codegen_addins(cg, rvm_asm(RVM_MOV, SP, FP, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(FP)|BIT(SP)|BIT(LR)));
	rvm_codegen_addins(cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
}

long rvm_codegen_adddata(rvm_codegen_t *cg, const char *name, unsigned int namesize, rconstpointer data, rsize_t size)
{
	rpointer buffer;
	unsigned long cursize = R_SIZE_ALIGN(r_array_length(cg->data), sizeof(ruword));

	r_array_setlength(cg->data, cursize + size + sizeof(ruword) + 1);
	buffer = r_array_slot(cg->data, cursize);
	r_memset(buffer, 0, size + sizeof(ruword));
	r_memmove(buffer, data, size);
	return rvm_codemap_addoffset(cg->codemap, name, namesize, rvm_codemap_lookupadd_s(cg->codemap, ".data"), cursize);
}


long rvm_codegen_adddata_s(rvm_codegen_t *cg, const char *name, rconstpointer data, rsize_t size)
{
	return rvm_codegen_adddata(cg, name, r_strlen(name), data, size);
}


long rvm_codegen_addstring(rvm_codegen_t *cg, const char *name, unsigned int namesize, const char* data)
{
	return rvm_codegen_adddata(cg, name, namesize, data, r_strlen(data) + 1);
}


long rvm_codegen_addstring_s(rvm_codegen_t *cg, const char *name, const char* data)
{
	return rvm_codegen_addstring(cg, name, r_strlen(name), data);
}


long rvm_codegen_add_numlabel_s(rvm_codegen_t *cg, const char *alphaname, long numname)
{
	char label[128];

	r_memset(label, 0, sizeof(label));
	r_snprintf(label, sizeof(label) - 1, "L%07ld__%s:", numname, alphaname);
	return rvm_codegen_addlabel_default_s(cg, label);
}


long rvm_codegen_invalid_add_numlabel_s(rvm_codegen_t *cg, const char *alphaname, long numname)
{
	char label[128];

	r_memset(label, 0, sizeof(label));
	r_snprintf(label, sizeof(label) - 1, "L%07ld__%s:", numname, alphaname);
	return rvm_codegen_invalid_addlabel_s(cg, label);
}

