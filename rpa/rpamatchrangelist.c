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
#include "rpamem.h"
#include "rpastring.h"
#include "rpamatchlist.h"

static unsigned int rpa_match_rangelist_getid(rpa_class_t *cls)
{
	return (RPA_MATCH_RANGELIST_CLASSID | RPA_MATCH_LIST_CLASSID | RPA_MATCH_CLASSID);
}


static const char *rpa_match_rangelist_getstr(rpa_class_t *cls)
{
	return "rangelist";
}


static int rpa_match_rangelist_dump(rpa_class_t *cls, char *buffer, unsigned int size)
{
	return 0;
}


static void rpa_match_rangelist_destroy(rpa_class_t *cls)
{
	rpa_match_t *match = (rpa_match_t *)cls;
	rpa_match_list_cleanup(match);
	rpa_free(match);
}

static rpa_class_methods_t rpa_match_rangelist_methods = {
	rpa_match_rangelist_getid,
	rpa_match_rangelist_getstr,
	rpa_match_rangelist_dump,
	rpa_match_rangelist_destroy,
};


rpa_match_t * rpa_match_rangelist_create_namesize(
	const char *name,
	unsigned int namesiz,
	rpa_matchfunc_t match_function_id)
{
	rpa_match_list_t *newMatch;
	newMatch = (rpa_match_list_t *)rpa_malloc(sizeof(*newMatch));
	if (!newMatch)
		return ((void*)0);
	rpa_memset(newMatch, 0, sizeof(*newMatch));
	return rpa_match_list_init((rpa_match_t *)newMatch, name, namesiz, &rpa_match_rangelist_methods, match_function_id);
}


rpa_match_t * rpa_match_rangelist_create(const char *name, rpa_matchfunc_t match_function_id)
{
	return rpa_match_rangelist_create_namesize(name, rpa_strlen(name), match_function_id);
}
