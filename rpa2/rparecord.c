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


rlong rpa_recordtree_parent(rarray_t *records, rlong rec, rulong type)
{
	rlong last = -1, parent = -1;

	if (rec < 0)
		return -1;

	for ( ;rec >= 0; rec = rpa_recordtree_next(records, last, RPA_RECORD_END)) {
		last = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	}
	parent = last + 1;
	if (parent >= r_array_length(records))
		return -1;
	return rpa_recordtree_get(records, parent, type);
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


void rpa_record_setusertype(rarray_t *records, rlong rec, ruint32 usertype, rvalset_t op)
{
	rparecord_t *prec;

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


rlong rpa_record_getusertype(rarray_t *records, rlong rec)
{
	rparecord_t *prec;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_START);
	if (rec >= r_array_length(records))
		return -1;
	prec = (rparecord_t *)r_array_slot(records, rec);
	return prec->usertype;
}


void rpa_record_dump(rarray_t *records, rlong rec)
{
	rparecord_t *prec = (rparecord_t *)r_array_slot(records, rec);
	rlong start, end, first, last, next, prev, parent;
	rchar buf[240];
	rint bufsize = sizeof(buf) - 1;
	rint n = 0, size;
	rchar optc = ' ';

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

//	n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "%5d  ( %7ld, %4d ) : ", rec, prec->ruleid, (ruint32)prec->userid);
	n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "%5ld (s: %3ld, e: %3ld, pp: %3ld, f: %3ld, l: %3ld, n: %3ld, p: %3ld) ( %4d, 0x%03x ) : ", rec, start, end, parent, first, last, next, prev, prec->userid, prec->usertype);
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
	n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, " : %c %c %c", optc, (prec->usertype & RPA_LOOP_PATH) ? 'L' : ' ', (prec->usertype & RPA_NONLOOP_PATH) ? 'N' : ' ');
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
