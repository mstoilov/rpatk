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

#include "rlib/rmem.h"
#include "rlib/rstring.h"
#include "rpa/rpaparser.h"
#include "rpa/rparecord.h"


rparecord_t *rpa_record_get(rarray_t *records, long rec)
{
	rparecord_t *prec;

	if (!records)
		return NULL;
	if (rec < 0 || rec >= r_array_length(records))
		return NULL;
	prec = (rparecord_t *)r_array_slot(records, rec);
	return prec;

}


long rpa_recordtree_get(rarray_t *records, long rec, unsigned long type)
{
	long i, s = 0;
	unsigned int startrec = (type & RPA_RECORD_START) ? 1 : 0;
	rparecord_t *prec;

	if (rec < 0 || rec >= r_array_length(records))
		return -1;
	prec = (rparecord_t *)r_array_slot(records, rec);
	if ((prec->type & RPA_RECORD_START)) {
		if (startrec)
			return rec;
		for (s = 0, i = rec; i < r_array_length(records); i++) {
			prec = (rparecord_t *)r_array_slot(records, i);
			if (prec->type & RPA_RECORD_START)
				++s;
			if (prec->type & RPA_RECORD_END)
				--s;
			if (s == 0)
				return i;
		}
	}
	prec = (rparecord_t *)r_array_slot(records, rec);
	if ((prec->type & RPA_RECORD_END)) {
		if (!startrec)
			return rec;
		for (s = 0, i = rec; i >= 0; i--) {
			prec = (rparecord_t *)r_array_slot(records, i);
			if (prec->type & RPA_RECORD_START)
				++s;
			if (prec->type & RPA_RECORD_END)
				--s;
			if (s == 0)
				return i;
		}
	}

	return -1;
}


long rpa_recordtree_firstchild(rarray_t *records, long rec, unsigned long type)
{
	rparecord_t *prec;

	if (rec < 0 || rec >= r_array_length(records))
		return -1;
	prec = (rparecord_t *)r_array_slot(records, rec);
	if (prec->type & RPA_RECORD_END) {
		if ((rec = rpa_recordtree_get(records, rec, RPA_RECORD_START)) < 0)
			return -1;
	}
	if (++rec >= r_array_length(records))
		return -1;
	prec = (rparecord_t *)r_array_slot(records, rec);
	if (prec->type & RPA_RECORD_START)
		return rpa_recordtree_get(records, rec, type);
	return -1;
}


long rpa_recordtree_lastchild(rarray_t *records, long rec, unsigned long type)
{
	rparecord_t *prec;

	if (rec < 0 || rec >= r_array_length(records))
		return -1;
	prec = (rparecord_t *)r_array_slot(records, rec);
	if (prec->type & RPA_RECORD_START) {
		if ((rec = rpa_recordtree_get(records, rec, RPA_RECORD_END)) < 0)
			return -1;
	}
	if (--rec < 0)
		return -1;
	prec = (rparecord_t *)r_array_slot(records, rec);
	if (prec->type & RPA_RECORD_END)
		return rpa_recordtree_get(records, rec, type);
	return -1;
}


long rpa_recordtree_next(rarray_t *records, long rec, unsigned long type)
{
	rparecord_t *prec;

	if (rec < 0 || rec >= r_array_length(records))
		return -1;
	prec = (rparecord_t *)r_array_slot(records, rec);
	if (prec->type & RPA_RECORD_START) {
		if ((rec = rpa_recordtree_get(records, rec, RPA_RECORD_END)) < 0)
			return -1;
	}
	if (++rec >= r_array_length(records))
		return -1;
	prec = (rparecord_t *)r_array_slot(records, rec);
	if (prec->type & RPA_RECORD_START)
		return rpa_recordtree_get(records, rec, type);
	return -1;
}


long rpa_recordtree_prev(rarray_t *records, long rec, unsigned long type)
{
	rparecord_t *prec;

	if (rec < 0 || rec >= r_array_length(records))
		return -1;
	prec = (rparecord_t *)r_array_slot(records, rec);
	if (prec->type & RPA_RECORD_END) {
		if ((rec = rpa_recordtree_get(records, rec, RPA_RECORD_START)) < 0)
			return -1;
	}
	if (--rec < 0)
		return -1;
	prec = (rparecord_t *)r_array_slot(records, rec);
	if (prec->type & RPA_RECORD_END)
		return rpa_recordtree_get(records, rec, type);
	return -1;
}


long rpa_recordtree_parent(rarray_t *records, long rec, unsigned long type)
{
	long last = -1, parent = -1;

	if (rec < 0 || rec >= r_array_length(records))
		return -1;
	for ( ;rec >= 0; rec = rpa_recordtree_next(records, last, RPA_RECORD_END)) {
		last = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	}
	parent = last + 1;
	if (parent >= r_array_length(records))
		return -1;
	return rpa_recordtree_get(records, parent, type);
}


long rpa_recordtree_size(rarray_t *records, long rec)
{
	long first = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	long last = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	if (first < 0 || last < 0)
		return -1;
	return (last - first + 1);
}


long rpa_recordtree_copy(rarray_t *dst, rarray_t *src, long rec)
{
	rparecord_t *prec;
	long size, i;
	rec = rpa_recordtree_get(src, rec, RPA_RECORD_START);
	size = rpa_recordtree_size(src, rec);

	for (i = 0; i < size; i++) {
		prec = rpa_record_get(src, i);
		r_array_add(dst, prec);
	}
	return size;
}


long rpa_recordtree_walk(rarray_t *records, long rec, long level, rpa_recordtree_callback callback, rpointer userdata)
{
	long child;

	if (level > 128)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	if (callback && callback(records, rec, userdata) < 0)
		return -1;
	for (child = rpa_recordtree_firstchild(records, rec, RPA_RECORD_START); child >= 0; child = rpa_recordtree_next(records, child, RPA_RECORD_START)) {
		if (rpa_recordtree_walk(records, child, level + 1, callback, userdata) < 0)
			return -1;
	}
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	if (callback && callback(records, rec, userdata) < 0)
		return -1;
	return 0;
}


static void rpa_recordptr_setusertype(rparecord_t *prec, ruint32 usertype, rvalset_t op)
{
	switch (op) {
	case RVALSET_OR:
		prec->usertype |= usertype;
		break;
	case RVALSET_XOR:
		prec->usertype ^= usertype;
		break;
	case RVALSET_AND:
		prec->usertype &= usertype;
		break;
	default:
		prec->usertype = usertype;
	}
}


long rpa_record_getruleuid(rarray_t *records, long rec)
{
	rparecord_t *prec;

	if (rec < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	if (rec >= r_array_length(records))
		return -1;
	prec = (rparecord_t *)r_array_slot(records, rec);
	return prec->ruleuid;
}


void rpa_record_setusertype(rarray_t *records, long rec, ruint32 usertype, rvalset_t op)
{
	rparecord_t *prec;

	if (rec < 0)
		return;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	if (rec >= r_array_length(records))
		return;
	prec = (rparecord_t *)r_array_slot(records, rec);
	rpa_recordptr_setusertype(prec, usertype, op);
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	if (rec >= r_array_length(records))
		return;
	prec = (rparecord_t *)r_array_slot(records, rec);
	rpa_recordptr_setusertype(prec, usertype, op);
}


long rpa_record_getusertype(rarray_t *records, long rec)
{
	rparecord_t *prec;

	if (rec < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	if (rec >= r_array_length(records))
		return -1;
	prec = (rparecord_t *)r_array_slot(records, rec);
	return prec->usertype;
}


int rpa_record_optchar(rparecord_t *prec, int defc)
{
	int optc = defc;

	if ((prec->usertype & RPA_MATCH_MASK) == RPA_MATCH_OPTIONAL)
		optc = '?';
	else if ((prec->usertype & RPA_MATCH_MASK) == RPA_MATCH_MULTIPLE)
		optc = '+';
	else if ((prec->usertype & RPA_MATCH_MASK) == RPA_MATCH_MULTIOPT)
		optc = '*';
	else
		optc = defc;
	return optc;
}


int rpa_record_loopchar(rparecord_t *prec, int defc)
{
	int loopc = defc;

	if ((prec->usertype & RPA_LOOP_PATH) && (prec->usertype & RPA_NONLOOP_PATH)) {
		/*
		 * This is an error, should never happen
		 */
		loopc = 'R';
	} else if ((prec->usertype & RPA_LOOP_PATH)) {
		loopc = 'L';
	} else if ((prec->usertype & RPA_NONLOOP_PATH)) {
		loopc = 'N';
	} else {
		loopc = defc;
	}
	return loopc;
}


void rpa_record_dump(rarray_t *records, long rec)
{
	rparecord_t *prec;
	long start, end, first, last, next, prev, parent;
	char buf[240];
	int bufsize = sizeof(buf) - 1;
	int n = 0, size;
	char optc = ' ';

	if (rec < 0 || rec >= r_array_length(records))
		return;
	prec = (rparecord_t *)r_array_slot(records, rec);
	if (prec->type & RPA_RECORD_END) {
		if ((prec->usertype & RPA_MATCH_MASK) == RPA_MATCH_OPTIONAL)
			optc = '?';
		else if ((prec->usertype & RPA_MATCH_MASK) == RPA_MATCH_MULTIPLE)
			optc = '+';
		else if ((prec->usertype & RPA_MATCH_MASK) == RPA_MATCH_MULTIOPT)
			optc = '*';
	}

	r_memset(buf, 0, bufsize);

	start = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	end = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	first = rpa_recordtree_firstchild(records, rec, RPA_RECORD_START);
	last = rpa_recordtree_lastchild(records, rec, RPA_RECORD_START);
	next = rpa_recordtree_next(records, rec, RPA_RECORD_START);
	prev = rpa_recordtree_prev(records, rec, RPA_RECORD_START);
	parent = rpa_recordtree_parent(records, rec, RPA_RECORD_START);

	n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "%5ld: [ s: %5ld, e: %5ld, p: %5ld ] ( %4d, 0x%03x ) : ", rec, start, end, parent, prec->ruleuid, prec->usertype);
	if (prec->type & RPA_RECORD_START)
		n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "START ");
	if (prec->type & RPA_RECORD_END)
		n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "END   ");
	n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "%s ", prec->rule);

	r_memset(buf + n, ' ', bufsize - n);
	n = 90;
	n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, " %5d, %4d", prec->top, prec->size);

	r_memset(buf + n, ' ', bufsize - n);
	n = 105;
	n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "[ 0x%016x ]", prec->userdata);

	r_memset(buf + n, ' ', bufsize - n);
	n = 130;
	n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, " %c %c %c", optc,
			(prec->usertype & RPA_LOOP_PATH) ? 'L' : ' ', (prec->usertype & RPA_NONLOOP_PATH) ? 'N' : ' ');
	n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, " : ");
	size = prec->inputsiz;
	if (size >= bufsize - n - 1)
		size = bufsize - n - 1;
	if (prec->type & RPA_RECORD_END) {
		r_strncpy(buf + n, prec->input, size);
		n += size;
		buf[n] = '\0';
	}

	r_printf("%s\n", buf);
}


void rpa_record_dumpindented(rarray_t *records, long rec, int level)
{
	char buffer[1024];
	rparecord_t *prec;
	int i, size;

	if (rec < 0 || rec >= r_array_length(records))
		return;
	r_memset(buffer, 0, sizeof(buffer));
	prec = (rparecord_t *)r_array_slot(records, rec);
	for (i = 0; i < level; i++)
		r_printf("   ");
	r_printf("   ");
	r_printf("(");
	r_printf("%s, %c, %c", prec->rule, rpa_record_optchar(prec, 'x'), rpa_record_loopchar(prec, 'x'));
	r_printf(")");
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	size = R_MIN(prec->inputsiz, sizeof(buffer) - 1);
	r_strncpy(buffer, prec->input, size);

	if (size == (sizeof(buffer) - 1))
		r_printf(" %s ...\n", buffer);
	else
		r_printf(" %s\n", buffer);
	return;
}


rarray_t *rpa_records_create()
{
	rarray_t *records;

	records = r_array_create(sizeof(rparecord_t));
	return records;
}

void rpa_records_destroy(rarray_t *records)
{
	r_array_destroy(records);
}


long rpa_records_length(rarray_t *records)
{
	return r_array_length(records);
}


rparecord_t *rpa_records_slot(rarray_t *records, long index)
{
	if (index < 0 || index >= r_array_length(records))
		return NULL;
	return (rparecord_t *)r_array_slot(records, index);
}
