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

#include "rstring.h"

#include "rpamatch.h"
#include "rpamatchval.h"
#include "rmem.h"
#include "rpastat.h"
#include "rpacharconv.h"


static unsigned int rpa_match_val_getid(rpa_class_t *cls)
{
	return (RPA_MATCH_VAL_CLASSID | RPA_MATCH_CLASSID);
}


static const char *rpa_match_val_getstr(rpa_class_t *cls)
{
	return "val";
}


static int rpa_match_val_dump(rpa_class_t *cls, char *buffer, unsigned int size)
{
	int ret;
	unsigned char mb[7];
	r_memset(mb, 0, sizeof(mb));
	ret = rpa_utf8_wctomb(((rpa_match_val_t*)cls)->val, (unsigned char *)buffer, size);
	if (ret < 0)
		return 0;
	if (ret < (int)size)
		buffer[ret] = 0;
	return ret;
}


static void rpa_match_val_destroy(rpa_class_t *cls)
{
	rpa_match_t *match = (rpa_match_t*)cls;
	rpa_match_val_cleanup(match);
	r_free(match);
}


static rpa_class_methods_t rpa_match_val_methods = {
	rpa_match_val_getid,
	rpa_match_val_getstr,
	rpa_match_val_dump,
	rpa_match_val_destroy,
};


rpa_match_t *rpa_match_val_init(
	rpa_match_t *match, 
	const char *name,
	unsigned int namesiz,
	rpa_class_methods_t *vptr,
	rpa_matchfunc_t match_function_id,
	unsigned long val)
{
	rpa_match_init(match, name, namesiz, match_function_id, vptr);
	((rpa_match_val_t*)match)->val = val;
	((rpa_match_val_t*)match)->icaseval = rpa_icasechar(val);
	return (rpa_match_t *)match;
}


void rpa_match_val_cleanup(rpa_match_t *match)
{
	rpa_match_cleanup(match);	
}


rpa_match_t * rpa_match_val_create_namesize(
	const char *name,
	unsigned int namesiz,
	rpa_matchfunc_t match_function_id,
	unsigned long val)
{
	rpa_match_val_t *newMatch;
	
	newMatch = (rpa_match_val_t *)r_malloc(sizeof(*newMatch));
	if (!newMatch)
		return ((void*)0);
	r_memset(newMatch, 0, sizeof(*newMatch));
	return rpa_match_val_init((rpa_match_t*)newMatch, name, namesiz, &rpa_match_val_methods, match_function_id, val);
}


rpa_match_t * rpa_match_val_create(
	const char *name,
	rpa_matchfunc_t match_function_id,
	unsigned long val)
{
	return rpa_match_val_create_namesize(name, r_strlen(name), match_function_id, val);
}


int rpa_match_val_chreq(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	unsigned int c;
	int ret;

	ret = rpa_stat_getchar(&c, stat, input);
	if (c == ((rpa_match_val_t*)match)->val)
		return ret;
	return 0;
}


int rpa_match_val_chrnoteq(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	unsigned int c;
	int ret;

	ret = rpa_stat_getchar(&c, stat, input);
	if (c != ((rpa_match_val_t*)match)->val)
		return ret;
	return 0;
}


int rpa_match_val_chreq_icase(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	unsigned int c;
	int ret;

	ret = rpa_stat_getchar(&c, stat, input);
	if (c == ((rpa_match_val_t*)match)->val)
		return ret;
	else if (c == ((rpa_match_val_t*)match)->icaseval)
		return ret;

	return 0;
}


int rpa_match_val_chrnoteq_icase(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	unsigned int c;
	int ret;

	ret = rpa_stat_getchar(&c, stat, input);
	if (c != ((rpa_match_val_t*)match)->val && c != ((rpa_match_val_t*)match)->icaseval)
		return ret;
	return 0;
}
