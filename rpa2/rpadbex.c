#include "rpadbex.h"
#include "rpaparser.h"
#include "rpaparseinfo.h"
#include "rmem.h"

struct rpadbex_s {
	rpa_parser_t *pa;
	rarray_t *records;
	rharray_t *rules;
	rarray_t *recstack;
};


static rint rpa_parseinfo_checkforloop(rpadbex_t *dbex, rlong rec, rlong loopstartrec, rlong loopendrec, rint inderction)
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
		if (prec->userid == RPA_PRODUCTION_AREF || prec->userid == RPA_PRODUCTION_AREF) {
			nrec = rpa_recordtree_firstchild(dbex->records, i, RPA_RECORD_END);
			if (nrec > 0) {
				rpa_ruleinfo_t *info;
				prec = (rparecord_t *)r_array_slot(dbex->records, nrec);
				info = (rpa_ruleinfo_t *)r_harray_get(dbex->rules, r_harray_lookup(dbex->rules, prec->input, prec->inputsiz));
				if (info) {
					lret = rpa_parseinfo_checkforloop(dbex, info->startrec, loopstartrec, loopendrec, inderction + 1);
					if (i >= loopstartrec && i <= loopendrec && lret) {
						rpa_record_setusertype(dbex->records, i, RPA_LOOP_PATH, RVALSET_OR);
						rpa_record_setusertype(dbex->records, nrec, RPA_LOOP_PATH, RVALSET_OR);
					}
					ret |= lret;
				}
			}
		} else {
			lret = rpa_parseinfo_checkforloop(dbex, i, loopstartrec, loopendrec, inderction + 1);
			if (i >= loopstartrec && i <= loopendrec && lret) {
				rpa_record_setusertype(dbex->records, i, RPA_LOOP_PATH, RVALSET_OR);
				ret |= lret;
			}

		}
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
		if (rpa_parseinfo_checkforloop(dbex, info->startrec, info->startrec, info->startrec + info->sizerecs - 1, 0)) {
			rpa_record_setusertype(dbex->records, info->startrec, RPA_LOOP_PATH, RVALSET_OR);
		}
	}

	/*
	 * Mark the non-loop branches.
	 */
	for (i = 0; i < r_array_length(dbex->records); i++) {
		rparecord_t *prec = (rparecord_t *)r_array_slot(dbex->records, i);
		if (prec->type == RPA_RECORD_START && prec->userid == RPA_PRODUCTION_ALTBRANCH && (prec->usertype & RPA_LOOP_PATH) == 0) {
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


static rparecord_t *rpa_dbex_record(rpadbex_t *dbex, rparule_t rid)
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


rpadbex_t *rpa_dbex_create(void)
{
	rpadbex_t *dbex = (rpadbex_t *) r_zmalloc(sizeof(*dbex));

	dbex->pa = rpa_parser_create();
	dbex->records = r_array_create(sizeof(rparecord_t));
	dbex->rules = r_harray_create(sizeof(rpa_ruleinfo_t));
	dbex->recstack = r_array_create(sizeof(rulong));

	return dbex;
}


void rpa_dbex_destroy(rpadbex_t *dbex)
{
	if (dbex) {
		rpa_parser_destroy(dbex->pa);
		r_object_destroy((robject_t *)dbex->records);
		r_object_destroy((robject_t *)dbex->rules);
		r_object_destroy((robject_t *)dbex->recstack);
		r_free(dbex);
	}
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
	for (rid = rpa_dbex_rule_first(dbex); rid >= 0; rid = rpa_dbex_rule_next(dbex, rid)) {
		if (rpa_dbex_rule_copy(dbex, rid, buffer, sizeof(buffer)) >= 0)
			r_printf("   %s\n", buffer);
	}
	return ret;
}


rint rpa_dbex_dumprecords(rpadbex_t *dbex)
{
	rint i;

	if (!dbex)
		return -1;
	for (i = 0; i < r_array_length(dbex->records); i++) {
		rpa_record_dump(dbex->records, i);
	}
	return 0;
}


rint rpa_dbex_dumpruleinfo(rpadbex_t *dbex)
{
	ruint i;
	rpa_ruleinfo_t *info;

	if (!dbex || !dbex->rules)
		return -1;
	for (i = 0; i < r_array_length(dbex->rules->names); i++) {
		rstr_t *name = r_array_index(dbex->rules->names, i, rstr_t*);
		info = (rpa_ruleinfo_t *)r_harray_get(dbex->rules, r_harray_lookup(dbex->rules, name->str, name->size));
		r_printf("(%7d, %4d) : %s\n", info->startrec, info->sizerecs, name->str);
	}
	return 0;
}


rlong rpa_dbex_rule_copy(rpadbex_t *dbex, rparule_t rid, rchar *buf, rsize_t bufsize)
{
	rparecord_t *prec;

	if (!dbex)
		return -1;
	if ((prec = rpa_dbex_record(dbex, rid)) == NULL)
		return -1;
	if (bufsize <= prec->inputsiz)
		return -1;
	r_memset(buf, 0, bufsize);
	r_strncpy(buf, prec->input, prec->inputsiz);
	return prec->inputsiz;
}


rlong rpa_dbex_rule_size(rpadbex_t *dbex, rparule_t rid)
{
	rparecord_t *prec;

	if (!dbex)
		return -1;
	if ((prec = rpa_dbex_record(dbex, rid)) == NULL)
		return -1;
	return prec->inputsiz;
}


rparule_t rpa_dbex_rule_first(rpadbex_t *dbex)
{
	if (!dbex || !dbex->rules)
		return -1;

	if (r_array_length(dbex->rules->members) > 0)
		return 0;
	return -1;
}


rparule_t rpa_dbex_rule_last(rpadbex_t *dbex)
{
	if (!dbex || !dbex->rules)
		return -1;

	if (r_array_length(dbex->rules->members) > 0)
		return r_array_length(dbex->rules->members) - 1;
	return -1;
}


rparule_t rpa_dbex_rule_next(rpadbex_t *dbex, rparule_t rid)
{
	if (!dbex || !dbex->rules)
		return -1;
	++rid;
	if (rid < r_array_length(dbex->rules->members))
		return rid;
	return -1;
}


rparule_t rpa_dbex_rule_prev(rpadbex_t *dbex, rparule_t rid)
{
	if (!dbex || !dbex->rules)
		return -1;
	--rid;
	if (rid >= 0)
		return rid;
	return -1;
}


ruint rpa_dbex_get_error(rpadbex_t *dbex);
rint rpa_dbex_compile(rpadbex_t *dbex);

const rchar *rpa_dbex_version();
