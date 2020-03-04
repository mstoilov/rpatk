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

#include "rpa/rpabitmap.h"
#include "rlib/rutf.h"
#include "rlib/rmem.h"
#include "rpa/rpaparser.h"
#include "rpa/rpadbexpriv.h"
#include "rpa/rpastatpriv.h"
#include "rpa/rpacompiler.h"

static long rpa_bitmap_set_startrec(rarray_t *records, long rec, rpabitmap_t bitmap)
{
	rparecord_t *record = rpa_record_get(records, rpa_recordtree_get(records, rec, RPA_RECORD_START));
	RPA_BITMAP_SETVAL(RPA_RECORD2BITMAP(record), bitmap);
	return 0;
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
	RPA_BITMAP_SETBIT(RPA_RECORD2BITMAP(record), wc % RPA_BITMAP_BITS);
	return 0;
}


static long rpa_bitmap_set_range(rarray_t *records, rparecord_t *record, long rec, rpointer userdata)
{
	rpa_bitmapcompiler_t *bc = (rpa_bitmapcompiler_t*)userdata;

	if (record->type & RPA_RECORD_START) {
		bc->beginchar = 0;
		bc->endchar = 0;
	} else {
		unsigned long wc1, wc2, wc;
		if (bc->beginchar < bc->endchar) {
			wc1 = bc->beginchar;
			wc2 = bc->endchar;
		} else {
			wc2 = bc->beginchar;
			wc1 = bc->endchar;
		}
		for (wc = wc1; wc <= wc2 && (wc - wc1) < RPA_BITMAP_BITS; wc++) {
			RPA_BITMAP_SETBIT(RPA_RECORD2BITMAP(record), (wc % RPA_BITMAP_BITS));
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
		RPA_BITMAP_SETBIT(RPA_RECORD2BITMAP(record), wc % RPA_BITMAP_BITS);
	}
	return 0;
}


static long rpa_bitmap_set_cls(rarray_t *records, rparecord_t *record, long rec, rpointer userdata)
{
	if (record->type & RPA_RECORD_END) {
		long child;

		for (child = rpa_recordtree_firstchild(records, rec, RPA_RECORD_END); child >= 0; child = rpa_recordtree_next(records, child, RPA_RECORD_END)) {
			rparecord_t *childrecord = rpa_record_get(records, child);
			RPA_BITMAP_ORBITS(RPA_RECORD2BITMAP(record), RPA_RECORD2BITMAP(childrecord));
		}
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
			RPA_BITMAP_ORBITS(RPA_RECORD2BITMAP(record), RPA_RECORD2BITMAP(childrecord));
			if (!(childrecord->usertype & RPA_MATCH_OPTIONAL))
				break;
		}
	}
	return 0;
}


static long rpa_bitmap_set_expression(rarray_t *records, rparecord_t *record, long rec, rpointer userdata)
{
	if (record->type & RPA_RECORD_END) {
		long child;

		for (child = rpa_recordtree_firstchild(records, rec, RPA_RECORD_END); child >= 0; child = rpa_recordtree_next(records, child, RPA_RECORD_END)) {
			rparecord_t *childrecord = rpa_record_get(records, child);
			RPA_BITMAP_ORBITS(RPA_RECORD2BITMAP(record), RPA_RECORD2BITMAP(childrecord));
			if (!(childrecord->usertype & RPA_MATCH_OPTIONAL))
				break;
		}
	}
	return 0;
}


static long rpa_bitmap_set_orop(rarray_t *records, rparecord_t *record, long rec, rpointer userdata)
{
	if (record->type & RPA_RECORD_END) {
		long child;

		for (child = rpa_recordtree_firstchild(records, rec, RPA_RECORD_END); child >= 0; child = rpa_recordtree_next(records, child, RPA_RECORD_END)) {
			rparecord_t *childrecord = rpa_record_get(records, child);
			RPA_BITMAP_ORBITS(RPA_RECORD2BITMAP(record), RPA_RECORD2BITMAP(childrecord));
		}
	}
	return 0;
}


static long rpa_bitmap_set_minop(rarray_t *records, rparecord_t *record, long rec, rpointer userdata)
{
	if (record->type & RPA_RECORD_END) {
		long child;

		child = rpa_recordtree_firstchild(records, rec, RPA_RECORD_END);
		if (child >= 0) {
			rparecord_t *childrecord = rpa_record_get(records, child);
			RPA_BITMAP_SETVAL(RPA_RECORD2BITMAP(record), RPA_BITMAP_GETVAL(RPA_RECORD2BITMAP(childrecord)));
		}
	}
	return 0;
}


static long rpa_bitmap_set_specialchar(rarray_t *records, rparecord_t *record, long rec, rpointer userdata)
{
	ruint32 wc = 0;

	if (r_utf8_mbtowc(&wc, (const unsigned char*) record->input, (const unsigned char*)record->input + record->inputsiz) < 0) {
		/*
		 * Error
		 */
		return -1;
	}
	wc = rpa_special_char(wc);

	if (wc == '.') {
		RPA_BITMAP_SETALL(RPA_RECORD2BITMAP(record));
	} else {
		RPA_BITMAP_SETBIT(RPA_RECORD2BITMAP(record), wc % RPA_BITMAP_BITS);
	}
	return 0;
}


static long rpa_bitmap_set_clsnum(rarray_t *records, rparecord_t *record, long rec, rpointer userdata)
{
	if (record->type & RPA_RECORD_END) {
		long child = rpa_recordtree_firstchild(records, rec, RPA_RECORD_END);
		if (child >= 0) {
			rparecord_t *childrecord = rpa_record_get(records, child);
			RPA_BITMAP_SETBIT(RPA_RECORD2BITMAP(record), childrecord->userdata % RPA_BITMAP_BITS);
		}
	}
	return 0;
}


static long rpa_bitmap_set_numrng(rarray_t *records, rparecord_t *record, long rec, rpointer userdata)
{
	if (record->type & RPA_RECORD_END) {
		long first = rpa_recordtree_firstchild(records, rec, RPA_RECORD_END);
		long second = rpa_recordtree_lastchild(records, rec, RPA_RECORD_END);
		if (first >= 0 && second >= 0) {
			ruword wc1, wc2, wc;
			rparecord_t *firstrecord = rpa_record_get(records, first);
			rparecord_t *secondrecord = rpa_record_get(records, second);
			if (firstrecord->userdata < secondrecord->userdata) {
				wc1 = firstrecord->userdata;
				wc2 = secondrecord->userdata;
			} else {
				wc2 = firstrecord->userdata;
				wc1 = secondrecord->userdata;
			}
			for (wc = wc1; wc <= wc2 && (wc - wc1) < RPA_BITMAP_BITS; wc++) {
				RPA_BITMAP_SETBIT(RPA_RECORD2BITMAP(record), (wc % RPA_BITMAP_BITS));
			}
		}
	}
	return 0;
}


static long rpa_bitmap_set_ref(rarray_t *records, rparecord_t *record, long rec, rpointer userdata)
{
	rpa_bitmapcompiler_t *bc = (rpa_bitmapcompiler_t*)userdata;
	if ((record->type & RPA_RECORD_END) && (record->usertype & RPA_LOOP_PATH) == 0) {
		long child = rpa_recordtree_firstchild(records, rec, RPA_RECORD_END);
		if (child >= 0) {
			rparecord_t *childrecord = rpa_record_get(records, child);
			rparule_t rid = rpa_dbex_lookup(bc->dbex, childrecord->input, childrecord->inputsiz);
			if (rid >= 0) {
				RPA_BITMAP_SETVAL(RPA_RECORD2BITMAP(record), rpa_dbex_getrulebitmap(bc->dbex, rid));
			}

		}
	}
	return 0;
}


static long rpa_bitmap_set_long(rarray_t *records, rparecord_t *record, long rec, rpointer userdata)
{
	ruint32 wc = 0;
	if (rpa_record2long(record, &wc) < 0)
		return -1;
	record->userdata = wc;
	return 0;
}


static long rpa_bitmap_set_notop(rarray_t *records, rparecord_t *record, long rec, rpointer userdata)
{
	if (record->type & RPA_RECORD_END) {
		long child = rpa_recordtree_firstchild(records, rec, RPA_RECORD_END);
		if (child >= 0) {
			RPA_BITMAP_SETALL(RPA_RECORD2BITMAP(record));
		}
	}
	return 0;
}


long rpa_bitmap_set(rarray_t *records, long rec, rpointer userdata)
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
	case RPA_PRODUCTION_REQOP:
	case RPA_PRODUCTION_NEGBRANCH:
	case RPA_PRODUCTION_BRACKETEXP:
	case RPA_PRODUCTION_ALTBRANCH:
	case RPA_PRODUCTION_ANONYMOUSRULE:
		rpa_bitmap_set_expression(records, record, rec, userdata);
		break;
	case RPA_PRODUCTION_OROP:
	case RPA_PRODUCTION_NOROP:
		rpa_bitmap_set_orop(records, record, rec, userdata);
		break;
	case RPA_PRODUCTION_SPECIALCHAR:
		rpa_bitmap_set_specialchar(records, record, rec, userdata);
		break;
	case RPA_PRODUCTION_HEX:
	case RPA_PRODUCTION_DEC:
		rpa_bitmap_set_long(records, record, rec, userdata);
		break;
	case RPA_PRODUCTION_CLSNUM:
		rpa_bitmap_set_clsnum(records, record, rec, userdata);
		break;
	case RPA_PRODUCTION_NUMRNG:
		rpa_bitmap_set_numrng(records, record, rec, userdata);
		break;
	case RPA_PRODUCTION_AREF:
	case RPA_PRODUCTION_CREF:
		rpa_bitmap_set_ref(records, record, rec, userdata);
		break;
	case RPA_PRODUCTION_NOTOP:
		rpa_bitmap_set_notop(records, record, rec, userdata);
		break;
	case RPA_PRODUCTION_MINOP:
		rpa_bitmap_set_minop(records, record, rec, userdata);
		break;

	default:
		/*
		 * We should never get here!
		 */
		RPA_BITMAP_SETALL(RPA_RECORD2BITMAP(record));
		break;
	};

	if (record->type & RPA_RECORD_END) {
		rpa_bitmap_set_startrec(records, rec, RPA_BITMAP_GETVAL(RPA_RECORD2BITMAP(record)));
	}
	return 0;
}
