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


#include "rlib/rmem.h"
#include "rlib/rstring.h"
#include "rlib/rutf.h"
#include "rlib/robject.h"
#include "rex/rexdb.h"
#include "rex/rexregex.h"
#include "rex/rexdfa.h"

#define REX_REGEX_HASHBYTES 3
#define REX_REGEX_HASHBITS 5

struct rexregex_s {
	robject_t obj;
	rexdfa_t *dfa;
};

void rex_regex_cleanup(robject_t *obj)
{
	rexregex_t *regex = (rexregex_t *)obj;
	if (regex)
		rex_dfa_destroy(regex->dfa);
	r_object_cleanup((robject_t*)regex);
}


robject_t *rex_regex_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy, const char *str, unsigned int size)
{
	long start;
	rexdb_t *nfadb = NULL, *dfadb = NULL;
	rexregex_t *regex = (rexregex_t *)obj;

	r_object_init(obj, type, cleanup, copy);
	regex->dfa = NULL;
	nfadb = rex_db_create(REXDB_TYPE_NFA);
	start = rex_db_addexpression(nfadb, -1, str, size, 0);
	if (start < 0)
		goto error;
	dfadb = rex_db_createdfa(nfadb, start);
	if (!dfadb)
		goto error;

	regex->dfa = rex_db_todfa(dfadb, 0);
	if (!regex->dfa)
		goto error;
	rex_dfa_hash(regex->dfa, REX_REGEX_HASHBYTES, REX_REGEX_HASHBITS);
	rex_db_destroy(dfadb);
	rex_db_destroy(nfadb);
	return obj;

error:
	rex_db_destroy(dfadb);
	rex_db_destroy(nfadb);
	rex_regex_destroy(regex);
	return NULL;
}


rexregex_t *rex_regex_create(const char *str, unsigned int size)
{
	rexregex_t *regex;

	regex = (rexregex_t*)r_object_create(sizeof(*regex));
	rex_regex_init((robject_t*)regex, R_OBJECT_REXREGEX, rex_regex_cleanup, NULL, str, size);
	return regex;
}


void rex_regex_destroy(rexregex_t *regex)
{
	r_object_destroy((robject_t*)regex);
}


rexregex_t *rex_regex_create_s(const char *str)
{
	return rex_regex_create(str, r_strlen(str));
}


#define REX_GREP_SHIFT_UTF8(__shift__, __count__, __bytes__, __bitsperbyte__, __shiftstart__, __end__) \
do { \
	int inc, i; \
	unsigned int mask = (1 << __bitsperbyte__) - 1; \
	ruint32 wc = 0; \
	for (i = 0; i < __count__; i++) { \
		wc = 0; \
		if (__shiftstart__ < __end__) { \
			wc = *(__shiftstart__); \
			inc = 1; \
			if (wc >= 0x80) { \
				inc = r_utf8_mbtowc(&wc, (const unsigned char*)(__shiftstart__), (const unsigned char*)__end__); \
			} \
			__shiftstart__ += inc; \
		} \
		__shift__ <<= __bitsperbyte__; \
		__shift__ |= (wc & mask); \
	} \
	__shift__ = (__shift__ & REX_DFA_HASHMASK(__bytes__, __bitsperbyte__)); \
} while (0)


#define REX_GREP_SHIFT_BYTE(__shift__, __count__, __bytes__, __bitsperbyte__, __shiftstart__, __end__) \
do { \
	int inc, i; \
	unsigned int mask = (1 << __bitsperbyte__) - 1; \
	ruint32 wc = 0; \
	for (i = 0; i < __count__; i++) { \
		wc = 0; \
		if (__shiftstart__ < __end__) { \
			wc = *(__shiftstart__); \
			inc = 1; \
			__shiftstart__ += inc; \
		} \
		__shift__ <<= __bitsperbyte__; \
		__shift__ |= (wc & mask); \
	} \
	__shift__ = (__shift__ & REX_DFA_HASHMASK(__bytes__, __bitsperbyte__)); \
} while (0)


static int rex_regex_match_utf8(rexregex_t *regex, const char *start, const char *end)
{
	int inc;
	ruint32 wc = 0;
	int ret = 0;
	long nstate = REX_DFA_STARTSTATE;
	const char *input = start;
	rexdfa_t *dfa = regex->dfa;
	rexdfs_t *s;

	while ((inc = r_utf8_mbtowc(&wc, (const unsigned char*)input, (const unsigned char*)end)) > 0) {
		REX_DFA_NEXT(dfa, nstate, wc, &nstate);
		if (nstate == 0)
			break;
		input += inc;
		s = REX_DFA_STATE(dfa, nstate);
		if (s->type == REX_STATETYPE_ACCEPT)
			ret = (int)(input - start);
	}
	return ret;
}


static int rex_regex_match_byte(rexregex_t *regex, const char *start, const char *end)
{
	ruint32 wc = 0;
	int ret = 0;
	long nstate = REX_DFA_STARTSTATE;
	const char *input = start;
	rexdfa_t *dfa = regex->dfa;
	rexdfs_t *s;

	while (input < end) {
		wc = *input;
		REX_DFA_NEXT(dfa, nstate, wc, &nstate);
		if (nstate == 0)
			break;
		input += 1;
		s = REX_DFA_STATE(dfa, nstate);
		if (s->type == REX_STATETYPE_ACCEPT)
			ret = (int)(input - start);
	}
	return ret;
}


int rex_regex_scan_byte(rexregex_t *regex, const char *start, const char *end, const char **where)
{
	int ret = 0;
	unsigned int shifter = 0;
	const char *nextshift = start;
	const char *input = start;
	rexdfa_t *dfa = regex->dfa;

	nextshift = start;
	REX_GREP_SHIFT_BYTE(shifter, REX_REGEX_HASHBYTES, REX_REGEX_HASHBYTES, REX_REGEX_HASHBITS, nextshift, end);

	while (input < end) {
		while ((ret = REX_BITARRAY_GET(dfa->bits, shifter)) == 0 && nextshift < end) {
			shifter <<= REX_REGEX_HASHBITS;
			shifter |= (((unsigned char)*nextshift) & ((1 << REX_REGEX_HASHBITS) - 1));
			shifter = (shifter & REX_DFA_HASHMASK(REX_REGEX_HASHBYTES, REX_REGEX_HASHBITS));
			nextshift += 1;
			input += 1;
		}
		if (ret)
			ret = rex_regex_match_byte(regex, input, end);
		if (ret == 0) {
			input += 1;
			if (nextshift < end) {
				shifter <<= REX_REGEX_HASHBITS;
				shifter |= (((unsigned char)*nextshift) & ((1 << REX_REGEX_HASHBITS) - 1));
				shifter = (shifter & REX_DFA_HASHMASK(REX_REGEX_HASHBYTES, REX_REGEX_HASHBITS));
				nextshift += 1;
			} else {
				REX_GREP_SHIFT_BYTE(shifter, 1, REX_REGEX_HASHBYTES, REX_REGEX_HASHBITS, nextshift, end);
			}
		} else if (ret > 0) {
			if (where)
				*where = input;
			return ret;
		} else {
			/*
			 * Error
			 */
			return -1;
		}
	}
	return 0;
}


int rex_regex_scan_utf8(rexregex_t *regex, const char *start, const char *end, const char **where)
{
	int ret = 0;
	unsigned int shifter = 0;
	const char *nextshift = start;
	const char *input = start;
	rexdfa_t *dfa = regex->dfa;

	nextshift = start;
	REX_GREP_SHIFT_UTF8(shifter, REX_REGEX_HASHBYTES, REX_REGEX_HASHBYTES, REX_REGEX_HASHBITS, nextshift, end);

	while (input < end) {
		while ((ret = REX_BITARRAY_GET(dfa->bits, shifter)) == 0 && nextshift < end && ((unsigned char)*nextshift) < 0x80 && ((unsigned char)*input) < 0x80) {
			shifter <<= REX_REGEX_HASHBITS;
			shifter |= (((unsigned char)*nextshift) & ((1 << REX_REGEX_HASHBITS) - 1));
			shifter = (shifter & REX_DFA_HASHMASK(REX_REGEX_HASHBYTES, REX_REGEX_HASHBITS));
			nextshift += 1;
			input += 1;
		}
		if (ret)
			ret = rex_regex_match_utf8(regex, input, end);
		if (ret == 0) {
			ruint32 wc = *input;
			if (wc >= 0x80) {
				ret = r_utf8_mbtowc(&wc, (const unsigned char*)input, (const unsigned char*)end);
				if (ret <= 0)
					ret = 1;
				input += ret;
			} else {
				input += 1;
			}
			if (nextshift < end && ((unsigned char)*nextshift) < 0x80) {
				shifter <<= REX_REGEX_HASHBITS;
				shifter |= (((unsigned char)*nextshift) & ((1 << REX_REGEX_HASHBITS) - 1));
				shifter = (shifter & REX_DFA_HASHMASK(REX_REGEX_HASHBYTES, REX_REGEX_HASHBITS));
				nextshift += 1;
			} else {
				REX_GREP_SHIFT_UTF8(shifter, 1, REX_REGEX_HASHBYTES, REX_REGEX_HASHBITS, nextshift, end);
			}
		} else if (ret > 0) {
			if (where)
				*where = input;
			return ret;
		} else {
			/*
			 * Error
			 */
			return -1;
		}
	}
	return 0;
}


int rex_regex_match(rexregex_t *regex, unsigned int encoding, const char *start, const char *end)
{
	switch (encoding) {
	case REX_ENCODING_BYTE:
		return rex_regex_match_byte(regex, start, end);
	case REX_ENCODING_UTF8:
	default:
		return rex_regex_match_utf8(regex, start, end);
	}
	return -1;
}


int rex_regex_scan(rexregex_t *regex, unsigned int encoding, const char *start, const char *end, const char **where)
{
	switch (encoding) {
	case REX_ENCODING_BYTE:
		return rex_regex_scan_byte(regex, start, end, where);
	case REX_ENCODING_UTF8:
	default:
		return rex_regex_scan_utf8(regex, start, end, where);
	}
	return -1;
}
