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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
 * Temporary here. Need to fix the encoding definitions.
 */
#include "rpa/rpastat.h"

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
	pGrep->startuid = -1L;
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


int rex_grep_dfamatch(rexgrep_t *pGrep, const char* input, const char *end)
{
	ruint32 wc = 0;
	int inc = 0, ret = 0;
	long nstate = REX_DFA_STARTSTATE;
	const char *start = input;
	rexdfa_t *dfa = pGrep->dfa;
	rexdfs_t *s;

	while ((inc = r_utf8_mbtowc(&wc, (const unsigned char*)input, (const unsigned char*)end)) > 0) {
		if ((nstate = rex_dfa_next(dfa, nstate, wc)) <= 0)
			break;
		input += inc;
		s = rex_dfa_state(dfa, nstate);
		if (s->type == REX_STATETYPE_ACCEPT)
			ret = input - start;
	}
	return ret;
}


int rex_grep_match(rexgrep_t *pGrep, const char* input, const char *end)
{
	int inc;
	ruint32 wc;
	rexdb_t *db;

	if (pGrep->usedfa)
		return rex_grep_dfamatch(pGrep, input, end);

	if (pGrep->startuid < 0) {
		return -1;
	}
	db = pGrep->nfa;

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


int rex_grep_scan(rexgrep_t *pGrep, const char* start, const char* end)
{
	int ret = 0;

	while (start < end) {
		ret = rex_grep_match(pGrep, start, end);
		if (ret < 0) {
			/*
			 * Error
			 */
			return -1;
		} else if (ret > 0) {
			if (pGrep->showfilename) {
				fprintf(stdout, "%s:", (const char*)pGrep->filename);
			}
			fwrite(start, 1, ret, stdout);
			fprintf(stdout, "\n");
			start += ret;
		} else {
			ruint32 wc;
			if ((ret = r_utf8_mbtowc(&wc, (const unsigned char*)start, (const unsigned char*)end)) <= 0)
				ret = 1;
			start += ret;
		}
	}
	return 0;
}


static int rex_grep_scan_do(rexgrep_t *pGrep, const char* start, const char* end)
{
	int ret = 0;

	while (start < end) {
		ret = rex_grep_match(pGrep, start, end);
		if (ret < 0) {
			/*
			 * Error
			 */
			return -1;
		} else if (ret > 0) {
			return ret;
		} else {
			ruint32 wc;
			if ((ret = r_utf8_mbtowc(&wc, (const unsigned char*)start, (const unsigned char*)end)) <= 0)
				ret = 1;
			start += ret;
		}
	}
	return 0;
}


int rex_grep_scan_lines(rexgrep_t *pGrep, const char* start, const char* end)
{
	int ret;
	const char *eol;

	for (eol = start; eol < end; eol++) {
		if (*eol == '\n') {
			ret = rex_grep_scan_do(pGrep, start, eol + 1);
			if (ret > 0) {
				if (pGrep->showfilename) {
					fprintf(stdout, "%s:", (const char*)pGrep->filename);
				}
				fwrite(start, 1, eol + 1 - start, stdout);
			}
			start = eol + 1;
		}
	}
	rex_grep_output_utf8_string(pGrep, "\n");
	return 0;
}


void rex_grep_scan_buffer(rexgrep_t *pGrep, rbuffer_t *buf)
{
	switch (pGrep->greptype) {
	case REX_GREPTYPE_SCANLINES:
		rex_grep_scan_lines(pGrep, buf->s, buf->s + buf->size);
		break;
	case REX_GREPTYPE_MATCH:
	case REX_GREPTYPE_SCAN:
	default:
		rex_grep_scan(pGrep, buf->s, buf->s + buf->size);
		break;
	};
}


void rex_grep_output(rexgrep_t *pGrep, const char *s, unsigned long size, unsigned int encoding)
{
	const unsigned char *input = (const unsigned char*)s;
	const unsigned char *end = input + size;
	unsigned int wc;
	int ret;

	if (encoding == RPA_ENCODING_UTF16LE || encoding == RPA_ENCODING_ICASE_UTF16LE) {
		while ((ret = (int)r_utf16_mbtowc(&wc, input, end)) != 0) {
			rex_grep_output_char(wc);
			input += ret;
		}
	} else {
		while ((ret = (int)r_utf8_mbtowc(&wc, input, end)) != 0) {
			rex_grep_output_char(wc);
			input += ret;
		}
	}
}


void rex_grep_output_utf8_string(rexgrep_t *pGrep, const char *s)
{
	rex_grep_output(pGrep, s, r_strlen(s), RPA_ENCODING_UTF8);
}


void rex_grep_output_utf16_string(rexgrep_t *pGrep, const unsigned short *s)
{
	unsigned long size = 0;
	const unsigned short *pstr = s;

	while (*pstr) {
		size += sizeof(unsigned short);
		pstr += 1;
	}
	rex_grep_output(pGrep, (const char*)s, size, RPA_ENCODING_UTF16LE);
}
