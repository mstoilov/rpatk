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
#include "rpa/rpabitmap.h"

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
	long ruleid;
	long ruleuid;
	unsigned long flags;
} rpa_rulepref_t;


/*
 * This should be renamed to rpa_expdef_s
 */
typedef struct rpa_ruledef_s {
	unsigned long branch;
	unsigned long start;
	long startidx;
	long dataidx;
	long endidx;
	long loopidx;
	long successidx;
	long failidx;
	long againidx;
	unsigned int ruleuid;
	unsigned int flags;
	rpabitmap_t bitmap;
	rpa_rulepref_t *rulepref;
} rpa_ruledef_t;


typedef struct rpa_compiler_s {
	rvm_codegen_t *cg;
	rharray_t *ruleprefs;
	rarray_t *expressions;
	rvm_scope_t *scope;
	unsigned long fpoff;
	rpair_t currange;
} rpa_compiler_t;


rpa_compiler_t *rpa_compiler_create();
void rpa_compiler_destroy(rpa_compiler_t *co);
long rpa_compiler_addblob(rpa_compiler_t *co, long ruleid, long ruleuid, unsigned long flags, const char *name, unsigned long namesize);
long rpa_compiler_addblob_s(rpa_compiler_t *co, long ruleid, long ruleuid, unsigned long flags, const char *name);

int rpa_compiler_loop_begin(rpa_compiler_t *co, const char *name, unsigned int namesize);
int rpa_compiler_loop_begin_s(rpa_compiler_t *co, const char *name);
int rpa_compiler_loop_end(rpa_compiler_t *co);

int rpa_compiler_rule_begin(rpa_compiler_t *co, const char *name, unsigned int namesize, rpabitmap_t bitmap);
int rpa_compiler_rule_begin_s(rpa_compiler_t *co, const char *name, rpabitmap_t bitmap);
int rpa_compiler_rule_end(rpa_compiler_t *co);

int rpa_compiler_inlinerule_begin(rpa_compiler_t *co, const char *name, unsigned int namesize, unsigned int flags);
int rpa_compiler_inlinerule_begin_s(rpa_compiler_t *co, const char *name, unsigned int flags);
int rpa_compiler_inlinerule_end(rpa_compiler_t *co);

int rpa_compiler_exp_begin(rpa_compiler_t *co, unsigned int flags);
int rpa_compiler_exp_end(rpa_compiler_t *co);

int rpa_compiler_altexp_begin(rpa_compiler_t *co, unsigned int flags, rpabitmap_t bitmap);
int rpa_compiler_altexp_end(rpa_compiler_t *co);

int rpa_compiler_branch_begin(rpa_compiler_t *co, unsigned int flags);
int rpa_compiler_branch_end(rpa_compiler_t *co);

int rpa_compiler_nonloopybranch_begin(rpa_compiler_t *co, unsigned int flags);
int rpa_compiler_nonloopybranch_end(rpa_compiler_t *co);

int rpa_compiler_class_begin(rpa_compiler_t *co, unsigned int flags);
int rpa_compiler_class_end(rpa_compiler_t *co);

int rpa_compiler_notexp_begin(rpa_compiler_t *co, unsigned int flags);
int rpa_compiler_notexp_end(rpa_compiler_t *co);

void rpa_compiler_index_reference(rpa_compiler_t *co, unsigned long index, unsigned int qflag);
void rpa_compiler_index_reference_nan(rpa_compiler_t *co, unsigned long index);
void rpa_compiler_index_reference_opt(rpa_compiler_t *co, unsigned long index);
void rpa_compiler_index_reference_mul(rpa_compiler_t *co, unsigned long index);
void rpa_compiler_index_reference_mop(rpa_compiler_t *co, unsigned long index);
void rpa_compiler_reference(rpa_compiler_t *co, const char *name, rsize_t namesize, unsigned int qflag);
void rpa_compiler_reference_nan(rpa_compiler_t *co, const char *name, rsize_t namesize);
void rpa_compiler_reference_opt(rpa_compiler_t *co, const char *name, rsize_t namesize);
void rpa_compiler_reference_mul(rpa_compiler_t *co, const char *name, rsize_t namesize);
void rpa_compiler_reference_mop(rpa_compiler_t *co, const char *name, rsize_t namesize);
void rpa_compiler_reference_s(rpa_compiler_t *co, const char *name, unsigned int qflag);
void rpa_compiler_reference_nan_s(rpa_compiler_t *co, const char *name);
void rpa_compiler_reference_opt_s(rpa_compiler_t *co, const char *name);
void rpa_compiler_reference_mul_s(rpa_compiler_t *co, const char *name);
void rpa_compiler_reference_mop_s(rpa_compiler_t *co, const char *name);

void rpa_compiler_rulepref_set_ruleid(rpa_compiler_t *co, const char *name, unsigned int namesize, long ruleid);
void rpa_compiler_rulepref_set_ruleid_s(rpa_compiler_t *co, const char *name, long ruleid);
void rpa_compiler_rulepref_set_ruleuid(rpa_compiler_t *co, const char *name, unsigned int namesize, long ruleuid);
void rpa_compiler_rulepref_set_ruleuid_s(rpa_compiler_t *co, const char *name, long ruleuid);
void rpa_compiler_rulepref_set_flag(rpa_compiler_t *co, const char *name, unsigned int namesize, unsigned long flag);
void rpa_compiler_rulepref_set_flag_s(rpa_compiler_t *co, const char *name, unsigned long flag);
void rpa_compiler_rulepref_clear_flag(rpa_compiler_t *co, const char *name, unsigned int namesize, unsigned long flag);
void rpa_compiler_rulepref_clear_flag_s(rpa_compiler_t *co, const char *name, unsigned long flag);
void rpa_compiler_rulepref_set(rpa_compiler_t *co, const char *name, unsigned int namesize, long ruleid, long ruleuid, unsigned long flags);
void rpa_compiler_rulepref_set_s(rpa_compiler_t *co, const char *name, long ruleid, long ruleuid, unsigned long flags);
#ifdef __cplusplus
}
#endif

#endif
