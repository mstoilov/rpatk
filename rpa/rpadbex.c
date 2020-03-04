/*
 *  Regular Pattern Analyzer Toolkit (RPA/Tk)
 *  Copyright (c) 2009-2012 Martin Stoilov
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

/**
 *@file rpadbex.c
 */


#include "rlib/rmem.h"
#include "rlib/rutf.h"
#include "rpa/rpabitmap.h"
#include "rpa/rpadbexpriv.h"
#include "rpa/rpadbex.h"
#include "rpa/rpastatpriv.h"


#define RPA_DBEX_SETERRINFO_CODE(__d__, __e__) do { (__d__)->err.code = __e__; (__d__)->err.mask |= RPA_ERRINFO_CODE; } while (0)
#define RPA_DBEX_SETERRINFO_OFFSET(__d__, __o__) do { (__d__)->err.offset = __o__; (__d__)->err.mask |= RPA_ERRINFO_OFFSET; } while (0)
#define RPA_DBEX_SETERRINFO_LINE(__d__, __l__) do { (__d__)->err.line = __l__; (__d__)->err.mask |= RPA_ERRINFO_LINE; } while (0)
#define RPA_DBEX_SETERRINFO_NAME(__d__, __n__, __s__) do { \
	(__d__)->err.mask |= RPA_ERRINFO_NAME; \
	r_memset((__d__)->err.name, 0, sizeof((__d__)->err.name)); \
	r_strncpy((__d__)->err.name, __n__, R_MIN(__s__, (sizeof((__d__)->err.name) - 1)));  } while (0)



static rparecord_t *rpa_dbex_rulerecord(rpadbex_t *dbex, rparule_t rid);
static rparecord_t *rpa_dbex_record(rpadbex_t *dbex, long rec);
static int rpa_dbex_rulename(rpadbex_t *dbex, long rec, const char **name, unsigned long *namesize);
static int rpa_parseinfo_loopdetect(rpadbex_t *dbex, long parent, long loopto);
static long rpa_dbex_firstinlined(rpadbex_t *dbex);
static int rpa_dbex_findinlined(rpadbex_t *dbex, long startrec);
static int rpa_dbex_playchildrecords(rpadbex_t *dbex, long rec);
static int rpa_dbex_playreversechildrecords(rpadbex_t *dbex, long rec);
static int rpa_dbex_playrecord(rpadbex_t *dbex, long rec);
static int rpa_dbex_rh_default(rpadbex_t *dbex, long rec);
static void rpa_dbex_buildbitmapinfo(rpadbex_t *dbex);
static void rpa_dbex_buildbitmapinfo_for_rule(rpadbex_t *dbex, rparule_t rid);
static void rpa_dbex_bitmapinfo_set_all(rpadbex_t *dbex, rparule_t rid);

static void rpa_dbex_buildbitmapinfo_for_rule(rpadbex_t *dbex, rparule_t rid)
{
	rpa_bitmapcompiler_t bc;
	rharray_t *rules = dbex->rules;
	rpa_ruleinfo_t *info;

	r_memset(&bc, 0, sizeof(bc));
	bc.dbex = dbex;
	if ((info = (rpa_ruleinfo_t *)r_harray_get(rules, rid)) != NULL) {
		rpa_rulepref_t *rulepref = rpa_compiler_rulepref_lookup_s(dbex->co, rpa_dbex_name(dbex, rid));
		rparecord_t *record = rpa_record_get(dbex->records, info->startrec);
		RPA_BITMAP_SETALL(RPA_RECORD2BITMAP(record));
		rpa_recordtree_walk(dbex->records, info->startrec, 0, rpa_bitmap_set, (rpointer)&bc);
		if (rulepref && (rulepref->flags & RPA_RFLAG_ABORTONFAIL))
			rpa_dbex_bitmapinfo_set_all(dbex, rid);
	}
}


ruword rpa_dbex_getrulebitmap(rpadbex_t *dbex, rparule_t rid)
{
	ruword bitmap = 0L;
	rharray_t *rules = dbex->rules;
	rpa_ruleinfo_t *info;

	if ((info = (rpa_ruleinfo_t *)r_harray_get(rules, rid)) != NULL) {
		rparecord_t *record = rpa_record_get(dbex->records, info->startrec);
		if (record) {
			if (!RPA_BITMAP_GETVAL(RPA_RECORD2BITMAP(record))) {
				rpa_dbex_buildbitmapinfo_for_rule(dbex, rid);
			}
			bitmap = RPA_BITMAP_GETVAL(RPA_RECORD2BITMAP(record));
		}
	}
	return bitmap;
}


static void rpa_dbex_bitmapinfo_set_all(rpadbex_t *dbex, rparule_t rid)
{
	rharray_t *rules = dbex->rules;
	rpa_ruleinfo_t *info;

	if ((info = (rpa_ruleinfo_t *)r_harray_get(rules, rid)) != NULL) {
		RPA_BITMAP_SETALL(RPA_RECORD2BITMAP(rpa_record_get(dbex->records, info->startrec)));
		RPA_BITMAP_SETALL(RPA_RECORD2BITMAP(rpa_record_get(dbex->records, rpa_recordtree_get(dbex->records, info->startrec, RPA_RECORD_END))));
	}
}


static void rpa_dbex_buildbitmapinfo(rpadbex_t *dbex)
{
	unsigned int i;
	rharray_t *rules = dbex->rules;

	for (i = 0; i < r_array_length(rules->members); i++) {
		rpa_dbex_getrulebitmap(dbex, i);
	}
}


void rpa_dbex_debug_recordhead(rpadbex_t *dbex, long rec)
{
	if (dbex->debug) {
		rarray_t *records = dbex->records;
		rparecord_t *prec = (rparecord_t *) r_array_slot(records, rec);
		dbex->headoff = rvm_codegen_getcodesize(dbex->co->cg);
		if (prec->type & RPA_RECORD_START) {
			rpa_record_dump(records, rec, 0);
		}
	}
}


void rpa_dbex_debug_recordtail(rpadbex_t *dbex, long rec)
{
	if (dbex->debug) {
		rarray_t *records = dbex->records;
		rparecord_t *prec = (rparecord_t *) r_array_slot(records, rec);
		rvm_asm_dump(NULL, rvm_codegen_getcode(dbex->co->cg, dbex->headoff), rvm_codegen_getcodesize(dbex->co->cg) - dbex->headoff);
		if (prec->type & RPA_RECORD_END) {
			rpa_record_dump(records, rec, 0);
		}
	}
}


static int rpa_dbex_rh_default(rpadbex_t *dbex, long rec)
{
	rarray_t *records = dbex->records;
	rparecord_t *prec;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_dbex_debug_recordtail(dbex, rec);
	if (rpa_dbex_playchildrecords(dbex, rec) < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_dbex_debug_recordtail(dbex, rec);
	return 0;
}



static int rpa_dbex_playrecord(rpadbex_t *dbex, long rec)
{
	rarray_t *records = dbex->records;
	rparecord_t *prec = (rparecord_t *)r_array_slot(records, rec);

	if (prec->ruleuid >= 0 && prec->ruleuid < RPA_PRODUCTION_COUNT && dbex->handlers[prec->ruleuid]) {
		return dbex->handlers[prec->ruleuid](dbex, rec);
	}
	return rpa_dbex_rh_default(dbex, rec);
}


static int rpa_dbex_playchildrecords(rpadbex_t *dbex, long rec)
{
	long child;
	rarray_t *records = dbex->records;

	for (child = rpa_recordtree_firstchild(records, rec, RPA_RECORD_START); child >= 0; child = rpa_recordtree_next(records, child, RPA_RECORD_START)) {
		if (rpa_dbex_playrecord(dbex, child) < 0)
			return -1;
	}
	return 0;
}


static int rpa_dbex_playreversechildrecords(rpadbex_t *dbex, long rec)
{
	long child;
	rarray_t *records = dbex->records;

	for (child = rpa_recordtree_lastchild(records, rec, RPA_RECORD_START); child >= 0; child = rpa_recordtree_prev(records, child, RPA_RECORD_START)) {
		if (rpa_dbex_playrecord(dbex, child) < 0)
			return -1;
	}

	return 0;
}


static long rpa_dbex_getmatchchr(unsigned long matchtype)
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


static long rpa_dbex_getmatchspecialchr(unsigned long matchtype)
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


int rpa_record2long(rparecord_t *prec, ruint32 *num)
{
	char *endptr = NULL;
	char buffer[64];

	if (!prec || !num || prec->inputsiz == 0 || prec->inputsiz >= sizeof(buffer))
		return -1;
	r_memset(buffer, 0, sizeof(buffer));
	r_memcpy(buffer, prec->input, prec->inputsiz);
	if (prec->ruleuid == RPA_PRODUCTION_HEX) {
		*num = (ruint32)r_strtoul(prec->input, &endptr, 16);
	} else if (prec->ruleuid == RPA_PRODUCTION_DEC) {
		*num = (ruint32)r_strtoul(prec->input, &endptr, 10);
	} else {
		return -1;
	}
	return 0;
}


static int rpa_dbex_rh_uid(rpadbex_t *dbex, long rec)
{
	const char *name = NULL;
	unsigned long namesize;
	ruint32 uid = 0;
	rparecord_t *pnumrec;
	rarray_t *records = dbex->records;
	rparecord_t *prec;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	if (rpa_dbex_rulename(dbex, rec, &name, &namesize) < 0) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_SYNTAXERROR);
		return -1;
	}
	pnumrec = rpa_dbex_record(dbex, rpa_recordtree_lastchild(dbex->records, rec, RPA_RECORD_END));
	if (!pnumrec) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_SYNTAXERROR);
		return -1;
	}
	if (rpa_record2long(pnumrec, &uid) < 0) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_SYNTAXERROR);
		return -1;
	}
	rpa_compiler_rulepref_set_ruleuid(dbex->co, name, namesize, uid);
	rpa_compiler_rulepref_set_flag(dbex->co, name, namesize, RPA_RFLAG_EMITRECORD);
	rpa_dbex_debug_recordtail(dbex, rec);
	if (rpa_dbex_playchildrecords(dbex, rec) < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_dbex_debug_recordtail(dbex, rec);
	return 0;
}


static int rpa_dbex_rh_abort(rpadbex_t *dbex, long rec)
{
	const char *name = NULL;
	unsigned long namesize;
	rarray_t *records = dbex->records;
	rparecord_t *prec;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	if (rpa_dbex_rulename(dbex, rec, &name, &namesize) < 0) {
		return -1;
	}
	rpa_compiler_rulepref_set_flag(dbex->co, name, namesize, RPA_RFLAG_ABORTONFAIL);
	rpa_dbex_debug_recordtail(dbex, rec);
	if (rpa_dbex_playchildrecords(dbex, rec) < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_dbex_debug_recordtail(dbex, rec);
	return 0;
}


static int rpa_dbex_rh_emit(rpadbex_t *dbex, long rec)
{
	const char *name = NULL;
	unsigned long namesize;
	rarray_t *records = dbex->records;
	rparecord_t *prec;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	if (rpa_dbex_rulename(dbex, rec, &name, &namesize) < 0) {
		return -1;
	}
	rpa_compiler_rulepref_set_flag(dbex->co, name, namesize, RPA_RFLAG_EMITRECORD);
	rpa_dbex_debug_recordtail(dbex, rec);
	if (rpa_dbex_playchildrecords(dbex, rec) < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_dbex_debug_recordtail(dbex, rec);
	return 0;
}


static int rpa_dbex_rh_noemit(rpadbex_t *dbex, long rec)
{
	const char *name = NULL;
	unsigned long namesize;
	rarray_t *records = dbex->records;
	rparecord_t *prec;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	if (rpa_dbex_rulename(dbex, rec, &name, &namesize) < 0) {
		return -1;
	}
	rpa_compiler_rulepref_clear_flag(dbex->co, name, namesize, RPA_RFLAG_EMITRECORD);
	rpa_dbex_debug_recordtail(dbex, rec);
	if (rpa_dbex_playchildrecords(dbex, rec) < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_dbex_debug_recordtail(dbex, rec);
	return 0;
}


static int rpa_dbex_setemit(rpadbex_t *dbex, rboolean emit)
{
	unsigned long i;
	rpa_ruleinfo_t *info;

	for (i = 0; i < r_array_length(dbex->rules->names); i++) {
		rstr_t *name = r_array_index(dbex->rules->names, i, rstr_t*);
		info = (rpa_ruleinfo_t *)r_harray_get(dbex->rules, i);
		if (info->type == RPA_RULEINFO_NAMEDRULE) {
			if (emit) {
				rpa_compiler_rulepref_set_flag(dbex->co, name->str, name->size, RPA_RFLAG_EMITRECORD);
			} else {
				rpa_compiler_rulepref_clear_flag(dbex->co, name->str, name->size, RPA_RFLAG_EMITRECORD);
			}
		}
	}
	return 0;
}


static int rpa_dbex_rh_emitall(rpadbex_t *dbex, long rec)
{
	rarray_t *records = dbex->records;
	rparecord_t *prec;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_dbex_setemit(dbex, TRUE);
	rpa_dbex_debug_recordtail(dbex, rec);
	if (rpa_dbex_playchildrecords(dbex, rec) < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_dbex_debug_recordtail(dbex, rec);
	return 0;
}


static int rpa_dbex_rh_emitnone(rpadbex_t *dbex, long rec)
{
	rarray_t *records = dbex->records;
	rparecord_t *prec;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_dbex_setemit(dbex, FALSE);
	rpa_dbex_debug_recordtail(dbex, rec);
	if (rpa_dbex_playchildrecords(dbex, rec) < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_dbex_debug_recordtail(dbex, rec);
	return 0;
}


static int rpa_dbex_rh_namedrule(rpadbex_t *dbex, long rec)
{
	const char *name = NULL;
	unsigned long namesize;
	rarray_t *records = dbex->records;
	rparecord_t *prec = (rparecord_t *) r_array_slot(dbex->records, rec);

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	if (rpa_dbex_rulename(dbex, rec, &name, &namesize) < 0) {

		return -1;
	}
	if (!r_array_empty(dbex->inlinestack)) {
		rpa_compiler_inlinerule_begin(dbex->co, name, namesize, 0);
	} else {
		rvm_codegen_addins(dbex->co->cg, rvm_asm(RPA_SHIFT, XX, XX, XX, 0));
		if (RPA_BITMAP_GETVAL(RPA_RECORD2BITMAP(prec))) {
			rvm_codegen_addins(dbex->co->cg, rvm_asm(RPA_EXITONBITMAP, DA, XX, XX, RPA_BITMAP_GETVAL(RPA_RECORD2BITMAP(prec))));
		}
		rvm_codegen_addins(dbex->co->cg, rvm_asm(RVM_BL, DA, XX, XX, 3));
		rvm_codegen_addins(dbex->co->cg, rvm_asm(RPA_EMITTAIL, XX, XX, XX, 0));
		rvm_codegen_addins(dbex->co->cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));

		if ((prec->usertype & RPA_LOOP_PATH)) {
			rpa_compiler_loop_begin(dbex->co, name, namesize);
		} else {
			rpa_compiler_rule_begin(dbex->co, name, namesize, RPA_BITMAP_GETVAL(RPA_RECORD2BITMAP(prec)));
		}
	}
	r_array_add(dbex->inlinestack, &rec);
	rpa_dbex_debug_recordtail(dbex, rec);
	if (rpa_dbex_playchildrecords(dbex, rec) < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	r_array_removelast(dbex->inlinestack);
	if (!r_array_empty(dbex->inlinestack)) {
		rpa_compiler_inlinerule_end(dbex->co);
	} else {
		if ((prec->usertype & RPA_LOOP_PATH)) {
			rpa_compiler_loop_end(dbex->co);
		} else {
			rpa_compiler_rule_end(dbex->co);
		}
	}
	rpa_dbex_debug_recordtail(dbex, rec);
	return 0;
}


static int rpa_dbex_rh_anonymousrule(rpadbex_t *dbex, long rec)
{
	rarray_t *records = dbex->records;
	rparecord_t *prec = (rparecord_t *) r_array_slot(dbex->records, rec);

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rvm_codegen_addins(dbex->co->cg, rvm_asm(RPA_SHIFT, XX, XX, XX, 0));
	if (RPA_BITMAP_GETVAL(RPA_RECORD2BITMAP(prec))) {
		rvm_codegen_addins(dbex->co->cg, rvm_asm(RPA_EXITONBITMAP, DA, XX, XX, RPA_BITMAP_GETVAL(RPA_RECORD2BITMAP(prec))));
	}
	rpa_compiler_exp_begin(dbex->co, RPA_MATCH_NONE, RPA_BITMAP_GETVAL(RPA_RECORD2BITMAP(prec)));
	rpa_dbex_debug_recordtail(dbex, rec);
	if (rpa_dbex_playchildrecords(dbex, rec) < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_compiler_exp_end(dbex->co);
	rvm_codegen_addins(dbex->co->cg, rvm_asm(RPA_EMITTAIL, XX, XX, XX, 0));
	rvm_codegen_addins(dbex->co->cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));
	rpa_dbex_debug_recordtail(dbex, rec);

	return 0;
}


static int rpa_dbex_rh_char(rpadbex_t *dbex, long rec)
{
	rparecord_t *prec;
	rarray_t *records = dbex->records;
	ruint32 wc = 0;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_dbex_debug_recordtail(dbex, rec);
	if (rpa_dbex_playchildrecords(dbex, rec) < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	if (r_utf8_mbtowc(&wc, (const unsigned char*) prec->input, (const unsigned char*)prec->input + prec->inputsiz) < 0) {

		return -1;
	}
	rvm_codegen_addins(dbex->co->cg, rvm_asm(rpa_dbex_getmatchchr(prec->usertype & RPA_MATCH_MASK), DA, XX, XX, wc));
	rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_dbex_debug_recordtail(dbex, rec);
	return 0;
}


static int rpa_dbex_rh_specialchar(rpadbex_t *dbex, long rec)
{
	ruint32 wc = 0;
	rarray_t *records = dbex->records;
	rparecord_t *prec;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_dbex_debug_recordtail(dbex, rec);
	if (rpa_dbex_playchildrecords(dbex, rec) < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	if (r_utf8_mbtowc(&wc, (const unsigned char*) prec->input, (const unsigned char*)prec->input + prec->inputsiz) < 0) {

		return -1;
	}
	rvm_codegen_addins(dbex->co->cg, rvm_asm(rpa_dbex_getmatchspecialchr(prec->usertype & RPA_MATCH_MASK), DA, XX, XX, wc));
	rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_dbex_debug_recordtail(dbex, rec);
	return 0;
}


static int rpa_dbex_rh_specialclschar(rpadbex_t *dbex, long rec)
{
	ruint32 wc = 0;
	rarray_t *records = dbex->records;
	rparecord_t *prec;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_dbex_debug_recordtail(dbex, rec);
	if (rpa_dbex_playchildrecords(dbex, rec) < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	if (r_utf8_mbtowc(&wc, (const unsigned char*) prec->input, (const unsigned char*)prec->input + prec->inputsiz) < 0) {

		return -1;
	}
	rvm_codegen_addins(dbex->co->cg, rvm_asm(rpa_dbex_getmatchspecialchr(prec->usertype & RPA_MATCH_MASK), DA, XX, XX, wc));
	rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_dbex_debug_recordtail(dbex, rec);
	return 0;
}


static int rpa_dbex_rh_cls(rpadbex_t *dbex, long rec)
{
	rarray_t *records = dbex->records;
	rparecord_t *prec;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_compiler_class_begin(dbex->co, prec->usertype & RPA_MATCH_MASK);
	rpa_dbex_debug_recordtail(dbex, rec);
	if (rpa_dbex_playchildrecords(dbex, rec) < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_compiler_class_end(dbex->co);
	rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_dbex_debug_recordtail(dbex, rec);
	return 0;
}


static int rpa_dbex_rh_clschar(rpadbex_t *dbex, long rec)
{
	ruint32 wc = 0;
	rarray_t *records = dbex->records;
	rparecord_t *prec;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_dbex_debug_recordtail(dbex, rec);
	if (rpa_dbex_playchildrecords(dbex, rec) < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	if (r_utf8_mbtowc(&wc, (const unsigned char*) prec->input, (const unsigned char*)prec->input + prec->inputsiz) < 0) {

		return -1;
	}
	rvm_codegen_addins(dbex->co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, wc));
	rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_dbex_debug_recordtail(dbex, rec);
	return 0;
}


static int rpa_dbex_rh_minexp(rpadbex_t *dbex, long rec)
{
	rarray_t *records = dbex->records;
	rparecord_t *prec;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_compiler_exp_begin(dbex->co, prec->usertype & RPA_MATCH_MASK, RPA_BITMAP_GETVAL(RPA_RECORD2BITMAP(prec)));
	rpa_dbex_debug_recordtail(dbex, rec);
	if (rpa_dbex_playreversechildrecords(dbex, rec) < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_compiler_exp_end(dbex->co);
	rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_dbex_debug_recordtail(dbex, rec);
	return 0;
}


static int rpa_dbex_rh_exp(rpadbex_t *dbex, long rec)
{
	rarray_t *records = dbex->records;
	rparecord_t *prec;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_compiler_exp_begin(dbex->co, (prec->usertype & RPA_MATCH_MASK), 0);
	rpa_dbex_debug_recordtail(dbex, rec);
	if (rpa_dbex_playchildrecords(dbex, rec) < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_compiler_exp_end(dbex->co);
	rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_dbex_debug_recordtail(dbex, rec);
	return 0;
}


static int rpa_dbex_rh_orop(rpadbex_t *dbex, long rec)
{
	rarray_t *records = dbex->records;
	rparecord_t *prec;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_compiler_altexp_begin(dbex->co, (prec->usertype & RPA_MATCH_MASK), RPA_BITMAP_GETVAL(RPA_RECORD2BITMAP(prec)));
	rpa_dbex_debug_recordtail(dbex, rec);
	if (rpa_dbex_playchildrecords(dbex, rec) < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_compiler_altexp_end(dbex->co);
	rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_dbex_debug_recordtail(dbex, rec);
	return 0;
}


static int rpa_dbex_rh_norop(rpadbex_t *dbex, long rec)
{
	rarray_t *records = dbex->records;
	rparecord_t *prec;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_compiler_altexp_begin(dbex->co, prec->usertype & RPA_MATCH_MASK, 0);
	rpa_dbex_debug_recordtail(dbex, rec);
	if (rpa_dbex_playchildrecords(dbex, rec) < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_compiler_altexp_end(dbex->co);
	rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_dbex_debug_recordtail(dbex, rec);
	return 0;
}


static int rpa_dbex_rh_notop(rpadbex_t *dbex, long rec)
{
	rarray_t *records = dbex->records;
	rparecord_t *prec;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_compiler_notexp_begin(dbex->co, prec->usertype & RPA_MATCH_MASK);
	rpa_dbex_debug_recordtail(dbex, rec);
	if (rpa_dbex_playchildrecords(dbex, rec) < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_compiler_notexp_end(dbex->co);
	rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_dbex_debug_recordtail(dbex, rec);
	return 0;
}


static int rpa_dbex_rh_range(rpadbex_t *dbex, long rec)
{
	rarray_t *records = dbex->records;
	rparecord_t *prec;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	dbex->co->currange.p1 = 0;
	dbex->co->currange.p2 = 0;
	rpa_dbex_debug_recordtail(dbex, rec);
	if (rpa_dbex_playchildrecords(dbex, rec) < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	if (dbex->co->currange.p1 < dbex->co->currange.p2)
		rvm_codegen_addins(dbex->co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, dbex->co->currange.p1, dbex->co->currange.p2));
	else
		rvm_codegen_addins(dbex->co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, dbex->co->currange.p2, dbex->co->currange.p1));
	rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_dbex_debug_recordtail(dbex, rec);
	return 0;
}


static int rpa_dbex_rh_numrange(rpadbex_t *dbex, long rec)
{
	rarray_t *records = dbex->records;
	rparecord_t *prec;
	rparecord_t *child;
	/*
	 * Fix me: probably we don't need to access the children from here. There should be a way just to
	 * play them a regular records!
	 */

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	child = rpa_dbex_record(dbex, rpa_recordtree_firstchild(dbex->records, rec, RPA_RECORD_END));
	if (rpa_record2long(child, &dbex->co->currange.p1) < 0)
		return -1;
	child = rpa_dbex_record(dbex, rpa_recordtree_lastchild(dbex->records, rec, RPA_RECORD_END));
	if (rpa_record2long(child, &dbex->co->currange.p2) < 0)
		return -1;
	rpa_dbex_debug_recordtail(dbex, rec);
	if (rpa_dbex_playchildrecords(dbex, rec) < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	if (dbex->co->currange.p1 < dbex->co->currange.p2)
		rvm_codegen_addins(dbex->co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, dbex->co->currange.p1, dbex->co->currange.p2));
	else
		rvm_codegen_addins(dbex->co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, dbex->co->currange.p2, dbex->co->currange.p1));
	rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_dbex_debug_recordtail(dbex, rec);
	return 0;
}


static int rpa_dbex_rh_clsnum(rpadbex_t *dbex, long rec)
{
	rarray_t *records = dbex->records;
	rparecord_t *prec;
	rparecord_t *child;
	ruint32 wc;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_dbex_debug_recordtail(dbex, rec);
	if (rpa_dbex_playchildrecords(dbex, rec) < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	child = rpa_dbex_record(dbex, rpa_recordtree_firstchild(dbex->records, rec, RPA_RECORD_END));
	if (rpa_record2long(child, &wc) < 0)
		return -1;
	rvm_codegen_addins(dbex->co->cg, rvm_asm(rpa_dbex_getmatchchr(prec->usertype & RPA_MATCH_MASK), DA, XX, XX, wc));
	rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_dbex_debug_recordtail(dbex, rec);
	return 0;
}


static int rpa_dbex_rh_beginchar(rpadbex_t *dbex, long rec)
{
	rarray_t *records = dbex->records;
	rparecord_t *prec;
	ruint32 wc = 0;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_dbex_debug_recordtail(dbex, rec);
	if (rpa_dbex_playchildrecords(dbex, rec) < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	if (r_utf8_mbtowc(&wc, (const unsigned char*) prec->input, (const unsigned char*)prec->input + prec->inputsiz) < 0) {

		return -1;
	}
	dbex->co->currange.p1 = wc;
	rpa_dbex_debug_recordtail(dbex, rec);
	return 0;
}


static int rpa_dbex_rh_endchar(rpadbex_t *dbex, long rec)
{
	rarray_t *records = dbex->records;
	rparecord_t *prec;
	ruint32 wc = 0;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_dbex_debug_recordtail(dbex, rec);
	if (rpa_dbex_playchildrecords(dbex, rec) < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	if (r_utf8_mbtowc(&wc, (const unsigned char*) prec->input, (const unsigned char*)prec->input + prec->inputsiz) < 0) {

		return -1;
	}
	dbex->co->currange.p2 = wc;
	rpa_dbex_debug_recordtail(dbex, rec);
	return 0;
}


static int rpa_dbex_rh_branch(rpadbex_t *dbex, long rec)
{
	rarray_t *records = dbex->records;
	rparecord_t *prec;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	if (prec->usertype & RPA_NONLOOP_PATH) {
		rpa_compiler_nonloopybranch_begin(dbex->co, (prec->usertype & RPA_MATCH_MASK));
	} else {
		rpa_compiler_branch_begin(dbex->co, (prec->usertype & RPA_MATCH_MASK), RPA_BITMAP_GETVAL(RPA_RECORD2BITMAP(prec)));
	}
	rpa_dbex_debug_recordtail(dbex, rec);
	if (rpa_dbex_playchildrecords(dbex, rec) < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	if (prec->usertype & RPA_NONLOOP_PATH) {
		rpa_compiler_nonloopybranch_end(dbex->co);
	} else {
		rpa_compiler_branch_end(dbex->co);
	}
	rpa_dbex_debug_recordtail(dbex, rec);
	return 0;
}


static void rpa_dbex_rh_loopref(rpadbex_t *dbex, rparecord_t *prec)
{
	/*
	 * We ignore, it doesn't make sense for loops:
	 * RPA_MATCH_MULTIPLE
	 */
	rpa_compiler_exp_begin(dbex->co, (prec->usertype & RPA_MATCH_OPTIONAL), 0);
	rvm_codegen_addins(dbex->co->cg, rvm_asm(RVM_CMP, R_LOO, DA, XX, 0));
	rvm_codegen_addins(dbex->co->cg, rvm_asm(RVM_BGRE, DA, XX, XX, 3));
	rvm_codegen_addins(dbex->co->cg, rvm_asm(RVM_MOVS, R0, DA, XX, -1));
	rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(dbex->co->cg, rvm_asm(RVM_ADD, R_TOP, R_TOP, R_LOO, 0));
	rvm_codegen_addins(dbex->co->cg, rvm_asm(RVM_MOVS, R0, R_LOO, XX, 0));
	rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_exp_end(dbex->co);
}


static int rpa_dbex_rh_aref(rpadbex_t *dbex, long rec)
{
	const char *name = NULL;
	unsigned long namesize;
	rpa_ruleinfo_t *info;
	rarray_t *records = dbex->records;
	rparecord_t *prec;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	if (rpa_dbex_rulename(dbex, rec, &name, &namesize) < 0) {

		return -1;
	}

	if ((prec->usertype & RPA_LOOP_PATH) && rpa_parseinfo_loopdetect(dbex, rec, rpa_dbex_firstinlined(dbex))) {
		info = (rpa_ruleinfo_t *) r_harray_get(dbex->rules, rpa_dbex_lookup(dbex, name, namesize));
		if (!info) {
			RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_UNRESOLVEDSYMBOL);
			RPA_DBEX_SETERRINFO_NAME(dbex, name, namesize);
			return -1;
		}
		if (rpa_dbex_findinlined(dbex, info->startrec)) {
			rpa_dbex_rh_loopref(dbex, prec);
		} else {
			if (prec->usertype & RPA_MATCH_OPTIONAL) {
				/*
				 * Most probably this is useless case - loop refs shouldn't have quantitative modifiers
				 * but in case they do we wrap the inlined production rule in quantitative expression.
				 * The inlined named rule can take the quantitative argument, but I just don't have
				 * a clean way to pass it from here - so, lets play the records inside an expression that
				 * has the right quantitative argument.
				 * We ignore, it doesn't make sense for loops:
				 * RPA_MATCH_MULTIPLE
				 */
				rpa_compiler_exp_begin(dbex->co, RPA_MATCH_OPTIONAL, 0);
				rpa_dbex_playrecord(dbex, info->startrec);
				rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
				rpa_compiler_exp_end(dbex->co);
			} else {
				rpa_dbex_playrecord(dbex, info->startrec);
			}
		}
	} else {
		rpa_compiler_reference(dbex->co, name, namesize, (prec->usertype & RPA_MATCH_MASK));
	}
	rvm_codegen_index_addrelocins(dbex->co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(dbex->co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_dbex_debug_recordtail(dbex, rec);
	if (rpa_dbex_playchildrecords(dbex, rec) < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = rpa_dbex_record(dbex, rec);
	R_ASSERT(prec);
	rpa_dbex_debug_recordhead(dbex, rec);
	rpa_dbex_debug_recordtail(dbex, rec);
	return 0;
}


rpadbex_t *rpa_dbex_create(void)
{
	rpadbex_t *dbex = (rpadbex_t *) r_zmalloc(sizeof(*dbex));

	dbex->co = rpa_compiler_create();
	dbex->pa = rpa_parser_create();
	dbex->text = r_array_create(sizeof(char *));
	dbex->records = r_array_create(sizeof(rparecord_t));
	dbex->temprecords = r_array_create(sizeof(rparecord_t));
	dbex->rules = r_harray_create(sizeof(rpa_ruleinfo_t));
	dbex->recstack = r_array_create(sizeof(unsigned long));
	dbex->inlinestack = r_array_create(sizeof(unsigned long));
	dbex->handlers = r_zmalloc(sizeof(rpa_dbex_recordhandler) * RPA_PRODUCTION_COUNT);
	rpa_dbex_cfgset(dbex, RPA_DBEXCFG_OPTIMIZATIONS, 1);
	rpa_dbex_cfgset(dbex, RPA_DBEXCFG_BITMAP, 1);

	dbex->handlers[RPA_PRODUCTION_NONE] = rpa_dbex_rh_default;
	dbex->handlers[RPA_PRODUCTION_NAMEDRULE] = rpa_dbex_rh_namedrule;
	dbex->handlers[RPA_PRODUCTION_ANONYMOUSRULE] = rpa_dbex_rh_anonymousrule;
	dbex->handlers[RPA_PRODUCTION_CLS] = rpa_dbex_rh_cls;
	dbex->handlers[RPA_PRODUCTION_CHAR] = rpa_dbex_rh_char;
	dbex->handlers[RPA_PRODUCTION_SPECIALCHAR] = rpa_dbex_rh_specialchar;
	dbex->handlers[RPA_PRODUCTION_SPECIALCLSCHAR] = rpa_dbex_rh_specialclschar;
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
	dbex->handlers[RPA_PRODUCTION_MINOP] = rpa_dbex_rh_minexp;
	dbex->handlers[RPA_PRODUCTION_DIRECTIVEEMIT] = rpa_dbex_rh_emit;
	dbex->handlers[RPA_PRODUCTION_DIRECTIVEABORT] = rpa_dbex_rh_abort;
	dbex->handlers[RPA_PRODUCTION_DIRECTIVENOEMIT] = rpa_dbex_rh_noemit;
	dbex->handlers[RPA_PRODUCTION_DIRECTIVEEMITALL] = rpa_dbex_rh_emitall;
	dbex->handlers[RPA_PRODUCTION_DIRECTIVEEMITNONE] = rpa_dbex_rh_emitnone;
	dbex->handlers[RPA_PRODUCTION_DIRECTIVEEMITID] = rpa_dbex_rh_uid;

	return dbex;
}


void rpa_dbex_destroy(rpadbex_t *dbex)
{
	unsigned long i;
	if (dbex) {
		for (i = 0; i < r_array_length(dbex->text); i++)
			r_free(r_array_index(dbex->text, i, char*));
		rpa_compiler_destroy(dbex->co);
		rpa_parser_destroy(dbex->pa);
		r_harray_destroy(dbex->rules);
		r_array_destroy(dbex->records);
		r_array_destroy(dbex->temprecords);
		r_array_destroy(dbex->recstack);
		r_array_destroy(dbex->inlinestack);
		r_array_destroy(dbex->text);
		r_free(dbex->handlers);
		r_free(dbex);
	}
}


static int rpa_parseinfo_loopdetect_do(rpadbex_t *dbex, long parent, long loopto, int inderction)
{
	unsigned long namesiz;
	const char *name;
	long i;
	int ret = 0;
	rparecord_t *prec;

	if (parent == loopto && inderction > 0)
		return 1;
	for (i = 0; i < (long)r_array_length(dbex->recstack); i++) {
		if (parent == r_array_index(dbex->recstack, i, long))
			return 0;
	}
	r_array_add(dbex->recstack, &parent);

	if (!(prec = (rparecord_t *)r_array_slot(dbex->records, parent)))
		return 0;
	if (prec->ruleuid == RPA_PRODUCTION_AREF || prec->ruleuid == RPA_PRODUCTION_CREF)
		i = parent;
	else
		i = rpa_recordtree_firstchild(dbex->records, parent, RPA_RECORD_START);
	for (; i >= 0; i = rpa_recordtree_next(dbex->records, i, RPA_RECORD_START)) {
		prec = (rparecord_t *)r_array_slot(dbex->records, i);
		if (prec->ruleuid == RPA_PRODUCTION_RULENAME)
			continue;
		if (prec->ruleuid == RPA_PRODUCTION_AREF || prec->ruleuid == RPA_PRODUCTION_CREF) {
			rpa_ruleinfo_t *info;
			if ((inderction > 0 || i != parent) && i == loopto) {
				/*
				 * We found what we are looking for
				 */
				ret = 1;
				break;
			}
			if (rpa_dbex_rulename(dbex, i, &name, &namesiz) < 0)
				R_ASSERT(0);
			info = (rpa_ruleinfo_t *) r_harray_get(dbex->rules, rpa_dbex_lookup(dbex, name, namesiz));
			if (!info)
				continue;
			if ((ret = rpa_parseinfo_loopdetect_do(dbex, info->startrec, loopto, inderction + 1)) > 0)
				break;
		} else {
			if ((ret = rpa_parseinfo_loopdetect_do(dbex, i, loopto, inderction + 1)) > 0)
				break;
		}

		if ((prec->usertype & RPA_MATCH_OPTIONAL) == 0 && (prec->ruleuid == RPA_PRODUCTION_CREF || prec->ruleuid == RPA_PRODUCTION_AREF ||
				prec->ruleuid == RPA_PRODUCTION_CHAR || prec->ruleuid == RPA_PRODUCTION_CLS || prec->ruleuid == RPA_PRODUCTION_SPECIALCHAR))
			break;

	}

	r_array_removelast(dbex->recstack);
	return ret;
}


static int rpa_parseinfo_loopdetect(rpadbex_t *dbex, long parent, long loopto)
{
	if (parent != loopto) {
		/*
		 * Make sure we are dealing with a loop first
		 */
		if (!rpa_parseinfo_loopdetect_do(dbex, loopto, parent, 0))
			return 0;
	}

	return (rpa_parseinfo_loopdetect_do(dbex, parent, loopto, 0)) ? 1 : 0;
}


static void rpa_parseinfo_marklooppath(rpadbex_t *dbex, long parent)
{
	long i;

	if (rpa_parseinfo_loopdetect(dbex, parent, parent) > 0) {
		rpa_record_setusertype(dbex->records, parent, RPA_LOOP_PATH, RVALSET_OR);
		for (i = rpa_recordtree_firstchild(dbex->records, parent, RPA_RECORD_START); i >= 0; i = rpa_recordtree_next(dbex->records, i, RPA_RECORD_START)) {
			rpa_parseinfo_marklooppath(dbex, i);
		}
	}
}


static int rpa_parseinfo_rule_checkforloop(rpadbex_t *dbex, const char *name, unsigned int namesize, long loopto)
{
	rpa_ruleinfo_t *info = info = (rpa_ruleinfo_t *)r_harray_get(dbex->rules, rpa_dbex_lookup(dbex, name, namesize));

	if (!info)
		return 0;
	return rpa_parseinfo_loopdetect(dbex, info->startrec, loopto);
}


static void rpa_dbex_buildloopinfo(rpadbex_t *dbex)
{
	size_t i;
	long p;
	rharray_t *rules = dbex->rules;
	rpa_ruleinfo_t *info;

	for (i = 0; i < r_array_length(rules->members); i++) {
		if ((info = (rpa_ruleinfo_t *)r_harray_get(rules, i)) != NULL)
			rpa_parseinfo_marklooppath(dbex, info->startrec);
	}

	/*
	 * Mark the non-loop branches.
	 */
	for (i = 0; i < r_array_length(dbex->records); i++) {
		rparecord_t *prec = (rparecord_t *)r_array_slot(dbex->records, i);
		if (prec->type == RPA_RECORD_START &&
			(prec->ruleuid == RPA_PRODUCTION_ALTBRANCH) &&
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
	rparecord_t *rec;
	rpa_ruleinfo_t info;
	unsigned int nrecords;
	const char *name = NULL;
	unsigned long namesize = 0;
	unsigned long i;

	if (dbex->rules) {
		r_object_destroy((robject_t *)dbex->rules);
		dbex->rules = NULL;
	}
	dbex->rules = r_harray_create(sizeof(rpa_ruleinfo_t));

	for (i = 0, nrecords = r_array_length(dbex->records); i < nrecords; i++) {
		if (!(rec = rpa_dbex_record(dbex, i)))
			continue;
		if ((rec->ruleuid == RPA_PRODUCTION_NAMEDRULE) && (rec->type & RPA_RECORD_START)) {
			r_memset(&info, 0, sizeof(info));
			info.type = RPA_RULEINFO_NAMEDRULE;
			info.startrec = i;
			info.sizerecs = rpa_recordtree_size(dbex->records, i);
			if (info.sizerecs < 0)
				continue;
			if (rpa_dbex_rulename(dbex, i, &name, &namesize) < 0) {
				continue;
			}
			r_harray_add(dbex->rules, name, namesize, &info);
			i += info.sizerecs - 1;
		} else if ((rec->ruleuid == RPA_PRODUCTION_ANONYMOUSRULE) && (rec->type & RPA_RECORD_START)) {
			r_memset(&info, 0, sizeof(info));
			info.type = RPA_RULEINFO_ANONYMOUSRULE;
			info.startrec = i;
			info.sizerecs = rpa_recordtree_size(dbex->records, i);
			if (info.sizerecs < 0)
				continue;
			if ((rec = rpa_dbex_record(dbex, rpa_recordtree_get(dbex->records, i, RPA_RECORD_END))))
				r_harray_add(dbex->rules, rec->input, rec->inputsiz, &info);
			i += info.sizerecs - 1;
		} else if ((rec->type & RPA_RECORD_START) && (rec->ruleuid >= RPA_PRODUCTION_DIRECTIVEEMIT) && (rec->ruleuid <= RPA_PRODUCTION_DIRECTIVEEMITID)) {
			r_memset(&info, 0, sizeof(info));
			info.type = RPA_RULEINFO_DIRECTIVE;
			info.startrec = i;
			info.sizerecs = rpa_recordtree_size(dbex->records, i);
			if (info.sizerecs < 0)
				continue;
			if ((rec = rpa_dbex_record(dbex, rpa_recordtree_get(dbex->records, i, RPA_RECORD_END))))
				r_harray_add(dbex->rules, rec->input, rec->inputsiz, &info);
			i += info.sizerecs - 1;
		}

	}
}


static long rpa_dbex_copy_handler(rarray_t *records, long rec, rpointer userdata)
{
	rpadbex_t *dbex = (rpadbex_t *)userdata;
	long index;

	rparecord_t *prec = (rparecord_t *)r_array_slot(records, rec);
	if (prec->ruleuid == RPA_PRODUCTION_OCCURENCE && (prec->type & RPA_RECORD_START)) {
		/*
		 * Ignore it
		 */
	} else if (prec->ruleuid == RPA_PRODUCTION_OCCURENCE && (prec->type & (RPA_RECORD_END))) {
		ruint32 usertype = RPA_MATCH_NONE;
		long lastrec = 0;
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
			break;
		};
		lastrec = r_array_length(dbex->records) - 1;
		if (lastrec >= 0)
			rpa_record_setusertype(dbex->records, lastrec, usertype, RVALSET_OR);
	} else if (prec->ruleuid) {
		index = r_array_add(dbex->records, prec);
		/*
		 * Optimizations. Lets apply the optimizations while we copy the records.
		 * This is probably not the most clean way to apply optimizations. In the future
		 * we should probably think of optimization pass right before compiling.
		 */
		if (dbex->optimizations) {
			if (prec->ruleuid == RPA_PRODUCTION_OROP && (prec->type & RPA_RECORD_END)) {
				rpa_optimiztion_orop(dbex->records, rpa_recordtree_get(dbex->records, index, RPA_RECORD_START));
			}
		}
	}

	return 0;
}


static void rpa_dbex_copyrecords(rpadbex_t *dbex)
{
	int i;
	rarray_t *records = dbex->temprecords;

	for (i = rpa_recordtree_get(records, 0, RPA_RECORD_START); i >= 0; i = rpa_recordtree_next(records, i, RPA_RECORD_START))
		rpa_recordtree_walk(records, i, 0, rpa_dbex_copy_handler, dbex);
}


static rparecord_t *rpa_dbex_record(rpadbex_t *dbex, long rec)
{
	if (!dbex || !dbex->rules)
		return NULL;
	return rpa_record_get(dbex->records, rec);
}


static rparecord_t *rpa_dbex_rulerecord(rpadbex_t *dbex, rparule_t rid)
{
	rparecord_t *prec;
	rpa_ruleinfo_t *info;
	long rec;

	if (!dbex || !dbex->rules)
		return NULL;
	info = r_harray_get(dbex->rules, rid);
	if (!info)
		return NULL;
	rec = info->startrec + info->sizerecs - 1;
	if (rec < 0 || rec >= (rparule_t)r_array_length(dbex->records))
		return NULL;
	prec = (rparecord_t *)r_array_slot(dbex->records, rec);
	return prec;
}


static int rpa_dbex_rulename(rpadbex_t *dbex, long rec, const char **name, unsigned long *namesize)
{
	rparecord_t *pnamerec = rpa_dbex_record(dbex, rpa_recordtree_firstchild(dbex->records, rpa_recordtree_get(dbex->records, rec, RPA_RECORD_START), RPA_RECORD_END));
	if (!pnamerec || !(pnamerec->ruleuid & RPA_PRODUCTION_RULENAME))
		return -1;
	*name = pnamerec->input;
	*namesize = pnamerec->inputsiz;
	return 0;
}


int rpa_dbex_open(rpadbex_t *dbex)
{
	if (!dbex)
		return -1;
	if (dbex->rules) {
		r_object_destroy((robject_t *)dbex->rules);
		dbex->rules = NULL;
	}
	dbex->compiled = 0;
	return 0;
}


void rpa_dbex_close(rpadbex_t *dbex)
{
	if (!dbex)
		return;
	rpa_dbex_buildruleinfo(dbex);
	rpa_dbex_buildloopinfo(dbex);
//	if (dbex->bitmap)
//		rpa_dbex_buildbitmapinfo(dbex);
}


long rpa_dbex_load(rpadbex_t *dbex, const char *rules, unsigned long size)
{
	long ret;
	char *text;

	if (!dbex)
		return -1;
	if (dbex->rules) {
		/*
		 * Dbex is not open
		 */
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_NOTOPEN);
		return -1;
	}

	text = r_strndup(rules, size);
	R_ASSERT(text);
	r_array_add(dbex->text, &text);
	r_array_setlength(dbex->temprecords, 0);
	if ((ret = rpa_parser_load(dbex->pa, text, size, dbex->temprecords)) < 0) {

		return -1;
	}
	if (ret != (long)size) {
		long line = 1;
		char *ptext = text;
		ptext += ret;
		for (line = 1; ptext >= text; --ptext) {
			if (*ptext == '\n')
				line += 1;
		}
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_SYNTAXERROR);
		RPA_DBEX_SETERRINFO_OFFSET(dbex, ret);
		RPA_DBEX_SETERRINFO_LINE(dbex, line);
		return -1;
	}
	rpa_dbex_copyrecords(dbex);
	return ret;
}


long rpa_dbex_load_s(rpadbex_t *dbex, const char *rules)
{
	return rpa_dbex_load(dbex, rules, r_strlen(rules));
}


void rpa_dbex_dumpindented(rpadbex_t *dbex, long rec, int level, const char *rulelabel)
{
	char buffer[1024];
	int i, size;
	rparecord_t *prec = rpa_dbex_record(dbex, rpa_recordtree_get(dbex->records, rec, RPA_RECORD_END));

	if (!prec)
		return;
	r_memset(buffer, 0, sizeof(buffer));
	if (dbex->bitmap)
		r_printf("[ 0x%016lx ] ", prec->userdata);
	for (i = 0; i < level + 1; i++)
		r_printf("   ");
	r_printf("(");
	r_printf("%s, %c, %c", rulelabel, rpa_record_optchar(prec, 'x'), rpa_record_loopchar(prec, 'x'));
	r_printf(")");
	size = R_MIN(prec->inputsiz, sizeof(buffer) - 1);
	r_strncpy(buffer, prec->input, size);

	if (size == (sizeof(buffer) - 1))
		r_printf(" %s ...\n", buffer);
	else
		r_printf(" %s\n", buffer);
	return;
}


static long rpa_dbex_firstinlined(rpadbex_t *dbex)
{
	long ret = r_array_empty(dbex->inlinestack) ? -1 : r_array_index(dbex->inlinestack, 0, long);
	return ret;
}


static int rpa_dbex_findinlined(rpadbex_t *dbex, long startrec)
{
	unsigned long i;
	for (i = 0; i < r_array_length(dbex->inlinestack); i++) {
		if (r_array_index(dbex->inlinestack, i, long) == startrec)
			return 1;
	}
	return 0;
}


static void rpa_dbex_dumptree_do(rpadbex_t *dbex, long rec, int level)
{
	rparecord_t *prec = rpa_dbex_record(dbex, rec);
	if (prec && prec->ruleuid == RPA_PRODUCTION_RULENAME)
		return;
	if (prec && (prec->ruleuid == RPA_PRODUCTION_AREF || prec->ruleuid == RPA_PRODUCTION_CREF)) {
		const char *name = NULL;
		unsigned long namesize = 0;
		int loop = 0;
		rpa_ruleinfo_t *info;

		if (rpa_dbex_rulename(dbex, rec, &name, &namesize) >= 0) {
			loop = rpa_parseinfo_rule_checkforloop(dbex, name, namesize, rpa_dbex_firstinlined(dbex));
			info = (rpa_ruleinfo_t *)r_harray_get(dbex->rules, rpa_dbex_lookup(dbex, name, namesize));
			if (loop && info){
				if (!rpa_dbex_findinlined(dbex, info->startrec)) {
					/*
					 * Temporary set the quantitative flags for the inlined rule to the parent
					 * reference, so they are printed correctly. After the printing is done
					 * restore the original flags.
					 */
					rparecord_t *prulestart = rpa_dbex_record(dbex, rpa_recordtree_get(dbex->records, info->startrec, RPA_RECORD_START));
					rparecord_t *pruleend = rpa_dbex_record(dbex, rpa_recordtree_get(dbex->records, info->startrec, RPA_RECORD_END));
					unsigned long optional = (prulestart->usertype & RPA_MATCH_OPTIONAL);
					prulestart->usertype |= (prec->usertype & RPA_MATCH_OPTIONAL);
					pruleend->usertype |= (prec->usertype & RPA_MATCH_OPTIONAL);
					r_array_add(dbex->inlinestack, &info->startrec);
					rpa_dbex_dumptree_do(dbex, info->startrec, level);
					r_array_removelast(dbex->inlinestack);
					if (!optional) {
						prulestart->usertype &= ~RPA_MATCH_OPTIONAL;
						pruleend->usertype &= ~RPA_MATCH_OPTIONAL;
					}
				} else {
					rpa_dbex_dumpindented(dbex, rpa_recordtree_get(dbex->records, rec, RPA_RECORD_END), level, "loopref");
				}
				return;
			}
		}
	}
	rpa_dbex_dumpindented(dbex, rpa_recordtree_get(dbex->records, rec, RPA_RECORD_END), level, prec->rule);
	for (rec = rpa_recordtree_firstchild(dbex->records, rec, RPA_RECORD_START); rec >= 0; rec = rpa_recordtree_next(dbex->records, rec, RPA_RECORD_START)) {
		rpa_dbex_dumptree_do(dbex, rec, level + 1);
	}
}


int rpa_dbex_dumptree(rpadbex_t *dbex, rparule_t rid)
{
	rpa_ruleinfo_t *info;

	if (!dbex)
		return -1;
	if (rid < 0) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_PARAM);
		return -1;
	}
	if (!dbex->rules) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_NOTCLOSED);
		return -1;
	}
	if (!(info = (rpa_ruleinfo_t *)r_harray_get(dbex->rules, rid))) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_NOTFOUND);
		return -1;
	}
	r_array_add(dbex->inlinestack, &info->startrec);
	rpa_dbex_dumptree_do(dbex, info->startrec, 0);
	r_array_removelast(dbex->inlinestack);
	return 0;
}


int rpa_dbex_dumpproductions(rpadbex_t *dbex)
{
	int ret = 0;
	rparule_t rid;
	char buffer[512];

	if (!dbex)
		return -1;
	if (!dbex->rules) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_NOTCLOSED);
		return -1;
	}
	for (rid = rpa_dbex_first(dbex); rid >= 0; rid = rpa_dbex_next(dbex, rid)) {
		ret = rpa_dbex_strncpy(dbex, buffer, rid, sizeof(buffer) - 1);
		if ( ret >= 0) {
			buffer[ret] = '\0';
			if (ret == sizeof(buffer))
				r_printf("   %s ...\n", buffer);
			else
				r_printf("   %s\n", buffer);
		}

	}
	return ret;
}


int rpa_dbex_dumprecords(rpadbex_t *dbex)
{
	unsigned long i;

	if (!dbex)
		return -1;
	if (!dbex->rules) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_NOTCLOSED);
		return -1;
	}
	for (i = 0; i < r_array_length(dbex->records); i++) {
		rpa_record_dump(dbex->records, i, 1);
	}
	return 0;
}


int rpa_dbex_dumpinfo(rpadbex_t *dbex)
{
	unsigned long i;
	rpa_ruleinfo_t *info;

	if (!dbex)
		return -1;
	if (!dbex->rules) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_NOTCLOSED);
		return -1;
	}
	for (i = 0; i < r_array_length(dbex->rules->names); i++) {
		rstr_t *name = r_array_index(dbex->rules->names, i, rstr_t*);
		info = (rpa_ruleinfo_t *)r_harray_get(dbex->rules, i);
		switch (info->type) {
		case RPA_RULEINFO_NAMEDRULE:
			r_printf("N ");
			break;
		case RPA_RULEINFO_ANONYMOUSRULE:
			r_printf("A ");
			break;
		case RPA_RULEINFO_DIRECTIVE:
			r_printf("D ");
			break;
		default:
			r_printf("  ");
			break;
		};
		r_printf("(%7d, %4d, code: %7ld, %5ld) : %s\n", info->startrec, info->sizerecs, info->codeoff, info->codesiz, name->str);
	}
	return 0;
}


int rpa_dbex_dumpuids(rpadbex_t *dbex)
{
	unsigned long i;
	long rec;
	rpa_ruleinfo_t *info;
	char *buffer = r_zmalloc(32 * sizeof(char));

	if (!dbex)
		return -1;
	if (!dbex->rules) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_NOTCLOSED);
		return -1;
	}
	for (i = 0; i < r_array_length(dbex->rules->names); i++) {
		info = (rpa_ruleinfo_t *)r_harray_get(dbex->rules, i);
		if (info->type == RPA_RULEINFO_DIRECTIVE) {
			rparecord_t *prec = rpa_dbex_record(dbex, rpa_recordtree_get(dbex->records, info->startrec, RPA_RECORD_END));
			if (prec->ruleuid == RPA_PRODUCTION_DIRECTIVEEMITID && prec->inputsiz) {
				rec = rpa_recordtree_firstchild(dbex->records, info->startrec, RPA_RECORD_START);
				while (rec >= 0) {
					prec = rpa_dbex_record(dbex, rpa_recordtree_get(dbex->records, rec, RPA_RECORD_END));
					if (prec->ruleuid == RPA_PRODUCTION_ALIASNAME) {
						ruint32 dec;
						if (rpa_record2long(rpa_dbex_record(dbex, rpa_recordtree_next(dbex->records, rec, RPA_RECORD_END)), &dec) < 0)
							break;
						buffer = r_realloc(buffer, prec->inputsiz + 1);
						r_memset(buffer, 0, prec->inputsiz + 1);
						r_memcpy(buffer, prec->input, prec->inputsiz);
						r_printf("#define %s %d\n", buffer, dec);
						break;
					}
					rec = rpa_recordtree_next(dbex->records, rec, RPA_RECORD_START);
				}
			}
		}
	}
	r_free(buffer);
	return 0;
}


int rpa_dbex_dumpcode(rpadbex_t* dbex, rparule_t rid)
{
	rpa_ruleinfo_t *info;
	if (!dbex)
		return -1;
	if (rid < 0) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_PARAM);
		return -1;
	}
	if (!dbex->rules) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_NOTCLOSED);
		return -1;
	}
	info = (rpa_ruleinfo_t *)r_harray_get(dbex->rules, rid);
	if (!info)
		return -1;
	rvm_asm_dump(dbex->pa->stat->cpu, rvm_codegen_getcode(dbex->co->cg, info->codeoff), info->codesiz);
	return 0;
}


long rpa_dbex_strlen(rpadbex_t *dbex, rparule_t rid)
{
	rparecord_t *prec;

	if (!dbex)
		return -1;
	if ((prec = rpa_dbex_rulerecord(dbex, rid)) == NULL) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_NOTFOUND);
		return -1;
	}
	return prec->inputsiz;
}


long rpa_dbex_strncpy(rpadbex_t *dbex, char *dst, rparule_t rid, unsigned long n)
{
	rparecord_t *prec;
	unsigned long size;

	if (!dbex)
		return -1;
	if ((prec = rpa_dbex_rulerecord(dbex, rid)) == NULL) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_NOTFOUND);
		return -1;
	}
	size = prec->inputsiz;
	if (n < size)
		size = n;
	r_strncpy(dst, prec->input, size);
	return size;
}


const char *rpa_dbex_name(rpadbex_t *dbex, rparule_t rid)
{
	rstr_t *name;

	if (!dbex)
		return NULL;
	if (!dbex->rules) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_NOTCLOSED);
		return NULL;
	}
	if (rid >= (rparule_t)r_array_length(dbex->rules->members))
		return NULL;
	name = r_array_index(dbex->rules->names, rid, rstr_t*);
	return name->str;
}


rparule_t rpa_dbex_first(rpadbex_t *dbex)
{
	if (!dbex)
		return -1;
	if (!dbex->rules) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_NOTCLOSED);
		return -1;
	}

	if (r_array_length(dbex->rules->members) <= 0) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_NOTFOUND);
		return -1;
	}
	return 0;
}


rparule_t rpa_dbex_last(rpadbex_t *dbex)
{
	if (!dbex)
		return -1;
	if (!dbex->rules) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_NOTCLOSED);
		return -1;
	}

	if (r_array_length(dbex->rules->members) <= 0) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_NOTFOUND);
		return -1;
	}
	return r_array_length(dbex->rules->members) - 1;
}


rparule_t rpa_dbex_lookup(rpadbex_t *dbex, const char *name, unsigned int namesize)
{
	rparule_t ret;

	if (!dbex) {
		return -1;
	}
	if (!dbex->rules) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_NOTCLOSED);
		return -1;
	}

	ret = (rparule_t) r_harray_taillookup(dbex->rules, name, namesize);
	if (ret < 0) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_NOTFOUND);
	}
	return ret;
}


rparule_t rpa_dbex_lookup_s(rpadbex_t *dbex, const char *name)
{
	return rpa_dbex_lookup(dbex, name, r_strlen(name));
}


rparule_t rpa_dbex_next(rpadbex_t *dbex, rparule_t rid)
{
	if (!dbex)
		return -1;
	if (!dbex->rules) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_NOTCLOSED);
		return -1;
	}

	++rid;
	if (rid < (rparule_t)r_array_length(dbex->rules->members))
		return rid;
	return -1;
}


rparule_t rpa_dbex_prev(rpadbex_t *dbex, rparule_t rid)
{
	if (!dbex)
		return -1;
	if (!dbex->rules) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_NOTCLOSED);
		return -1;
	}
	--rid;
	if (rid >= 0)
		return rid;
	return -1;
}


long rpa_dbex_lasterror(rpadbex_t *dbex)
{
	if (!dbex)
		return -1;
	return dbex->err.code;
}


long rpa_dbex_lasterrorinfo(rpadbex_t *dbex, rpa_errinfo_t *errinfo)
{
	if (!dbex || !errinfo)
		return -1;
	r_memcpy(errinfo, &dbex->err, sizeof(rpa_errinfo_t));
	return 0;
}


const char *rpa_dbex_version()
{
	return RPA_VERSION;
}


static int rpa_dbex_compile_rule(rpadbex_t *dbex, rparule_t rid)
{
	long codeoff;
	rpa_ruleinfo_t *info = (rpa_ruleinfo_t *)r_harray_get(dbex->rules, rid);

	if (!info)
		return -1;
	codeoff = rvm_codegen_getcodesize(dbex->co->cg);
	/*
	 * Set the rid in the rulepref, so the compiler associates this rule
	 * with the correct rid.
	 */
	rpa_compiler_rulepref_set_ruleid_s(dbex->co, rpa_dbex_name(dbex, rid), rid);

	/*
	 * Build the bitmap
	 */
	if (dbex->bitmap)
		rpa_dbex_getrulebitmap(dbex, rid);
	if (rpa_dbex_playrecord(dbex, info->startrec) < 0)
		return -1;
	info->codeoff = codeoff;
	info->codesiz = rvm_codegen_getcodesize(dbex->co->cg) - codeoff;
	return 0;
}


int rpa_dbex_compile(rpadbex_t *dbex)
{
	rparule_t rid;
	rvm_codelabel_t *labelerr;

	if (!dbex)
		return -1;
	if (!dbex->rules) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_NOTCLOSED);
		return -1;
	}
	/*
	 * By default all production rules emit
	 */
	if (dbex->co)
		rpa_compiler_destroy(dbex->co);
	dbex->co = rpa_compiler_create();
	rpa_dbex_setemit(dbex, TRUE);

	for (rid = rpa_dbex_first(dbex); rid >= 0; rid = rpa_dbex_next(dbex, rid)) {
		if (rpa_dbex_compile_rule(dbex, rid) < 0) {
			RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_COMPILE);
			return -1;
		}
	}

	if (rvm_codegen_relocate(dbex->co->cg, &labelerr) < 0) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_UNRESOLVEDSYMBOL);
		RPA_DBEX_SETERRINFO_NAME(dbex, labelerr->name->str, labelerr->name->size);
		return -1;
	}
	dbex->compiled = 1;
	return 0;
}


rvm_asmins_t *rpa_dbex_executable(rpadbex_t *dbex)
{
	if (!dbex)
		return NULL;
	if (!dbex->rules) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_NOTCLOSED);
		return NULL;
	}
	if (!dbex->compiled || rvm_codegen_getcodesize(dbex->co->cg) == 0) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_NOTCOMPILED);
		return NULL;
	}
	return rvm_codegen_getcode(dbex->co->cg, 0);
}


long rpa_dbex_executableoffset(rpadbex_t *dbex, rparule_t rid)
{
	rpa_ruleinfo_t *info;

	if (!dbex)
		return -1;
	if (!dbex->rules) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_NOTCLOSED);
		return -1;
	}
	if (!dbex->compiled) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_NOTCOMPILED);
		return -1;
	}
	info = (rpa_ruleinfo_t *)r_harray_get(dbex->rules, rid);
	if (!info) {
		RPA_DBEX_SETERRINFO_CODE(dbex, RPA_E_NOTFOUND);
		return -1;
	}
	return info->codeoff;
}


long rpa_dbex_cfgset(rpadbex_t *dbex, unsigned long cfg, unsigned long val)
{
	if (!dbex)
		return -1;
	if (cfg == RPA_DBEXCFG_OPTIMIZATIONS) {
		dbex->optimizations = val;
		return 0;
	} else if(cfg == RPA_DBEXCFG_DEBUG) {
		dbex->debug = val;
		return 0;
	} else if(cfg == RPA_DBEXCFG_BITMAP) {
		dbex->bitmap = val;
		return 0;
	}
	return -1;
}


long rpa_dbex_cfgget(rpadbex_t *dbex, unsigned long cfg)
{
	if (!dbex)
		return -1;
	if (cfg == RPA_DBEXCFG_OPTIMIZATIONS) {
		return dbex->optimizations;
	} else if(cfg == RPA_DBEXCFG_DEBUG) {
		return dbex->debug;
	} else if(cfg == RPA_DBEXCFG_BITMAP) {
		return dbex->bitmap;
	}
	return -1;
}

