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


#ifndef _RPAGREP_H_
#define _RPAGREP_H_

#include <stdio.h>
#include "rex/rexdb.h"


#ifdef __cplusplus
extern "C" {
#endif

#define REX_GREP_FORCE_UTF16 1
#define REX_GREP_FORCE_BYTE 2

#define REX_GREPTYPE_SCANLINES 0
#define REX_GREPTYPE_SCAN 1
#define REX_GREPTYPE_MATCH 2
#define REX_GREPTYPE_PARSE 3


typedef struct rpa_grep_s {
	rexdb_t *rexdb;
	unsigned long scanmilisec;
	unsigned int icase;
	unsigned int encoding;
	unsigned int greptype;
	unsigned int showtime;
	unsigned int forceEncoding;
	int ret;
	void *filename;
} rpa_grep_t;


rpa_grep_t *rpa_grep_create();
void rpa_grep_destroy(rpa_grep_t *pGrep);
void rpa_grep_close(rpa_grep_t *pGrep);
int rpa_grep_load_pattern(rpa_grep_t *pGrep, rpa_buffer_t *buf);
int rpa_grep_load_string_pattern(rpa_grep_t *pGrep, rpa_buffer_t *buf);
int rpa_grep_match(rpa_grep_t *pGrep, const char* buffer, unsigned long size);
int rpa_grep_parse(rpa_grep_t *pGrep, const char* buffer, unsigned long size);
int rpa_grep_scan(rpa_grep_t *pGrep, const char* buffer, unsigned long size);
int rpa_grep_scan_lines(rpa_grep_t *pGrep, const char* buffer, unsigned long size);
void rpa_grep_scan_buffer(rpa_grep_t *pGrep, rpa_buffer_t *buf);
void rpa_grep_print_filename(rpa_grep_t *pGrep);
void rpa_grep_output_char(int c);
void rpa_grep_output(rpa_grep_t *pGrep, const char *s, unsigned long size, unsigned int encoding);
void rpa_grep_output_utf8_string(rpa_grep_t *pGrep, const char *s);
void rpa_grep_output_utf16_string(rpa_grep_t *pGrep, const unsigned short *s);


#ifdef __cplusplus
}
#endif

#endif
