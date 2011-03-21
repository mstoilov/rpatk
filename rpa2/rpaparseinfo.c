#include "rmem.h"
#include "rpaparser.h"
#include "rpaparseinfo.h"


static int rpa_parseinfo_buildrefinfo_for_rule(rpa_parseinfo_t *pi, rpastat_t *stat, const char *name, ruint namesiz)
{
	ruint i;
	rpa_ruleinfo_t *info, *refinfo;
	rparecord_t *rec;

	info = (rpa_ruleinfo_t *)r_harray_get(pi->rules, r_harray_lookup(pi->rules, name, namesiz));
	if (!info)
		return -1;
	info->startref = r_array_length(pi->refs);
	for (i = info->startrec; i < info->sizerecs; i++) {
		rec = (rparecord_t *)r_array_slot(stat->records, i);
		if ((rec->userid == RPA_PRODUCTION_AREF) && (rec->type & RPA_RECORD_END)) {
			refinfo = (rpa_ruleinfo_t *)r_harray_get(pi->rules, r_harray_lookup(pi->rules, rec->input, rec->inputsiz));
			r_array_add(pi->refs, &refinfo->startrec);
		}
	}
	info->sizerefs = r_array_length(pi->refs) - info->startref;
	return 0;
}


static void rpa_parseinfo_buildrefinfo(rpa_parseinfo_t *pi)
{
	ruint i;
	rharray_t *rules = pi->rules;

}


static void rpa_parseinfo_buildruleinfo(rpa_parseinfo_t *pi)
{
	rparecord_t *rec, *namerec;
	rpa_ruleinfo_t info;
	ruint nrecords;
	rint i, nrec;
	rharray_t *	rules = pi->rules;

	for (i = 0, nrecords = r_array_length(pi->records); i < nrecords; i++) {
		rec = (rparecord_t *)r_array_slot(pi->records, i);
		if ((rec->userid == RPA_PRODUCTION_NAMEDRULE) && (rec->type & RPA_RECORD_START)) {
			r_memset(&info, 0, sizeof(info));
			info.startrec = i;
			info.sizerecs = rpa_recordtree_get(pi->records, i, RPA_RECORD_END);
			if (info.sizerecs < 0)
				continue;
			info.sizerecs = info.sizerecs - i + 1;

			/*
			 * The name record must be the first child
			 */
			nrec = rpa_recordtree_firstchild(pi->records, i, RPA_RECORD_END);
			if (nrec < 0)
				continue;
			namerec = (rparecord_t *)r_array_slot(pi->records, nrec);
			if ((namerec->userid == RPA_PRODUCTION_RULENAME) && (namerec->type & RPA_RECORD_END)) {
				r_harray_add(rules, namerec->input, namerec->inputsiz, &info);
				i += info.sizerecs - 1;
			}
		}
	}
}


rpa_parseinfo_t *rpa_parseinfo_create(rpastat_t *stat)
{
	rint i;
	rpa_parseinfo_t *pi;
	rparecord_t *prec;

	if ((pi = (rpa_parseinfo_t *)r_zmalloc(sizeof(*pi))) == NULL)
		return NULL;
	pi->records = r_array_create(sizeof(rparecord_t));
	pi->rules = r_harray_create(sizeof(rpa_ruleinfo_t));
	pi->refs = r_array_create(sizeof(rulong));
	for (i = 0; i < r_array_length(stat->records); i++) {
		prec = (rparecord_t *)r_array_slot(stat->records, i);
		if (prec->userid != RPA_RECORD_INVALID_UID)
			r_array_add(pi->records, prec);
	}
	rpa_parseinfo_buildruleinfo(pi);
	rpa_parseinfo_buildrefinfo(pi);
	return pi;
}


void rpa_parseinfo_destroy(rpa_parseinfo_t *pi)
{
	if (pi) {
		r_object_destroy((robject_t *)pi->records);
		r_object_destroy((robject_t *)pi->refs);
		r_object_destroy((robject_t *)pi->rules);
		r_free(pi);
	}
}


void rpa_parseinfo_dump(rpa_parseinfo_t *pi)
{
	ruint i;
	rharray_t *rules = pi->rules;
	rpa_ruleinfo_t *info;

	r_printf("Parser Info (%d): \n", r_carray_length(rules->members));
	for (i = 0; i < r_array_length(rules->names); i++) {
		rstr_t *name = r_array_index(rules->names, i, rstr_t*);
		info = (rpa_ruleinfo_t *)r_harray_get(rules, r_harray_lookup(rules, name->str, name->size));
		r_printf("(%7d, %4d), (%7d, %4d) : %s\n", info->startrec, info->sizerecs, info->startref, info->sizerefs, name->str);
	}
}


void rpa_parseinfo_dump_records(rpa_parseinfo_t *pi)
{
	rint i;

	for (i = 0; i < r_array_length(pi->records); i++) {
		rpa_record_dump(pi->records, i);
	}
}
