#include "rpacompiler.h"
#include "rpadbex.h"
#include "rpaparser.h"
#include "rmem.h"
#include "rutf.h"

typedef rint (*rpa_dbex_recordhandler)(rpadbex_t *dbex, rlong rec);

typedef struct rpa_ruleinfo_s {
	rlong startrec;
	rlong sizerecs;
	rlong codeoff;
	rlong codesiz;
} rpa_ruleinfo_t;


struct rpadbex_s {
	rpa_compiler_t *co;
	rpa_parser_t *pa;
	rarray_t *records;
	rharray_t *rules;
	rarray_t *recstack;
	rpa_dbex_recordhandler *handlers;
	ruint error;
	rvm_codelabel_t *labelerr;
	rulong init;
};

static rparecord_t *rpa_dbex_rulerecord(rpadbex_t *dbex, rparule_t rid);
static rparecord_t *rpa_dbex_record(rpadbex_t *dbex, rlong rec);
static rint rpa_dbex_rulename(rpadbex_t *dbex, rlong rec, const rchar **name, rsize_t *namesize);


static rlong rpa_dbex_getmatchchr(rulong matchtype)
{
	switch (matchtype & RPA_MATCH_MASK) {
	default:
	case RPA_MATCH_NONE:
		return RPA_MATCHCHR_NAN;
		break;
	case RPA_MATCH_MULTIPLE:
		return RPA_MATCHCHR_MUL;
		break;
	case RPA_MATCH_OPTIONAL:
		return RPA_MATCHCHR_OPT;
		break;
	case RPA_MATCH_MULTIOPT:
		return RPA_MATCHCHR_MOP;
		break;
	};
	return RPA_MATCHCHR_NAN;
}


static rlong rpa_dbex_getmatchspecialchr(rulong matchtype)
{
	switch (matchtype & RPA_MATCH_MASK) {
	default:
	case RPA_MATCH_NONE:
		return RPA_MATCHSPCHR_NAN;
		break;
	case RPA_MATCH_MULTIPLE:
		return RPA_MATCHSPCHR_MUL;
		break;
	case RPA_MATCH_OPTIONAL:
		return RPA_MATCHSPCHR_OPT;
		break;
	case RPA_MATCH_MULTIOPT:
		return RPA_MATCHSPCHR_MOP;
		break;
	};
	return RPA_MATCHSPCHR_NAN;
}


static rint rpa_dbex_rh_namedrule(rpadbex_t *dbex, rlong rec)
{
	const rchar *name = NULL;
	rsize_t namesize;
	rparecord_t *prec = (rparecord_t *) r_array_slot(dbex->records, rec);

	if (prec->type & RPA_RECORD_START) {
		if (rpa_dbex_rulename(dbex, rec, &name, &namesize) < 0) {

			return -1;
		}

		rvm_codegen_addins(dbex->co->cg, rvm_asm(RPA_SHIFT, XX, XX, XX, 0));
		rvm_codegen_addins(dbex->co->cg, rvm_asm(RVM_BL, DA, XX, XX, 2));
		rvm_codegen_addins(dbex->co->cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));

		if (prec->usertype & RPA_LOOP_PATH) {
			rpa_compiler_loop_begin(dbex->co, name, namesize);
		} else {
			rpa_compiler_rule_begin(dbex->co, name, namesize);
		}

	} else if (prec->type & RPA_RECORD_END) {
		if (prec->usertype & RPA_LOOP_PATH) {
			rpa_compiler_loop_end(dbex->co);
		} else {
			rpa_compiler_rule_end(dbex->co);
		}

	}
	return 0;
}


static rint rpa_dbex_rh_anonymousrule(rpadbex_t *dbex, rlong rec)
{
	rparecord_t *prec = (rparecord_t *) r_array_slot(dbex->records, rec);

	if (prec->type & RPA_RECORD_START) {
		rvm_codegen_addins(dbex->co->cg, rvm_asm(RPA_SHIFT, XX, XX, XX, 0));
		rvm_codegen_addins(dbex->co->cg, rvm_asm(RVM_BL, DA, XX, XX, 2));
		rvm_codegen_addins(dbex->co->cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));
		rpa_compiler_exp_begin(dbex->co);

	} else if (prec->type & RPA_RECORD_END) {
		rpa_compiler_exp_end(dbex->co, RPA_MATCH_NONE);
	}

	return 0;
}


static rint rpa_dbex_rh_char(rpadbex_t *dbex, rlong rec)
{
	rparecord_t *prec = (rparecord_t *) r_array_slot(dbex->records, rec);

	if (prec->type & RPA_RECORD_END) {
		ruint32 wc = 0;
		if (r_utf8_mbtowc(&wc, (const ruchar*) prec->input, (const ruchar*)prec->input + prec->inputsiz) < 0) {

			return -1;
		}
		rvm_codegen_addins(dbex->co->cg, rvm_asm(rpa_dbex_getmatchchr(prec->usertype & RPA_MATCH_MASK), DA, XX, XX, wc));
		rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	}

	return 0;
}


static rint rpa_dbex_rh_specialchar(rpadbex_t *dbex, rlong rec)
{
	rparecord_t *prec = (rparecord_t *) r_array_slot(dbex->records, rec);

	if (prec->type & RPA_RECORD_END) {
		ruint32 wc = 0;
		if (r_utf8_mbtowc(&wc, (const ruchar*) prec->input, (const ruchar*)prec->input + prec->inputsiz) < 0) {

			return -1;
		}
		rvm_codegen_addins(dbex->co->cg, rvm_asm(rpa_dbex_getmatchspecialchr(prec->usertype & RPA_MATCH_MASK), DA, XX, XX, wc));
		rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	}

	return 0;
}


static rint rpa_dbex_rh_cls(rpadbex_t *dbex, rlong rec)
{
	rparecord_t *prec = (rparecord_t *) r_array_slot(dbex->records, rec);

	if (prec->type & RPA_RECORD_START) {
		rpa_compiler_class_begin(dbex->co);

	} else if (prec->type & RPA_RECORD_END) {
		rpa_compiler_class_end(dbex->co, prec->usertype & RPA_MATCH_MASK);
		rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	}

	return 0;
}


static rint rpa_dbex_rh_clschar(rpadbex_t *dbex, rlong rec)
{
	rparecord_t *prec = (rparecord_t *) r_array_slot(dbex->records, rec);

	if (prec->type & RPA_RECORD_END) {
		ruint32 wc = 0;
		if (r_utf8_mbtowc(&wc, (const ruchar*) prec->input, (const ruchar*)prec->input + prec->inputsiz) < 0) {

			return -1;
		}
		rvm_codegen_addins(dbex->co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, wc));
		rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	}

	return 0;
}


static rint rpa_dbex_rh_exp(rpadbex_t *dbex, rlong rec)
{
	rparecord_t *prec = (rparecord_t *) r_array_slot(dbex->records, rec);

	if (prec->type & RPA_RECORD_START) {
		rpa_compiler_exp_begin(dbex->co);

	} else if (prec->type & RPA_RECORD_END) {
		rpa_compiler_exp_end(dbex->co, prec->usertype & RPA_MATCH_MASK);
		rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	}

	return 0;
}


static rint rpa_dbex_rh_orop(rpadbex_t *dbex, rlong rec)
{
	rparecord_t *prec = (rparecord_t *) r_array_slot(dbex->records, rec);

	if (prec->type & RPA_RECORD_START) {
		rpa_compiler_altexp_begin(dbex->co);

	} else if (prec->type & RPA_RECORD_END) {
		rpa_compiler_altexp_end(dbex->co, prec->usertype & RPA_MATCH_MASK);
		rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	}

	return 0;
}


static rint rpa_dbex_rh_norop(rpadbex_t *dbex, rlong rec)
{
	rparecord_t *prec = (rparecord_t *) r_array_slot(dbex->records, rec);

	if (prec->type & RPA_RECORD_START) {
		rpa_compiler_altexp_begin(dbex->co);

	} else if (prec->type & RPA_RECORD_END) {
		rpa_compiler_altexp_end(dbex->co, prec->usertype & RPA_MATCH_MASK);
		rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	}

	return 0;
}


static rint rpa_dbex_rh_notop(rpadbex_t *dbex, rlong rec)
{
	rparecord_t *prec = (rparecord_t *) r_array_slot(dbex->records, rec);

	if (prec->type & RPA_RECORD_START) {
		rpa_compiler_notexp_begin(dbex->co);

	} else if (prec->type & RPA_RECORD_END) {
		rpa_compiler_notexp_end(dbex->co, prec->usertype & RPA_MATCH_MASK);
		rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	}

	return 0;
}


static rint rpa_dbex_rh_range(rpadbex_t *dbex, rlong rec)
{
	rparecord_t *prec = (rparecord_t *) r_array_slot(dbex->records, rec);

	if (prec->type & RPA_RECORD_START) {
		dbex->co->currange.p1 = 0;
		dbex->co->currange.p2 = 0;
	} else if (prec->type & RPA_RECORD_END) {
		if (dbex->co->currange.p1 < dbex->co->currange.p2)
			rvm_codegen_addins(dbex->co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, dbex->co->currange.p1, dbex->co->currange.p2));
		else
			rvm_codegen_addins(dbex->co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, dbex->co->currange.p2, dbex->co->currange.p1));
		rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	}

	return 0;
}


static rint rpa_record2long(rparecord_t *prec, ruint32 *num)
{
	rchar *endptr = NULL;
	rchar buffer[64];

	if (prec->inputsiz == 0 || prec->inputsiz >= sizeof(buffer))
		return -1;
	r_memset(buffer, 0, sizeof(buffer));
	r_memcpy(buffer, prec->input, prec->inputsiz);
	if (prec->userid == RPA_PRODUCTION_HEX) {
		*num = (ruint32)r_strtoul(prec->input, &endptr, 16);
	} else if (prec->userid == RPA_PRODUCTION_DEC) {
		*num = (ruint32)r_strtoul(prec->input, &endptr, 10);
	} else {
		return -1;
	}
	return 0;
}


static rint rpa_dbex_rh_numrange(rpadbex_t *dbex, rlong rec)
{
	rparecord_t *prec = (rparecord_t *) rpa_dbex_record(dbex, rec);

	if (!prec)
		return -1;

	if (prec->type & RPA_RECORD_START) {
		rparecord_t *child;
		child = rpa_dbex_record(dbex, rpa_recordtree_firstchild(dbex->records, rec, RPA_RECORD_END));
		if (rpa_record2long(child, &dbex->co->currange.p1) < 0)
			return -1;
		child = rpa_dbex_record(dbex, rpa_recordtree_lastchild(dbex->records, rec, RPA_RECORD_END));
		if (rpa_record2long(child, &dbex->co->currange.p2) < 0)
			return -1;
	} else if (prec->type & RPA_RECORD_END) {
		if (dbex->co->currange.p1 < dbex->co->currange.p2)
			rvm_codegen_addins(dbex->co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, dbex->co->currange.p1, dbex->co->currange.p2));
		else
			rvm_codegen_addins(dbex->co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, dbex->co->currange.p2, dbex->co->currange.p1));
		rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	}

	return 0;
}


static rint rpa_dbex_rh_clsnum(rpadbex_t *dbex, rlong rec)
{
	rparecord_t *prec = (rparecord_t *) rpa_dbex_record(dbex, rec);

	if (!prec)
		return -1;
	if (prec->type & RPA_RECORD_START) {

	} else if (prec->type & RPA_RECORD_END) {
		ruint32 wc;
		rparecord_t *child;
		child = rpa_dbex_record(dbex, rpa_recordtree_firstchild(dbex->records, rec, RPA_RECORD_END));
		if (rpa_record2long(child, &wc) < 0)
			return -1;
		rvm_codegen_addins(dbex->co->cg, rvm_asm(rpa_dbex_getmatchchr(prec->usertype & RPA_MATCH_MASK), DA, XX, XX, wc));
		rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	}

	return 0;
}


static rint rpa_dbex_rh_beginchar(rpadbex_t *dbex, rlong rec)
{
	rparecord_t *prec = (rparecord_t *) r_array_slot(dbex->records, rec);

	if (prec->type & RPA_RECORD_START) {

	} else if (prec->type & RPA_RECORD_END) {
		ruint32 wc = 0;
		if (r_utf8_mbtowc(&wc, (const ruchar*) prec->input, (const ruchar*)prec->input + prec->inputsiz) < 0) {

			return -1;
		}
		dbex->co->currange.p1 = wc;
	}

	return 0;
}


static rint rpa_dbex_rh_endchar(rpadbex_t *dbex, rlong rec)
{
	rparecord_t *prec = (rparecord_t *) r_array_slot(dbex->records, rec);

	if (prec->type & RPA_RECORD_START) {

	} else if (prec->type & RPA_RECORD_END) {
		ruint32 wc = 0;
		if (r_utf8_mbtowc(&wc, (const ruchar*) prec->input, (const ruchar*)prec->input + prec->inputsiz) < 0) {

			return -1;
		}
		dbex->co->currange.p2 = wc;
	}

	return 0;
}


static rint rpa_dbex_rh_branch(rpadbex_t *dbex, rlong rec)
{
	rparecord_t *prec = (rparecord_t *) r_array_slot(dbex->records, rec);

	if (prec->type & RPA_RECORD_START) {
		if (prec->usertype & RPA_NONLOOP_PATH) {
			rpa_compiler_nonloopybranch_begin(dbex->co);
		} else {
			rpa_compiler_branch_begin(dbex->co);
		}
	} else if (prec->type & RPA_RECORD_END) {

		if (prec->usertype & RPA_NONLOOP_PATH) {
			rpa_compiler_nonloopybranch_end(dbex->co, prec->usertype & RPA_MATCH_MASK);
		} else {
			rpa_compiler_branch_end(dbex->co, prec->usertype & RPA_MATCH_MASK);
		}

	}

	return 0;
}


static rint rpa_dbex_rh_aref(rpadbex_t *dbex, rlong rec)
{
	const rchar *name = NULL;
	rsize_t namesize;
	rparecord_t *prec = (rparecord_t *) r_array_slot(dbex->records, rec);

	if (prec->type & RPA_RECORD_START) {
		if (rpa_dbex_rulename(dbex, rec, &name, &namesize) < 0) {

			return -1;
		}

		rpa_compiler_reference(dbex->co, name, namesize, (prec->usertype & RPA_MATCH_MASK));
		rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	} else if (prec->type & RPA_RECORD_END) {

	}
	return 0;
}


rpadbex_t *rpa_dbex_create(void)
{
	rpadbex_t *dbex = (rpadbex_t *) r_zmalloc(sizeof(*dbex));

	dbex->co = rpa_compiler_create();
	dbex->pa = rpa_parser_create();
	dbex->records = r_array_create(sizeof(rparecord_t));
	dbex->rules = r_harray_create(sizeof(rpa_ruleinfo_t));
	dbex->recstack = r_array_create(sizeof(rulong));
	dbex->handlers = r_zmalloc(sizeof(rpa_dbex_recordhandler) * RPA_PRODUCTION_COUNT);

	dbex->handlers[RPA_PRODUCTION_NAMEDRULE] = rpa_dbex_rh_namedrule;
	dbex->handlers[RPA_PRODUCTION_ANONYMOUSRULE] = rpa_dbex_rh_anonymousrule;
	dbex->handlers[RPA_PRODUCTION_CLS] = rpa_dbex_rh_cls;
	dbex->handlers[RPA_PRODUCTION_CHAR] = rpa_dbex_rh_char;
	dbex->handlers[RPA_PRODUCTION_SPECIALCHAR] = rpa_dbex_rh_specialchar;
	dbex->handlers[RPA_PRODUCTION_CLSCHAR] = rpa_dbex_rh_clschar;
	dbex->handlers[RPA_PRODUCTION_AREF] = rpa_dbex_rh_aref;
	dbex->handlers[RPA_PRODUCTION_CREF] = rpa_dbex_rh_aref;
	dbex->handlers[RPA_PRODUCTION_BRACKETEXP] = rpa_dbex_rh_exp;
	dbex->handlers[RPA_PRODUCTION_OROP] = rpa_dbex_rh_orop;
	dbex->handlers[RPA_PRODUCTION_NOTOP] = rpa_dbex_rh_notop;
	dbex->handlers[RPA_PRODUCTION_ALTBRANCH] = rpa_dbex_rh_branch;
	dbex->handlers[RPA_PRODUCTION_NEGBRANCH] = rpa_dbex_rh_branch;
	dbex->handlers[RPA_PRODUCTION_CHARRNG] = rpa_dbex_rh_range;
	dbex->handlers[RPA_PRODUCTION_NUMRNG] = rpa_dbex_rh_numrange;
	dbex->handlers[RPA_PRODUCTION_CLSNUM] = rpa_dbex_rh_clsnum;
	dbex->handlers[RPA_PRODUCTION_BEGINCHAR] = rpa_dbex_rh_beginchar;
	dbex->handlers[RPA_PRODUCTION_ENDCHAR] = rpa_dbex_rh_endchar;
	dbex->handlers[RPA_PRODUCTION_NOROP] = rpa_dbex_rh_norop;
	dbex->handlers[RPA_PRODUCTION_REQOP] = rpa_dbex_rh_exp;
	dbex->handlers[RPA_PRODUCTION_MINOP] = rpa_dbex_rh_exp;


	return dbex;
}


void rpa_dbex_destroy(rpadbex_t *dbex)
{
	if (dbex) {
		rpa_compiler_destroy(dbex->co);
		rpa_parser_destroy(dbex->pa);
		r_object_destroy((robject_t *)dbex->records);
		r_object_destroy((robject_t *)dbex->rules);
		r_object_destroy((robject_t *)dbex->recstack);
		r_free(dbex->handlers);
		r_free(dbex);
	}
}


static rint rpa_parseinfo_checkforloop_old(rpadbex_t *dbex, rlong rec, rlong loopstartrec, rlong loopendrec, rint inderction)
{
	rlong nrec, i;
	rint lret, ret = 0;

	if (rec == loopstartrec && inderction > 0)
		return 1;

	for (i = 0; i < r_array_length(dbex->recstack); i++) {
		if (rec == r_array_index(dbex->recstack, i, rlong))
			return 0;
	}

	r_array_add(dbex->recstack, &rec);

	for (i = rpa_recordtree_firstchild(dbex->records, rec, RPA_RECORD_START); i >= 0; i = rpa_recordtree_next(dbex->records, i, RPA_RECORD_START)) {
		rparecord_t *prec = (rparecord_t *)r_array_slot(dbex->records, i);
		if (prec->userid == RPA_PRODUCTION_AREF || prec->userid == RPA_PRODUCTION_CREF) {
			nrec = rpa_recordtree_firstchild(dbex->records, i, RPA_RECORD_END);
			if (nrec > 0) {
				rpa_ruleinfo_t *info;
				prec = (rparecord_t *)r_array_slot(dbex->records, nrec);
				info = (rpa_ruleinfo_t *)r_harray_get(dbex->rules, r_harray_lookup(dbex->rules, prec->input, prec->inputsiz));
				if (info) {
					lret = rpa_parseinfo_checkforloop_old(dbex, info->startrec, loopstartrec, loopendrec, inderction + 1);
					if (i >= loopstartrec && i <= loopendrec && lret) {
						rpa_record_setusertype(dbex->records, i, RPA_LOOP_PATH, RVALSET_OR);
						rpa_record_setusertype(dbex->records, nrec, RPA_LOOP_PATH, RVALSET_OR);
					}
					ret |= lret;
				}
			}
		} else {
			lret = rpa_parseinfo_checkforloop_old(dbex, i, loopstartrec, loopendrec, inderction + 1);
			if (i >= loopstartrec && i <= loopendrec && lret) {
				rpa_record_setusertype(dbex->records, i, RPA_LOOP_PATH, RVALSET_OR);
				ret |= lret;
			}

		}
	}

	r_array_removelast(dbex->recstack);
	return ret;
}


static rint rpa_parseinfo_checkforloop(rpadbex_t *dbex, rlong parent, rlong loopto, rint inderction)
{
	rsize_t namesiz;
	const rchar *name;
	rlong i;
	rint lret, ret = 0;
	rlong parent_end = rpa_recordtree_get(dbex->records, parent, RPA_RECORD_END);

	if (parent == loopto && inderction > 0)
		return 1;

	for (i = 0; i < r_array_length(dbex->recstack); i++) {
		if (parent == r_array_index(dbex->recstack, i, rlong))
			return 0;
	}

	r_array_add(dbex->recstack, &parent);

	for (i = rpa_recordtree_firstchild(dbex->records, parent, RPA_RECORD_START); i >= 0; i = rpa_recordtree_next(dbex->records, i, RPA_RECORD_START)) {
		rparecord_t *prec = (rparecord_t *)r_array_slot(dbex->records, i);
		if (prec->userid == RPA_PRODUCTION_RULENAME)
			continue;
		if (prec->userid == RPA_PRODUCTION_AREF || prec->userid == RPA_PRODUCTION_CREF) {
			rpa_ruleinfo_t *info;
			if (rpa_dbex_rulename(dbex, i, &name, &namesiz) < 0)
				R_ASSERT(0);
			info = (rpa_ruleinfo_t *) r_harray_get(dbex->rules, r_harray_lookup(dbex->rules, name, namesiz));
			if (!info)
				continue;
			ret |= rpa_parseinfo_checkforloop(dbex, info->startrec, loopto, inderction + 1);
		} else {
			lret = rpa_parseinfo_checkforloop(dbex, i, loopto, inderction + 1);
			if (i >= parent && i <= parent_end && lret) {
				rpa_record_setusertype(dbex->records, i, RPA_LOOP_PATH, RVALSET_OR);
				ret |= lret;
			}
		}

		if ((prec->usertype & RPA_MATCH_OPTIONAL) == 0 && (prec->userid == RPA_PRODUCTION_CREF || prec->userid == RPA_PRODUCTION_AREF ||
				prec->userid == RPA_PRODUCTION_CHAR || prec->userid == RPA_PRODUCTION_CLS || prec->userid == RPA_PRODUCTION_SPECIALCHAR ||
				prec->userid == RPA_PRODUCTION_BRACKETEXP))
			break;

	}

	r_array_removelast(dbex->recstack);
	return ret;
}


static void rpa_dbex_buildloopinfo(rpadbex_t *dbex)
{
	ruint i, p;
	rharray_t *rules = dbex->rules;
	rpa_ruleinfo_t *info;

	for (i = 0; i < r_array_length(rules->members); i++) {
		info = (rpa_ruleinfo_t *)r_harray_get(rules, i);
		if (rpa_parseinfo_checkforloop(dbex, info->startrec, info->startrec, 0)) {
			rpa_record_setusertype(dbex->records, info->startrec, RPA_LOOP_PATH, RVALSET_OR);
		}
	}

	/*
	 * Mark the non-loop branches.
	 */
	for (i = 0; i < r_array_length(dbex->records); i++) {
		rparecord_t *prec = (rparecord_t *)r_array_slot(dbex->records, i);
		if (prec->type == RPA_RECORD_START &&
			(prec->userid == RPA_PRODUCTION_ALTBRANCH) &&
			(prec->usertype & RPA_LOOP_PATH) == 0) {
			p = rpa_recordtree_parent(dbex->records, i, RPA_RECORD_START);
			if (p >= 0) {
				prec = (rparecord_t *)r_array_slot(dbex->records, p);
				if (prec && (prec->usertype & RPA_LOOP_PATH))
					rpa_record_setusertype(dbex->records, i, RPA_NONLOOP_PATH, RVALSET_OR);
			}
		}
	}
}


static void rpa_dbex_buildruleinfo(rpadbex_t *dbex)
{
	rparecord_t *rec, *namerec;
	rpa_ruleinfo_t info;
	ruint nrecords;
	rint i, nrec;

	if (dbex->rules) {
		r_object_destroy((robject_t *)dbex->rules);
		dbex->rules = NULL;
	}
	dbex->rules = r_harray_create(sizeof(rpa_ruleinfo_t));

	for (i = 0, nrecords = r_array_length(dbex->records); i < nrecords; i++) {
		rec = (rparecord_t *)r_array_slot(dbex->records, i);
		if ((rec->userid == RPA_PRODUCTION_NAMEDRULE) && (rec->type & RPA_RECORD_START)) {
			r_memset(&info, 0, sizeof(info));
			info.startrec = i;
			info.sizerecs = rpa_recordtree_get(dbex->records, i, RPA_RECORD_END);
			if (info.sizerecs < 0)
				continue;
			info.sizerecs = info.sizerecs - i + 1;

			/*
			 * The name record must be the first child
			 */
			nrec = rpa_recordtree_firstchild(dbex->records, i, RPA_RECORD_END);
			if (nrec < 0)
				continue;
			namerec = (rparecord_t *)r_array_slot(dbex->records, nrec);
			if ((namerec->userid == RPA_PRODUCTION_RULENAME) && (namerec->type & RPA_RECORD_END)) {
				r_harray_replace(dbex->rules, namerec->input, namerec->inputsiz, &info);
				i += info.sizerecs - 1;
			}
		} else if ((rec->userid == RPA_PRODUCTION_ANONYMOUSRULE) && (rec->type & RPA_RECORD_START)) {
			r_memset(&info, 0, sizeof(info));
			info.startrec = i;
			info.sizerecs = rpa_recordtree_get(dbex->records, i, RPA_RECORD_END);
			if (info.sizerecs < 0)
				continue;
			info.sizerecs = info.sizerecs - i + 1;
			r_harray_replace_s(dbex->rules, "$anonymous", &info);
			i += info.sizerecs - 1;
		}
	}
}


static void rpa_dbex_copyrecords(rpadbex_t *dbex, rarray_t *records)
{
	rint i;
	rparecord_t *prec;

	for (i = 0; i < r_array_length(records); i++) {
		prec = (rparecord_t *)r_array_slot(records, i);
		if (prec->userid == RPA_PRODUCTION_OCCURENCE && (prec->type & RPA_RECORD_START)) {
			/*
			 * Ignore it
			 */
		} else if (prec->userid == RPA_PRODUCTION_OCCURENCE && (prec->type & (RPA_RECORD_MATCH | RPA_RECORD_END))) {
			ruint32 usertype = RPA_MATCH_NONE;
			rlong lastrec = 0;
			/*
			 * Don't copy it but set the usertype of the previous record accordingly.
			 */
			switch (*prec->input) {
			case '?':
				usertype = RPA_MATCH_OPTIONAL;
				break;
			case '+':
				usertype = RPA_MATCH_MULTIPLE;
				break;
			case '*':
				usertype = RPA_MATCH_MULTIOPT;
				break;
			default:
				usertype = RPA_MATCH_NONE;
			};
			lastrec = r_array_length(dbex->records) - 1;
			if (lastrec >= 0)
				rpa_record_setusertype(dbex->records, lastrec, usertype, RVALSET_OR);
		} else if (prec->userid != RPA_RECORD_INVALID_UID) {
			r_array_add(dbex->records, prec);
		}
	}

}


static rparecord_t *rpa_dbex_record(rpadbex_t *dbex, rlong rec)
{
	rparecord_t *prec;

	if (!dbex || !dbex->rules)
		return NULL;
	if (rec < 0 || rec >= r_array_length(dbex->records))
		return NULL;
	prec = (rparecord_t *)r_array_slot(dbex->records, rec);
	return prec;

}


static rparecord_t *rpa_dbex_rulerecord(rpadbex_t *dbex, rparule_t rid)
{
	rparecord_t *prec;
	rpa_ruleinfo_t *info;
	rlong rec;

	if (!dbex || !dbex->rules)
		return NULL;
	info = r_harray_get(dbex->rules, rid);
	if (!info)
		return NULL;
	rec = info->startrec + info->sizerecs - 1;
	if (rec < 0 || rec >= r_array_length(dbex->records))
		return NULL;
	prec = (rparecord_t *)r_array_slot(dbex->records, rec);
	return prec;
}


static rint rpa_dbex_rulename(rpadbex_t *dbex, rlong rec, const rchar **name, rsize_t *namesize)
{
	rparecord_t *pnamerec = rpa_dbex_record(dbex, rpa_recordtree_firstchild(dbex->records, rec, RPA_RECORD_END));
	if (!pnamerec || !(pnamerec->userid & RPA_PRODUCTION_RULENAME))
		return -1;
	*name = pnamerec->input;
	*namesize = pnamerec->inputsiz;
	return 0;
}


rint rpa_dbex_open(rpadbex_t *dbex)
{
	if (!dbex)
		return -1;
	if (dbex->rules) {
		r_object_destroy((robject_t *)dbex->rules);
		dbex->rules = NULL;
	}
	return 0;
}


void rpa_dbex_close(rpadbex_t *dbex)
{
	if (!dbex)
		return;
	rpa_dbex_buildruleinfo(dbex);
	rpa_dbex_buildloopinfo(dbex);
}


rlong rpa_dbex_load(rpadbex_t *dbex, const rchar *rules, rsize_t size)
{
	rlong ret;

	if (!dbex)
		return -1;
	if (dbex->rules) {
		/*
		 * Dbex is not open
		 */
		return -1;
	}
	if ((ret = rpa_parser_load(dbex->pa, rules, size)) < 0)
		return -1;
	rpa_dbex_copyrecords(dbex, dbex->pa->stat->records);
	return ret;
}


rlong rpa_dbex_load_s(rpadbex_t *dbex, const rchar *rules)
{
	return rpa_dbex_load(dbex, rules, r_strlen(rules));
}



static void rpa_dbex_dumptree_do(rpadbex_t *dbex, rlong rec, rint level)
{
	rparecord_t *prec = (rparecord_t *)r_array_slot(dbex->records, rec);
	if (prec && prec->userid == RPA_PRODUCTION_RULENAME)
		return;
	rpa_record_dumpindented(dbex->records, rpa_recordtree_get(dbex->records, rec, RPA_RECORD_END), level);
	prec = (rparecord_t *)r_array_slot(dbex->records, rec);
	if (prec && (prec->userid == RPA_PRODUCTION_AREF || prec->userid == RPA_PRODUCTION_CREF))
		return;
	if (prec && (prec->userid == RPA_PRODUCTION_CHARRNG || prec->userid == RPA_PRODUCTION_NUMRNG))
		return;
	if (prec && (prec->userid == RPA_PRODUCTION_CLSNUM))
		return;
	for (rec = rpa_recordtree_firstchild(dbex->records, rec, RPA_RECORD_START); rec >= 0; rec = rpa_recordtree_next(dbex->records, rec, RPA_RECORD_START)) {
		rpa_dbex_dumptree_do(dbex, rec, level + 1);
	}
}


rint rpa_dbex_dumptree(rpadbex_t *dbex, const rchar *name)
{
	rpa_ruleinfo_t *info;

	if (!dbex || !dbex->rules)
		return -1;

	info = (rpa_ruleinfo_t *)r_harray_get(dbex->rules, r_harray_lookup_s(dbex->rules, name));
	if (!info)
		return -1;
	rpa_dbex_dumptree_do(dbex, info->startrec, 0);
	return 0;
}


rint rpa_dbex_dumprules(rpadbex_t *dbex)
{
	rint ret = 0;
	rparule_t rid;
	rchar buffer[512];

	if (!dbex || !dbex->rules)
		return -1;
	for (rid = rpa_dbex_first(dbex); rid >= 0; rid = rpa_dbex_next(dbex, rid)) {
		ret = rpa_dbex_copy(dbex, rid, buffer, sizeof(buffer));
		if ( ret >= 0) {
			if (ret == sizeof(buffer))
				r_printf("   %s ...\n", buffer);
			else
				r_printf("   %s\n", buffer);
		}

	}
	return ret;
}


rint rpa_dbex_dumprecords(rpadbex_t *dbex)
{
	rlong i;

	if (!dbex)
		return -1;
	for (i = 0; i < r_array_length(dbex->records); i++) {
		rpa_record_dump(dbex->records, i);
	}
	return 0;
}


rint rpa_dbex_dumpinfo(rpadbex_t *dbex)
{
	ruint i;
	rpa_ruleinfo_t *info;

	if (!dbex || !dbex->rules)
		return -1;
	for (i = 0; i < r_array_length(dbex->rules->names); i++) {
		rstr_t *name = r_array_index(dbex->rules->names, i, rstr_t*);
		info = (rpa_ruleinfo_t *)r_harray_get(dbex->rules, r_harray_lookup(dbex->rules, name->str, name->size));
		r_printf("(%7d, %4d, code: %7ld, %5ld) : %s\n", info->startrec, info->sizerecs, info->codeoff, info->codesiz, name->str);
	}
	return 0;
}


rint rpa_dbex_dumpcode(rpadbex_t* dbex, const rchar *rule)
{
	rpa_ruleinfo_t *info;
	if (!dbex || !dbex->rules)
		return -1;

	info = (rpa_ruleinfo_t *)r_harray_get(dbex->rules, r_harray_lookup_s(dbex->rules, rule));
	if (!info)
		return -1;
	rvm_asm_dump(rvm_codegen_getcode(dbex->co->cg, info->codeoff), info->codesiz);
	return 0;
}

rsize_t rpa_dbex_copy(rpadbex_t *dbex, rparule_t rid, rchar *buf, rsize_t bufsize)
{
	rparecord_t *prec;
	rsize_t size;

	if (!dbex)
		return -1;
	if ((prec = rpa_dbex_rulerecord(dbex, rid)) == NULL)
		return -1;
	size = prec->inputsiz;
	if (bufsize <= size)
		size = bufsize - 1;
	r_memset(buf, 0, bufsize);
	r_strncpy(buf, prec->input, size);
	return size + 1;
}


rparule_t rpa_dbex_first(rpadbex_t *dbex)
{
	if (!dbex || !dbex->rules)
		return -1;

	if (r_array_length(dbex->rules->members) > 0)
		return 0;
	return -1;
}


rparule_t rpa_dbex_last(rpadbex_t *dbex)
{
	if (!dbex || !dbex->rules)
		return -1;

	if (r_array_length(dbex->rules->members) > 0)
		return r_array_length(dbex->rules->members) - 1;
	return -1;
}


rparule_t rpa_dbex_default(rpadbex_t *dbex)
{
	return rpa_dbex_last(dbex);
}


rparule_t rpa_dbex_next(rpadbex_t *dbex, rparule_t rid)
{
	if (!dbex || !dbex->rules)
		return -1;
	++rid;
	if (rid < r_array_length(dbex->rules->members))
		return rid;
	return -1;
}


rparule_t rpa_dbex_prev(rpadbex_t *dbex, rparule_t rid)
{
	if (!dbex || !dbex->rules)
		return -1;
	--rid;
	if (rid >= 0)
		return rid;
	return -1;
}


ruint rpa_dbex_get_error(rpadbex_t *dbex)
{
	return dbex->error;
}


const rchar *rpa_dbex_version()
{
	return "2.0";
}


static rlong rpa_dbex_play_recordhandler(rpadbex_t *dbex, rlong rec)
{
	rparecord_t *prec = (rparecord_t *) r_array_slot(dbex->records, rec);
	rpa_dbex_recordhandler handler = dbex->handlers[prec->userid];
	if (handler) {
		if (handler(dbex, rec) < 0)
			return -1;
	}
	return 0;
}


static rlong rpa_dbex_play_recordhandlers(rpadbex_t *dbex, rlong rec, rlong nrecs)
{
	rparecord_t *prec;
	rlong i, res = 0;

	for (i = rec; i < rec + nrecs; i++) {
		prec = (rparecord_t *) r_array_slot(dbex->records, i);

		if (prec->userid == RPA_PRODUCTION_MINOP && (prec->type & RPA_RECORD_START)) {
			rlong lastchild = rpa_recordtree_lastchild(dbex->records, i, RPA_RECORD_START);
			rlong firstchild = rpa_recordtree_firstchild(dbex->records, i, RPA_RECORD_START);
			if (firstchild < 0 || lastchild < 0 || firstchild == lastchild)
				return -1;
			if ((res = rpa_dbex_play_recordhandler(dbex, i)) < 0)
				return -1;
			if ((res = rpa_dbex_play_recordhandlers(dbex, lastchild, rpa_recordtree_size(dbex->records, lastchild))) < 0)
				return -1;
			if ((res = rpa_dbex_play_recordhandlers(dbex, firstchild, lastchild - firstchild)) < 0)
				return -1;
			if ((res = rpa_dbex_play_recordhandler(dbex, rpa_recordtree_get(dbex->records, i, RPA_RECORD_END))) < 0)
				return -1;
			i += rpa_recordtree_size(dbex->records, i) - 1;
			continue;
		}

		if (rpa_dbex_play_recordhandler(dbex, i) < 0)
			return -1;
	}

	return i;
}


static rint rpa_dbex_compile_rule(rpadbex_t *dbex, rparule_t rid)
{
	rlong codeoff;
	rpa_ruleinfo_t *info = (rpa_ruleinfo_t *)r_harray_get(dbex->rules, rid);

	if (!info)
		return -1;
	codeoff = rvm_codegen_getcodesize(dbex->co->cg);
	if (rpa_dbex_play_recordhandlers(dbex, info->startrec, info->sizerecs) < 0)
		return -1;
	info->codeoff = codeoff;
	info->codesiz = rvm_codegen_getcodesize(dbex->co->cg) - codeoff;

	return 0;
}


rint rpa_dbex_compile(rpadbex_t *dbex)
{
	rparule_t rid;

	if (!dbex || !dbex->rules)
		return -1;

	if (dbex->co)
		rpa_compiler_destroy(dbex->co);
	dbex->co = rpa_compiler_create();

	dbex->init = rvm_codegen_addins(dbex->co->cg, rvm_asml(RVM_NOP, XX, XX, XX, -1));
	rvm_codegen_addrelocins_s(dbex->co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_nan", rvm_asm(RPA_SETBXLNAN, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(dbex->co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_opt", rvm_asm(RPA_SETBXLOPT, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(dbex->co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_mul", rvm_asm(RPA_SETBXLMUL, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(dbex->co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_mop", rvm_asm(RPA_SETBXLMOP, DA, XX, XX, 0));
	rvm_codegen_addins(dbex->co->cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));

	for (rid = rpa_dbex_first(dbex); rid >= 0; rid = rpa_dbex_next(dbex, rid)) {
		if (rpa_dbex_compile_rule(dbex, rid) < 0) {
			return -1;
		}
	}


	if (rvm_codegen_relocate(dbex->co->cg, &dbex->labelerr) < 0) {
		r_printf("RPA_DBEX: Unresolved symbol: %s\n", dbex->labelerr->name->str);
		return -1;
	}


	return 0;
}


rvm_asmins_t *rvm_dbex_getcode(rpadbex_t *dbex)
{
	if (!dbex || !dbex->rules)
		return NULL;

	return rvm_codegen_getcode(dbex->co->cg, 0);
}


rlong rvm_dbex_codeoffset(rpadbex_t *dbex, rparule_t rid)
{
	rpa_ruleinfo_t *info;

	if (!dbex || !dbex->rules)
		return -1;

	info = (rpa_ruleinfo_t *)r_harray_get(dbex->rules, rid);
	if (!info)
		return -1;

	return info->codeoff;
}


rlong rvm_dbex_initoffset(rpadbex_t *dbex)
{
	if (!dbex)
		return -1;
	return dbex->init;
}
