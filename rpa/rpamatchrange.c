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
#include "rpamatchrange.h"
#include "rpastring.h"
#include "rpamem.h"
#include "rpastat.h"


static unsigned int rpa_match_range_getid(rpa_class_t *cls)
{
	return (RPA_MATCH_RANGE_CLASSID | RPA_MATCH_CLASSID);
}


static const char *rpa_match_range_getstr(rpa_class_t *cls)
{
	return "range";
}


static int rpa_match_range_dump(rpa_class_t *cls, char *buffer, unsigned int size)
{
	return 0;
}


static void rpa_match_range_destroy(rpa_class_t *cls)
{
	rpa_match_t *match = (rpa_match_t*)cls;
	rpa_match_range_cleanup(match);
	rpa_free(match);
}


static rpa_class_methods_t rpa_match_range_methods = {
	rpa_match_range_getid,
	rpa_match_range_getstr,
	rpa_match_range_dump,
	rpa_match_range_destroy,
};


rpa_match_t *rpa_match_range_init(
	rpa_match_t *match, 
	const char *name,
	unsigned int namesiz,
	rpa_class_methods_t *vptr,
	rpa_matchfunc_t match_function_id,
	unsigned long low,
	unsigned long high)
{
	rpa_match_init(match, name, namesiz, match_function_id, vptr);
	((rpa_match_range_t *)match)->low = low;
	((rpa_match_range_t *)match)->high = high;
	((rpa_match_range_t *)match)->icaselow = rpa_icasechar(low);
	((rpa_match_range_t *)match)->icasehigh = rpa_icasechar(high);
	return match;
}

void rpa_match_range_cleanup(rpa_match_t *match)
{
	rpa_match_cleanup(match);
}


rpa_match_t * rpa_match_range_create_namesize(
	const char *name,
	unsigned int namesiz,
	rpa_matchfunc_t match_function_id,
	unsigned long low,
	unsigned long high)
{
	rpa_match_range_t *newMatch;
	
	newMatch = (rpa_match_range_t *)rpa_malloc(sizeof(*newMatch));
	if (!newMatch)
		return ((void*)0);
	rpa_memset(newMatch, 0, sizeof(*newMatch));
	return rpa_match_range_init((rpa_match_t*)newMatch, name, namesiz, &rpa_match_range_methods, match_function_id, low, high);
}


rpa_match_t * rpa_match_range_create(
	const char *name,
	rpa_matchfunc_t match_function_id,
	unsigned long low,
	unsigned long high)
{
	return rpa_match_range_create_namesize(name, rpa_strlen(name), match_function_id, low, high);
}


int rpa_match_range_chrinrange(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	unsigned int c;
	int ret;

	ret = rpa_stat_getchar(&c, stat, input);
	if (c >= ((rpa_match_range_t*)match)->low && c <= ((rpa_match_range_t*)match)->high)
		return ret;
	return 0;
}


int rpa_match_range_chrinrange_icase(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	unsigned int c;
	int ret;

	ret = rpa_stat_getchar(&c, stat, input);
	if (c >= ((rpa_match_range_t*)match)->low && c <= ((rpa_match_range_t*)match)->high)
		return ret;
	else if (c >= ((rpa_match_range_t*)match)->icaselow && c <= ((rpa_match_range_t*)match)->icasehigh)
		return ret;
		
	return 0;
}


int rpa_match_range_chrnotinrange(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	unsigned int c;
	int ret;

	ret = rpa_stat_getchar(&c, stat, input);
	if (c < ((rpa_match_range_t*)match)->low && c > ((rpa_match_range_t*)match)->high)
		return ret;
	return 0;
}


int rpa_match_range_chrnotinrange_icase(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	unsigned int c;
	int ret;

	ret = rpa_stat_getchar(&c, stat, input);
	if ((c < ((rpa_match_range_t*)match)->low && c > ((rpa_match_range_t*)match)->high) &&
		(c < ((rpa_match_range_t*)match)->icaselow && c > ((rpa_match_range_t*)match)->icasehigh))
		return ret;

	return 0;
}
