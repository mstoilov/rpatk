#include "rmem.h"
#include "rvmcodegen.h"
#include "rvmcpu.h"


rvm_codegen_t *rvm_codegen_create()
{
	rvm_codegen_t *cg;

	cg = (rvm_codegen_t *)r_malloc(sizeof(*cg));
	if (!cg)
		return (NULL);
	r_memset(cg, 0, sizeof(*cg));
	cg->code = r_array_create(sizeof(rvm_asmins_t));
	cg->data = r_array_create(sizeof(rbyte));
	cg->codemap = rvm_codemap_create();
	cg->relocmap = rvm_relocmap_create();
	return cg;
}


void rvm_codegen_destroy(rvm_codegen_t *cg)
{
	rvm_codemap_destroy(cg->codemap);
	rvm_relocmap_destroy(cg->relocmap);
	r_object_destroy((robject_t*)cg->data);
	r_object_destroy((robject_t*)cg->code);
	r_free(cg);
}


void rvm_codegen_clear(rvm_codegen_t *cg)
{
	r_array_setlength(cg->code, 0);
	r_array_setlength(cg->data, 0);
	rvm_codemap_clear(cg->codemap);
	rvm_relocmap_clear(cg->relocmap);
}


rvm_asmins_t *rvm_codegen_getcode(rvm_codegen_t *cg, ruint index)
{
	return (rvm_asmins_t *)r_array_slot(cg->code, index);
}


rulong rvm_codegen_getcodesize(rvm_codegen_t *cg)
{
	return r_array_length(cg->code);
}

void rvm_codegen_setcodesize(rvm_codegen_t *cg, ruint size)
{
	r_array_setlength(cg->code, size);
}


ruint rvm_codegen_addins(rvm_codegen_t *cg, rvm_asmins_t ins)
{
	return r_array_add(cg->code, &ins);
}


rlong rvm_codegen_redefinelabel(rvm_codegen_t *cg, rlong index)
{
	rvm_codelabel_t *label = rvm_codemap_label(cg->codemap, index);

	if (!label)
		return -1;
	return rvm_codemap_addoffset(cg->codemap, label->name->str, label->name->size, rvm_codemap_lookup_s(cg->codemap, ".code"), RVM_CODE2BYTE_OFFSET(rvm_codegen_getcodesize(cg)));
}


rlong rvm_codegen_validlabel(rvm_codegen_t *cg, rlong index)
{
	return rvm_codemap_validindex(cg->codemap, index);
}


rlong rvm_codegen_redefinepointer(rvm_codegen_t *cg, rlong index, rpointer data)
{
	rvm_codelabel_t *label = rvm_codemap_label(cg->codemap, index);

	if (!label)
		return -1;
	return rvm_codemap_addpointer(cg->codemap, label->name->str, label->name->size, data);
}


rlong rvm_codegen_addlabel(rvm_codegen_t *cg, const rchar* name, ruint namesize)
{
	return rvm_codemap_addoffset(cg->codemap, name, namesize, rvm_codemap_lookup_s(cg->codemap, ".code"), RVM_CODE2BYTE_OFFSET(rvm_codegen_getcodesize(cg)));
}


rlong rvm_codegen_addlabel_s(rvm_codegen_t *cg, const rchar* name)
{
	return rvm_codegen_addlabel(cg, name, r_strlen(name));
}


rlong rvm_codegen_invalid_addlabel(rvm_codegen_t *cg, const rchar* name, ruint namesize)
{
	return rvm_codemap_invalid_add(cg->codemap, name, namesize);
}


rlong rvm_codegen_invalid_addlabel_s(rvm_codegen_t *cg, const rchar* name)
{
	return rvm_codegen_invalid_addlabel(cg, name, r_strlen(name));
}


ruint rvm_codegen_addlabelins(rvm_codegen_t *cg, const rchar* name, ruint namesize, rvm_asmins_t ins)
{
	rvm_codemap_addoffset(cg->codemap, name, namesize, rvm_codemap_lookup_s(cg->codemap, ".code"), RVM_CODE2BYTE_OFFSET(rvm_codegen_getcodesize(cg)));
	return rvm_codegen_addins(cg, ins);
}


ruint rvm_codegen_addlabelins_s(rvm_codegen_t *cg, const rchar* name, rvm_asmins_t ins)
{
	return rvm_codegen_addlabelins(cg, name, r_strlen(name), ins);
}


ruint rvm_codegen_index_addrelocins(rvm_codegen_t *cg, rvm_reloctype_t type, rulong index, rvm_asmins_t ins)
{
	rvm_relocmap_add(cg->relocmap, type, rvm_codegen_getcodesize(cg), index);
	return rvm_codegen_addins(cg, ins);
}


ruint rvm_codegen_addrelocins(rvm_codegen_t *cg, rvm_reloctype_t type, const rchar* name, ruint namesize, rvm_asmins_t ins)
{
	return rvm_codegen_index_addrelocins(cg, type, rvm_codemap_lookup(cg->codemap, name, namesize), ins);
}


ruint rvm_codegen_addrelocins_s(rvm_codegen_t *cg, rvm_reloctype_t type, const rchar* name, rvm_asmins_t ins)
{
	return rvm_codegen_addrelocins(cg, type, name, r_strlen(name), ins);
}


rint rvm_codegen_relocate(rvm_codegen_t *cg, rvm_codelabel_t **err)
{
	rvm_codemap_addpointer_s(cg->codemap, ".code", r_array_slot(cg->code, 0));
	rvm_codemap_addpointer_s(cg->codemap, ".data", r_array_slot(cg->data, 0));
	return rvm_relocmap_relocate(cg->relocmap, cg->codemap, (rvm_asmins_t *)r_array_slot(cg->code, 0), err);
}


ruint rvm_codegen_insertins(rvm_codegen_t *cg, ruint index, rvm_asmins_t ins)
{
	return r_array_insert(cg->code, index, &ins);
}


ruint rvm_codegen_replaceins(rvm_codegen_t *cg, ruint index, rvm_asmins_t ins)
{
	return r_array_replace(cg->code, index, &ins);

}


ruint rvm_codegen_funcstart(rvm_codegen_t *cg, const rchar* name, ruint namesize, ruint args)
{
	ruint start;
	rvm_codegen_addins(cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));
	start = rvm_codegen_addlabelins(cg, name, namesize, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(FP)|BIT(SP)|BIT(LR)));
	rvm_codegen_addins(cg, rvm_asm(RVM_MOV, FP, SP, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_ADD, SP, SP, DA, args));
//	rvm_codemap_addoffset(cg->codemap, name, namesize, rvm_codemap_lookup_s(cg->codemap, ".code"), start);
	return start;
}


ruint rvm_codegen_funcstart_s(rvm_codegen_t *cg, const rchar* name, ruint args)
{
	return rvm_codegen_funcstart(cg, name, r_strlen(name), args);
}


ruint rvm_codegen_vargs_funcstart(rvm_codegen_t *cg, const rchar* name, ruint namesize)
{
	ruint start;
	rvm_codegen_addins(cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));
	start = rvm_codegen_addlabelins(cg, name, namesize, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(FP)|BIT(SP)|BIT(LR)));
	rvm_codegen_addins(cg, rvm_asm(RVM_MOV, FP, SP, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_ADD, SP, SP, R0, 0));
//	rvm_codemap_addoffset(cg->codemap, name, namesize, rvm_codemap_lookup_s(cg->codemap, ".code"), start);
	return start;
}


ruint rvm_codegen_vargs_funcstart_s(rvm_codegen_t *cg, const rchar* name)
{
	return rvm_codegen_vargs_funcstart(cg, name, r_strlen(name));
}


void rvm_codegen_funcend(rvm_codegen_t *cg)
{
	rvm_codegen_addins(cg, rvm_asm(RVM_MOV, SP, FP, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(FP)|BIT(SP)|BIT(LR)));
	rvm_codegen_addins(cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
}

rlong rvm_codegen_adddata(rvm_codegen_t *cg, const rchar *name, ruint namesize, rconstpointer data, rsize_t size)
{
	rpointer buffer;
	rulong cursize = R_SIZE_ALIGN(r_array_length(cg->data), sizeof(rword));

	r_array_setlength(cg->data, cursize + size + sizeof(rword));
	buffer = r_array_slot(cg->data, cursize);
	r_memset(buffer, 0, size + sizeof(rword));
	r_memmove(buffer, data, size);
	return rvm_codemap_addoffset(cg->codemap, name, namesize, rvm_codemap_lookup_s(cg->codemap, ".data"), cursize);
}


rlong rvm_codegen_adddata_s(rvm_codegen_t *cg, const rchar *name, rconstpointer data, rsize_t size)
{
	return rvm_codegen_adddata(cg, name, r_strlen(name), data, size);
}


rlong rvm_codegen_addstring(rvm_codegen_t *cg, const rchar *name, ruint namesize, const rchar* data)
{
	return rvm_codegen_adddata(cg, name, namesize, data, r_strlen(data) + 1);
}


rlong rvm_codegen_addstring_s(rvm_codegen_t *cg, const rchar *name, const rchar* data)
{
	return rvm_codegen_addstring(cg, name, r_strlen(name), data);
}


rlong rvm_codegen_add_numlabel_s(rvm_codegen_t *cg, const rchar *alphaname, rlong numname)
{
	rchar label[128];

	r_memset(label, 0, sizeof(label));
	r_snprintf(label, sizeof(label) - 1, "L%07ld__%s:", numname, alphaname);
	return rvm_codegen_addlabel_s(cg, label);
}


rlong rvm_codegen_invalid_add_numlabel_s(rvm_codegen_t *cg, const rchar *alphaname, rlong numname)
{
	rchar label[128];

	r_memset(label, 0, sizeof(label));
	r_snprintf(label, sizeof(label) - 1, "L%07ld__%s:", numname, alphaname);
	return rvm_codegen_invalid_addlabel_s(cg, label);
}

