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
#include "rlib/rstring.h"
#include "rpa/rpaparser.h"
#include "rpa/rpastatpriv.h"

static int rpa_parser_init(rpa_parser_t *pa);


rpa_parser_t *rpa_parser_create()
{
	rpa_parser_t *pa;

	pa = (rpa_parser_t *)r_zmalloc(sizeof(*pa));
	r_memset(pa, 0, sizeof(*pa));
	pa->co = rpa_compiler_create();
	pa->stat = rpa_stat_create(NULL, RPA_PARSER_STACK);
	if (rpa_parser_init(pa) < 0) {
		rpa_parser_destroy(pa);
		return NULL;
	}
	return pa;
}


void rpa_parser_destroy(rpa_parser_t *pa)
{
	if (pa) {
		rpa_compiler_destroy(pa->co);
		rpa_stat_destroy(pa->stat);
		r_free(pa);
	}
}


long rpa_parser_load(rpa_parser_t *pa, const char *prods, unsigned long size, rarray_t *records)
{
	long ret = 0;
	rpainput_t *ptp;
	rpa_compiler_t *co = pa->co;
	rpastat_t *stat = pa->stat;

	rpa_stat_cachedisable(stat, 0);
	if (rpa_stat_exec(stat, rvm_codegen_getcode(co->cg, 0), pa->main, RPA_ENCODING_UTF8, prods, prods, prods + size, records) < 0)
		return -1;
	ret = (long)RVM_CPUREG_GETL(stat->cpu, R0);
	if (ret < 0)
		return 0;
	ptp = &stat->instack[ret];
	return (long)(ptp->input - prods);
}


long rpa_parser_load_s(rpa_parser_t *pa, const char *prods, rarray_t *records)
{
	return rpa_parser_load(pa, prods, r_strlen(prods), records);
}


static void rpa_production_directives(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "directives", 0);

	rpa_compiler_altexp_begin(co, RPA_MATCH_MULTIPLE, 0);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "emit");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "noemit");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "emitall");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "emitnone");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "abort");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);


	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "emitid");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);
	rpa_compiler_altexp_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	/*
	 * Skip any junk we might have up to the end of the line
	 */
	rpa_compiler_notexp_begin(co, RPA_MATCH_MULTIOPT);
	rpa_compiler_class_begin(co, RPA_MATCH_NONE);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\r'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\n'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_notexp_end(co);

	rpa_compiler_rule_end(co);
}


static void rpa_production_bnf(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

//	rpa_compiler_rulepref_set_s(co, "bnf", 0, RPA_PRODUCTION_BNF);
	rpa_compiler_rule_begin_s(co, "bnf", 0);

	rpa_compiler_altexp_begin(co, RPA_MATCH_MULTIPLE, 0);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "space");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "directives");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "comment");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_class_begin(co, RPA_MATCH_MULTIPLE);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\r'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\n'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ';'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\0'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "namedrule");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "anonymousrule");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);


	rpa_compiler_altexp_end(co);

	rpa_compiler_rule_end(co);
}


static void rpa_production_directive_emitid(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "emitid", 0, RPA_PRODUCTION_DIRECTIVEEMITID, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "emitid", 0);

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '#'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_opt_s(co, "space");
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '!'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_opt_s(co, "space");
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'e'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'm'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'i'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 't'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'i'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'd'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_nan_s(co, "space");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_nan_s(co, "rulename");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_nan_s(co, "space");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_opt_s(co, "aliasname");
	rpa_compiler_reference_opt_s(co, "space");
	rpa_compiler_reference_nan_s(co, "dec");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_directive_emit(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "emit", 0, RPA_PRODUCTION_DIRECTIVEEMIT, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "emit", 0);

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '#'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_opt_s(co, "space");
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '!'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_opt_s(co, "space");
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'e'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'm'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'i'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 't'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_nan_s(co, "space");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_nan_s(co, "rulename");

	rpa_compiler_rule_end(co);
}


static void rpa_production_directive_abort(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "abort", 0, RPA_PRODUCTION_DIRECTIVEABORT, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "abort", 0);

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '#'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_opt_s(co, "space");
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '!'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_opt_s(co, "space");
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'a'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'b'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'o'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'r'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 't'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_nan_s(co, "space");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_nan_s(co, "rulename");

	rpa_compiler_rule_end(co);
}


static void rpa_production_directive_emitall(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "emitall", 0, RPA_PRODUCTION_DIRECTIVEEMITALL, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "emitall", 0);

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '#'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_opt_s(co, "space");
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '!'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_opt_s(co, "space");
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'e'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'm'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'i'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 't'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'a'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'l'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'l'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_rule_end(co);
}


static void rpa_production_directive_emitnone(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "emitnone", 0, RPA_PRODUCTION_DIRECTIVEEMITNONE, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "emitnone", 0);

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '#'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_opt_s(co, "space");
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '!'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_opt_s(co, "space");
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'e'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'm'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'i'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 't'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'n'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'o'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'n'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'e'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_rule_end(co);
}


static void rpa_production_directive_noemit(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "noemit", 0, RPA_PRODUCTION_DIRECTIVENOEMIT, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "noemit", 0);

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '#'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_opt_s(co, "space");
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '!'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_opt_s(co, "space");
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'n'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'o'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'e'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'm'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'i'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 't'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_nan_s(co, "space");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_nan_s(co, "rulename");

	rpa_compiler_rule_end(co);
}


static void rpa_production_comment(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "comment", 0);

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '#'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_exp_begin(co, RPA_MATCH_MULTIOPT, 0);
	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_class_begin(co, RPA_MATCH_NONE);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\r'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\n'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\0'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHSPCHR_NAN, DA, XX, XX, '.'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_exp_end(co);

	rpa_compiler_rule_end(co);
}


static void rpa_production_namedrule(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "namedrule", 0, RPA_PRODUCTION_NAMEDRULE, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "namedrule", 0);

	rpa_compiler_reference_opt_s(co, "space");

	rpa_compiler_reference_nan_s(co, "rulename");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_reference_nan_s(co, "assign");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_reference_mul_s(co, "orexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_anonymousrule(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "anonymousrule", 0, RPA_PRODUCTION_ANONYMOUSRULE, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "anonymousrule", 0);

	rpa_compiler_reference_opt_s(co, "space");

	rpa_compiler_reference_mul_s(co, "orexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_space(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "space", 0);

	rpa_compiler_class_begin(co, RPA_MATCH_MULTIPLE);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ' '));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\t'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_rulename(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "rulename", 0, RPA_PRODUCTION_RULENAME, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "rulename", 0);

	rpa_compiler_class_begin(co, RPA_MATCH_NONE);
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, 'a', 'z'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, 'A', 'Z'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '_'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));

	rpa_compiler_class_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_class_begin(co, RPA_MATCH_MULTIOPT);
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, 'a', 'z'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, 'A', 'Z'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, '0', '9'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '_'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_aliasname(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "aliasname", 0, RPA_PRODUCTION_ALIASNAME, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "aliasname", 0);

	rpa_compiler_class_begin(co, RPA_MATCH_NONE);
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, 'a', 'z'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, 'A', 'Z'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_class_begin(co, RPA_MATCH_MULTIOPT);
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, 'a', 'z'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, 'A', 'Z'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, '0', '9'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '_'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_assign(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "assign", 0);
	rpa_compiler_exp_begin(co, RPA_MATCH_NONE, 0);

	rpa_compiler_reference_opt_s(co, "space");

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ':'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ':'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '='));

	rpa_compiler_reference_mop_s(co, "space");

	rpa_compiler_exp_end(co);
	rpa_compiler_rule_end(co);
}

/*
 * None of thesese " ~:#@^-|&+*?\"\'[]()<>.;\n\r\0"
 */
static void rpa_production_regexchar(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;
	rpa_compiler_rule_begin_s(co, "regexchar", 0);

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\0'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ' '));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '~'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '#'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '^'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '-'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '|'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '&'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '+'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '*'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '?'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '"'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\''));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '('));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ')'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '['));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ']'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '<'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '>'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '.'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ';'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\t'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\r'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\n'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_reference_nan_s(co, "char");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_char(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "char", 0, RPA_PRODUCTION_CHAR, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "char", 0);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHSPCHR_NAN, DA, XX, XX, '.'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_rule_end(co);
}


static void rpa_production_escapedchar(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "escapedchar", 0);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\\'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_altexp_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "specialchar");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);
	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "char");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);
	rpa_compiler_altexp_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_rule_end(co);
}


static void rpa_production_specialchar(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "specialchar", 0, RPA_PRODUCTION_SPECIALCHAR, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "specialchar", 0);

	rpa_compiler_class_begin(co, RPA_MATCH_NONE);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'r'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'n'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 't'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '.'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
//	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '~'));
//	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


/*
 * None of the " #\\-[]\t\n\r\0"
 */
static void rpa_production_clschars(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ' '));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '#'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\\'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '-'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '['));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ']'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\t'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\r'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\n'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\0'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
}


static void rpa_production_clschar(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "clschar", 0, RPA_PRODUCTION_CLSCHAR, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "clschar", 0);

	rpa_production_clschars(pa);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHSPCHR_NAN, DA, XX, XX, '.'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_rule_end(co);
}


static void rpa_production_beginchar(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "beginchar", 0, RPA_PRODUCTION_BEGINCHAR, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "beginchar", 0);

//	rpa_compiler_class_begin(co, RPA_MATCH_NONE);
	rpa_production_clschars(pa);
//	rpa_compiler_class_end(co);
//	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHSPCHR_NAN, DA, XX, XX, '.'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_endchar(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "endchar", 0, RPA_PRODUCTION_ENDCHAR, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "endchar", 0);

//	rpa_compiler_class_begin(co, RPA_MATCH_NONE);
	rpa_production_clschars(pa);
//	rpa_compiler_class_end(co);
//	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHSPCHR_NAN, DA, XX, XX, '.'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_rule_end(co);
}


static void rpa_production_occurence(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "occurence", 0, RPA_PRODUCTION_OCCURENCE, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "occurence", 0);

	rpa_compiler_class_begin(co, RPA_MATCH_NONE);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '?'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '+'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '*'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_charrng(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "charrng", 0, RPA_PRODUCTION_CHARRNG, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "charrng", 0);
	rpa_compiler_reference_nan_s(co, "beginchar");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '-'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_nan_s(co, "endchar");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_rule_end(co);
}


static void rpa_production_numrng(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "numrng", 0, RPA_PRODUCTION_NUMRNG, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "numrng", 0);

	rpa_compiler_reference_nan_s(co, "num");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '-'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_nan_s(co, "num");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_sqstr(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "sqstr", 0);

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\''));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_exp_begin(co, RPA_MATCH_MULTIPLE, 0);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\''));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_reference_nan_s(co, "char");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_exp_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\''));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_dqstr(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "dqstr", 0);

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '"'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_exp_begin(co, RPA_MATCH_MULTIPLE, 0);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '"'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_reference_nan_s(co, "char");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_exp_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '"'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_alphacls(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "alphacls", 0);

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '['));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_altexp_begin(co, RPA_MATCH_MULTIPLE, 0);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "charrng");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "clschar");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "escapedchar");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_altexp_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ']'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_clsnum(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "clsnum", 0, RPA_PRODUCTION_CLSNUM, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "clsnum", 0);

	rpa_compiler_reference_nan_s(co, "num");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_numcls(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "numcls", 0);

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '['));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_altexp_begin(co, RPA_MATCH_MULTIPLE, 0);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "numrng");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLEQ, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);


	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "clsnum");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_altexp_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ']'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_cls(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "cls", 0, RPA_PRODUCTION_CLS, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "cls", 0);

	rpa_compiler_altexp_begin(co, RPA_MATCH_NONE, 0);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "numcls");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "alphacls");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);
	rpa_compiler_altexp_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_dec(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "dec", 0, RPA_PRODUCTION_DEC, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "dec", 0);

	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, '1', '9'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_MOP, DA, XX, XX, '0', '9'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_hex(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "hex", 0, RPA_PRODUCTION_HEX, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "hex", 0);

	rpa_compiler_class_begin(co, RPA_MATCH_MULTIPLE);
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, '0', '9'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, 'a', 'f'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, 'A', 'F'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));


	rpa_compiler_rule_end(co);
}


static void rpa_production_num(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "num", 0);

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '#'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_altexp_begin(co, RPA_MATCH_NONE, 0);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_OPT, DA, XX, XX, '0'));
	rpa_compiler_class_begin(co, RPA_MATCH_NONE);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'x'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'X'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_reference_nan_s(co, "hex");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "dec");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);
	rpa_compiler_altexp_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_aref(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "aref", 0, RPA_PRODUCTION_AREF, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "aref", 0);

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '<'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_reference_nan_s(co, "rulename");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '>'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_cref(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "cref", 0, RPA_PRODUCTION_CREF, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "cref", 0);

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '<'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ':'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_reference_nan_s(co, "rulename");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ':'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '>'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_terminal(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "terminal", 0);

	rpa_compiler_altexp_begin(co, RPA_MATCH_NONE, 0);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "cls");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "sqstr");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "dqstr");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "cref");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "aref");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "bracketexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_altexp_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));


	rpa_compiler_rule_end(co);
}


static void rpa_production_qchar(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "qchar", 0);

	rpa_compiler_altexp_begin(co, RPA_MATCH_NONE, 0);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "escapedchar");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "regexchar");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "specialchar");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);
	rpa_compiler_altexp_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_opt_s(co, "space");
	rpa_compiler_reference_opt_s(co, "occurence");
	rpa_compiler_reference_opt_s(co, "space");

	rpa_compiler_rule_end(co);
}

static void rpa_production_qexp(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "qexp", 0);

	rpa_compiler_altexp_begin(co, RPA_MATCH_NONE, 0);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "terminal");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_opt_s(co, "space");
	rpa_compiler_reference_opt_s(co, "occurence");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_opt_s(co, "space");
	rpa_compiler_branch_end(co);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "qchar");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);
	rpa_compiler_altexp_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_anchorop(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "anchorop", 0, RPA_PRODUCTION_ANCHOROP, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "anchorop", 0);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '~'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_opt_s(co, "space");
	rpa_compiler_reference_nan_s(co, "qexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_rule_end(co);
}


static void rpa_production_anchorexp(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "anchorexp", 0);
	rpa_compiler_altexp_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "anchorop");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);
	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "qexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);
	rpa_compiler_altexp_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_opt_s(co, "space");
	rpa_compiler_rule_end(co);
}


static void rpa_production_notop(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "notop", 0, RPA_PRODUCTION_NOTOP, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "notop", 0);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '^'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_opt_s(co, "space");
	rpa_compiler_reference_nan_s(co, "anchorexp");

	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_notexp(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "notexp", 0);
	rpa_compiler_altexp_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "notop");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);
	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
//	rpa_compiler_reference_nan_s(co, "qexp");
	rpa_compiler_reference_nan_s(co, "anchorexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);
	rpa_compiler_altexp_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_opt_s(co, "space");
	rpa_compiler_rule_end(co);
}


static void rpa_production_exp(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

//	rpa_compiler_rulepref_set_s(co, "exp", 0, RPA_PRODUCTION_EXP);
	rpa_compiler_rule_begin_s(co, "exp", 0);
	rpa_compiler_reference_mul_s(co, "notexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_rule_end(co);
}


static void rpa_production_bracketexp(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "bracketexp", 0, RPA_PRODUCTION_BRACKETEXP, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "bracketexp", 0);

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '('));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_reference_opt_s(co, "space");

	rpa_compiler_reference_nan_s(co, "orexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_reference_opt_s(co, "space");

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ')'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}



static void rpa_production_negbranch(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "negbranch", 0, RPA_PRODUCTION_NEGBRANCH, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "negbranch", 0);

	rpa_compiler_reference_nan_s(co, "exp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_norop(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "norop", 0, RPA_PRODUCTION_NOROP, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "norop", 0);

	rpa_compiler_exp_begin(co, RPA_MATCH_MULTIPLE, 0);
	rpa_compiler_reference_opt_s(co, "space");
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '-'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_opt_s(co, "space");

	rpa_compiler_reference_mul_s(co, "negbranch");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_exp_end(co);

	rpa_compiler_rule_end(co);
}


static void rpa_production_reqop(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "reqop", 0, RPA_PRODUCTION_REQOP, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "reqop", 0);

	rpa_compiler_reference_nan_s(co, "exp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_minop(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "minop", 0, RPA_PRODUCTION_MINOP, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "minop", 0);

	rpa_compiler_reference_nan_s(co, "reqop");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_reference_nan_s(co, "norop");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_minexp(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "minexp", 0);
	rpa_compiler_altexp_begin(co, RPA_MATCH_NONE, 0);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "minop");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "exp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);
	rpa_compiler_altexp_end(co);

	rpa_compiler_rule_end(co);
}


static void rpa_production_altbranch(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "altbranch", 0, RPA_PRODUCTION_ALTBRANCH, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "altbranch", 0);

	rpa_compiler_reference_nan_s(co, "minexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_orop(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rulepref_set_s(co, "orop", 0, RPA_PRODUCTION_OROP, RPA_RFLAG_EMITRECORD);
	rpa_compiler_rule_begin_s(co, "orop", 0);

	rpa_compiler_reference_nan_s(co, "altbranch");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_exp_begin(co, RPA_MATCH_MULTIPLE, 0);
	rpa_compiler_reference_opt_s(co, "space");
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_OPT, DA, XX, XX, '\r'));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_OPT, DA, XX, XX, '\n'));
	rpa_compiler_reference_opt_s(co, "space");
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '|'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_reference_opt_s(co, "space");
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_OPT, DA, XX, XX, '\r'));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_OPT, DA, XX, XX, '\n'));
	rpa_compiler_reference_opt_s(co, "space");

	rpa_compiler_reference_nan_s(co, "altbranch");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_exp_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_orexp(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "orexp", 0);
	rpa_compiler_altexp_begin(co, RPA_MATCH_NONE, 0);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "orop");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "minexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);
	rpa_compiler_altexp_end(co);

	rpa_compiler_rule_end(co);
}


static int rpa_parser_init(rpa_parser_t *pa)
{
	rvm_codelabel_t *err;
	rpa_compiler_t *co = pa->co;

	pa->main = rvm_codegen_addins(co->cg, rvm_asml(RVM_NOP, XX, XX, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, SP, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, R_LOO, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, R_TOP, DA, XX, -1));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_SHIFT, XX, XX, XX, 0));
	rpa_compiler_reference_nan_s(co, "bnf");
	rvm_codegen_addins(co->cg, rvm_asm(RPA_EMITTAIL, XX, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, -2));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));

	rpa_production_directive_emitall(pa);
	rpa_production_directive_emitnone(pa);
	rpa_production_directive_emit(pa);
	rpa_production_directive_noemit(pa);
	rpa_production_directive_emitid(pa);
	rpa_production_directive_abort(pa);
	rpa_production_directives(pa);
	rpa_production_char(pa);
	rpa_production_clsnum(pa);
	rpa_production_clschar(pa);
	rpa_production_occurence(pa);
	rpa_production_charrng(pa);
	rpa_production_numrng(pa);
	rpa_production_alphacls(pa);
	rpa_production_numcls(pa);
	rpa_production_cls(pa);
	rpa_production_beginchar(pa);
	rpa_production_endchar(pa);

	rpa_production_escapedchar(pa);
	rpa_production_specialchar(pa);
	rpa_production_regexchar(pa);
	rpa_production_qchar(pa);

	rpa_production_dec(pa);
	rpa_production_hex(pa);
	rpa_production_num(pa);
	rpa_production_sqstr(pa);
	rpa_production_dqstr(pa);
	rpa_production_aref(pa);
	rpa_production_cref(pa);
	rpa_production_terminal(pa);
	rpa_production_qexp(pa);
	rpa_production_anchorop(pa);
	rpa_production_anchorexp(pa);
	rpa_production_notop(pa);
	rpa_production_notexp(pa);
	rpa_production_negbranch(pa);
	rpa_production_norop(pa);
	rpa_production_minop(pa);
	rpa_production_minexp(pa);
	rpa_production_reqop(pa);
	rpa_production_orop(pa);
	rpa_production_altbranch(pa);
	rpa_production_orexp(pa);
	rpa_production_exp(pa);
	rpa_production_bracketexp(pa);
	rpa_production_space(pa);
	rpa_production_assign(pa);
	rpa_production_rulename(pa);
	rpa_production_aliasname(pa);
	rpa_production_namedrule(pa);
	rpa_production_anonymousrule(pa);
	rpa_production_comment(pa);
	rpa_production_bnf(pa);

	if (rvm_codegen_relocate(co->cg, &err) < 0) {
		r_printf("RPA_PARSER: Unresolved symbol: %s\n", err->name->str);
		return -1;
	}

	return 0;
}


void rpa_production_(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "", 0);

	rpa_compiler_rule_end(co);
}
