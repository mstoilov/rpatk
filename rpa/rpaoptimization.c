/*
 *  Regular Pattern Analyzer Toolkit (RPA/Tk)
 *  Copyright (c) 2009-2010 Martin Stoilov
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

#include "rpa/rpaoptimization.h"
#include "rpa/rpaparser.h"


static long rpa_copy_handler(rarray_t *records, long rec, rpointer userdata)
{
	rarray_t *dst = (rarray_t *)userdata;
	rparecord_t *prec = (rparecord_t *)rpa_record_get(records, rec);
	r_array_add(dst, prec);
	return 0;
}


static long rpa_copy_singletonorop_handler(rarray_t *records, long rec, rpointer userdata)
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


static long rpa_singleton_handler(rarray_t *records, long rec, rpointer userdata)
{
	long first, last;
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


void rpa_optimiztion_orop(rarray_t *records, long orop)
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
