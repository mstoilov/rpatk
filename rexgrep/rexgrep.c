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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "rlib/rutf.h"
#include "rlib/rmem.h"
#include "rex/rextransition.h"
#include "rex/rexdfasimulator.h"
#include "rexgrep.h"
#include "rexgrepdep.h"

#define MAX_STACK 256000


rexgrep_t *rex_grep_create()
{
	rexgrep_t *pGrep;
	
	pGrep = (rexgrep_t *)r_malloc(sizeof(*pGrep));
	if (!pGrep)
		return (void *)0;
	r_memset(pGrep, 0, sizeof(*pGrep));
	pGrep->nfa = rex_db_create(REXDB_TYPE_NFA);
	pGrep->si = rex_nfasimulator_create();
	pGrep->dfasi = rex_dfasimulator_create();
	pGrep->ret = 1;
	pGrep->startuid = 0UL;
	rex_db_setblanks_s(pGrep->nfa, "");
	return pGrep;
}


void rex_grep_destroy(rexgrep_t *pGrep)
{
	if (!pGrep)
		return;
	rex_db_destroy(pGrep->nfa);
	rex_dfa_destroy(pGrep->dfa);
	rex_nfasimulator_destroy(pGrep->si);
	rex_dfasimulator_destroy(pGrep->dfasi);
	r_free(pGrep);
}


int rex_grep_load_string_pattern(rexgrep_t *pGrep, rbuffer_t *buf)
{
	return rex_grep_load_pattern(pGrep, buf);
}


int rex_grep_load_pattern(rexgrep_t *pGrep, rbuffer_t *buf)
{
	pGrep->startuid = rex_db_addexpression(pGrep->nfa, pGrep->startuid, buf->s, buf->size, 0);
	if (pGrep->startuid < 0) {
		return -1;
	}
	return 0;
}


int rex_grep_nfamatch(rexgrep_t *pGrep, const char* input, const char *end)
{
	int inc;
	ruint32 wc;
	rexdb_t *db = pGrep->nfa;

	if (pGrep->startuid < 0) {
		return -1;
	}
	rex_nfasimulator_start(pGrep->si, db, pGrep->startuid);
	while ((inc = r_utf8_mbtowc(&wc, (const unsigned char*)input, (const unsigned char*)end)) > 0) {
		if (rex_nfasimulator_next(pGrep->si, db, wc, inc) == 0)
			break;
		input += inc;
	}
	if (r_array_length(pGrep->si->accepts) > 0) {
		rex_accept_t *acc = (rex_accept_t *)r_array_lastslot(pGrep->si->accepts);
		return acc->inputsize;
	}
	return 0;
}


static int rex_grep_dfamatch(rexgrep_t *pGrep, const char* input, const char *end)
{
	int inc;
	ruint32 wc = 0;
	int ret = 0;
	long nstate = REX_DFA_STARTSTATE;
	const char *start = input;
	rexdfa_t *dfa = pGrep->dfa;
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


#define REX_GREP_SHIFT(__shift__, __count__, __bytes__, __bitsperbyte__, __shiftstart__, __end__) \
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


static int rex_grep_dfascan(rexgrep_t *pGrep, const char* start, const char* end, int alloutput)
{
	int ret = 0;
	unsigned int shifter = 0;
	const char *nextshift = start;
	const char *input = start;
	rexdfa_t *dfa = pGrep->dfa;

	nextshift = start;
	REX_GREP_SHIFT(shifter, REX_HASH_BYTES, REX_HASH_BYTES, REX_HASH_BITS, nextshift, end);

	while (input < end) {
		while ((ret = REX_BITARRAY_GET(dfa->bits, shifter)) == 0 && nextshift < end && ((unsigned char)*nextshift) < 0x80 && ((unsigned char)*input) < 0x80) {
			shifter <<= REX_HASH_BITS;
			shifter |= (((unsigned char)*nextshift) & ((1 << REX_HASH_BITS) - 1));
			shifter = (shifter & REX_DFA_HASHMASK(REX_HASH_BYTES, REX_HASH_BITS));
			nextshift += 1;
			input += 1;
		}
		if (ret)
			ret = rex_grep_dfamatch(pGrep, input, end);
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
				shifter <<= REX_HASH_BITS;
				shifter |= (((unsigned char)*nextshift) & ((1 << REX_HASH_BITS) - 1));
				shifter = (shifter & REX_DFA_HASHMASK(REX_HASH_BYTES, REX_HASH_BITS));
				nextshift += 1;
			} else {
				REX_GREP_SHIFT(shifter, 1, REX_HASH_BYTES, REX_HASH_BITS, nextshift, end);
			}
		} else if (ret > 0) {
			if (pGrep->showfilename) {
				fprintf(stdout, "%s:", (const char*)pGrep->filename);
			}
			if (alloutput) {
				fwrite(start, 1, end - start, stdout);
				break;
			} else {
				fwrite(input, 1, ret, stdout);
				fprintf(stdout, "\n");
			}
			input += ret;
			shifter = 0;
			nextshift = input;
			REX_GREP_SHIFT(shifter, REX_HASH_BYTES, REX_HASH_BYTES, REX_HASH_BITS, nextshift, end);
		} else {
			/*
			 * Error
			 */
			return -1;
		}
	}
	return 0;
}


static int rex_grep_nfascan(rexgrep_t *pGrep, const char* start, const char* end, int alloutput)
{
	int ret = 0;
	const char *input = start;

	while (input < end) {
		ret = rex_grep_nfamatch(pGrep, input, end);
		if (ret < 0) {
			/*
			 * Error
			 */
			return -1;
		} else if (ret > 0) {
			if (pGrep->showfilename) {
				fprintf(stdout, "%s:", (const char*)pGrep->filename);
			}
			if (alloutput) {
				fwrite(start, 1, end - start, stdout);
				break;
			} else {
				fwrite(input, 1, ret, stdout);
				fprintf(stdout, "\n");
			}
			input += ret;
		} else {
			ruint32 wc;
			if ((ret = r_utf8_mbtowc(&wc, (const unsigned char*)input, (const unsigned char*)end)) <= 0)
				ret = 1;
			input += ret;
		}
	}
	return 0;
}


static int rex_grep_dfascanlines(rexgrep_t *pGrep, const char* start, const char* end)
{
	int ret;
	const char *eol;

	for (eol = start; eol < end; eol++) {
		if (*eol == '\n' || (eol + 1) == end) {
			ret = rex_grep_dfascan(pGrep, start, eol + 1, 1);
			if (ret < 0) {
				/*
				 * Error
				 */
			}
			start = eol + 1;
		}
	}
	return 0;
}


static int rex_grep_nfascanlines(rexgrep_t *pGrep, const char* start, const char* end)
{
	int ret;
	const char *eol;

	for (eol = start; eol < end; eol++) {
		if (*eol == '\n' || (eol + 1) == end) {
			ret = rex_grep_nfascan(pGrep, start, eol + 1, 1);
			if (ret < 0) {
				/*
				 * Error
				 */
			}
			start = eol + 1;
		}
	}
	return 0;
}


void rex_grep_scan_buffer(rexgrep_t *pGrep, rbuffer_t *buf)
{
	switch (pGrep->greptype) {
	case REX_GREPTYPE_SCANLINES:
		if (pGrep->usedfa) {
			rex_grep_dfascanlines(pGrep, buf->s, buf->s + buf->size);
		} else {
			rex_grep_nfascanlines(pGrep, buf->s, buf->s + buf->size);
		}
		break;
	case REX_GREPTYPE_MATCH:
	case REX_GREPTYPE_SCAN:
	default:
		if (pGrep->usedfa) {
			rex_grep_dfascan(pGrep, buf->s, buf->s + buf->size, 0);
		}else {
			rex_grep_nfascan(pGrep, buf->s, buf->s + buf->size, 0);
		}
		break;
	};
}


void rex_grep_output(rexgrep_t *pGrep, const char *s, unsigned long size, unsigned int encoding)
{
	fwrite(s, 1, size, stdout);
}


void rex_grep_output_utf8_string(rexgrep_t *pGrep, const char *s)
{
	rex_grep_output(pGrep, s, r_strlen(s), 0);
}

