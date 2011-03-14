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
	pa->stat = rpa_stat_create(RPA_PARSER_STACK);
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

	if (rvm_cpu_exec_debug(stat->cpu, rvm_codegen_getcode(co->cg, 0), pa->main) < 0)
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

	rpa_compiler_rule_begin_s(co, "rpa_production_bnf");

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_space", rvm_asm(RPA_BXLWHT, R_MNODE_OPT, DA, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_rulename", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_assign", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_orexp", rvm_asm(RPA_BXLWHT, R_MNODE_MUL, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RVM_PRN, R0, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_space(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "rpa_production_space");

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

	rpa_compiler_rule_begin_s(co, "rpa_production_rulename");

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

	rpa_compiler_rule_begin_s(co, "rpa_production_assign");
	rpa_compiler_exp_begin(co);

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_space", rvm_asm(RPA_BXLWHT, R_MNODE_OPT, DA, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ':'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ':'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '='));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_space", rvm_asm(RPA_BXLMOP, DA, XX, XX, 0));

	rpa_compiler_exp_end(co, RPA_MATCH_NONE);
	rpa_compiler_rule_end(co);
}

/*
 * None of thesese " ~:#@^-|&%+*?\"\'[](){}<>.;\n\r\0"
 */
static void rpa_production_regexchar(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "rpa_production_regexchar");

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
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '%'));
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
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '{'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '}'));
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
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_char", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_exp_end(co, RPA_MATCH_NONE);

	rpa_compiler_rule_end(co);
}


static void rpa_production_char(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "rpa_production_char");
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHSPCHR_NAN, DA, XX, XX, '.'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_rule_end(co);
}


static void rpa_production_escapedchar(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "rpa_production_escapedchar");

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\\'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_char", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_rule_end(co);
}


static void rpa_production_specialchar(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "rpa_production_specialchar");


	rpa_compiler_notexp_begin(co);
	rpa_compiler_class_begin(co);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '.'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '~'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co, RPA_MATCH_NONE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_notexp_end(co, RPA_MATCH_NONE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_char", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));


	rpa_compiler_rule_end(co);
}


/*
 * None of the " #\\-[]\t\n\r\0"
 */
static void rpa_production_clschar(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "rpa_production_clschar");

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
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_char", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_exp_end(co, RPA_MATCH_NONE);

	rpa_compiler_rule_end(co);
}


static void rpa_production_occurence(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "rpa_production_occurence");

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

	rpa_compiler_rule_begin_s(co, "rpa_production_charrng");
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_char", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '-'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_char", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_rule_end(co);
}


static void rpa_production_numrng(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "rpa_production_numrng");

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_num", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '-'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_num", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_sqstr(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "rpa_production_sqstr");

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\''));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_exp_begin(co);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '\''));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_char", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
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

	rpa_compiler_rule_begin_s(co, "rpa_production_dqstr");

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '"'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_exp_begin(co);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '"'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_char", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
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

	rpa_compiler_rule_begin_s(co, "rpa_production_alphacls");

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '['));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_altexp_begin(co);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_escapedchar", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_charrng", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_clschar", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
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

	rpa_compiler_rule_begin_s(co, "rpa_production_numcls");

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '['));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_altexp_begin(co);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_numrng", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLEQ, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);


	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_num", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_altexp_end(co, RPA_MATCH_MULTIPLE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ']'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}

static void rpa_production_dec(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "rpa_production_dec");

	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, '1', '9'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_MOP, DA, XX, XX, '0', '9'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_hex(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "rpa_production_hex");

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

	rpa_compiler_rule_begin_s(co, "rpa_production_num");

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

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_hex", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_dec", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);
	rpa_compiler_altexp_end(co, RPA_MATCH_NONE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_aref(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "rpa_production_aref");

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '<'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_rulename", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '>'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_cref(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "rpa_production_cref");

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '<'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ':'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_rulename", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
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

	rpa_compiler_rule_begin_s(co, "rpa_production_terminal");

	rpa_compiler_altexp_begin(co);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_numcls", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_alphacls", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_sqstr", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_dqstr", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_cref", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_aref", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '('));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_space", rvm_asm(RPA_BXLWHT, R_MNODE_OPT, DA, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_orexp", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_space", rvm_asm(RPA_BXLWHT, R_MNODE_OPT, DA, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ')'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_altexp_end(co, RPA_MATCH_NONE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));


	rpa_compiler_rule_end(co);
}


static void rpa_production_qchar(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "rpa_production_qchar");

	rpa_compiler_altexp_begin(co);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_escapedchar", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_regexchar", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_specialchar", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);
	rpa_compiler_altexp_end(co, RPA_MATCH_NONE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_space", rvm_asm(RPA_BXLWHT, R_MNODE_OPT, DA, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_occurence", rvm_asm(RPA_BXLWHT, R_MNODE_OPT, DA, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_space", rvm_asm(RPA_BXLWHT, R_MNODE_OPT, DA, XX, 0));

	rpa_compiler_rule_end(co);
}

static void rpa_production_qexp(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "rpa_production_qexp");

	rpa_compiler_altexp_begin(co);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_terminal", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_space", rvm_asm(RPA_BXLWHT, R_MNODE_OPT, DA, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_occurence", rvm_asm(RPA_BXLWHT, R_MNODE_OPT, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_space", rvm_asm(RPA_BXLWHT, R_MNODE_OPT, DA, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_qchar", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);
	rpa_compiler_altexp_end(co, RPA_MATCH_NONE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_notop(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "rpa_production_notop");
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '^'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_qexp", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_notexp(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "rpa_production_notexp");

	rpa_compiler_altexp_begin(co);
	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_notop", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_qexp", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_altexp_end(co, RPA_MATCH_NONE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_space", rvm_asm(RPA_BXLWHT, R_MNODE_OPT, DA, XX, 0));


	rpa_compiler_rule_end(co);
}


static void rpa_production_exp(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "rpa_production_exp");
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_notexp", rvm_asm(RPA_BXLMUL, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_rule_end(co);
}


static void rpa_production_minop(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "rpa_production_minop");

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_minexp", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_space", rvm_asm(RPA_BXLWHT, R_MNODE_OPT, DA, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '-'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_space", rvm_asm(RPA_BXLWHT, R_MNODE_OPT, DA, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_exp", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_minexp(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_loop_begin_s(co, "rpa_production_minexp");
	rpa_compiler_altexp_begin(co);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_minop", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_nonloopybranch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_exp", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_nonloopybranch_end(co, RPA_MATCH_NONE);
	rpa_compiler_altexp_end(co, RPA_MATCH_NONE);

	rpa_compiler_loop_end(co);
}


static void rpa_production_orop(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_rule_begin_s(co, "rpa_production_orop");

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_orexp", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_space", rvm_asm(RPA_BXLOPT, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '|'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_space", rvm_asm(RPA_BXLOPT, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_minexp", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


static void rpa_production_orexp(rpa_parser_t *pa)
{
	rpa_compiler_t *co = pa->co;

	rpa_compiler_loop_begin_s(co, "rpa_production_orexp");
	rpa_compiler_altexp_begin(co);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_orop", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_nonloopybranch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_minexp", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_nonloopybranch_end(co, RPA_MATCH_NONE);
	rpa_compiler_altexp_end(co, RPA_MATCH_NONE);

	rpa_compiler_loop_end(co);
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

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_nan", rvm_asm(RVM_MOV, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_mul", rvm_asm(RVM_MOV, R_MNODE_MUL, DA, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_opt", rvm_asm(RVM_MOV, R_MNODE_OPT, DA, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_mop", rvm_asm(RVM_MOV, R_MNODE_MOP, DA, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_nan", rvm_asm(RPA_SETBXLNAN, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_opt", rvm_asm(RPA_SETBXLOPT, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_mul", rvm_asm(RPA_SETBXLMUL, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_mop", rvm_asm(RPA_SETBXLMOP, DA, XX, XX, 0));


	rvm_codegen_addins(co->cg, rvm_asm(RPA_SHIFT, XX, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_production_bnf", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, -2));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));

	rpa_production_char(pa);
	rpa_production_occurence(pa);
	rpa_production_clschar(pa);
	rpa_production_charrng(pa);
	rpa_production_numrng(pa);
	rpa_production_alphacls(pa);
	rpa_production_numcls(pa);

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
	rpa_production_minop(pa);
	rpa_production_minexp(pa);
	rpa_production_orop(pa);
	rpa_production_orexp(pa);
	rpa_production_exp(pa);
	rpa_production_space(pa);
	rpa_production_assign(pa);
	rpa_production_rulename(pa);
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

	rpa_compiler_rule_begin_s(co, "rpa_production_");

	rpa_compiler_rule_end(co);
}
