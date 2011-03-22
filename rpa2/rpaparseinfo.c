#include "rmem.h"
#include "rpaparser.h"
#include "rpaparseinfo.h"



static rint rpa_parseinfo_checkforloop(rpa_parseinfo_t *pi, rlong rec, rlong loopstartrec, rlong loopendrec, rint inderction)
{
	rlong nrec, i;
	rint lret, ret = 0;

	if (rec == loopstartrec && inderction > 0)
		return 1;

	for (i = 0; i < r_array_length(pi->recstack); i++) {
		if (rec == r_array_index(pi->recstack, i, rlong))
			return 0;
	}

	r_array_add(pi->recstack, &rec);

	for (i = rpa_recordtree_firstchild(pi->records, rec, RPA_RECORD_START); i >= 0; i = rpa_recordtree_next(pi->records, i, RPA_RECORD_START)) {
		rparecord_t *prec = (rparecord_t *)r_array_slot(pi->records, i);
		if (prec->userid == RPA_PRODUCTION_AREF || prec->userid == RPA_PRODUCTION_AREF) {
			nrec = rpa_recordtree_firstchild(pi->records, i, RPA_RECORD_END);
			if (nrec > 0) {
				rpa_ruleinfo_t *info;
				prec = (rparecord_t *)r_array_slot(pi->records, nrec);
				info = (rpa_ruleinfo_t *)r_harray_get(pi->rules, r_harray_lookup(pi->rules, prec->input, prec->inputsiz));
				if (info) {
					lret = rpa_parseinfo_checkforloop(pi, info->startrec, loopstartrec, loopendrec, inderction + 1);
					if (i >= loopstartrec && i <= loopendrec && lret) {
						rpa_record_setusertype(pi->records, i, RPA_LOOP_PATH, RVALSET_OR);
						rpa_record_setusertype(pi->records, nrec, RPA_LOOP_PATH, RVALSET_OR);
					}
					ret |= lret;
				}
			}
		} else {
			lret = rpa_parseinfo_checkforloop(pi, i, loopstartrec, loopendrec, inderction + 1);
			if (i >= loopstartrec && i <= loopendrec && lret) {
				rpa_record_setusertype(pi->records, i, RPA_LOOP_PATH, RVALSET_OR);
				ret |= lret;
			}

		}
	}

	r_array_removelast(pi->recstack);
	return ret;
}


static void rpa_parseinfo_buildloopinfo(rpa_parseinfo_t *pi)
{
	ruint i, p;
	rharray_t *rules = pi->rules;
	rpa_ruleinfo_t *info;

	for (i = 0; i < r_array_length(rules->members); i++) {
		info = (rpa_ruleinfo_t *)r_harray_get(rules, i);
		if (rpa_parseinfo_checkforloop(pi, info->startrec, info->startrec, info->startrec + info->sizerecs - 1, 0)) {
			rpa_record_setusertype(pi->records, info->startrec, RPA_LOOP_PATH, RVALSET_OR);
		}
	}

	/*
	 * Mark the non-loop branches.
	 */
	for (i = 0; i < r_array_length(pi->records); i++) {
		rparecord_t *prec = (rparecord_t *)r_array_slot(pi->records, i);
		if (prec->type == RPA_RECORD_START && prec->userid == RPA_PRODUCTION_ALTBRANCH && (prec->usertype & RPA_LOOP_PATH) == 0) {
			p = rpa_recordtree_parent(pi->records, i, RPA_RECORD_START);
			if (p >= 0) {
				prec = (rparecord_t *)r_array_slot(pi->records, p);
				if (prec && (prec->usertype & RPA_LOOP_PATH))
					rpa_record_setusertype(pi->records, i, RPA_NONLOOP_PATH, RVALSET_OR);
			}
		}
	}
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


static void rpa_parseinfo_copyrecords(rpa_parseinfo_t *pi, rpastat_t *stat)
{
	rint i;
	rparecord_t *prec;

	for (i = 0; i < r_array_length(stat->records); i++) {
		prec = (rparecord_t *)r_array_slot(stat->records, i);
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
			switch (stat->instack[prec->top].wc) {
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
			lastrec = r_array_length(pi->records) - 1;
			if (lastrec >= 0)
				rpa_record_setusertype(pi->records, lastrec, usertype, RVALSET_OR);
		} else if (prec->userid != RPA_RECORD_INVALID_UID) {
			r_array_add(pi->records, prec);
		}
	}

}


rpa_parseinfo_t *rpa_parseinfo_create(rpastat_t *stat)
{
	rpa_parseinfo_t *pi;

	if ((pi = (rpa_parseinfo_t *)r_zmalloc(sizeof(*pi))) == NULL)
		return NULL;
	pi->records = r_array_create(sizeof(rparecord_t));
	pi->rules = r_harray_create(sizeof(rpa_ruleinfo_t));
	pi->refs = r_array_create(sizeof(rulong));
	pi->recstack = r_array_create(sizeof(rulong));
	rpa_parseinfo_copyrecords(pi, stat);
	rpa_parseinfo_buildruleinfo(pi);
	rpa_parseinfo_buildloopinfo(pi);
	return pi;
}


void rpa_parseinfo_destroy(rpa_parseinfo_t *pi)
{
	if (pi) {
		r_object_destroy((robject_t *)pi->records);
		r_object_destroy((robject_t *)pi->rules);
		r_object_destroy((robject_t *)pi->recstack);
		r_object_destroy((robject_t *)pi->refs);
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
