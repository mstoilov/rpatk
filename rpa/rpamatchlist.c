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
#include "rpamnode.h"
#include "rpamatchlist.h"
#include "rstring.h"
#include "rmem.h"
#include "rpastat.h"
#include "rpasearch.h"


static unsigned int rpa_match_list_getid(rpa_class_t *cls)
{
	return (RPA_MATCH_LIST_CLASSID | RPA_MATCH_CLASSID);
}


static const char *rpa_match_list_getstr(rpa_class_t *cls)
{
	return "list";
}


static int rpa_match_list_dump(rpa_class_t *cls, char *buffer, unsigned int size)
{
	return 0;
}


static void rpa_match_list_destroy(rpa_class_t *cls)
{
	rpa_match_t *match = (rpa_match_t *)cls;
	rpa_match_list_cleanup(match);
	r_free(match);
}

static rpa_class_methods_t rpa_match_list_methods = {
	rpa_match_list_getid,
	rpa_match_list_getstr,
	rpa_match_list_dump,
	rpa_match_list_destroy,
};


void rpa_match_list_cleanup(rpa_match_t *match)
{
	rpa_match_list_cleanup_dataptr(match);
	rpa_match_cleanup(match);
}

rpa_match_t *rpa_match_list_init(
	rpa_match_t *matchlist, 
	const char *name,
	unsigned int namesiz,
	rpa_class_methods_t *vptr,
	rpa_matchfunc_t match_function_id)
{
	rpa_match_init(matchlist, name, namesiz, match_function_id, vptr);
	rpa_list_init(&((rpa_match_list_t*)matchlist)->head);
	rpa_match_list_init_dataptr(matchlist, name, namesiz, vptr, match_function_id);
	return matchlist;
}


rpa_match_t * rpa_match_list_create_namesize(
	const char *name,
	unsigned int namesiz,
	rpa_matchfunc_t match_function_id)
{
	rpa_match_list_t *newMatch;
	newMatch = (rpa_match_list_t *)r_malloc(sizeof(*newMatch));
	if (!newMatch)
		return ((void*)0);
	r_memset(newMatch, 0, sizeof(*newMatch));
	return rpa_match_list_init((rpa_match_t*)newMatch, name, namesiz, (rpa_class_methods_t*)&rpa_match_list_methods, match_function_id);
}


rpa_match_t * rpa_match_list_create(const char *name, rpa_matchfunc_t match_function_id)
{
	return rpa_match_list_create_namesize(name, r_strlen(name), match_function_id);
}


int rpa_match_list(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	rpa_head_t *head;
	const char *initial;
	rpa_list_t *pos;
	rpa_mnode_t *hcur;
	int ret;

	if (!stat->checkstack(stat))
		return 0;
	initial = input;
	head = &((rpa_match_list_t *)match)->head;
	rpa_list_for_each(pos, head) {
		hcur = rpa_list_entry(pos, rpa_mnode_t, mlink);
		ret = stat->ntable[hcur->flags & RPA_MNODEFUNC_MASK](hcur, stat, input);
		if (ret < 0)
			return 0;		
		input += ret;
		if (!ret && !(hcur->flags & RPA_MATCH_OPTIONAL))
			return 0;
	}
	return (int)(input - initial);
}


int rpa_match_list_icase(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	/* 
	 * TBD. The rpa_match_list_icase probably needs to go away.
	 */
	return rpa_match_list(match, stat, input);
}



int rpa_match_list_alt(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	rpa_head_t *head;
	rpa_list_t *pos;
	rpa_mnode_t *hcur;
	int ret;

	head = &((rpa_match_list_t *)match)->head;
	rpa_list_for_each(pos, head) {
		hcur = rpa_list_entry(pos, rpa_mnode_t, mlink);
		ret = stat->ntable[hcur->flags & RPA_MNODEFUNC_MASK](hcur, stat, input);
		if (ret > 0)
			return ret;
	}
	return 0;
}


int rpa_match_list_best_alt(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	rpa_head_t *head;
	rpa_list_t *pos;
	rpa_mnode_t *hcur;
	int ret, mret = 0;

	head = &((rpa_match_list_t *)match)->head;
	rpa_list_for_each(pos, head) {
		hcur = rpa_list_entry(pos, rpa_mnode_t, mlink);
		ret = stat->ntable[hcur->flags & RPA_MNODEFUNC_MASK](hcur, stat, input);
		if (ret > mret)
			mret = ret;
	}
	return mret;
}



int rpa_match_list_noalt(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	rpa_head_t *head;
	rpa_list_t *pos;
	rpa_mnode_t *hcur;
	int ret;
	unsigned int wc;

	head = &((rpa_match_list_t *)match)->head;
	rpa_list_for_each(pos, head) {
		hcur = rpa_list_entry(pos, rpa_mnode_t, mlink);
		ret = stat->ntable[hcur->flags & RPA_MNODEFUNC_MASK](hcur, stat, input);
		if (ret > 0)
			return 0;
	}
	ret = rpa_stat_getchar(&wc, stat, input);
	if (ret < 0)
		ret = 0;
	return ret;	
}


int rpa_match_list_alt_icase(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	rpa_head_t *head;
	rpa_list_t *pos;
	rpa_mnode_t *hcur;
	int ret;

	head = &((rpa_match_list_t *)match)->head;
	rpa_list_for_each(pos, head) {
		hcur = rpa_list_entry(pos, rpa_mnode_t, mlink);
		ret = stat->ntable[hcur->flags & RPA_MNODEFUNC_MASK](hcur, stat, input);
		if (ret > 0)
			return ret;
	}
	return 0;
}



int rpa_match_list_and(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	rpa_head_t *head;
	rpa_list_t *pos;
	rpa_mnode_t *hcur;
	int ret = 0;
	
	head = &((rpa_match_list_t *)match)->head;
	rpa_list_for_each(pos, head) {
		hcur = rpa_list_entry(pos, rpa_mnode_t, mlink);
		ret = stat->ntable[hcur->flags & RPA_MNODEFUNC_MASK](hcur, stat, input);
		if (ret <= 0)
			return 0;
	}
	return ret;
}


int rpa_match_list_and_icase(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	rpa_head_t *head;
	rpa_list_t *pos;
	rpa_mnode_t *hcur;
	int ret = 0;
	
	head = &((rpa_match_list_t *)match)->head;
	rpa_list_for_each(pos, head) {
		hcur = rpa_list_entry(pos, rpa_mnode_t, mlink);
		ret = stat->ntable[hcur->flags & RPA_MNODEFUNC_MASK](hcur, stat, input);
		if (ret <= 0)
			return 0;
	}
	return ret;
}


int rpa_match_list_contain(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	rpa_head_t *head;
	rpa_list_t *first, *second;
	rpa_mnode_t *hcur;
	const char *savedend = stat->end;
	int ret, mret;

	head = &((rpa_match_list_t *)match)->head;
	if (!(first = rpa_list_first(head)))
		return 0;
	if (!(second = rpa_list_next(head, first)))
		return 0;
	hcur = rpa_list_entry(first, rpa_mnode_t, mlink);
	ret = stat->ntable[hcur->flags & RPA_MNODEFUNC_MASK](hcur, stat, input);
	if (ret <= 0)
		return 0;
	stat->end = input + ret;
	hcur = rpa_list_entry(second, rpa_mnode_t, mlink);
	mret = stat->mtable[RPA_MATCHFUNC_SCAN](hcur->match, stat, input);
	/* Restore the end of buffer, if it is not set ot 0 (abort operation) */
	if (stat->end)
		stat->end = savedend;
	if (!mret)
		return 0;
	return ret;
}



int rpa_match_list_at(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	rpa_head_t *head;
	rpa_list_t *first, *second;
	rpa_mnode_t *hcur;
	const char *savedend = stat->end;
	int ret;

	head = &((rpa_match_list_t *)match)->head;
	if (!(first = rpa_list_first(head)))
		return 0;
	if (!(second = rpa_list_next(head, first)))
		return 0;
	hcur = rpa_list_entry(second, rpa_mnode_t, mlink);
	ret = stat->ntable[hcur->flags & RPA_MNODEFUNC_MASK](hcur, stat, input);
	if (ret <= 0)
		return 0;
	stat->end = input + ret;
	hcur = rpa_list_entry(first, rpa_mnode_t, mlink);
	ret = stat->ntable[hcur->flags & RPA_MNODEFUNC_MASK](hcur, stat, input);
	if (stat->end)
		stat->end = savedend;
	if (ret <= 0)
		return 0;
	return ret;
}

/*
int rpa_match_list_at_icase(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	rpa_head_t *head;
	rpa_list_t *first, *second;
	rpa_mnode_t *hcur;
	const char *savedend = stat->end;
	int ret;

	head = &((rpa_match_list_t *)match)->head;
	if (!(first = rpa_list_first(head)))
		return 0;
	if (!(second = rpa_list_next(head, first)))
		return 0;
	hcur = rpa_list_entry(second, rpa_mnode_t, mlink);
	ret = stat->ntable[hcur->flags & RPA_MNODEFUNC_MASK](hcur, stat, input);
	if (ret <= 0)
		return 0;
	stat->end = input + ret;
	hcur = rpa_list_entry(first, rpa_mnode_t, mlink);
	ret = stat->ntable[hcur->flags & RPA_MNODEFUNC_MASK](hcur, stat, input);
	if (stat->end)
		stat->end = savedend;
	if (ret <= 0)
		return 0;
	return ret;
}
*/

int rpa_match_list_minus(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	rpa_head_t *head = &((rpa_match_list_t *)match)->head;
	rpa_list_t *pos;
	rpa_mnode_t *hcur;
	int ret = 0, mret = 0, i = 0;
	int usecache = stat->usecache;

	rpa_list_for_each(pos, head) {
		hcur = rpa_list_entry(pos, rpa_mnode_t, mlink);
		if (i++ == 0) {
			if ((ret = stat->ntable[hcur->flags & RPA_MNODEFUNC_MASK](hcur, stat, input)) <= 0)
				return 0;
		} else {
			/*
			 * Disable cache. Anything that matches here MUST not be cached.
			 */
			stat->usecache = 0;
			mret  = stat->ntable[hcur->flags & RPA_MNODEFUNC_MASK](hcur, stat, input);
			stat->usecache = usecache;
			if (mret > 0)
				return 0;
		}
	}
	return ret;
}


int rpa_match_list_minus_icase(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	rpa_head_t *head = &((rpa_match_list_t *)match)->head;
	rpa_list_t *pos;
	rpa_mnode_t *hcur;
	int ret = 0, mret = 0, i = 0;
	int usecache = stat->usecache;

	rpa_list_for_each(pos, head) {
		hcur = rpa_list_entry(pos, rpa_mnode_t, mlink);
		if (i++ == 0) {
			if ((ret = stat->ntable[hcur->flags & RPA_MNODEFUNC_MASK](hcur, stat, input)) <= 0)
				return 0;
		} else {
			/*
			 * Disable cache. Anything that matches here MUST not be cached.
			 */
			stat->usecache = 0;
			mret = stat->ntable[hcur->flags & RPA_MNODEFUNC_MASK](hcur, stat, input);
			stat->usecache = usecache;
			if (mret > 0)
				return 0;
		}
	}
	return ret;
}


int rpa_match_list_not(rpa_match_t *match, rpa_stat_t *stat, const char *input)
{
	rpa_head_t *head = &((rpa_match_list_t *)match)->head;
	rpa_list_t *pos;
	rpa_mnode_t *hcur;
	int ret;
	unsigned int wc;
	int usecache = stat->usecache;

	rpa_list_for_each(pos, head) {
		hcur = rpa_list_entry(pos, rpa_mnode_t, mlink);
		/*
		 * Disable cache. Anything that matches here MUST not be cached.
		 */
		stat->usecache = 0;
		ret = stat->ntable[hcur->flags & RPA_MNODEFUNC_MASK](hcur, stat, input);
		stat->usecache = usecache;
		if (ret > 0)
			return 0;
	}
	
	ret = rpa_stat_getchar(&wc, stat, input);
	if (ret < 0)
		ret = 0;
	return ret;
}


static unsigned int rpa_match_nlist_getid(rpa_class_t *cls)
{
	return (RPA_MATCH_NLIST_CLASSID | RPA_MATCH_LIST_CLASSID | RPA_MATCH_CLASSID);
}


static const char *rpa_match_nlist_getstr(rpa_class_t *cls)
{
	return "nlist";
}


static int rpa_match_nlist_dump(rpa_class_t *cls, char *buffer, unsigned int size)
{
	return 0;
}


static void rpa_match_nlist_destroy(rpa_class_t *cls)
{
	rpa_match_list_destroy(cls);
}

static rpa_class_methods_t rpa_match_nlist_methods = {
	rpa_match_nlist_getid,
	rpa_match_nlist_getstr,
	rpa_match_nlist_dump,
	rpa_match_nlist_destroy,
};


rpa_match_t * rpa_match_nlist_create_namesize(
	const char *name,
	unsigned int namesiz,
	rpa_matchfunc_t match_function_id)
{
	rpa_match_nlist_t *newMatch;
	newMatch = (rpa_match_nlist_t *)r_malloc(sizeof(*newMatch));
	if (!newMatch)
		return ((void*)0);
	r_memset(newMatch, 0, sizeof(*newMatch));
	return rpa_match_list_init((rpa_match_t *)newMatch, name, namesiz, &rpa_match_nlist_methods, match_function_id);
}
