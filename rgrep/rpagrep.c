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

#include "rpagrep.h"
#include "rpagreputf.h"
#include "rpagrepdep.h"

#define MAX_STACK 256000

rpa_buffer_t * rpa_buffer_init(rpa_buffer_t *str, char *s, unsigned int size)
{
	str->s = s;
	str->size = size;
	return str;
}


void rpa_buffer_free(rpa_buffer_t *str)
{
	if (str) {
		free(str->s);
		free(str);
	}	
}


rpa_buffer_t * rpa_buffer_alloc(unsigned int size)
{
	rpa_buffer_t * str;

	str = (rpa_buffer_t *)malloc(sizeof(rpa_buffer_t));
	if (!str)
		return (void*)0;
	memset(str, 0, sizeof(*str));
	if (!(str->s = (char *)malloc((size + 1) * sizeof(char)))) {
		free(str);
		return (void*)0;
	}
	memset(str->s, 0, size + 1);
	str->size = size;
	str->destroy = rpa_buffer_free;
	return str;
}


int rpa_buffer_realloc(rpa_buffer_t *str, unsigned int size)
{
	char *s;

	s = (char *)realloc(str->s, size);
	if (!s)
		return -1;
	str->s = s;
	str->size = size;
	return 0;
}


void rpa_buffer_destroy(rpa_buffer_t *str)
{
	if (str && str->destroy)
		str->destroy(str);	
}


rpa_grep_t *rpa_grep_create()
{
	rpa_grep_t *pGrep;
	
	pGrep = (rpa_grep_t *)malloc(sizeof(*pGrep));
	if (!pGrep)
		return (void *)0;
	memset(pGrep, 0, sizeof(*pGrep));
	pGrep->hDbex = rpa_dbex_create();
	return pGrep;
}

void rpa_grep_close(rpa_grep_t *pGrep)
{
	if (pGrep->hDbex)
		rpa_dbex_destroy(pGrep->hDbex);
	pGrep->hDbex = 0;	
}


void rpa_grep_destroy(rpa_grep_t *pGrep)
{
	if (!pGrep)
		return;
	rpa_grep_close(pGrep);
	free(pGrep);
}


int rpa_grep_load_string_pattern(rpa_grep_t *pGrep, rpa_buffer_t *buf)
{
	rpa_dbex_open(pGrep->hDbex);
	if (rpa_dbex_load_s(pGrep->hDbex, buf->s) < 0) {
		fprintf(stdout, "ERROR: %s\n", (rpa_dbex_get_error(pGrep->hDbex) == RPA_E_SYNTAX_ERROR) ? "Syntax Error." : "Pattern Loading failed.");
		rpa_dbex_close(pGrep->hDbex);
		return -1;
	}
	rpa_dbex_close(pGrep->hDbex);
	pGrep->hPattern = rpa_dbex_default(pGrep->hDbex);
	return 0;
}


int rpa_grep_load_pattern(rpa_grep_t *pGrep, rpa_buffer_t *buf)
{
	int ret, line;
	int inputsize = buf->size;
	const char *pattern = buf->s;

	if (rpa_dbex_open(pGrep->hDbex) < 0) {
		fprintf(stdout, "Failed to open expression database.\n");
		goto error;
	}

	while ((ret = rpa_dbex_load(pGrep->hDbex, pattern, inputsize)) > 0) {
		inputsize -= ret;
		pattern += ret;
	}
	if (ret < 0) {
		for (line = 1; pattern >= buf->s; --pattern) {
			if (*pattern == '\n')
				line += 1;
		}
		fprintf(stdout, "Line: %d, ERROR: %s\n", line, (rpa_dbex_get_error(pGrep->hDbex) == RPA_E_SYNTAX_ERROR) ? "Syntax Error." : "Pattern Loading failed.");
		goto error;
	}	
	
	rpa_dbex_close(pGrep->hDbex);
	pGrep->hPattern = rpa_dbex_default(pGrep->hDbex);
	return 0;
	
error:
	rpa_dbex_close(pGrep->hDbex);
	return -1;
}


void rpa_grep_list_patterns(rpa_grep_t *pGrep)
{
	rpa_dbex_dumprules(pGrep->hDbex);
}


void rpa_grep_dump_pattern_records(rpa_grep_t *pGrep)
{
	rpa_dbex_dumprecords(pGrep->hDbex);
}


void rpa_grep_dump_pattern_info(rpa_grep_t *pGrep)
{
	rpa_dbex_compile(pGrep->hDbex);
	rpa_dbex_dumpinfo(pGrep->hDbex);
}


int rpa_grep_match(rpa_grep_t *pGrep, const char* buffer, unsigned long size)
{
	int ret = 0;
	rpastat_t *hStat;
	const char *input = buffer, *start = buffer, *end = buffer + size;

	hStat = rpa_stat_create(pGrep->hDbex, 0);
	if (!hStat)
		return -1;
	rpa_stat_encodingset(hStat, pGrep->encoding);
	hStat->debug = pGrep->execdebug;
	ret = rpa_stat_match(hStat, pGrep->hPattern, input, start, end);
	if (ret > 0) {
		rpa_grep_print_filename(pGrep);
		rpa_grep_output(pGrep, input, ret, pGrep->encoding);
		rpa_grep_output_utf8_string(pGrep, "\n");
	}
	pGrep->cachehit = hStat->cache->hit;
	rpa_stat_destroy(hStat);
	return 0;
}


int rpa_grep_parse(rpa_grep_t *pGrep, const char* buffer, unsigned long size)
{
	rlong i;
	rpastat_t *hStat;
	rarray_t *records;
	rparecord_t *prec;
	const char *input = buffer, *start = buffer, *end = buffer + size;

	hStat = rpa_stat_create(pGrep->hDbex, 0);
	if (!hStat)
		return -1;
	rpa_stat_encodingset(hStat, pGrep->encoding);
	hStat->debug = pGrep->execdebug;
	records = rpa_stat_parse(hStat, pGrep->hPattern, input, start, end);
	if (records) {
		for (i = 0; i < r_array_length(records); i++) {
			prec = (rparecord_t *)r_array_slot(records, i);
			if (prec->type & RPA_RECORD_END) {
				rpa_grep_output_utf8_string(pGrep, prec->rule);
				rpa_grep_output_utf8_string(pGrep, ": ");
				rpa_grep_output(pGrep, prec->input, prec->inputsiz, pGrep->encoding);
				rpa_grep_output_utf8_string(pGrep, "\n");
			}
		}
		r_array_destroy(records);
	}
	pGrep->cachehit = hStat->cache->hit;
	rpa_stat_destroy(hStat);
	return 0;
}


int rpa_grep_scan(rpa_grep_t *pGrep, const char* buffer, unsigned long size)
{
	int ret = 0;
	rpastat_t *hStat;
	int displayed = 0;	
	const char *matched;
	const char *input = buffer, *start = buffer, *end = buffer + size;

	hStat = rpa_stat_create(pGrep->hDbex, 0);
	if (!hStat)
		return -1;
	rpa_stat_encodingset(hStat, pGrep->encoding);
	hStat->debug = pGrep->execdebug;
	pGrep->cachehit = hStat->cache->hit;

again:
	ret = rpa_stat_scan(hStat, pGrep->hPattern, input, start, end, &matched);
	pGrep->cachehit += hStat->cache->hit;
	if (ret > 0) {
		if (!displayed) {
			displayed = 1;
			rpa_grep_print_filename(pGrep);
		}
		rpa_grep_output(pGrep, matched, ret, pGrep->encoding);
		rpa_grep_output_utf8_string(pGrep, "\n");
	}
	if (ret && matched + ret < end) {
		input = matched + ret;
		goto again;
	}

	rpa_stat_destroy(hStat);
	return 0;
}


int rpa_grep_scan_lines(rpa_grep_t *pGrep, const char* buffer, unsigned long size)
{
	int ret = 0;
	rpastat_t *hStat;
	const char *matched;
	int displayed = 0;
	unsigned long lines = 0;
	const char *end = buffer + size, *lstart = buffer, *lend;

	hStat = rpa_stat_create(pGrep->hDbex, 0);
	if (!hStat)
		return -1;
	rpa_stat_encodingset(hStat, pGrep->encoding);
	hStat->debug = pGrep->execdebug;
	
again:
	if (pGrep->encoding == RPA_ENCODING_UTF16LE || pGrep->encoding == RPA_ENCODING_ICASE_UTF16LE) {
		for (lend = lstart; lend < end; lend += sizeof(unsigned short)) {
			if (*((unsigned short*)lend) == L'\n') {
				++lines;
				lend += sizeof(unsigned short);
				break;
			}
		}
	} else {
		for (lend = lstart; lend < end; lend += sizeof(unsigned char)) {
			if (*((unsigned char*)lend) == '\n') {
				++lines;
				lend += sizeof(unsigned char);
				break;
			}
		}
	}
	if (!lines)
		return 0;
	ret = rpa_stat_scan(hStat, pGrep->hPattern, lstart, lstart, lend, &matched);
	if (ret > 0) {
		if (!displayed) {
			displayed = 1;
			rpa_grep_print_filename(pGrep);
		}
		rpa_grep_output(pGrep, lstart, lend - lstart, pGrep->encoding);
	}
	if (lend < end) {
		lstart = lend;
		goto again;
	}
	rpa_stat_destroy(hStat);
	return 0;
}


void rpa_grep_scan_buffer(rpa_grep_t *pGrep, rpa_buffer_t *buf)
{
	const char *input;
	unsigned long size;
	clock_t btime, scanclocks;

	if (pGrep->forceEncoding == RPA_GREP_FORCE_BYTE) {
		input = buf->s;
		size = buf->size;
		pGrep->encoding =  pGrep->icase ? RPA_ENCODING_ICASE_BYTE : RPA_ENCODING_BYTE;
	} else if (pGrep->forceEncoding == RPA_GREP_FORCE_UTF16) {
		if (buf->size >= 2 && buf->s[0] == -1 && buf->s[1] == -2) {
			input = buf->s + 2;
			size = buf->size - 2;
		} else {
			input = buf->s;
			size = buf->size;
		}
		pGrep->encoding =  pGrep->icase ? RPA_ENCODING_ICASE_UTF16LE : RPA_ENCODING_UTF16LE;
	} else if (buf->size >= 2 && buf->s[0] == -1 && buf->s[1] == -2) {
		input = buf->s + 2;
		size = buf->size - 2;
		pGrep->encoding =  pGrep->icase ? RPA_ENCODING_ICASE_UTF16LE : RPA_ENCODING_UTF16LE;
	} else {
		pGrep->encoding = pGrep->icase ? RPA_ENCODING_ICASE_UTF8 : RPA_ENCODING_UTF8;
		input = buf->s;
		size = buf->size;
	}		

	btime = clock();

	switch (pGrep->greptype) {
	case RPA_GREPTYPE_SCANLINES:
		rpa_grep_scan_lines(pGrep, input, size);
		break;
	case RPA_GREPTYPE_MATCH:
		rpa_grep_match(pGrep, input, size);
		break;
	case RPA_GREPTYPE_PARSE:
		rpa_grep_parse(pGrep, input, size);
		break;
	case RPA_GREPTYPE_SCAN:
		rpa_grep_scan(pGrep, input, size);
		break;
	default:
		rpa_grep_scan(pGrep, input, size);
		break;
	};

	scanclocks = clock() - btime;
	pGrep->scanmilisec += (unsigned long)(((unsigned long long)1000)*scanclocks/CLOCKS_PER_SEC);
}


rpa_buffer_t *rpa_buffer_loadfile(FILE *pFile)
{
	unsigned int memchunk = 256;
	int ret = 0, inputsize = 0;
	rpa_buffer_t *buf;
	
	buf = rpa_buffer_alloc(2 * memchunk);
	if (!buf)
		return (void*)0;
	
	do {
		if ((buf->size - inputsize) < memchunk) {
			if (rpa_buffer_realloc(buf, buf->size + memchunk) < 0) {
				fprintf(stderr, "Out of memory!\n");
				exit(1);
			}
		}
		ret = fread(&buf->s[inputsize], 1, memchunk - 1, pFile);
		if ((ret <= 0) && ferror(pFile)) {
			rpa_buffer_destroy(buf);
			return (void*)0;
		}
		inputsize += ret;
		buf->s[inputsize] = '\0';
		buf->size = inputsize;
	} while (!feof(pFile));	
	
	return buf;
}


static int rpa_callback_output(rpastat_t * stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{

	return size;
}


static int rpa_callback_matched_output(rpastat_t * stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rpa_grep_t *pGrep = (rpa_grep_t *)userdata;

	rpa_grep_output_utf8_string(pGrep, name);
	rpa_grep_output_utf8_string(pGrep, ": ");
	rpa_grep_output(pGrep, input, size, pGrep->encoding);
	rpa_grep_output_utf8_string(pGrep, "\n");

	return size;
}


void rpa_grep_setup_callback(rpa_grep_t *pGrep, rpa_buffer_t *pattern)
{

}


void rpa_grep_setup_matched_callback(rpa_grep_t *pGrep, rpa_buffer_t *pattern)
{

}


void rpa_grep_dump_pattern_tree(rpa_grep_t *pGrep, rpa_buffer_t *pattern)
{
	rpa_dbex_dumptree(pGrep->hDbex, pattern->s);
}


void rpa_grep_output(rpa_grep_t *pGrep, const char *s, unsigned long size, unsigned int encoding)
{
	const unsigned char *input = (const unsigned char*)s;
	const unsigned char *end = input + size;
	unsigned int wc;
	int ret;
	
	if (encoding == RPA_ENCODING_UTF16LE || encoding == RPA_ENCODING_ICASE_UTF16LE) {
		while ((ret = (int)rpa_grep_utf16_mbtowc(&wc, input, end)) != 0) {
			rpa_grep_output_char(wc);
			input += ret;
		}
	} else {
		while ((ret = (int)rpa_grep_utf8_mbtowc(&wc, input, end)) != 0) {
			rpa_grep_output_char(wc);
			input += ret;
		}
	}
}


void rpa_grep_output_utf8_string(rpa_grep_t *pGrep, const char *s)
{
	rpa_grep_output(pGrep, s, strlen(s), RPA_ENCODING_UTF8);
}


void rpa_grep_output_utf16_string(rpa_grep_t *pGrep, const unsigned short *s)
{
	unsigned long size = 0;
	const unsigned short *pstr = s;

	while (*pstr) {
		size += sizeof(unsigned short);
		pstr += 1;
	}
	rpa_grep_output(pGrep, (const char*)s, size, RPA_ENCODING_UTF16LE);
}
