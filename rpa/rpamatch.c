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

#include "rpamatch.h"
#include "rpamatchlist.h"
#include "rpastring.h"
#include "rpamem.h"
#include "rpautf.h"
#include "rpastat.h"


static unsigned int rpa_match_getid(rpa_class_t *cls)
{
	return (RPA_MATCH_CLASSID);
}


static const char *rpa_match_getstr(rpa_class_t *cls)
{
	return "match";
}


static int rpa_match_dump(rpa_class_t *cls, char *buffer, unsigned int size)
{
	return 0;
}


static void rpa_match_destroy(rpa_class_t *cls)
{
	rpa_match_t *match = (rpa_match_t*)cls;
	rpa_match_cleanup(match);
	rpa_free(match);
}


static rpa_class_methods_t rpa_match_methods = {
	rpa_match_getid,
	rpa_match_getstr,
	rpa_match_dump,
	rpa_match_destroy,
};


void rpa_match_setup_name(
	rpa_match_t *match, 
	const char *name,
	unsigned int namesiz)
{
	if (match->name) {
		rpa_free(match->name);
	}
	if (!name) {
		namesiz = 0;
		match->name = (void*)0;
		return;		
	}
	match->name = rpa_malloc(namesiz + 1);
	match->namesiz = namesiz;
	rpa_memset(match->name, 0, namesiz + 1);
	if (name)
		rpa_strncpy(match->name, name, namesiz);		
}


rpa_match_t *rpa_match_init(
	rpa_match_t *match, 
	const char *name,
	unsigned int namesiz,
	rpa_matchfunc_t match_function_id,
	rpa_class_methods_t *vptr)
{
	rpa_memset(match, 0, sizeof(*match));
	rpa_class_init((rpa_class_t*)match, vptr);
	match->match_function_id = match_function_id;
	rpa_match_setup_name(match, name, namesiz);
	return match;
}


void rpa_match_cleanup(rpa_match_t *match)
{
	rpa_free(match->name);
}


rpa_match_t * rpa_match_create_namesize(
	const char *name,
	unsigned int namesiz,
	rpa_matchfunc_t match_function_id)
{
	rpa_match_t *newMatch;
	
	newMatch = (rpa_match_t *)rpa_malloc(sizeof(*newMatch));
	if (!newMatch)
		return ((void*)0);
	rpa_memset(newMatch, 0, sizeof(*newMatch));
	return rpa_match_init(newMatch, name, namesiz, match_function_id, &rpa_match_methods);
}


rpa_match_t *rpa_match_create(
	const char *name,
	rpa_matchfunc_t match_function_id)
{
	return rpa_match_create_namesize(name, rpa_strlen(name), match_function_id);
}

void rpa_match_set_mathfunc(rpa_match_t *match, rpa_matchfunc_t match_function_id)
{
	match->match_function_id = match_function_id;
}


int rpa_match_newline(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	int ret = 0;
	unsigned int wc;	

	if (input == stat->start || *(input - 1) == '\n') {
		ret = rpa_stat_getchar(&wc, stat, input);
		if (ret < 0)
			ret = 1;
	}
	return ret;
}


int rpa_match_lstchr(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	int ret = 0;
	unsigned int wc;

	ret = rpa_stat_getchar(&wc, stat, input);
	if (ret < 0)
		return 0;
	if (input + ret == stat->end) {
		return ret;
	}
	return 0;
}


int rpa_match_abort(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	rpa_stat_abort(stat);
	return 0;
}


int rpa_match_fail(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	stat->fail = 1;
	rpa_stat_abort(stat);
	return 0;
}


int rpa_match_chreqany(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	unsigned int c;
	int ret;

	ret = rpa_stat_getchar(&c, stat, input);
	if (ret < 0)
		return 0;
	return ret;
}


int rpa_match_scan_utf8(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	int ret = 0, inc = 0;
	unsigned int wc;
	const char *end = stat->end;
	const char *start = stat->start;
	const char *lastprogress = 0;

MATCHAGAIN:
	if (input >= stat->end) {
		if (stat->fail && input < end) {
			stat->end = end;
			stat->fail = 0;
		} else {
			return 0;
		}
	}
	if (stat->progress && (input - lastprogress) > RPA_MIN_PROGRESS) {
		lastprogress = input;
		stat->progress(stat->progress_userdata, input, start, end);
	}
	inc = rpa_utf8_mbtowc(&wc, (const unsigned char*)input, (const unsigned char*)end);
	if (inc <= 0) 		
		inc = 1;
	ret = stat->mtable[match->match_function_id](match, stat, input);
	if (!ret) {
		input += inc;
		goto MATCHAGAIN;
	}
	if (ret)
		stat->where = input;
	return ret;
}


int rpa_match_scan_utf8_icase(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	int ret = 0, inc = 0;
	unsigned int wc;
	const char *end = stat->end;
	const char *start = stat->start;
	const char *lastprogress = 0;

MATCHAGAIN:
	if (input >= stat->end) {
		if (stat->fail && input < end) {
			stat->end = end;
			stat->fail = 0;
		} else {
			return 0;
		}
	}
	if (stat->progress && (input - lastprogress) > RPA_MIN_PROGRESS) {
		lastprogress = input;
		stat->progress(stat->progress_userdata, input, start, end);
	}
	inc = rpa_utf8_mbtowc(&wc, (const unsigned char*)input, (const unsigned char*)end);
	if (inc <= 0) 		
		inc = 1;
	ret = stat->mtable[match->match_function_id](match, stat, input);
	if (!ret) {
		input += inc;
		goto MATCHAGAIN;
	}

	if (ret)
		stat->where = input;
	return ret;
}


int rpa_match_scan_utf16(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	int ret = 0;
	const char *end = stat->end;
	const char *start = stat->start;
	const char *lastprogress = 0;

MATCHAGAIN:
	if (input >= stat->end) {
		if (stat->fail && input < end) {
			stat->end = end;
			stat->fail = 0;
		} else {
			return 0;
		}
	}
	if (stat->progress && (input - lastprogress) > RPA_MIN_PROGRESS) {
		lastprogress = input;
		stat->progress(stat->progress_userdata, input, start, end);
	}
	ret = stat->mtable[match->match_function_id](match, stat, input);
	if (!ret) {
		input += 2;
		goto MATCHAGAIN;
	}
	if (ret)
		stat->where = input;
	return ret;
}


int rpa_match_scan_utf16_icase(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	int ret = 0;
	const char *end = stat->end;
	const char *start = stat->start;
	const char *lastprogress = 0;

MATCHAGAIN:
	if (input >= stat->end) {
		if (stat->fail && input < end) {
			stat->end = end;
			stat->fail = 0;
		} else {
			return 0;
		}
	}
	if (stat->progress && (input - lastprogress) > RPA_MIN_PROGRESS) {
		lastprogress = input;
		stat->progress(stat->progress_userdata, input, start, end);
	}
	ret = stat->mtable[match->match_function_id](match, stat, input);
	if (!ret) {
		input += 2;
		goto MATCHAGAIN;
	}
	if (ret)
		stat->where = input;
	return ret;
}


int rpa_match_scan_byte(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	int ret = 0;
	const char *end = stat->end;
	const char *start = stat->start;
	const char *lastprogress = 0;

MATCHAGAIN:
	if (input >= stat->end) {
		if (stat->fail && input < end) {
			stat->end = end;
			stat->fail = 0;
		} else {
			return 0;
		}
	}
	if (stat->progress && (input - lastprogress) > RPA_MIN_PROGRESS) {
		lastprogress = input;
		stat->progress(stat->progress_userdata, input, start, end);
	}
	ret = stat->mtable[match->match_function_id](match, stat, input);
	if (!ret) {
		input += 1;
		goto MATCHAGAIN;
	}
	if (ret)
		stat->where = input;
	return ret;
}


int rpa_match_scan_byte_icase(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	int ret = 0;
	const char *end = stat->end;
	const char *start = stat->start;
	const char *lastprogress = 0;

MATCHAGAIN:
	if (input >= stat->end) {
		if (stat->fail && input < end) {
			stat->end = end;
			stat->fail = 0;
		} else {
			return 0;
		}
	}
	if (stat->progress && (input - lastprogress) > RPA_MIN_PROGRESS) {
		lastprogress = input;
		stat->progress(stat->progress_userdata, input, start, end);
	}
	ret = stat->mtable[match->match_function_id](match, stat, input);
	if (!ret) {
		input += 1;
		goto MATCHAGAIN;
	}
	if (ret)
		stat->where = input;
	return ret;
}
