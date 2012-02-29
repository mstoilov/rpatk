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


#ifndef _RPAGREP_H_
#define _RPAGREP_H_

#include <stdio.h>
#include "rlib/rbuffer.h"
#include "rex/rexdb.h"
#include "rex/rexdfa.h"
#include "rex/rexfragment.h"
#include "rex/rexnfasimulator.h"
#include "rex/rexdfasimulator.h"

#ifdef __cplusplus
extern "C" {
#endif

#define REX_GREP_FORCE_UTF16 1
#define REX_GREP_FORCE_BYTE 2

#define REX_GREPTYPE_SCANLINES 0
#define REX_GREPTYPE_SCAN 1
#define REX_GREPTYPE_MATCH 2
#define REX_GREPTYPE_PARSE 3

#define REX_HASH_BYTES 6
#define REX_HASH_BITS 3

typedef struct rexgrep_s {
	rexdb_t *nfa;
	rexdfa_t *dfa;
	rex_nfasimulator_t *si;
	rex_dfasimulator_t *dfasi;
	long startuid;
	unsigned long scsize;
	unsigned long scanmilisec;
	unsigned int icase;
	unsigned int encoding;
	unsigned int greptype;
	unsigned int showtime;
	unsigned int showfilename;
	unsigned int usedfa;
	unsigned int withsubstates;
	unsigned int forceEncoding;
	int ret;
	void *filename;
} rexgrep_t;


rexgrep_t *rex_grep_create();
void rex_grep_destroy(rexgrep_t *pGrep);
void rex_grep_close(rexgrep_t *pGrep);
int rex_grep_load_pattern(rexgrep_t *pGrep, rbuffer_t *buf);
int rex_grep_load_string_pattern(rexgrep_t *pGrep, rbuffer_t *buf);
int rex_grep_match(rexgrep_t *pGrep, const char* start, const char* end);
int rex_grep_scan(rexgrep_t *pGrep, const char* start, const char* end);
int rex_grep_scan_lines(rexgrep_t *pGrep, const char* start, const char* end);
void rex_grep_scan_buffer(rexgrep_t *pGrep, rbuffer_t *buf);
void rex_grep_print_filename(rexgrep_t *pGrep);
void rex_grep_output_char(int c);
void rex_grep_output(rexgrep_t *pGrep, const char *s, unsigned long size, unsigned int encoding);
void rex_grep_output_utf8_string(rexgrep_t *pGrep, const char *s);
void rex_grep_output_utf16_string(rexgrep_t *pGrep, const unsigned short *s);


#ifdef __cplusplus
}
#endif

#endif
