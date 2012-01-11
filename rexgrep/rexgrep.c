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
#include "rex/rexcompiler.h"
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
	pGrep->ret = 1;
	return pGrep;
}


void rex_grep_destroy(rexgrep_t *pGrep)
{
	if (!pGrep)
		return;
	rex_db_destroy(pGrep->nfa);
	r_free(pGrep);
}


static void rex_grep_matchfound(rexgrep_t *pGrep)
{
	pGrep->ret = 0;
}


int rex_grep_load_string_pattern(rexgrep_t *pGrep, rbuffer_t *buf)
{
	return rex_grep_load_pattern(pGrep, buf);
}


int rex_grep_load_pattern(rexgrep_t *pGrep, rbuffer_t *buf)
{
	rexfragment_t *frag;
	rexcompiler_t *co = rex_compiler_create(pGrep->nfa);
	frag = rex_compiler_addexpression(co, pGrep->lastfrag, buf->s, buf->size, NULL);
	if (!frag) {
		return -1;
	}
	pGrep->lastfrag = frag;
	return 0;
}


int rex_grep_match(rexgrep_t *pGrep, const char* buffer, unsigned long size)
{
	int inc;
	ruint32 wc;
	const char *input = buffer, *end = buffer + size;

	if (!pGrep->lastfrag) {

		return -1;
	}
	rex_nfasimulator_start(pGrep->si, pGrep->nfa, pGrep->lastfrag->start);
	while ((inc = r_utf8_mbtowc(&wc, (const unsigned char*)input, (const unsigned char*)end)) > 0) {
		if (rex_nfasimulator_next(pGrep->si, pGrep->nfa, wc, inc) == 0)
			break;
		input += inc;
	}

	if (r_array_length(pGrep->si->accepts)) {
		rex_accept_t *acc = (rex_accept_t *)r_array_lastslot(pGrep->si->accepts);
		rex_grep_matchfound(pGrep);
		rex_grep_print_filename(pGrep);
		rex_grep_output(pGrep, buffer, acc->inputsize, pGrep->encoding);
		rex_grep_output_utf8_string(pGrep, "\n");
	}
	return 0;
}


int rex_grep_scan(rexgrep_t *pGrep, const char* buffer, unsigned long size)
{
	int inc;
	ruint32 wc;
	int displayed = 0;	
	const char *input = buffer, *start = buffer, *end = buffer + size;

	if (!pGrep->lastfrag) {

		return -1;
	}
again:
	input = start;
	rex_nfasimulator_start(pGrep->si, pGrep->nfa, pGrep->lastfrag->start);
	while ((inc = r_utf8_mbtowc(&wc, (const unsigned char*)input, (const unsigned char*)end)) > 0) {
		if (rex_nfasimulator_next(pGrep->si, pGrep->nfa, wc, inc) == 0)
			break;
		input += inc;
	}

	if (r_array_length(pGrep->si->accepts) > 0) {
		rex_accept_t *acc = (rex_accept_t *)r_array_lastslot(pGrep->si->accepts);
		rex_grep_matchfound(pGrep);
		if (!displayed) {
			displayed = 1;
			rex_grep_print_filename(pGrep);
		}
		rex_grep_output(pGrep, start, acc->inputsize, pGrep->encoding);
		rex_grep_output_utf8_string(pGrep, "\n");
		start += acc->inputsize;
	} else {
		inc = r_utf8_mbtowc(&wc, (const unsigned char*)input, (const unsigned char*)end);
		start += inc;
	}
	if (start < end)
		goto again;
	return 0;
}


int rex_grep_scan_lines(rexgrep_t *pGrep, const char* buffer, unsigned long size)
{
	int inc;
	ruint32 wc;
	int displayed = 0;
	const char *input = buffer, *start = buffer, *end = buffer + size;

	if (!pGrep->lastfrag) {

		return -1;
	}
again:
	input = start;
	rex_nfasimulator_start(pGrep->si, pGrep->nfa, pGrep->lastfrag->start);
	while ((inc = r_utf8_mbtowc(&wc, (const unsigned char*)input, (const unsigned char*)end)) > 0 && wc != '\n') {
		rex_nfasimulator_next(pGrep->si, pGrep->nfa, wc, inc);
		input += inc;
	}

	if (r_array_length(pGrep->si->accepts) > 0) {
		rex_accept_t *acc = (rex_accept_t *)r_array_lastslot(pGrep->si->accepts);
		rex_grep_matchfound(pGrep);
		if (!displayed) {
			displayed = 1;
			rex_grep_print_filename(pGrep);
		}
		rex_grep_output(pGrep, start, input - start, pGrep->encoding);
		rex_grep_output_utf8_string(pGrep, "\n");
		start += acc->inputsize;
	} else {
		inc = r_utf8_mbtowc(&wc, (const unsigned char*)start, (const unsigned char*)end);
		start += inc;
	}
	if (start < end)
		goto again;
	return 0;
}


void rex_grep_scan_buffer(rexgrep_t *pGrep, rbuffer_t *buf)
{
	switch (pGrep->greptype) {
	case REX_GREPTYPE_SCANLINES:
		rex_grep_scan_lines(pGrep, buf->s, buf->size);
		break;
	case REX_GREPTYPE_MATCH:
		rex_grep_match(pGrep, buf->s, buf->size);
		break;
	case REX_GREPTYPE_SCAN:
	default:
		rex_grep_scan(pGrep, buf->s, buf->size);
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
