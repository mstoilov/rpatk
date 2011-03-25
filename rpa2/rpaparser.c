#include "rmem.h"
#include "rstring.h"
#include "rpaparser.h"

static rint rpa_parser_init(rpa_parser_t *pa);
static void rpa_production_(rpa_parser_t *pa);


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


rlong rpa_parser_load(rpa_parser_t *pa, const rchar *prods, rsize_t size)
{
	rlong ret = 0;
	rpa_compiler_t *co = pa->co;
	rpastat_t *stat = pa->stat;

	rpa_stat_init(stat, prods, prods, prods + size);
	rpa_stat_cachedisable(stat, 0);

	if (rvm_cpu_exec(stat->cpu, rvm_codegen_getcode(co->cg, 0), pa->main) < 0)
		return -1;
	ret = (rlong)RVM_CPUREG_GETL(stat->cpu, R0);
	if (ret < 0)
		return 0;
	return ret;
}


rlong rpa_parser_load_s(rpa_parser_t *pa, const rchar *prods)
{
	return rpa_parser_load(pa, prods, r_strlen(prods));
}


static void rpa_production_bnf(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

//	rpa_compiler_add_ruleuid_s(co, "bnf", RPA_PRODUCTION_BNF);
	rpa_compiler_rule_begin_s(co, "bnf");

	rpa_compiler_altexp_begin(co);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "space", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "comment", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rpa_compiler_class_begin(co);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\r'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\n'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ';'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co, RPA_MATCH_MULTIPLE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "namedrule", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "anonymousrule", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);


	rpa_compiler_altexp_end(co, RPA_MATCH_MULTIPLE);

	rpa_compiler_rule_end(co);
}


static void rpa_production_comment(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "comment");

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '#'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_exp_begin(co);
	rpa_compiler_branch_begin(co);
	rpa_compiler_class_begin(co);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\r'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\n'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\0'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co, RPA_MATCH_NONE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHSPCHR_NAN, DA, XX, XX, '.'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_exp_end(co, RPA_MATCH_MULTIOPT);

	rpa_compiler_rule_end(co);
}


static void rpa_production_namedrule(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_add_ruleuid_s(co, "namedrule", RPA_PRODUCTION_NAMEDRULE);
	rpa_compiler_rule_begin_s(co, "namedrule");

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "space", rvm_asm(RPA_BXLOPT, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rulename", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "assign", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "orexp", rvm_asm(RPA_BXLMUL, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_anonymousrule(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_add_ruleuid_s(co, "anonymousrule", RPA_PRODUCTION_ANONYMOUSRULE);
	rpa_compiler_rule_begin_s(co, "anonymousrule");

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "space", rvm_asm(RPA_BXLOPT, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "orexp", rvm_asm(RPA_BXLMUL, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_space(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "space");

	rpa_compiler_class_begin(co);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ' '));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\t'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co, RPA_MATCH_MULTIPLE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_rulename(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_add_ruleuid_s(co, "rulename", RPA_PRODUCTION_RULENAME);
	rpa_compiler_rule_begin_s(co, "rulename");

	rpa_compiler_class_begin(co);
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, 'a', 'z'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, 'A', 'Z'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co, RPA_MATCH_NONE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_class_begin(co);
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, 'a', 'z'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, 'A', 'Z'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, '0', '9'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co, RPA_MATCH_MULTIOPT);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_assign(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "assign");
	rpa_compiler_exp_begin(co);

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "space", rvm_asm(RPA_BXLOPT, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ':'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ':'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '='));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "space", rvm_asm(RPA_BXLMOP, DA, XX, XX, 0));

	rpa_compiler_exp_end(co, RPA_MATCH_NONE);
	rpa_compiler_rule_end(co);
}

/*
 * None of thesese " ~:#@^-|&+*?\"\'[]()<>.;\n\r\0"
 */
static void rpa_production_regexchar(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

//	rpa_compiler_add_ruleuid_s(co, "regexchar", RPA_PRODUCTION_REGEXCHAR);
	rpa_compiler_rule_begin_s(co, "regexchar");

	rpa_compiler_exp_begin(co);
	rpa_compiler_branch_begin(co);
	rpa_compiler_class_begin(co);
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
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\t'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\r'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\n'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\0'));
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
	rpa_compiler_class_end(co, RPA_MATCH_NONE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "char", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_exp_end(co, RPA_MATCH_NONE);

	rpa_compiler_rule_end(co);
}


static void rpa_production_char(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_add_ruleuid_s(co, "char", RPA_PRODUCTION_CHAR);
	rpa_compiler_rule_begin_s(co, "char");
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHSPCHR_NAN, DA, XX, XX, '.'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_rule_end(co);
}


static void rpa_production_escapedchar(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_add_ruleuid_s(co, "escapedchar", RPA_PRODUCTION_ESCAPEDCHAR);
	rpa_compiler_rule_begin_s(co, "escapedchar");

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\\'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "char", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_rule_end(co);
}


static void rpa_production_specialchar(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_add_ruleuid_s(co, "specialchar", RPA_PRODUCTION_SPECIALCHAR);
	rpa_compiler_rule_begin_s(co, "specialchar");

	rpa_compiler_class_begin(co);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '.'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '~'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co, RPA_MATCH_NONE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


/*
 * None of the " #\\-[]\t\n\r\0"
 */
static void rpa_production_clschar(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

//	rpa_compiler_add_ruleuid_s(co, "clschar", RPA_PRODUCTION_CLSCHAR);
	rpa_compiler_rule_begin_s(co, "clschar");

	rpa_compiler_exp_begin(co);
	rpa_compiler_branch_begin(co);
	rpa_compiler_class_begin(co);
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
	rpa_compiler_class_end(co, RPA_MATCH_NONE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "char", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_exp_end(co, RPA_MATCH_NONE);

	rpa_compiler_rule_end(co);
}


static void rpa_production_occurence(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_add_ruleuid_s(co, "occurence", RPA_PRODUCTION_OCCURENCE);
	rpa_compiler_rule_begin_s(co, "occurence");

	rpa_compiler_class_begin(co);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '?'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '+'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '*'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co, RPA_MATCH_NONE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_charrng(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_add_ruleuid_s(co, "charrng", RPA_PRODUCTION_CHARRNG);
	rpa_compiler_rule_begin_s(co, "charrng");
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "char", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '-'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "char", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_rule_end(co);
}


static void rpa_production_numrng(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_add_ruleuid_s(co, "numrng", RPA_PRODUCTION_NUMRNG);
	rpa_compiler_rule_begin_s(co, "numrng");

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "num", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '-'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "num", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_sqstr(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "sqstr");

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\''));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_exp_begin(co);
	rpa_compiler_branch_begin(co);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\''));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "char", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_exp_end(co, RPA_MATCH_MULTIPLE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\''));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_dqstr(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "dqstr");

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '"'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_exp_begin(co);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '"'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "char", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_exp_end(co, RPA_MATCH_MULTIPLE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '"'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_alphacls(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "alphacls");

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '['));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_altexp_begin(co);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "escapedchar", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "charrng", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "clschar", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_altexp_end(co, RPA_MATCH_MULTIPLE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ']'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_numcls(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "numcls");

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '['));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_altexp_begin(co);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "numrng", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLEQ, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);


	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "num", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_altexp_end(co, RPA_MATCH_MULTIPLE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ']'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_cls(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_add_ruleuid_s(co, "cls", RPA_PRODUCTION_CLS);
	rpa_compiler_rule_begin_s(co, "cls");

	rpa_compiler_altexp_begin(co);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "numcls", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "alphacls", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);
	rpa_compiler_altexp_end(co, RPA_MATCH_NONE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_dec(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_add_ruleuid_s(co, "dec", RPA_PRODUCTION_DEC);
	rpa_compiler_rule_begin_s(co, "dec");

	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, '1', '9'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_MOP, DA, XX, XX, '0', '9'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_hex(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_add_ruleuid_s(co, "hex", RPA_PRODUCTION_HEX);
	rpa_compiler_rule_begin_s(co, "hex");

	rpa_compiler_class_begin(co);
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, 'a', 'f'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, 'A', 'F'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, '0', '9'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co, RPA_MATCH_MULTIPLE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));


	rpa_compiler_rule_end(co);
}


static void rpa_production_num(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "num");

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '#'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_altexp_begin(co);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_OPT, DA, XX, XX, '0'));
	rpa_compiler_class_begin(co);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'x'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'X'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co, RPA_MATCH_NONE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "hex", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "dec", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);
	rpa_compiler_altexp_end(co, RPA_MATCH_NONE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_aref(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_add_ruleuid_s(co, "aref", RPA_PRODUCTION_AREF);
	rpa_compiler_rule_begin_s(co, "aref");

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '<'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rulename", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '>'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_cref(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_add_ruleuid_s(co, "cref", RPA_PRODUCTION_CREF);
	rpa_compiler_rule_begin_s(co, "cref");

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '<'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ':'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rulename", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
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

	rpa_compiler_rule_begin_s(co, "terminal");

	rpa_compiler_altexp_begin(co);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "cls", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "sqstr", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "dqstr", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "cref", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "aref", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "bracketexp", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_altexp_end(co, RPA_MATCH_NONE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));


	rpa_compiler_rule_end(co);
}


static void rpa_production_qchar(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "qchar");

	rpa_compiler_altexp_begin(co);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "escapedchar", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "regexchar", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "specialchar", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);
	rpa_compiler_altexp_end(co, RPA_MATCH_NONE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "space", rvm_asm(RPA_BXLOPT, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "occurence", rvm_asm(RPA_BXLOPT, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "space", rvm_asm(RPA_BXLOPT, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}

static void rpa_production_qexp(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "qexp");

	rpa_compiler_altexp_begin(co);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "terminal", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "space", rvm_asm(RPA_BXLOPT, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "occurence", rvm_asm(RPA_BXLOPT, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "space", rvm_asm(RPA_BXLOPT, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "qchar", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);
	rpa_compiler_altexp_end(co, RPA_MATCH_NONE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_notop(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_add_ruleuid_s(co, "notop", RPA_PRODUCTION_NOTOP);
	rpa_compiler_rule_begin_s(co, "notop");
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '^'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "qexp", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_notexp(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "notexp");

	rpa_compiler_altexp_begin(co);
	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "notop", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "qexp", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_altexp_end(co, RPA_MATCH_NONE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "space", rvm_asm(RPA_BXLOPT, DA, XX, XX, 0));


	rpa_compiler_rule_end(co);
}


static void rpa_production_exp(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

//	rpa_compiler_add_ruleuid_s(co, "exp", RPA_PRODUCTION_EXP);
	rpa_compiler_rule_begin_s(co, "exp");
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "notexp", rvm_asm(RPA_BXLMUL, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_rule_end(co);
}


static void rpa_production_bracketexp(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_add_ruleuid_s(co, "bracketexp", RPA_PRODUCTION_BRACKETEXP);
	rpa_compiler_rule_begin_s(co, "bracketexp");

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '('));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "space", rvm_asm(RPA_BXLOPT, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "orexp", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "space", rvm_asm(RPA_BXLOPT, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ')'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}



static void rpa_production_negbranch(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_add_ruleuid_s(co, "negbranch", RPA_PRODUCTION_NEGBRANCH);
	rpa_compiler_rule_begin_s(co, "negbranch");

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "exp", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_negexp(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_add_ruleuid_s(co, "negexp", RPA_PRODUCTION_NEGEXP);
	rpa_compiler_rule_begin_s(co, "negexp");

	rpa_compiler_exp_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "space", rvm_asm(RPA_BXLOPT, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '-'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "space", rvm_asm(RPA_BXLOPT, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "negbranch", rvm_asm(RPA_BXLMUL, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_exp_end(co, RPA_MATCH_MULTIPLE);

	rpa_compiler_rule_end(co);
}


static void rpa_production_minop(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_add_ruleuid_s(co, "minop", RPA_PRODUCTION_MINOP);
	rpa_compiler_rule_begin_s(co, "minop");

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "exp", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "negexp", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_minexp(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "minexp");
	rpa_compiler_altexp_begin(co);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "minop", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_nonloopybranch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "exp", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_nonloopybranch_end(co, RPA_MATCH_NONE);
	rpa_compiler_altexp_end(co, RPA_MATCH_NONE);

	rpa_compiler_rule_end(co);
}


static void rpa_production_altbranch(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_add_ruleuid_s(co, "altbranch", RPA_PRODUCTION_ALTBRANCH);
	rpa_compiler_rule_begin_s(co, "altbranch");

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "minexp", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_orop(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_add_ruleuid_s(co, "orop", RPA_PRODUCTION_OROP);
	rpa_compiler_rule_begin_s(co, "orop");

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "altbranch", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_exp_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "space", rvm_asm(RPA_BXLOPT, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_OPT, DA, XX, XX, '\r'));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_OPT, DA, XX, XX, '\n'));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "space", rvm_asm(RPA_BXLOPT, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '|'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "space", rvm_asm(RPA_BXLOPT, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_OPT, DA, XX, XX, '\r'));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_OPT, DA, XX, XX, '\n'));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "space", rvm_asm(RPA_BXLOPT, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "altbranch", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_exp_end(co, RPA_MATCH_MULTIPLE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_orexp(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "orexp");
	rpa_compiler_altexp_begin(co);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "orop", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_nonloopybranch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "minexp", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_nonloopybranch_end(co, RPA_MATCH_NONE);
	rpa_compiler_altexp_end(co, RPA_MATCH_NONE);

	rpa_compiler_rule_end(co);
}


static rint rpa_parser_init(rpa_parser_t *pa)
{
	rvm_codelabel_t *err;
	rpa_compiler_t *co = pa->co;

	pa->main = rvm_codegen_addins(co->cg, rvm_asml(RVM_NOP, XX, XX, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, SP, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, R_LOO, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, R_TOP, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, R_WHT, DA, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_nan", rvm_asm(RPA_SETBXLNAN, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_mul", rvm_asm(RPA_SETBXLMUL, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_opt", rvm_asm(RPA_SETBXLOPT, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_mop", rvm_asm(RPA_SETBXLMOP, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_nan", rvm_asm(RPA_SETBXLNAN, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_opt", rvm_asm(RPA_SETBXLOPT, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_mul", rvm_asm(RPA_SETBXLMUL, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_mop", rvm_asm(RPA_SETBXLMOP, DA, XX, XX, 0));


	rvm_codegen_addins(co->cg, rvm_asm(RPA_SHIFT, XX, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "bnf", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, -2));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));

	rpa_production_char(pa);
	rpa_production_occurence(pa);
	rpa_production_clschar(pa);
	rpa_production_charrng(pa);
	rpa_production_numrng(pa);
	rpa_production_alphacls(pa);
	rpa_production_numcls(pa);
	rpa_production_cls(pa);

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
	rpa_production_notop(pa);
	rpa_production_notexp(pa);
	rpa_production_negbranch(pa);
	rpa_production_negexp(pa);
	rpa_production_minop(pa);
	rpa_production_minexp(pa);
	rpa_production_orop(pa);
	rpa_production_altbranch(pa);
	rpa_production_orexp(pa);
	rpa_production_exp(pa);
	rpa_production_bracketexp(pa);
	rpa_production_space(pa);
	rpa_production_assign(pa);
	rpa_production_rulename(pa);
	rpa_production_namedrule(pa);
	rpa_production_anonymousrule(pa);
	rpa_production_comment(pa);
	rpa_production_bnf(pa);


	rpa_production_(pa);

	if (rvm_codegen_relocate(co->cg, &err) < 0) {
		r_printf("RPA_PARSER: Unresolved symbol: %s\n", err->name->str);
		return -1;
	}

	return 0;
}


static void rpa_production_(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "");

	rpa_compiler_rule_end(co);
}
