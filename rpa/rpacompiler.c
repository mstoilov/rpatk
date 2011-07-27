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
#include "rpa/rpacompiler.h"
#include "rlib/rstring.h"


static long rpa_codegen_add_numlabel_s(rvm_codegen_t *cg, const char *alphaname, long numname)
{
#if 0
	/*
	 * The alphaname and numname, might be used for
	 * debugging at some point, but not used for now
	 */
	char label[128];

	r_memset(label, 0, sizeof(label));
	r_snprintf(label, sizeof(label) - 1, "L%07ld__%s:", numname, alphaname);
	return rvm_codegen_addlabel_default_s(cg, label);
#else
	return rvm_codegen_addlabel_default_s(cg, NULL);
#endif
}


static long rpa_codegen_invalid_add_numlabel_s(rvm_codegen_t *cg, const char *alphaname, long numname)
{
#if 0
	/*
	 * The alphaname and numname, might be used for
	 * debugging at some point, but not used for now
	 */
	char label[128];

	r_memset(label, 0, sizeof(label));
	r_snprintf(label, sizeof(label) - 1, "L%07ld__%s:", numname, alphaname);
	return rvm_codegen_invalid_addlabel_s(cg, label);
#else
	return rvm_codegen_invalid_addlabel_s(cg, NULL);
#endif
}


void rpa_compiler_index_reference_nan(rpa_compiler_t *co, unsigned long index)
{
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, index, rvm_asm(RVM_BL, DA, XX, XX, 0));
}


void rpa_compiler_reference_nan(rpa_compiler_t *co, const char *name, unsigned int namesize)
{
	rpa_compiler_index_reference_nan(co, rvm_codemap_lookupadd(co->cg->codemap, name, namesize));
}


void rpa_compiler_reference_nan_s(rpa_compiler_t *co, const char *name)
{
	rpa_compiler_reference_nan(co, name, r_strlen(name));
}


void rpa_compiler_index_reference_opt(rpa_compiler_t *co, unsigned long index)
{
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, index, rvm_asm(RVM_BL, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOVS, R0, R0, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BGRE, DA, XX, XX, 2));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, 0));
}


void rpa_compiler_reference_opt(rpa_compiler_t *co, const char *name, unsigned int namesize)
{
	rpa_compiler_index_reference_opt(co, rvm_codemap_lookupadd(co->cg->codemap, name, namesize));
}


void rpa_compiler_reference_opt_s(rpa_compiler_t *co, const char *name)
{
	rpa_compiler_reference_opt(co, name, r_strlen(name));
}


void rpa_compiler_index_reference_mul(rpa_compiler_t *co, unsigned long index)
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


void rpa_compiler_reference_mul(rpa_compiler_t *co, const char *name, unsigned int namesize)
{
	rpa_compiler_index_reference_mul(co, rvm_codemap_lookupadd(co->cg->codemap, name, namesize));

}


void rpa_compiler_reference_mul_s(rpa_compiler_t *co, const char *name)
{
	rpa_compiler_reference_mul(co, name, r_strlen(name));
}


void rpa_compiler_index_reference_mop(rpa_compiler_t *co, unsigned long index)
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


void rpa_compiler_reference_mop(rpa_compiler_t *co, const char *name, unsigned int namesize)
{
	rpa_compiler_index_reference_mop(co, rvm_codemap_lookupadd(co->cg->codemap, name, namesize));
}


void rpa_compiler_reference_mop_s(rpa_compiler_t *co, const char *name)
{
	rpa_compiler_reference_mop(co, name, r_strlen(name));
}


void rpa_compiler_index_reference(rpa_compiler_t *co, unsigned long index, unsigned int qflag)
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


void rpa_compiler_reference(rpa_compiler_t *co, const char *name, unsigned int namesize, unsigned int qflag)
{
	rpa_compiler_index_reference(co, rvm_codemap_lookupadd(co->cg->codemap, name, namesize), qflag);
}


void rpa_compiler_reference_s(rpa_compiler_t *co, const char *name, unsigned int qflag)
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


rpa_rulepref_t *rpa_compiler_rulepref_lookup(rpa_compiler_t *co, const char *name, unsigned int namesize)
{
	long index = r_harray_lookup(co->ruleprefs, name, namesize);
	if (index < 0)
		return NULL;
	return (rpa_rulepref_t *)r_harray_slot(co->ruleprefs, index);
}


rpa_rulepref_t *rpa_compiler_rulepref_lookup_s(rpa_compiler_t *co, const char *name)
{
	return rpa_compiler_rulepref_lookup(co, name, r_strlen(name));
}


rpa_rulepref_t *rpa_compiler_rulepref(rpa_compiler_t *co, const char *name, unsigned int namesize)
{
	long index = r_harray_lookup(co->ruleprefs, name, namesize);
	if (index < 0)
		index = r_harray_add(co->ruleprefs, name, namesize, NULL);
	return (rpa_rulepref_t *)r_harray_slot(co->ruleprefs, index);
}


rpa_rulepref_t *rpa_compiler_rulepref_s(rpa_compiler_t *co, const char *name)
{
	return rpa_compiler_rulepref(co, name, r_strlen(name));
}


void rpa_compiler_rulepref_set_ruleid(rpa_compiler_t *co, const char *name, unsigned int namesize, long ruleid)
{
	rpa_rulepref_t *rulepref = rpa_compiler_rulepref(co, name, namesize);

	R_ASSERT(rulepref);
	rulepref->ruleid = ruleid;
}


void rpa_compiler_rulepref_set_ruleid_s(rpa_compiler_t *co, const char *name, long ruleid)
{
	rpa_compiler_rulepref_set_ruleid(co, name, r_strlen(name), ruleid);
}


void rpa_compiler_rulepref_set_ruleuid(rpa_compiler_t *co, const char *name, unsigned int namesize, long ruleuid)
{
	rpa_rulepref_t *rulepref = rpa_compiler_rulepref(co, name, namesize);

	R_ASSERT(rulepref);
	rulepref->ruleuid = ruleuid;
}


void rpa_compiler_rulepref_set_ruleuid_s(rpa_compiler_t *co, const char *name, long ruleuid)
{
	rpa_compiler_rulepref_set_ruleuid(co, name, r_strlen(name), ruleuid);
}


void rpa_compiler_rulepref_set_flag(rpa_compiler_t *co, const char *name, unsigned int namesize, unsigned long flag)
{
	rpa_rulepref_t *rulepref = rpa_compiler_rulepref(co, name, namesize);

	R_ASSERT(rulepref);
	rulepref->flags |= flag;
}


void rpa_compiler_rulepref_set_flag_s(rpa_compiler_t *co, const char *name, unsigned long flag)
{
	rpa_compiler_rulepref_set_flag(co, name, r_strlen(name), flag);
}


void rpa_compiler_rulepref_clear_flag(rpa_compiler_t *co, const char *name, unsigned int namesize, unsigned long flag)
{
	rpa_rulepref_t *rulepref = rpa_compiler_rulepref(co, name, namesize);

	R_ASSERT(rulepref);
	rulepref->flags &= ~flag;
}


void rpa_compiler_rulepref_clear_flag_s(rpa_compiler_t *co, const char *name, unsigned long flag)
{
	rpa_compiler_rulepref_clear_flag(co, name, r_strlen(name), flag);
}


void rpa_compiler_rulepref_set(rpa_compiler_t *co, const char *name, unsigned int namesize, long ruleid, long ruleuid, unsigned long flags)
{
	rpa_rulepref_t *rulepref = rpa_compiler_rulepref(co, name, namesize);

	R_ASSERT(rulepref);
	rulepref->ruleid = ruleid;
	rulepref->ruleuid = ruleuid;
	rulepref->flags = flags;
}


void rpa_compiler_rulepref_set_s(rpa_compiler_t *co, const char *name, long ruleid, long ruleuid, unsigned long flags)
{
	rpa_compiler_rulepref_set(co, name, r_strlen(name), ruleid, ruleuid, flags);
}


#define RPA_RULEBLOB_SIZE (RPA_RULENAME_MAXSIZE + sizeof(rpa_ruledata_t) + 2*sizeof(unsigned long))

long rpa_compiler_addblob(rpa_compiler_t *co, long ruleid, long ruleuid, unsigned long flags, const char *name, unsigned long namesize)
{
	char blob[RPA_RULEBLOB_SIZE];
	char *ptr;
	rpa_ruledata_t *pblob = (rpa_ruledata_t *)blob;

	if (namesize >= RPA_RULENAME_MAXSIZE)
		return -1;
	r_memset(pblob, 0, RPA_RULEBLOB_SIZE);
	ptr = blob + sizeof(rpa_ruledata_t);
	pblob->name = (unsigned long)(ptr - blob);
	pblob->ruleid = ruleid;
	pblob->ruleuid = ruleuid;
	pblob->flags = flags;
	pblob->namesize = namesize;
	r_strncpy(ptr, name, namesize);
	ptr += namesize;
	pblob->size = (long)(ptr - blob + 1);
	return rvm_codegen_adddata_s(co->cg, NULL, pblob, pblob->size);
}


long rpa_compiler_addblob_s(rpa_compiler_t *co, long ruleid, long ruleuid, unsigned long flags, const char *name)
{
	return 0;
}


int rpa_compiler_loop_begin(rpa_compiler_t *co, const char *name, unsigned int namesize)
{
	rpa_ruledef_t exp;
	long ruleuid = 0;
	long ruleid = 0;
	unsigned long flags = 0;

	r_memset(&exp, 0, sizeof(exp));
	exp.rulepref = rpa_compiler_rulepref_lookup(co, name, namesize);
	if (exp.rulepref) {
		flags = exp.rulepref->flags;
		ruleuid = exp.rulepref->ruleuid;
		ruleid = exp.rulepref->ruleid;
	}
	exp.start = rvm_codegen_getcodesize(co->cg);
	exp.startidx = rvm_codegen_addlabel_default(co->cg, name, namesize);
	exp.endidx = rpa_codegen_invalid_add_numlabel_s(co->cg, "__end", exp.start);
	exp.successidx = rpa_codegen_invalid_add_numlabel_s(co->cg, "__success", exp.start);
	exp.failidx = rpa_codegen_invalid_add_numlabel_s(co->cg, "__fail", exp.start);
	exp.againidx = rpa_codegen_invalid_add_numlabel_s(co->cg, "__again", exp.start);
	exp.dataidx = rpa_compiler_addblob(co, ruleid, ruleuid, flags, name, namesize);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_CLR, R3, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_CLR, R0, XX, XX, 0));
	exp.loopidx = rpa_codegen_add_numlabel_s(co->cg, "__loop", exp.start);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(R_REC)|BIT(R_LOO)|BIT(R_TOP)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_LOO, R3, XX, 0));
	if (exp.rulepref && (exp.rulepref->flags & RPA_RFLAG_EMITRECORD)) {
		rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BLOB, exp.dataidx, rvm_asm(RPA_EMITSTART, DA, R_TOP, XX, 0));
	}
	r_array_add(co->expressions, &exp);
	return 0;
}


int rpa_compiler_loop_begin_s(rpa_compiler_t *co, const char *name)
{
	return rpa_compiler_loop_begin(co, name, r_strlen(name));
}


int rpa_compiler_loop_end(rpa_compiler_t *co)
{
	rpa_ruledef_t exp = r_array_pop(co->expressions, rpa_ruledef_t);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R3, R_LOO, XX, 0));		// Save LOO to R3 before restoring the old one
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R0, XX, XX, 0));		// Pop the accumulated ret, although ignored here
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(R1)|BIT(R_LOO)|BIT(R_OTP)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R_OTP, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, exp.failidx, rvm_asm(RVM_BEQ, DA, XX, XX, 0));	// ------------- R_TOP is the same
	if (exp.rulepref && (exp.rulepref->flags & RPA_RFLAG_EMITRECORD)) {                                            //       |
		rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BLOB, exp.dataidx, rvm_asm(RPA_EMITEND, DA, R_OTP, R0, 0)); //      |
	}
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R3, R0, XX, 0));                                                //          |
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_TOP, R_OTP, XX, 0));                                          //          |
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, exp.loopidx, rvm_asm(RVM_B, DA, XX, XX, 0));        //          |
	rvm_codegen_redefinelabel_default(co->cg, exp.failidx);	                                                    //          |
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_REC, R1, XX, 0));	         //        <-------------------------------------
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));

	/*
	 *  END FAILED:
	 */
	rvm_codegen_redefinelabel_default(co->cg, exp.endidx);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R3, R_LOO, XX, 0));		// Save LOO to R3 before restoring the old one
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R0, XX, XX, 0));		// Pop the accumulated ret, use it to save the status for return
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(R_REC)|BIT(R_LOO)|BIT(R_TOP)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_CMP, R0, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BGRE, DA, XX, XX, 3));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, -1));
	if (exp.rulepref && (exp.rulepref->flags & RPA_RFLAG_ABORTONFAIL)) {
		rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BLOB, exp.dataidx, rvm_asm(RPA_ABORT, DA, R_OTP, R0, 0));
	} else {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	}
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ADD, R_TOP, R_TOP, R3, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	return 0;
}


int rpa_compiler_rule_begin(rpa_compiler_t *co, const char *name, unsigned int namesize, rpabitmap_t bitmap)
{
	rpa_ruledef_t exp;
	long ruleuid = 0;
	long ruleid = 0;
	unsigned long flags = 0;

	r_memset(&exp, 0, sizeof(exp));
	exp.rulepref = rpa_compiler_rulepref_lookup(co, name, namesize);
	if (exp.rulepref) {
		flags = exp.rulepref->flags;
		ruleuid = exp.rulepref->ruleuid;
		ruleid = exp.rulepref->ruleid;
	}
	exp.start = rvm_codegen_getcodesize(co->cg);
	exp.startidx = rvm_codegen_addlabel_default(co->cg, name, namesize);
	exp.endidx = rpa_codegen_invalid_add_numlabel_s(co->cg, "__end", exp.start);
	exp.dataidx = rpa_compiler_addblob(co, ruleid, ruleuid, flags, name, namesize);
	exp.bitmap = bitmap;
	if (exp.bitmap) {
		rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHBITMAP, DA, XX, XX, exp.bitmap));
		rvm_codegen_addins(co->cg, rvm_asm(RVM_BXLES, LR, XX, XX, 0));
	}
	rvm_codegen_addins(co->cg, rvm_asm(RPA_CHECKCACHE, DA, R_TOP, XX, exp.start));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BXNEQ, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R_REC, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, LR, XX, XX, 0));
	if (exp.rulepref && (exp.rulepref->flags & RPA_RFLAG_EMITRECORD)) {
		rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BLOB, exp.dataidx, rvm_asm(RPA_EMITSTART, DA, R_TOP, XX, 0));
	}

	r_array_add(co->expressions, &exp);
	return 0;
}


int rpa_compiler_rule_begin_s(rpa_compiler_t *co, const char *name, rpabitmap_t bitmap)
{
	return rpa_compiler_rule_begin(co, name, r_strlen(name), bitmap);
}


int rpa_compiler_rule_end(rpa_compiler_t *co)
{
	rpa_ruledef_t exp = r_array_pop(co->expressions, rpa_ruledef_t);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_OTP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R_OTP, 0));

	if (exp.rulepref && (exp.rulepref->flags & RPA_RFLAG_EMITRECORD)) {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_BEQ, DA, XX, XX, 4));
		rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BLOB, exp.dataidx, rvm_asm(RPA_EMITEND, DA, R_OTP, R0, 0));
	} else {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_BEQ, DA, XX, XX, 3));
	}
	rvm_codegen_addins(co->cg, rvm_asml(RPA_SETCACHE, DA, R1, R_REC, exp.start));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_REC, R1, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, -1));
	if (exp.rulepref && (exp.rulepref->flags & RPA_RFLAG_ABORTONFAIL)) {
		rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BLOB, exp.dataidx, rvm_asm(RPA_ABORT, DA, R_OTP, R0, 0));
	}
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_redefinelabel_default(co->cg, exp.endidx);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_REC, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_OTP, R_TOP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asml(RPA_SETCACHE, DA, R_REC, R_REC, exp.start));
	if (exp.rulepref && (exp.rulepref->flags & RPA_RFLAG_ABORTONFAIL)) {
		rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BLOB, exp.dataidx, rvm_asm(RPA_ABORT, DA, R_OTP, R0, 0));
	}
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	return 0;
}


int rpa_compiler_inlinerule_begin(rpa_compiler_t *co, const char *name, unsigned int namesize, unsigned int flags)
{
	rpa_ruledef_t exp;
	long ruleuid = 0;
	long ruleid = 0;
	unsigned long ruleflags = 0;

	r_memset(&exp, 0, sizeof(exp));
	exp.rulepref = rpa_compiler_rulepref_lookup(co, name, namesize);
	if (exp.rulepref) {
		ruleflags = exp.rulepref->flags;
		ruleuid = exp.rulepref->ruleuid;
		ruleid = exp.rulepref->ruleid;
	}

	exp.branch = rvm_codegen_addins(co->cg, rvm_asm(RVM_B, DA, XX, XX, 0));
	exp.start = rvm_codegen_getcodesize(co->cg);
	exp.startidx = rpa_codegen_add_numlabel_s(co->cg, "__inlined", exp.start);
	exp.endidx = rpa_codegen_invalid_add_numlabel_s(co->cg, "__end", exp.start);
	exp.dataidx = rpa_compiler_addblob(co, ruleid, ruleuid, ruleflags, name, namesize);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R_REC, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, LR, XX, XX, 0));
	if (exp.rulepref && (exp.rulepref->flags & RPA_RFLAG_EMITRECORD)) {
		rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BLOB, exp.dataidx, rvm_asm(RPA_EMITSTART, DA, R_TOP, XX, 0));
	}
	r_array_add(co->expressions, &exp);
	return 0;
}


int rpa_compiler_inlinerule_begin_s(rpa_compiler_t *co, const char *name, unsigned int flags)
{
	return rpa_compiler_inlinerule_begin(co, name, r_strlen(name), flags);
}


int rpa_compiler_inlinerule_end(rpa_compiler_t *co)
{
	rpa_ruledef_t exp = r_array_pop(co->expressions, rpa_ruledef_t);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_OTP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R_OTP, 0));
	if (exp.rulepref && (exp.rulepref->flags & RPA_RFLAG_EMITRECORD)) {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_BEQ, DA, XX, XX, 3));
		rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BLOB, exp.dataidx, rvm_asm(RPA_EMITEND, DA, R_OTP, R0, 0));
	} else {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_BEQ, DA, XX, XX, 2));
	}
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_REC, R1, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_redefinelabel_default(co->cg, exp.endidx);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_REC, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_replaceins(co->cg, exp.branch, rvm_asm(RVM_B, DA, XX, XX, rvm_codegen_getcodesize(co->cg) - exp.branch));
	rpa_compiler_index_reference(co, exp.startidx, (exp.flags & RPA_MATCH_MASK));

	return 0;
}


int rpa_compiler_exp_begin(rpa_compiler_t *co, unsigned int flags, rpabitmap_t bitmap)
{
	rpa_ruledef_t exp;

	exp.flags = flags;
	exp.branch = rvm_codegen_addins(co->cg, rvm_asm(RVM_B, DA, XX, XX, 0));
	exp.start = rvm_codegen_getcodesize(co->cg);
	exp.startidx = rpa_codegen_add_numlabel_s(co->cg, "__begin", exp.start);
	exp.endidx = rpa_codegen_invalid_add_numlabel_s(co->cg, "__end", exp.start);
	exp.bitmap = bitmap;
	if (exp.bitmap) {
		rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHBITMAP, DA, XX, XX, exp.bitmap));
		rvm_codegen_addins(co->cg, rvm_asm(RVM_BXLES, LR, XX, XX, 0));
	}
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R_REC, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, LR, XX, XX, 0));
	r_array_add(co->expressions, &exp);
	return 0;
}


int rpa_compiler_exp_end(rpa_compiler_t *co)
{
	rpa_ruledef_t exp = r_array_pop(co->expressions, rpa_ruledef_t);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R1, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_redefinelabel_default(co->cg, exp.endidx);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_REC, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_replaceins(co->cg, exp.branch, rvm_asm(RVM_B, DA, XX, XX, rvm_codegen_getcodesize(co->cg) - exp.branch));
	rpa_compiler_index_reference(co, exp.startidx, (exp.flags & RPA_MATCH_MASK));
	return 0;
}


int rpa_compiler_altexp_begin(rpa_compiler_t *co, unsigned int flags, rpabitmap_t bitmap)
{

	rpa_ruledef_t exp;

	exp.flags = flags;
	exp.branch = rvm_codegen_addins(co->cg, rvm_asm(RVM_B, DA, XX, XX, 0));
	exp.start = rvm_codegen_getcodesize(co->cg);
	exp.startidx = rpa_codegen_add_numlabel_s(co->cg, "__begin", exp.start);
	exp.endidx = rpa_codegen_invalid_add_numlabel_s(co->cg, "__end", exp.start);
	exp.bitmap = bitmap;
	if (exp.bitmap) {
		rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHBITMAP, DA, XX, XX, exp.bitmap));
		rvm_codegen_addins(co->cg, rvm_asm(RVM_BXLES, LR, XX, XX, 0));
	}
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, LR, XX, XX, 0));

	r_array_add(co->expressions, &exp);
	return 0;
}


int rpa_compiler_altexp_end(rpa_compiler_t *co)
{
	rpa_ruledef_t exp = r_array_pop(co->expressions, rpa_ruledef_t);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_redefinelabel_default(co->cg, exp.endidx);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R1, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));

	rvm_codegen_replaceins(co->cg, exp.branch, rvm_asm(RVM_B, DA, XX, XX, rvm_codegen_getcodesize(co->cg) - exp.branch));
	rpa_compiler_index_reference(co, exp.startidx, (exp.flags & RPA_MATCH_MASK));
//	rvm_codegen_addins(co->cg, rvm_asm(RPA_VERIFYBITMAP, DA, XX, XX, exp.bitmap));
	return 0;
}


int rpa_compiler_branch_begin(rpa_compiler_t *co, unsigned int flags, rpabitmap_t bitmap)
{
	rpa_ruledef_t exp;

	exp.flags = flags;
	exp.branch = rvm_codegen_addins(co->cg, rvm_asm(RVM_B, DA, XX, XX, 0));
	exp.start = rvm_codegen_getcodesize(co->cg);
	exp.startidx = rpa_codegen_add_numlabel_s(co->cg, "__begin", exp.start);
	exp.endidx = rpa_codegen_invalid_add_numlabel_s(co->cg, "__end", exp.start);
	exp.bitmap = bitmap;
	if (exp.bitmap) {
		rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHBITMAP, DA, XX, XX, exp.bitmap));
		rvm_codegen_addins(co->cg, rvm_asm(RVM_BXLES, LR, XX, XX, 0));
	}
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R_REC, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, LR, XX, XX, 0));

	r_array_add(co->expressions, &exp);
	return 0;
}


int rpa_compiler_branch_end(rpa_compiler_t *co)
{
	rpa_ruledef_t exp = r_array_pop(co->expressions, rpa_ruledef_t);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R1, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_redefinelabel_default(co->cg, exp.endidx);
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


int rpa_compiler_nonloopybranch_begin(rpa_compiler_t *co, unsigned int flags)
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


int rpa_compiler_nonloopybranch_end(rpa_compiler_t *co)
{
	rpa_ruledef_t exp = r_array_pop(co->expressions, rpa_ruledef_t);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R1, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_LOO, R0, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_redefinelabel_default(co->cg, exp.endidx);
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


int rpa_compiler_class_begin(rpa_compiler_t *co, unsigned int flags)
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


int rpa_compiler_class_end(rpa_compiler_t *co)
{
	rpa_ruledef_t exp = r_array_pop(co->expressions, rpa_ruledef_t);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_redefinelabel_default(co->cg, exp.endidx);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R1, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));

	rvm_codegen_replaceins(co->cg, exp.branch, rvm_asm(RVM_B, DA, XX, XX, rvm_codegen_getcodesize(co->cg) - exp.branch));
	rpa_compiler_index_reference(co, exp.startidx, (exp.flags & RPA_MATCH_MASK));
	return 0;
}


int rpa_compiler_notexp_begin(rpa_compiler_t *co, unsigned int flags)
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


int rpa_compiler_notexp_end(rpa_compiler_t *co)
{
	rpa_ruledef_t exp = r_array_pop(co->expressions, rpa_ruledef_t);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_REC, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_redefinelabel_default(co->cg, exp.endidx);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHSPCHR_NAN, DA, XX, XX, '.'));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));

	rvm_codegen_replaceins(co->cg, exp.branch, rvm_asm(RVM_B, DA, XX, XX, rvm_codegen_getcodesize(co->cg) - exp.branch));
	rpa_compiler_index_reference(co, exp.startidx, (exp.flags & RPA_MATCH_MASK));
	return 0;
}


int rpa_compiler_negexp_begin(rpa_compiler_t *co, unsigned int flags)
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


int rpa_compiler_negexp_end(rpa_compiler_t *co)
{
	rpa_ruledef_t exp = r_array_pop(co->expressions, rpa_ruledef_t);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_REC, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_redefinelabel_default(co->cg, exp.endidx);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R_TOP, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHSPCHR_NAN, DA, XX, XX, '.'));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_replaceins(co->cg, exp.branch, rvm_asm(RVM_B, DA, XX, XX, rvm_codegen_getcodesize(co->cg) - exp.branch));
	rpa_compiler_index_reference(co, exp.startidx, (exp.flags & RPA_MATCH_MASK));
	return 0;
}
