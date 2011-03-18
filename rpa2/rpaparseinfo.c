#include "rmem.h"
#include "rpaparser.h"
#include "rpaparseinfo.h"


rpa_parseinfo_t *rpa_parseinfo_create(rpastat_t *stat)
{
	rparecord_t *rec;
	rpa_ruleinfo_t info;
	rpa_parseinfo_t *pi;
	rharray_t *rules;
	ruint nrecords;
	ruint i, j;

	if ((pi = (rpa_parseinfo_t *)r_zmalloc(sizeof(*pi))) == NULL)
		return NULL;
	pi->rules = r_harray_create(sizeof(rpa_ruleinfo_t));
	pi->refs = r_array_create(sizeof(ruint32));
	rules = pi->rules;

	for (i = 0, nrecords = r_array_length(stat->records); i < nrecords; i++) {
		rec = (rparecord_t *)r_array_slot(stat->records, i);
		if ((rec->userid == RPA_PRODUCTION_NAMEDRULE) && (rec->type & RPA_RECORD_START)) {
			for (j = i + 1; j < nrecords; j++) {
				rec = (rparecord_t *)r_array_slot(stat->records, j);
				if ((rec->userid == RPA_PRODUCTION_NAMEDRULE) && (rec->type & RPA_RECORD_END))
					break;
				if ((rec->userid == RPA_PRODUCTION_RULENAME) && (rec->type & RPA_RECORD_END)) {
					info.record = i;
					r_harray_add(rules, rec->input, rec->inputsiz, &info);
					break;
				}

			}
		}
	}

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
		r_printf("%7d : %s\n", info->record, name->str);
	}
}
