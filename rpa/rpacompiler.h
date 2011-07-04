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

#ifndef _RPACOMPILER_H_
#define _RPACOMPILER_H_

#include "rvm/rvmcodegen.h"
#include "rvm/rvmscope.h"
#include "rpa/rpavm.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RPA_RULENAME_MAXSIZE 256


#define RPA_COMPILER_CURRENTEXP(__co__) ((rpa_ruledef_t*)r_array_lastslot((__co__)->expressions))

/*
 * The rpa_rulepref_t user preferences are compiled into
 * rpa_ruledata_t and access in runtime.
 */
typedef struct rpa_rulepref_s {
	rlong ruleid;
	rlong ruleuid;
	rulong flags;
} rpa_rulepref_t;


/*
 * This should be renamed to rpa_expdef_s
 */
typedef struct rpa_ruledef_s {
	rulong branch;
	rulong start;
	rlong startidx;
	rlong dataidx;
	rlong endidx;
	rlong loopidx;
	rlong successidx;
	rlong failidx;
	rlong againidx;
	ruinteger ruleuid;
	ruinteger flags;
	rpa_rulepref_t *rulepref;
} rpa_ruledef_t;


typedef struct rpa_compiler_s {
	rvm_codegen_t *cg;
	rharray_t *ruleprefs;
	rarray_t *expressions;
	rvm_scope_t *scope;
	rulong fpoff;
	rpair_t currange;
} rpa_compiler_t;


rpa_compiler_t *rpa_compiler_create();
void rpa_compiler_destroy(rpa_compiler_t *co);
rlong rpa_compiler_addblob(rpa_compiler_t *co, rlong ruleid, rlong ruleuid, rulong flags, const rchar *name, rulong namesize);
rlong rpa_compiler_addblob_s(rpa_compiler_t *co, rlong ruleid, rlong ruleuid, rulong flags, const rchar *name);

rinteger rpa_compiler_loop_begin(rpa_compiler_t *co, const rchar *name, ruinteger namesize);
rinteger rpa_compiler_loop_begin_s(rpa_compiler_t *co, const rchar *name);
rinteger rpa_compiler_loop_end(rpa_compiler_t *co);

rinteger rpa_compiler_rule_begin(rpa_compiler_t *co, const rchar *name, ruinteger namesize);
rinteger rpa_compiler_rule_begin_s(rpa_compiler_t *co, const rchar *name);
rinteger rpa_compiler_rule_end(rpa_compiler_t *co);

rinteger rpa_compiler_inlinerule_begin(rpa_compiler_t *co, const rchar *name, ruinteger namesize, ruinteger flags);
rinteger rpa_compiler_inlinerule_begin_s(rpa_compiler_t *co, const rchar *name, ruinteger flags);
rinteger rpa_compiler_inlinerule_end(rpa_compiler_t *co);

rinteger rpa_compiler_exp_begin(rpa_compiler_t *co, ruinteger flags);
rinteger rpa_compiler_exp_end(rpa_compiler_t *co);

rinteger rpa_compiler_altexp_begin(rpa_compiler_t *co, ruinteger flags);
rinteger rpa_compiler_altexp_end(rpa_compiler_t *co);

rinteger rpa_compiler_branch_begin(rpa_compiler_t *co, ruinteger flags);
rinteger rpa_compiler_branch_end(rpa_compiler_t *co);

rinteger rpa_compiler_nonloopybranch_begin(rpa_compiler_t *co, ruinteger flags);
rinteger rpa_compiler_nonloopybranch_end(rpa_compiler_t *co);

rinteger rpa_compiler_class_begin(rpa_compiler_t *co, ruinteger flags);
rinteger rpa_compiler_class_end(rpa_compiler_t *co);

rinteger rpa_compiler_notexp_begin(rpa_compiler_t *co, ruinteger flags);
rinteger rpa_compiler_notexp_end(rpa_compiler_t *co);

void rpa_compiler_index_reference(rpa_compiler_t *co, rulong index, ruinteger qflag);
void rpa_compiler_index_reference_nan(rpa_compiler_t *co, rulong index);
void rpa_compiler_index_reference_opt(rpa_compiler_t *co, rulong index);
void rpa_compiler_index_reference_mul(rpa_compiler_t *co, rulong index);
void rpa_compiler_index_reference_mop(rpa_compiler_t *co, rulong index);
void rpa_compiler_reference(rpa_compiler_t *co, const rchar *name, rsize_t namesize, ruinteger qflag);
void rpa_compiler_reference_nan(rpa_compiler_t *co, const rchar *name, rsize_t namesize);
void rpa_compiler_reference_opt(rpa_compiler_t *co, const rchar *name, rsize_t namesize);
void rpa_compiler_reference_mul(rpa_compiler_t *co, const rchar *name, rsize_t namesize);
void rpa_compiler_reference_mop(rpa_compiler_t *co, const rchar *name, rsize_t namesize);
void rpa_compiler_reference_s(rpa_compiler_t *co, const rchar *name, ruinteger qflag);
void rpa_compiler_reference_nan_s(rpa_compiler_t *co, const rchar *name);
void rpa_compiler_reference_opt_s(rpa_compiler_t *co, const rchar *name);
void rpa_compiler_reference_mul_s(rpa_compiler_t *co, const rchar *name);
void rpa_compiler_reference_mop_s(rpa_compiler_t *co, const rchar *name);

void rpa_compiler_rulepref_set_ruleid(rpa_compiler_t *co, const rchar *name, ruinteger namesize, rlong ruleid);
void rpa_compiler_rulepref_set_ruleid_s(rpa_compiler_t *co, const rchar *name, rlong ruleid);
void rpa_compiler_rulepref_set_ruleuid(rpa_compiler_t *co, const rchar *name, ruinteger namesize, rlong ruleuid);
void rpa_compiler_rulepref_set_ruleuid_s(rpa_compiler_t *co, const rchar *name, rlong ruleuid);
void rpa_compiler_rulepref_set_flag(rpa_compiler_t *co, const rchar *name, ruinteger namesize, rulong flag);
void rpa_compiler_rulepref_set_flag_s(rpa_compiler_t *co, const rchar *name, rulong flag);
void rpa_compiler_rulepref_clear_flag(rpa_compiler_t *co, const rchar *name, ruinteger namesize, rulong flag);
void rpa_compiler_rulepref_clear_flag_s(rpa_compiler_t *co, const rchar *name, rulong flag);
void rpa_compiler_rulepref_set(rpa_compiler_t *co, const rchar *name, ruinteger namesize, rlong ruleid, rlong ruleuid, rulong flags);
void rpa_compiler_rulepref_set_s(rpa_compiler_t *co, const rchar *name, rlong ruleid, rlong ruleuid, rulong flags);
#ifdef __cplusplus
}
#endif

#endif
