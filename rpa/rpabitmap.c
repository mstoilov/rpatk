/*
 *  Regular Pattern Analyzer (RPA)
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

#include "rpa/rpabitmap.h"
#include "rlib/rutf.h"
#include "rlib/rmem.h"
#include "rpa/rpaparser.h"
#include "rpa/rpadbexpriv.h"


static long rpa_bitmap_set(rarray_t *records, long rec, rpointer userdata);

void rpa_dbex_buildbitmapinfo_for_rule(rpadbex_t *dbex, rparule_t rid)
{
	rpa_bitmapcompiler_t bc;
	rharray_t *rules = dbex->rules;
	rpa_ruleinfo_t *info;

	r_memset(&bc, 0, sizeof(bc));
	bc.dbex = dbex;
	if ((info = (rpa_ruleinfo_t *)r_harray_get(rules, rid)) != NULL)
		rpa_recordtree_walk(dbex->records, info->startrec, 0, rpa_bitmap_set, (rpointer)&bc);

}


void rpa_dbex_buildbitmapinfo(rpadbex_t *dbex)
{
	unsigned int i;
	rharray_t *rules = dbex->rules;

	for (i = 0; i < r_array_length(rules->members); i++) {
		rpa_dbex_buildbitmapinfo_for_rule(dbex, i);
	}
}



static long rpa_bitmap_set_char(rarray_t *records, rparecord_t *record, long rec, rpointer userdata)
{
	ruint32 wc = 0;

	if (r_utf8_mbtowc(&wc, (const unsigned char*) record->input, (const unsigned char*)record->input + record->inputsiz) < 0) {
		/*
		 * Error
		 */
		return -1;
	}

	RPA_BITMAP_SETBIT(record, wc % RPA_BITMAP_BITS);
	return 0;
}


static long rpa_bitmap_set_range(rarray_t *records, rparecord_t *record, long rec, rpointer userdata)
{
	rpa_bitmapcompiler_t *bc = (rpa_bitmapcompiler_t*)userdata;

	if (record->type & RPA_RECORD_START) {
		bc->beginchar = 0;
		bc->endchar = 0;
	} else {
		long wc1, wc2, wc;
		if (bc->beginchar < bc->endchar) {
			wc1 = bc->beginchar;
			wc2 = bc->endchar;
		} else {
			wc2 = bc->beginchar;
			wc1 = bc->endchar;
		}
		for (wc = wc1; wc <= wc2 && (wc - wc1) < RPA_BITMAP_BITS; wc++) {
			RPA_BITMAP_SETBIT(record, (wc % RPA_BITMAP_BITS));
		}


	}
	return 0;
}


static long rpa_bitmap_set_beginchar(rarray_t *records, rparecord_t *record, long rec, rpointer userdata)
{
	rpa_bitmapcompiler_t *bc = (rpa_bitmapcompiler_t*)userdata;

	if (record->type & RPA_RECORD_END) {
		ruint32 wc = 0;

		if (r_utf8_mbtowc(&wc, (const unsigned char*) record->input, (const unsigned char*)record->input + record->inputsiz) < 0) {
			/*
			 * Error
			 */
			return -1;
		}
		bc->beginchar = wc;
	}
	return 0;
}


static long rpa_bitmap_set_endchar(rarray_t *records, rparecord_t *record, long rec, rpointer userdata)
{
	rpa_bitmapcompiler_t *bc = (rpa_bitmapcompiler_t*)userdata;

	if (record->type & RPA_RECORD_END) {
		ruint32 wc = 0;

		if (r_utf8_mbtowc(&wc, (const unsigned char*) record->input, (const unsigned char*)record->input + record->inputsiz) < 0) {
			/*
			 * Error
			 */
			return -1;
		}
		bc->endchar = wc;
	}
	return 0;
}


static long rpa_bitmap_set_clschar(rarray_t *records, rparecord_t *record, long rec, rpointer userdata)
{
	if (record->type & RPA_RECORD_END) {
		ruint32 wc = 0;

		if (r_utf8_mbtowc(&wc, (const unsigned char*) record->input, (const unsigned char*)record->input + record->inputsiz) < 0) {
			/*
			 * Error
			 */
			return -1;
		}
		RPA_BITMAP_SETBIT(record, wc % RPA_BITMAP_BITS);
	}
	return 0;
}


static long rpa_bitmap_set_cls(rarray_t *records, rparecord_t *record, long rec, rpointer userdata)
{
	if (record->type & RPA_RECORD_END) {
		long child;

		for (child = rpa_recordtree_firstchild(records, rec, RPA_RECORD_END); child >= 0; child = rpa_recordtree_next(records, child, RPA_RECORD_END)) {
			rparecord_t *childrecord = rpa_record_get(records, child);
			RPA_BITMAP_ORBITS(record, childrecord);
		}
		return 0;

	}
	return 0;
}


static long rpa_bitmap_set_namedrule(rarray_t *records, rparecord_t *record, long rec, rpointer userdata)
{
	if (record->type & RPA_RECORD_END) {
		long child;
		child = rpa_recordtree_firstchild(records, rec, RPA_RECORD_END);

		for (child = rpa_recordtree_next(records, child, RPA_RECORD_END); child >= 0; child = rpa_recordtree_next(records, child, RPA_RECORD_END)) {
			rparecord_t *childrecord = rpa_record_get(records, child);
			RPA_BITMAP_ORBITS(record, childrecord);
			if (!(childrecord->usertype & RPA_MATCH_OPTIONAL))
				break;
		}
		return 0;

	}
	return 0;
}


static long rpa_bitmap_set_expression(rarray_t *records, rparecord_t *record, long rec, rpointer userdata)
{
	if (record->type & RPA_RECORD_END) {
		long child;

		for (child = rpa_recordtree_firstchild(records, rec, RPA_RECORD_END); child >= 0; child = rpa_recordtree_next(records, child, RPA_RECORD_END)) {
			rparecord_t *childrecord = rpa_record_get(records, child);
			RPA_BITMAP_ORBITS(record, childrecord);
			if (!(childrecord->usertype & RPA_MATCH_OPTIONAL))
				break;
		}
		return 0;

	}
	return 0;
}


static long rpa_bitmap_set_orop(rarray_t *records, rparecord_t *record, long rec, rpointer userdata)
{
	if (record->type & RPA_RECORD_END) {
		long child;

		for (child = rpa_recordtree_firstchild(records, rec, RPA_RECORD_END); child >= 0; child = rpa_recordtree_next(records, child, RPA_RECORD_END)) {
			rparecord_t *childrecord = rpa_record_get(records, child);
			RPA_BITMAP_ORBITS(record, childrecord);
		}
		return 0;

	}
	return 0;
}


static long rpa_bitmap_set(rarray_t *records, long rec, rpointer userdata)
{
//	rpa_bitmapcompiler_t *bc = (rpa_bitmapcompiler_t*)userdata;
	rparecord_t *record = rpa_record_get(records, rec);

	switch (record->ruleuid) {
	case RPA_PRODUCTION_CHAR:
		rpa_bitmap_set_char(records, record, rec, userdata);
		break;
	case RPA_PRODUCTION_CHARRNG:
		rpa_bitmap_set_range(records, record, rec, userdata);
		break;
	case RPA_PRODUCTION_BEGINCHAR:
		rpa_bitmap_set_beginchar(records, record, rec, userdata);
		break;
	case RPA_PRODUCTION_ENDCHAR:
		rpa_bitmap_set_endchar(records, record, rec, userdata);
		break;
	case RPA_PRODUCTION_CLSCHAR:
		rpa_bitmap_set_clschar(records, record, rec, userdata);
		break;
	case RPA_PRODUCTION_CLS:
		rpa_bitmap_set_cls(records, record, rec, userdata);
		break;
	case RPA_PRODUCTION_NAMEDRULE:
		rpa_bitmap_set_namedrule(records, record, rec, userdata);
		break;
	case RPA_PRODUCTION_BRACKETEXP:
	case RPA_PRODUCTION_ALTBRANCH:
	case RPA_PRODUCTION_ANONYMOUSRULE:
		rpa_bitmap_set_expression(records, record, rec, userdata);
		break;
	case RPA_PRODUCTION_OROP:
		rpa_bitmap_set_orop(records, record, rec, userdata);
		break;

	default:
		break;
	};

	if (record) {
	}

	return 0;
}
