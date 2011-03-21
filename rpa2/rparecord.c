#include "rmem.h"
#include "rstring.h"
#include "rparecord.h"



rlong rpa_recordtree_get(rarray_t *records, rlong rec, rulong type)
{
	rlong i, s = 0;
	ruint startrec = (type & RPA_RECORD_START) ? 1 : 0;
	rparecord_t *prec;

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


rlong rpa_recordtree_firstchild(rarray_t *records, rlong rec, rulong type)
{
	rparecord_t *prec;

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


rlong rpa_recordtree_lastchild(rarray_t *records, rlong rec, rulong type)
{
	rparecord_t *prec;

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


rlong rpa_recordtree_next(rarray_t *records, rlong rec, rulong type)
{
	rparecord_t *prec;

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


rlong rpa_recordtree_prev(rarray_t *records, rlong rec, rulong type)
{
	rparecord_t *prec;

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


void rpa_record_dump(rarray_t *records, rlong rec)
{
	rparecord_t *prec = (rparecord_t *)r_array_slot(records, rec);
	rlong start, end, first, last, next, prev;
	rchar buf[240];
	rint bufsize = sizeof(buf) - 1;
	rint n = 0, size;

	r_memset(buf, 0, bufsize);

	start = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	end = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	first = rpa_recordtree_firstchild(records, rec, RPA_RECORD_START);
	last = rpa_recordtree_lastchild(records, rec, RPA_RECORD_START);
	next = rpa_recordtree_next(records, rec, RPA_RECORD_START);
	prev = rpa_recordtree_prev(records, rec, RPA_RECORD_START);

//	n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "%5d  ( %7ld, %4d ) : ", rec, prec->ruleid, (ruint32)prec->userid);
	n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "%5ld (s: %3ld, e: %3ld, f: %3ld, l: %3ld, n: %3ld, p: %3ld) ( %7d, %4d ) : ", rec, start, end, first, last, next, prev, prec->ruleid, (ruint32)prec->userid);
	if (prec->type & RPA_RECORD_START)
		n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "START ");
	if (prec->type & RPA_RECORD_MATCH)
		n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "MATCH ");
	if (prec->type & RPA_RECORD_END)
		n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "END ");
	n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "%s(%d) ", prec->rule, prec->type);

	r_memset(buf + n, ' ', bufsize - n);
	n = 120;
	n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, " : %5d, %3d", prec->top, prec->size);

	r_memset(buf + n, ' ', bufsize - n);
	n = 140;
	n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, " : ");
	size = prec->inputsiz;
	if (size >= bufsize - n - 1)
		size = bufsize - n - 1;
	if (prec->type & RPA_RECORD_END) {
		r_strncpy(buf + n, prec->input, prec->inputsiz);
		n += size;
		buf[n] = '\0';
	}

	r_printf("%s\n", buf);
}
