#include "rmem.h"
#include "rpacompiler.h"
#include "rstring.h"


void rpacompiler_mnode_nan(rpa_compiler_t *co)
{
	rvm_codegen_addlabel_s(co->cg, "rpacompiler_mnode_nan");
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, R_WHT, XX, XX, 0));
}


void rpacompiler_mnode_opt(rpa_compiler_t *co)
{
	rvm_codegen_addlabel_s(co->cg, "rpacompiler_mnode_opt");
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BXL, R_WHT, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_CMP, R0, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BXGRE, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_CMP, R0, R0, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
}


void rpacompiler_mnode_mul(rpa_compiler_t *co)
{
	rvm_codegen_addlabel_s(co->cg, "rpacompiler_mnode_mul");
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BXL, R_WHT, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_CMP, R0, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BGRE, DA, XX, XX, 2));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, PC, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_CLR, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ADD, R0, R0, R1, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BXL, R_WHT, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_CMP, R0, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BGRE, DA, XX, XX, -5));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ADDS, R0, R1, DA, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, PC, XX, XX, 0));
}


void rpacompiler_mnode_mop(rpa_compiler_t *co)
{
	rvm_codegen_addlabel_s(co->cg, "rpacompiler_mnode_mop");
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BXL, R_WHT, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_CMP, R0, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BGRE, DA, XX, XX, 4));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_CMP, R0, R0, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, PC, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_CLR, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ADD, R0, R0, R1, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BXL, R_WHT, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_CMP, R0, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BGRE, DA, XX, XX, -5));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ADDS, R0, R1, DA, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, PC, XX, XX, 0));
}


rpa_compiler_t *rpa_compiler_create()
{
	rpa_compiler_t *co;

	co = r_malloc(sizeof(*co));
	r_memset(co, 0, sizeof(*co));
	co->cg = rvm_codegen_create();
	co->scope = rvm_scope_create();
	co->current.labelidx = -1;
	rpacompiler_mnode_nan(co);
	rpacompiler_mnode_opt(co);
	rpacompiler_mnode_mul(co);
	rpacompiler_mnode_mop(co);
	return co;
}


void rpa_compiler_destroy(rpa_compiler_t *co)
{
	if (co) {
		rvm_codegen_destroy(co->cg);
		rvm_scope_destroy(co->scope);
	}
	r_free(co);
}


rint rpa_compiler_rule_begin(rpa_compiler_t *co, const rchar *name, ruint namesize)
{
	if (co->current.labelidx >= 0)
		return -1;
	co->current.labelidx = rvm_codegen_addlabel(co->cg, name, namesize);
	r_snprintf(co->current.end, sizeof(co->current.end) - 1, "%__end:ld", co->current.labelidx);
	co->current.end[sizeof(co->current.end) - 1] = '\0';
	co->current.endidx = rvm_codemap_invalid_add_s(co->cg->codemap, co->current.end);
	co->current.emitidx = rvm_codegen_adddata_s(co->cg, NULL, name, namesize);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_STRING, co->current.emitidx, rvm_asm(RPA_EMITSTART, DA, R_TOP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(R_TOP)|BIT(R_WHT)|BIT(LR)));
	return 0;
}


rint rpa_compiler_rule_begin_s(rpa_compiler_t *co, const rchar *name)
{
	return rpa_compiler_rule_begin(co, name, r_strlen(name));
}


rint rpa_compiler_rule_end(rpa_compiler_t *co)
{
	if (co->current.labelidx < 0)
		return -1;
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(R1)|BIT(R_WHT)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R1, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_STRING, co->current.emitidx, rvm_asm(RPA_EMITEND, DA, R1, R0, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_addlabel_s(co->cg, co->current.end);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(R_TOP)|BIT(R_WHT)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	co->current.labelidx = -1;
	return 0;
}

