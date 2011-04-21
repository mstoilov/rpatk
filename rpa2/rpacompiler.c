#include "rmem.h"
#include "rpacompiler.h"
#include "rstring.h"

#define OPTIMIZE_MNODE_NAN 1

static rlong rpa_codegen_add_numlabel_s(rvm_codegen_t *cg, const rchar *alphaname, rlong numname)
{
	rchar label[128];

	r_memset(label, 0, sizeof(label));
	r_snprintf(label, sizeof(label) - 1, "L%07ld__%s:", numname, alphaname);
	return rvm_codegen_addlabel_s(cg, label);
}


static rlong rpa_codegen_invalid_add_numlabel_s(rvm_codegen_t *cg, const rchar *alphaname, rlong numname)
{
	rchar label[128];

	r_memset(label, 0, sizeof(label));
	r_snprintf(label, sizeof(label) - 1, "L%07ld__%s:", numname, alphaname);
	return rvm_codegen_invalid_addlabel_s(cg, label);
}


void rpa_compiler_index_reference_nan(rpa_compiler_t *co, rulong index)
{
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, index, rvm_asm(RVM_BL, DA, XX, XX, 0));
}


void rpa_compiler_reference_nan(rpa_compiler_t *co, const rchar *name, rsize_t namesize)
{
	rpa_compiler_index_reference_nan(co, rvm_codemap_lookup(co->cg->codemap, name, namesize));
}


void rpa_compiler_reference_nan_s(rpa_compiler_t *co, const rchar *name)
{
	rpa_compiler_reference_nan(co, name, r_strlen(name));
}


void rpa_compiler_index_reference_opt(rpa_compiler_t *co, rulong index)
{
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, index, rvm_asm(RVM_BL, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOVS, R0, R0, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BGRE, DA, XX, XX, 2));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, 0));
}


void rpa_compiler_reference_opt(rpa_compiler_t *co, const rchar *name, rsize_t namesize)
{
	rpa_compiler_index_reference_opt(co, rvm_codemap_lookup(co->cg->codemap, name, namesize));
}


void rpa_compiler_reference_opt_s(rpa_compiler_t *co, const rchar *name)
{
	rpa_compiler_reference_opt(co, name, r_strlen(name));
}


void rpa_compiler_index_reference_mul(rpa_compiler_t *co, rulong index)
{
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R_TOP, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, index, rvm_asm(RVM_BL, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOVS, R0, R0, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BGRE, DA, XX, XX, -2));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R0, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BGRE, DA, XX, XX, 2));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, -1));
}


void rpa_compiler_reference_mul(rpa_compiler_t *co, const rchar *name, rsize_t namesize)
{
	rpa_compiler_index_reference_mul(co, rvm_codemap_lookup(co->cg->codemap, name, namesize));

}


void rpa_compiler_reference_mul_s(rpa_compiler_t *co, const rchar *name)
{
	rpa_compiler_reference_mul(co, name, r_strlen(name));
}


void rpa_compiler_index_reference_mop(rpa_compiler_t *co, rulong index)
{
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R_TOP, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, index, rvm_asm(RVM_BL, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOVS, R0, R0, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BGRE, DA, XX, XX, -2));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R0, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BGRE, DA, XX, XX, 2));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, 0));
}


void rpa_compiler_reference_mop(rpa_compiler_t *co, const rchar *name, rsize_t namesize)
{
	rpa_compiler_index_reference_mop(co, rvm_codemap_lookup(co->cg->codemap, name, namesize));
}


void rpa_compiler_reference_mop_s(rpa_compiler_t *co, const rchar *name)
{
	rpa_compiler_reference_mop(co, name, r_strlen(name));
}


void rpa_compiler_index_reference(rpa_compiler_t *co, rulong index, ruint qflag)
{
	if (qflag == RPA_MATCH_OPTIONAL) {
		rpa_compiler_index_reference_opt(co, index);
	} else if (qflag == RPA_MATCH_MULTIPLE) {
		rpa_compiler_index_reference_mul(co, index);
	} else if (qflag == RPA_MATCH_MULTIOPT) {
		rpa_compiler_index_reference_mop(co, index);
	} else {
		rpa_compiler_index_reference_nan(co, index);
	}
}


void rpa_compiler_reference(rpa_compiler_t *co, const rchar *name, rsize_t namesize, ruint qflag)
{
	rpa_compiler_index_reference(co, rvm_codemap_lookup(co->cg->codemap, name, namesize), qflag);
}


void rpa_compiler_reference_s(rpa_compiler_t *co, const rchar *name, ruint qflag)
{
	rpa_compiler_reference(co, name, r_strlen(name), qflag);
}


rpa_compiler_t *rpa_compiler_create()
{
	rpa_compiler_t *co;

	co = (rpa_compiler_t *)r_malloc(sizeof(*co));
	r_memset(co, 0, sizeof(*co));
	co->cg = rvm_codegen_create();
	co->scope = rvm_scope_create();
	co->expressions = r_array_create(sizeof(rpa_ruledef_t));
	co->ruleprefs = r_harray_create(sizeof(rpa_rulepref_t));
	return co;
}


void rpa_compiler_destroy(rpa_compiler_t *co)
{
	if (co) {
		rvm_codegen_destroy(co->cg);
		rvm_scope_destroy(co->scope);
		r_object_destroy((robject_t*)co->ruleprefs);
		r_object_destroy((robject_t*)co->expressions);
		r_free(co);
	}
}


rpa_rulepref_t *rpa_compiler_rulepref_lookup(rpa_compiler_t *co, const rchar *name, ruint namesize)
{
	rlong index = r_harray_lookup(co->ruleprefs, name, namesize);
	if (index < 0)
		return NULL;
	return (rpa_rulepref_t *)r_harray_slot(co->ruleprefs, index);
}


rpa_rulepref_t *rpa_compiler_rulepref_lookup_s(rpa_compiler_t *co, const rchar *name)
{
	return rpa_compiler_rulepref_lookup(co, name, r_strlen(name));
}


rpa_rulepref_t *rpa_compiler_rulepref(rpa_compiler_t *co, const rchar *name, ruint namesize)
{
	rlong index = r_harray_lookup(co->ruleprefs, name, namesize);
	if (index < 0)
		index = r_harray_add(co->ruleprefs, name, namesize, NULL);
	return (rpa_rulepref_t *)r_harray_slot(co->ruleprefs, index);
}


rpa_rulepref_t *rpa_compiler_rulepref_s(rpa_compiler_t *co, const rchar *name)
{
	return rpa_compiler_rulepref(co, name, r_strlen(name));
}


void rpa_compiler_rulepref_set_ruleuid(rpa_compiler_t *co, const rchar *name, ruint namesize, rlong ruleuid)
{
	rpa_rulepref_t *rulepref = rpa_compiler_rulepref(co, name, namesize);

	R_ASSERT(rulepref);
	rulepref->ruleuid = ruleuid;
}


void rpa_compiler_rulepref_set_ruleuid_s(rpa_compiler_t *co, const rchar *name, rlong ruleuid)
{
	rpa_compiler_rulepref_set_ruleuid(co, name, r_strlen(name), ruleuid);
}


void rpa_compiler_rulepref_set_flag(rpa_compiler_t *co, const rchar *name, ruint namesize, rulong flag)
{
	rpa_rulepref_t *rulepref = rpa_compiler_rulepref(co, name, namesize);

	R_ASSERT(rulepref);
	rulepref->flags |= flag;
}


void rpa_compiler_rulepref_set_flag_s(rpa_compiler_t *co, const rchar *name, rulong flag)
{
	rpa_compiler_rulepref_set_flag(co, name, r_strlen(name), flag);
}


void rpa_compiler_rulepref_clear_flag(rpa_compiler_t *co, const rchar *name, ruint namesize, rulong flag)
{
	rpa_rulepref_t *rulepref = rpa_compiler_rulepref(co, name, namesize);

	R_ASSERT(rulepref);
	rulepref->flags &= ~flag;
}


void rpa_compiler_rulepref_clear_flag_s(rpa_compiler_t *co, const rchar *name, rulong flag)
{
	rpa_compiler_rulepref_clear_flag(co, name, r_strlen(name), flag);
}


void rpa_compiler_rulepref_set_ruleuid_flags(rpa_compiler_t *co, const rchar *name, ruint namesize, rlong ruleuid, rulong flags)
{
	rpa_rulepref_t *rulepref = rpa_compiler_rulepref(co, name, namesize);

	R_ASSERT(rulepref);
	rulepref->ruleuid = ruleuid;
	rulepref->flags = flags;
}


void rpa_compiler_rulepref_set_ruleuid_flags_s(rpa_compiler_t *co, const rchar *name, rlong ruleuid, rulong flags)
{
	rpa_compiler_rulepref_set_ruleuid_flags(co, name, r_strlen(name), ruleuid, flags);
}


#define RPA_RULEBLOB_SIZE (RPA_RULENAME_MAXSIZE + sizeof(rpa_ruledata_t) + 2*sizeof(rulong))

rlong rpa_compiler_addblob(rpa_compiler_t *co, rlong ruleid, rlong ruleuid, rulong flags, const rchar *name, rulong namesize)
{
	rchar blob[RPA_RULEBLOB_SIZE];
	rchar *ptr;
	rpa_ruledata_t *pblob = (rpa_ruledata_t *)blob;

	if (namesize >= RPA_RULENAME_MAXSIZE)
		return -1;
	r_memset(pblob, 0, RPA_RULEBLOB_SIZE);
	ptr = blob + sizeof(rpa_ruledata_t);
	pblob->name = ptr - blob;
	pblob->ruleid = ruleid;
	pblob->ruleuid = ruleuid;
	pblob->flags = flags;
	pblob->namesize = namesize;
	r_strncpy(ptr, name, namesize);
	ptr += namesize;
	pblob->size = ptr - blob + 1;
	return rvm_codegen_adddata_s(co->cg, NULL, pblob, pblob->size);
}


rlong rpa_compiler_addblob_s(rpa_compiler_t *co, rlong ruleid, rlong ruleuid, rulong flags, const rchar *name)
{
	return 0;
}


rint rpa_compiler_loop_begin(rpa_compiler_t *co, const rchar *name, ruint namesize)
{
	rpa_ruledef_t exp;
	rlong ruleuid = RPA_RECORD_INVALID_UID;
	rulong flags = 0;
	rpa_rulepref_t *rulepref = rpa_compiler_rulepref_lookup(co, name, namesize);

	if (rulepref) {
		flags = rulepref->flags;
		ruleuid = rulepref->ruleuid;
	}

	r_memset(&exp, 0, sizeof(exp));
	exp.start = rvm_codegen_getcodesize(co->cg);
	exp.startidx = rvm_codegen_addlabel(co->cg, name, namesize);
	exp.endidx = rpa_codegen_invalid_add_numlabel_s(co->cg, "__end", exp.start);
	exp.successidx = rpa_codegen_invalid_add_numlabel_s(co->cg, "__success", exp.start);
	exp.failidx = rpa_codegen_invalid_add_numlabel_s(co->cg, "__fail", exp.start);
	exp.againidx = rpa_codegen_invalid_add_numlabel_s(co->cg, "__again", exp.start);
	exp.dataidx = rpa_compiler_addblob(co, exp.start, ruleuid, flags, name, namesize);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_CLR, R3, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_CLR, R0, XX, XX, 0));
	exp.loopidx = rpa_codegen_add_numlabel_s(co->cg, "__loop", exp.start);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(R_REC)|BIT(R_LOO)|BIT(R_TOP)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_LOO, R3, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BLOB, exp.dataidx, rvm_asm(RPA_EMITSTART, DA, R_TOP, XX, 0));

	r_array_add(co->expressions, &exp);
	return 0;
}


rint rpa_compiler_loop_begin_s(rpa_compiler_t *co, const rchar *name)
{
	return rpa_compiler_loop_begin(co, name, r_strlen(name));
}


rint rpa_compiler_loop_end(rpa_compiler_t *co)
{
	rpa_ruledef_t exp = r_array_pop(co->expressions, rpa_ruledef_t);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R3, R_LOO, XX, 0));		// Save LOO to R3 before restoring the old one
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R0, XX, XX, 0));		// Pop the accumulated ret, although ignored here
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(R1)|BIT(R_LOO)|BIT(R_OTP)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R_OTP, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, exp.failidx, rvm_asm(RVM_BEQ, DA, XX, XX, 0));	// ------------- R_TOP is the same
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BLOB, exp.dataidx, rvm_asm(RPA_EMITEND, DA, R_OTP, R0, 0));	//          |
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R3, R0, XX, 0));												//          |
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_TOP, R_OTP, XX, 0));											//          |
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, exp.loopidx, rvm_asm(RVM_B, DA, XX, XX, 0));		//          |
	rvm_codegen_redefinelabel(co->cg, exp.failidx);						                                        //          |
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_REC, R1, XX, 0));			//        <-------------------------------------
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));

	/*
	 *  END FAILED:
	 */
	rvm_codegen_redefinelabel(co->cg, exp.endidx);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R3, R_LOO, XX, 0));		// Save LOO to R3 before restoring the old one
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R0, XX, XX, 0));		// Pop the accumulated ret, use it to save the status for return
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(R_REC)|BIT(R_LOO)|BIT(R_TOP)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_CMP, R0, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BGRE, DA, XX, XX, 3));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ADD, R_TOP, R_TOP, R3, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	return 0;
}


rint rpa_compiler_rule_begin(rpa_compiler_t *co, const rchar *name, ruint namesize)
{
	rpa_ruledef_t exp;
	rlong ruleuid = RPA_RECORD_INVALID_UID;
	rulong flags = 0;
	rpa_rulepref_t *rulepref = rpa_compiler_rulepref_lookup(co, name, namesize);

	if (rulepref) {
		flags = rulepref->flags;
		ruleuid = rulepref->ruleuid;
	}

	r_memset(&exp, 0, sizeof(exp));
	exp.start = rvm_codegen_getcodesize(co->cg);
	exp.startidx = rvm_codegen_addlabel(co->cg, name, namesize);
	exp.endidx = rpa_codegen_invalid_add_numlabel_s(co->cg, "__end", exp.start);
	exp.dataidx = rpa_compiler_addblob(co, exp.start, ruleuid, flags, name, namesize);

	rvm_codegen_addins(co->cg, rvm_asm(RPA_CHECKCACHE, DA, R_TOP, XX, exp.start));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BXGRE, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R_REC, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, LR, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BLOB, exp.dataidx, rvm_asm(RPA_EMITSTART, DA, R_TOP, XX, 0));

	r_array_add(co->expressions, &exp);
	return 0;
}


rint rpa_compiler_rule_begin_s(rpa_compiler_t *co, const rchar *name)
{
	return rpa_compiler_rule_begin(co, name, r_strlen(name));
}


rint rpa_compiler_rule_end(rpa_compiler_t *co)
{
	rpa_ruledef_t exp = r_array_pop(co->expressions, rpa_ruledef_t);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_OTP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R_OTP, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BEQ, DA, XX, XX, 4));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BLOB, exp.dataidx, rvm_asm(RPA_EMITEND, DA, R_OTP, R0, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RPA_SETCACHE, DA, R1, R_REC, exp.start));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_REC, R1, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_redefinelabel(co->cg, exp.endidx);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_REC, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	return 0;
}


rint rpa_compiler_inlinerule_begin(rpa_compiler_t *co, const rchar *name, ruint namesize, ruint flags)
{
	rpa_ruledef_t exp;
	rlong ruleuid = RPA_RECORD_INVALID_UID;
	rulong ruleflags = 0;
	rpa_rulepref_t *rulepref = rpa_compiler_rulepref_lookup(co, name, namesize);

	if (rulepref) {
		ruleflags = rulepref->flags;
		ruleuid = rulepref->ruleuid;
	}

	r_memset(&exp, 0, sizeof(exp));
	exp.branch = rvm_codegen_addins(co->cg, rvm_asm(RVM_B, DA, XX, XX, 0));
	exp.start = rvm_codegen_getcodesize(co->cg);
	exp.startidx = rvm_codegen_addlabel(co->cg, name, namesize);
	exp.endidx = rpa_codegen_invalid_add_numlabel_s(co->cg, "__end", exp.start);
	exp.dataidx = rpa_compiler_addblob(co, exp.start, ruleuid, ruleflags, name, namesize);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R_REC, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, LR, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BLOB, exp.dataidx, rvm_asm(RPA_EMITSTART, DA, R_TOP, XX, 0));
	r_array_add(co->expressions, &exp);
	return 0;
}


rint rpa_compiler_inlinerule_begin_s(rpa_compiler_t *co, const rchar *name, ruint flags)
{
	return rpa_compiler_inlinerule_begin(co, name, r_strlen(name), flags);
}


rint rpa_compiler_inlinerule_end(rpa_compiler_t *co)
{
	rpa_ruledef_t exp = r_array_pop(co->expressions, rpa_ruledef_t);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_OTP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R_OTP, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BEQ, DA, XX, XX, 4));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BLOB, exp.dataidx, rvm_asm(RPA_EMITEND, DA, R_OTP, R0, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_REC, R1, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_redefinelabel(co->cg, exp.endidx);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_REC, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_replaceins(co->cg, exp.branch, rvm_asm(RVM_B, DA, XX, XX, rvm_codegen_getcodesize(co->cg) - exp.branch));
	rpa_compiler_index_reference(co, exp.startidx, (exp.flags & RPA_MATCH_MASK));

	return 0;
}


rint rpa_compiler_exp_begin(rpa_compiler_t *co, ruint flags)
{
	rpa_ruledef_t exp;

	exp.flags = flags;
	exp.branch = rvm_codegen_addins(co->cg, rvm_asm(RVM_B, DA, XX, XX, 0));
	exp.start = rvm_codegen_getcodesize(co->cg);
	exp.startidx = rpa_codegen_add_numlabel_s(co->cg, "__begin", exp.start);
	exp.endidx = rpa_codegen_invalid_add_numlabel_s(co->cg, "__end", exp.start);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R_REC, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, LR, XX, XX, 0));
	r_array_add(co->expressions, &exp);
	return 0;
}


rint rpa_compiler_exp_end(rpa_compiler_t *co)
{
	rpa_ruledef_t exp = r_array_pop(co->expressions, rpa_ruledef_t);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R1, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_redefinelabel(co->cg, exp.endidx);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_REC, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_replaceins(co->cg, exp.branch, rvm_asm(RVM_B, DA, XX, XX, rvm_codegen_getcodesize(co->cg) - exp.branch));
	rpa_compiler_index_reference(co, exp.startidx, (exp.flags & RPA_MATCH_MASK));
	return 0;
}


rint rpa_compiler_altexp_begin(rpa_compiler_t *co, ruint flags)
{
	rpa_ruledef_t exp;

	exp.flags = flags;
	exp.branch = rvm_codegen_addins(co->cg, rvm_asm(RVM_B, DA, XX, XX, 0));
	exp.start = rvm_codegen_getcodesize(co->cg);
	exp.startidx = rpa_codegen_add_numlabel_s(co->cg, "__begin", exp.start);
	exp.endidx = rpa_codegen_invalid_add_numlabel_s(co->cg, "__end", exp.start);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, LR, XX, XX, 0));

	r_array_add(co->expressions, &exp);
	return 0;
}


rint rpa_compiler_altexp_end(rpa_compiler_t *co)
{
	rpa_ruledef_t exp = r_array_pop(co->expressions, rpa_ruledef_t);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_redefinelabel(co->cg, exp.endidx);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R1, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));

	rvm_codegen_replaceins(co->cg, exp.branch, rvm_asm(RVM_B, DA, XX, XX, rvm_codegen_getcodesize(co->cg) - exp.branch));
	rpa_compiler_index_reference(co, exp.startidx, (exp.flags & RPA_MATCH_MASK));
	return 0;
}


rint rpa_compiler_branch_begin(rpa_compiler_t *co, ruint flags)
{
	rpa_ruledef_t exp;

	exp.flags = flags;
	exp.branch = rvm_codegen_addins(co->cg, rvm_asm(RVM_B, DA, XX, XX, 0));
	exp.start = rvm_codegen_getcodesize(co->cg);
	exp.startidx = rpa_codegen_add_numlabel_s(co->cg, "__begin", exp.start);
	exp.endidx = rpa_codegen_invalid_add_numlabel_s(co->cg, "__end", exp.start);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R_REC, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, LR, XX, XX, 0));

	r_array_add(co->expressions, &exp);
	return 0;
}


rint rpa_compiler_branch_end(rpa_compiler_t *co)
{
	rpa_ruledef_t exp = r_array_pop(co->expressions, rpa_ruledef_t);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R1, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_redefinelabel(co->cg, exp.endidx);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_REC, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_replaceins(co->cg, exp.branch, rvm_asm(RVM_B, DA, XX, XX, rvm_codegen_getcodesize(co->cg) - exp.branch));
	rpa_compiler_index_reference(co, exp.startidx, (exp.flags & RPA_MATCH_MASK));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));

	return 0;
}


rint rpa_compiler_nonloopybranch_begin(rpa_compiler_t *co, ruint flags)
{
	rpa_ruledef_t exp;

	exp.flags = flags;
	exp.branch = rvm_codegen_addins(co->cg, rvm_asm(RVM_B, DA, XX, XX, 0));
	exp.start = rvm_codegen_getcodesize(co->cg);
	exp.startidx = rpa_codegen_add_numlabel_s(co->cg, "__begin", exp.start);
	exp.endidx = rpa_codegen_invalid_add_numlabel_s(co->cg, "__end", exp.start);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R_REC, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_CMP, R_LOO, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, exp.endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	r_array_add(co->expressions, &exp);
	return 0;

}


rint rpa_compiler_nonloopybranch_end(rpa_compiler_t *co)
{
	rpa_ruledef_t exp = r_array_pop(co->expressions, rpa_ruledef_t);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R1, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_LOO, R0, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_redefinelabel(co->cg, exp.endidx);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_REC, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_replaceins(co->cg, exp.branch, rvm_asm(RVM_B, DA, XX, XX, rvm_codegen_getcodesize(co->cg) - exp.branch));
	rpa_compiler_index_reference(co, exp.startidx, (exp.flags & RPA_MATCH_MASK));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));

	return 0;
}


rint rpa_compiler_class_begin(rpa_compiler_t *co, ruint flags)
{
	rpa_ruledef_t exp;

	exp.flags = flags;
	exp.branch = rvm_codegen_addins(co->cg, rvm_asm(RVM_B, DA, XX, XX, 0));
	exp.start = rvm_codegen_getcodesize(co->cg);
	exp.startidx = rpa_codegen_add_numlabel_s(co->cg, "__begin", exp.start);
	exp.endidx = rpa_codegen_invalid_add_numlabel_s(co->cg, "__end", exp.start);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, LR, XX, XX, 0));

	r_array_add(co->expressions, &exp);
	return 0;
}


rint rpa_compiler_class_end(rpa_compiler_t *co)
{
	rpa_ruledef_t exp = r_array_pop(co->expressions, rpa_ruledef_t);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_redefinelabel(co->cg, exp.endidx);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R1, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));

	rvm_codegen_replaceins(co->cg, exp.branch, rvm_asm(RVM_B, DA, XX, XX, rvm_codegen_getcodesize(co->cg) - exp.branch));
	rpa_compiler_index_reference(co, exp.startidx, (exp.flags & RPA_MATCH_MASK));
	return 0;
}


rint rpa_compiler_notexp_begin(rpa_compiler_t *co, ruint flags)
{
	rpa_ruledef_t exp;

	exp.flags = flags;
	exp.branch = rvm_codegen_addins(co->cg, rvm_asm(RVM_B, DA, XX, XX, 0));
	exp.start = rvm_codegen_getcodesize(co->cg);
	exp.startidx = rpa_codegen_add_numlabel_s(co->cg, "__begin", exp.start);
	exp.endidx = rpa_codegen_invalid_add_numlabel_s(co->cg, "__end", exp.start);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R_REC, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, LR, XX, XX, 0));

	r_array_add(co->expressions, &exp);
	return 0;
}


rint rpa_compiler_notexp_end(rpa_compiler_t *co)
{
	rpa_ruledef_t exp = r_array_pop(co->expressions, rpa_ruledef_t);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_REC, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_redefinelabel(co->cg, exp.endidx);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHSPCHR_NAN, DA, XX, XX, '.'));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));

	rvm_codegen_replaceins(co->cg, exp.branch, rvm_asm(RVM_B, DA, XX, XX, rvm_codegen_getcodesize(co->cg) - exp.branch));
	rpa_compiler_index_reference(co, exp.startidx, (exp.flags & RPA_MATCH_MASK));
	return 0;
}


rint rpa_compiler_negexp_begin(rpa_compiler_t *co, ruint flags)
{
	rpa_ruledef_t exp;

	exp.flags = flags;
	exp.branch = rvm_codegen_addins(co->cg, rvm_asm(RVM_B, DA, XX, XX, 0));
	exp.start = rvm_codegen_getcodesize(co->cg);
	exp.startidx = rpa_codegen_add_numlabel_s(co->cg, "__begin", exp.start);
	exp.endidx = rpa_codegen_invalid_add_numlabel_s(co->cg, "__end", exp.start);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R_REC, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, LR, XX, XX, 0));

	r_array_add(co->expressions, &exp);
	return 0;

}


rint rpa_compiler_negexp_end(rpa_compiler_t *co)
{
	rpa_ruledef_t exp = r_array_pop(co->expressions, rpa_ruledef_t);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_REC, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_redefinelabel(co->cg, exp.endidx);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHSPCHR_NAN, DA, XX, XX, '.'));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_replaceins(co->cg, exp.branch, rvm_asm(RVM_B, DA, XX, XX, rvm_codegen_getcodesize(co->cg) - exp.branch));
	rpa_compiler_index_reference(co, exp.startidx, (exp.flags & RPA_MATCH_MASK));
	return 0;
}
