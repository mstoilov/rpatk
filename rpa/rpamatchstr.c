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
#include "rpamatchstr.h"
#include "rpamem.h"
#include "rpastring.h"
#include "rpastat.h"


static unsigned int rpa_match_str_getid(rpa_class_t *cls)
{
	return (RPA_MATCH_STR_CLASSID | RPA_MATCH_CLASSID);
}


static const char *rpa_match_str_getstr(rpa_class_t *cls)
{
	return "str";
}


static int rpa_match_str_dump(rpa_class_t *cls, char *buffer, unsigned int size)
{
	return 0;
}


static void rpa_match_str_destroy(rpa_class_t *cls)
{
	rpa_match_t *match = (rpa_match_t*)cls;
	rpa_match_str_cleanup(match);
	rpa_free(match);
}

static rpa_class_methods_t rpa_match_str_methods = {
	rpa_match_str_getid,
	rpa_match_str_getstr,
	rpa_match_str_dump,
	rpa_match_str_destroy,
};


rpa_match_t *rpa_match_str_init(
	rpa_match_t *match, 
	const char *name,
	unsigned int namesiz,
	rpa_class_methods_t *vptr,
	rpa_matchfunc_t match_function_id)
{
	rpa_match_init(match, name, namesiz, match_function_id, vptr);
	return (rpa_match_t *)match;
}


void rpa_match_str_cleanup(rpa_match_t *match)
{
	rpa_free(((rpa_match_str_t*)match)->str);
	rpa_match_cleanup(match);
}


rpa_match_t * rpa_match_str_create_namesize(
	const char *name,
	unsigned int namesiz,
	rpa_matchfunc_t match_function_id)
{
	rpa_match_str_t *newMatch;
	
	newMatch = (rpa_match_str_t *)rpa_malloc(sizeof(*newMatch) );
	if (!newMatch)
		return ((void*)0);
	rpa_memset(newMatch, 0, sizeof(*newMatch));
	return rpa_match_str_init((rpa_match_t*)newMatch, name, namesiz, &rpa_match_str_methods, match_function_id);
}


rpa_match_t * rpa_match_str_create(
	const char *name,
	rpa_matchfunc_t match_function_id)
{
	return rpa_match_str_create_namesize(name, rpa_strlen(name), match_function_id);
}


rpa_strval_t *rpa_match_str_alloc_strval(rpa_match_t *match, unsigned long count)
{
	rpa_match_str_t *matchstr = (rpa_match_str_t*)match;
	if (matchstr->str)
		rpa_free(matchstr->str);
	matchstr->count = 0;
	matchstr->str = (rpa_strval_t *) rpa_malloc(sizeof(rpa_strval_t) * (count + 1));
	if (!matchstr->str)
		return (void*)0;
	matchstr->count = count;
	return matchstr->str;
}


void rpa_match_str_setval(rpa_match_t *match, unsigned int val, rpa_word_t offset)
{
	rpa_match_str_t *matchstr = (rpa_match_str_t*)match;

	if (matchstr->str) {
		matchstr->str[offset].val = val;
		matchstr->str[offset].icaseval = rpa_icasechar(val);
	}
}


int rpa_match_str(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	int ret;
	unsigned int c;
	const char *initial = input;
	rpa_match_str_t *strmatch = (rpa_match_str_t *)match;
	rpa_strval_t *str = strmatch->str, *strend = str + strmatch->count;

	while (str < strend) {
		ret = rpa_stat_getchar(&c, stat, input);
		if (c != str->val)
			return 0;
		input += ret;
		str++;
	}
	return (int)(input - initial);
}


int rpa_match_str_icase(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	int ret;
	unsigned int c;
	const char *initial = input;
	rpa_match_str_t *strmatch = (rpa_match_str_t *)match;
	rpa_strval_t *str = strmatch->str, *strend = str + strmatch->count;

	while (str < strend) {
		ret = rpa_stat_getchar(&c, stat, input);
		if (c != str->val && c != str->icaseval)
			return 0;
		input += ret;
		str++;
	}
	return (int)(input - initial);
}


int rpa_match_icase_str(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	return 0;
}


int rpa_match_str_chrinstr(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	return 0;
}


int rpa_match_icase_str_chrinstr(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	return 0;
}


int rpa_match_str_chrnotinstr(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	return 0;
}


int rpa_match_icase_str_chrnotinstr(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	return 0;
}
