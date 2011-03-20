#include "rmem.h"
#include "rpaparser.h"
#include "rpaparseinfo.h"


static void rpa_parseinfo_buildrefinfo(rpa_parseinfo_t *pi, rpastat_t *stat)
{
	ruint i, j;
	rparecord_t *rec;
	rpa_ruleinfo_t *info;
	rharray_t *rules = pi->rules;

	for (i = 0; i < r_array_length(rules->names); i++) {
		rstr_t *name = r_array_index(rules->names, i, rstr_t*);
		info = (rpa_ruleinfo_t *)r_harray_get(rules, r_harray_lookup(rules, name->str, name->size));
		if (info) {
			info->startref = r_array_length(pi->refs);
			for (j = info->startrec; j < info->sizerecs; j++) {
				rec = (rparecord_t *)r_array_slot(stat->records, j);
				if ((rec->userid == RPA_PRODUCTION_AREF) && (rec->type & RPA_RECORD_END)) {

				}
			}
		}
	}
}


static void rpa_parseinfo_buildruleinfo(rpa_parseinfo_t *pi, rpastat_t *stat)
{
	rparecord_t *rec, *namerec;
	rpa_ruleinfo_t info;
	ruint nrecords;
	ruint i, j;
	rharray_t *	rules = pi->rules;

	for (i = 0, nrecords = r_array_length(stat->records); i < nrecords; i++) {
		rec = (rparecord_t *)r_array_slot(stat->records, i);
		if ((rec->userid == RPA_PRODUCTION_NAMEDRULE) && (rec->type & RPA_RECORD_START)) {
			r_memset(&info, 0, sizeof(info));
			info.startrec = i;
			namerec = NULL;
			for (j = i + 1; j < nrecords; j++) {
				rec = (rparecord_t *)r_array_slot(stat->records, j);
				if ((rec->userid == RPA_PRODUCTION_NAMEDRULE) && (rec->type & RPA_RECORD_END)) {
					info.sizerecs = 1 + j - i;
					break;
				}
				if ((rec->userid == RPA_PRODUCTION_RULENAME) && (rec->type & RPA_RECORD_END)) {
					namerec = (rparecord_t *)r_array_slot(stat->records, j);
					continue;
				}
			}
			if (namerec)
				r_harray_add(rules, namerec->input, namerec->inputsiz, &info);
		}
	}
}


rpa_parseinfo_t *rpa_parseinfo_create(rpastat_t *stat)
{
	rpa_parseinfo_t *pi;

	if ((pi = (rpa_parseinfo_t *)r_zmalloc(sizeof(*pi))) == NULL)
		return NULL;
	pi->rules = r_harray_create(sizeof(rpa_ruleinfo_t));
	pi->refs = r_array_create(sizeof(ruint32));
	rpa_parseinfo_buildruleinfo(pi, stat);
	rpa_parseinfo_buildrefinfo(pi, stat);
	return pi;
}


void rpa_parseinfo_destroy(rpa_parseinfo_t *pi)
{
	if (pi) {
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
