#include "rpaoptimization.h"
#include "rpaparser.h"


static rlong rpa_copy_handler(rarray_t *records, rlong rec, rpointer userdata)
{
	rarray_t *dst = (rarray_t *)userdata;
	rparecord_t *prec = (rparecord_t *)rpa_record_get(records, rec);
	r_array_add(dst, prec);
	return 0;
}


static rlong rpa_copy_singletonorop_handler(rarray_t *records, rlong rec, rpointer userdata)
{
	rarray_t *dst = (rarray_t *)userdata;
	rparecord_t *prec = (rparecord_t *)rpa_record_get(records, rec);

	if (prec->ruleuid == RPA_PRODUCTION_OROP) {
		prec->ruleuid = RPA_PRODUCTION_CLS;
		prec->rule = "cls";
	} else if (prec->ruleuid == RPA_PRODUCTION_CHAR) {
		prec->ruleuid = RPA_PRODUCTION_CLSCHAR;
	} else if (prec->ruleuid == RPA_PRODUCTION_ALTBRANCH || prec->ruleuid == RPA_PRODUCTION_CLS) {
		/*
		 * Ignore it.
		 */
		return 0;
	}
	r_array_add(dst, prec);
	return 0;
}


static rlong rpa_singleton_handler(rarray_t *records, rlong rec, rpointer userdata)
{
	rlong first, last;
	rparecord_t *prec = rpa_record_get(records, rec);

	if (prec->ruleuid == RPA_PRODUCTION_ALTBRANCH && (prec->type & RPA_RECORD_START)) {
		first = rpa_recordtree_firstchild(records, rec, RPA_RECORD_START);
		last = rpa_recordtree_lastchild(records, rec, RPA_RECORD_START);
		if (first < 0 || first != last)
			return -1;
		prec = rpa_record_get(records, first);
		if (rpa_record_getusertype(records, first) & RPA_MATCH_MULTIPLE)
			return -1;
		if (prec->ruleuid != RPA_PRODUCTION_CLS && prec->ruleuid != RPA_PRODUCTION_CHAR)
			return -1;
	}
	return 0;
}


void rpa_optimiztion_orop(rarray_t *records, rlong orop)
{
	rarray_t *temp;

	if (rpa_recordtree_walk(records, orop, 0, rpa_singleton_handler, NULL) < 0)
		return;

	temp = r_array_create(sizeof(rparecord_t));
	rpa_recordtree_walk(records, orop, 0, rpa_copy_singletonorop_handler, temp);
	r_array_setlength(records, orop);
	rpa_recordtree_walk(temp, 0, 0, rpa_copy_handler, records);

	r_array_destroy(temp);
}
