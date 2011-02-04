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

#include "rpamnode.h"
#include "rmem.h"
#include "rstring.h"
#include "rpamatch.h"
#include "rpadbexpriv.h"
#include "rpadbex.h"
#include "rpamatchstr.h"
#include "rpamatchrange.h"
#include "rpamatchval.h"
#include "rpamatchlist.h"
#include "rpadebug.h"
#include "rpaerror.h"
#include "rpalist.h"
#include "rpasearch.h"


typedef struct rpa_match_s rpa_pattern_s;
typedef struct rpa_match_s rpa_group_s;


void rpa_dbex_close_do(rpa_dbex_handle hDbex)
{
	rpa_head_t mhead = RPA_LIST_HEAD(mhead);
	rpa_pattern_handle hPattern = ((void*)0);
	rpa_mnode_t mnode;

	if (!hDbex)
		return;
	
	r_memset(&mnode, 0, sizeof(mnode));
	for (hPattern = rpa_dbex_first_pattern(hDbex); hPattern; hPattern = rpa_dbex_next_pattern(hDbex, hPattern)) {
		mnode.match = (rpa_match_t *)hPattern->var.v.ptr;
		mnode.flags = 0;

		if (rpa_mnode_check_for_loop(&mnode, mnode.match, &mhead, 0))
			((rpa_match_nlist_t*)(mnode.match))->loopy = 1;
	}

	hDbex->rwlock = 0;
}


void rpa_dbex_close(rpa_dbex_handle hDbex)
{
	rpa_dbex_close_do(hDbex);
}


void rpa_dbex_cleanup(rpa_dbex_handle hDbex)
{
	rpa_varlink_destroy_all(&hDbex->treehead);
	r_free(hDbex->namehash);
	if (hDbex->parser)
		rpa_parser_destroy(hDbex->parser);
	if (hDbex->parser)
		rpa_vm_destroy(hDbex->vm);
	rpa_stat_cleanup(&hDbex->stat);
}

rpa_dbex_handle rpa_dbex_init(rpa_dbex_handle hDbex, unsigned int namehashEntries)
{
	unsigned int i;
	if (namehashEntries < 2)
		namehashEntries = 2;
	for (i = 0; namehashEntries; i++)
		namehashEntries >>= 1;
	namehashEntries = 1 << (i - 1);
	r_memset(hDbex, 0, sizeof(*hDbex));
	rpa_list_init(&hDbex->treehead);
	rpa_list_init(&hDbex->callbacks);
	rpa_list_init(&hDbex->callbackmnodes);
	hDbex->namehashEntries = namehashEntries;
	hDbex->namehash = r_malloc(namehashEntries* sizeof(rpa_head_t));
	for (i = 0; i < namehashEntries; i++)
		rpa_list_init(&hDbex->namehash[i]);
	if (rpa_dbex_check_parser(hDbex) < 0) {
		rpa_dbex_cleanup(hDbex);
		return (void*)0;
	}
	if (rpa_dbex_check_vm(hDbex) < 0) {
		rpa_dbex_cleanup(hDbex);
		return (void*)0;
	}
	rpa_stat_init(&hDbex->stat);
	if (rpa_dbex_int_ascii_matchptr_array(hDbex) < 0) {
		rpa_dbex_cleanup(hDbex);
		return (void*)0;
	}
	return hDbex;
}


rpa_dbex_handle rpa_dbex_create_with_hash(unsigned int uHashEntries)
{
	rpa_dbex_handle hDbex;

	if (!(hDbex = r_malloc(sizeof(*hDbex))))
		return ((void*)0);
	return rpa_dbex_init(hDbex, uHashEntries);
}


rpa_dbex_handle rpa_dbex_create(void)
{
	return rpa_dbex_create_with_hash(RPA_NAMEHASH_SIZE);
}


void rpa_dbex_destroy(rpa_dbex_handle hDbex)
{
	rpa_dbex_cleanup(hDbex);
	r_free((void *)hDbex);
}


int rpa_dbex_load(rpa_dbex_handle hDbex, const char *patterns, unsigned int size)
{
	int ret = 0, vmret = 0;
	
	if (!hDbex)
		return -1;
	if (!hDbex->rwlock) {
		hDbex->lastError = RPA_E_NOTOPEN;
		return -1;
	}
	if (!patterns) {
		hDbex->lastError = RPA_E_INVALID_PARAM;
		return -1;
	}
	if (size == 0) {
		return 0;
	}

	rpa_stat_set_encoding(&hDbex->parser->stat, RPA_ENCODING_UTF8);
	ret = rpa_parser_exec(hDbex->parser, patterns, size);
	if (ret == 0) {
		hDbex->lastError = RPA_E_SYNTAX_ERROR;
		return -1;
	}
	if (hDbex->parser->vmcode_off) {
#ifdef VMEXECDEBUG
		vmret = rpa_vm_exec_debug(hDbex->vm, hDbex->parser->vmcode, 0);
#else
		vmret = rpa_vm_exec(hDbex->vm, hDbex->parser->vmcode, 0);
		if (vmret < 0) {
			hDbex->lastError = RPA_E_SYNTAX_ERROR;
			return -1;
		}
#endif
		
	}
	return ret;
}


int rpa_dbex_load_string(rpa_dbex_handle hDbex, const char *patterns)
{
	int ret, inputsize;

	if (!patterns) {
		hDbex->lastError = RPA_E_INVALID_PARAM;
		return -1;
	}
	inputsize = r_strlen(patterns) + 1;
	while ((ret = rpa_dbex_load(hDbex, patterns, inputsize)) > 0) {
		inputsize -= ret;
		patterns += ret;
	}
	return ret;
}


int rpa_dbex_open(rpa_dbex_handle hDbex)
{
	if (!hDbex)
		return -1;
	hDbex->rwlock = 1;
	return 0;
}


rpa_pattern_handle rpa_dbex_get_pattern(rpa_dbex_handle hDbex, const char *name)
{
	rpa_pattern_handle hPattern = ((void*)0);
	const char *patternName;

	for (hPattern = rpa_dbex_first_pattern(hDbex); hPattern; hPattern = rpa_dbex_next_pattern(hDbex, hPattern)) {
		patternName = rpa_dbex_pattern_name(hDbex, hPattern);
		if (rpa_dbex_pattern_name(hDbex, hPattern) && r_strcmp(patternName, name) == 0)
			return hPattern;
	}
	return ((void *)0);
}


rpa_pattern_handle rpa_dbex_default_pattern(rpa_dbex_handle hDbex)
{
	if (hDbex)
		return hDbex->defaultPattern;
	return (void*)0;
}


unsigned int rpa_dbex_get_error(rpa_dbex_handle hDbex)
{
	if (hDbex)
		return hDbex->lastError;
	return RPA_E_NONE;
}


rpa_pattern_handle rpa_dbex_next_pattern(rpa_dbex_handle hDbex, rpa_pattern_handle hCur)
{
	rpa_link_t *pos;
	rpa_varlink_t *pVarLinkMatch;

	for (pos = rpa_list_next(&hDbex->treehead, hCur ? &hCur->lnk : &hDbex->treehead); pos; pos = rpa_list_next(&hDbex->treehead, pos)) {
		pVarLinkMatch = rpa_list_entry(pos, rpa_varlink_t, lnk);
		if (pVarLinkMatch->var.userdata4 == MATCH_CLASS_NAMEDMATCHPTR) {
			return pVarLinkMatch;
		}
	}
	return ((void *)0);
}


rpa_pattern_handle rpa_dbex_prev_pattern(rpa_dbex_handle hDbex, rpa_pattern_handle hCur)
{
	rpa_link_t *pos;
	rpa_varlink_t *pVarLinkMatch;

	for (pos = rpa_list_prev(&hDbex->treehead, hCur ? &hCur->lnk : &hDbex->treehead); pos; pos = rpa_list_prev(&hDbex->treehead, pos)) {
		pVarLinkMatch = rpa_list_entry(pos, rpa_varlink_t, lnk);
		if (pVarLinkMatch->var.userdata4 == MATCH_CLASS_NAMEDMATCHPTR) {
			return pVarLinkMatch;
		}
	}
	return ((void *)0);
}


rpa_pattern_handle rpa_dbex_first_pattern(rpa_dbex_handle hDbex)
{
	return rpa_dbex_next_pattern(hDbex, 0);
}


rpa_pattern_handle rpa_dbex_last_pattern(rpa_dbex_handle hDbex)
{
	return rpa_dbex_prev_pattern(hDbex, 0);
}


const char *rpa_dbex_pattern_name(rpa_dbex_handle hDbex, rpa_pattern_handle hPattern)
{
	const char *name = ((void *)0);
	if (!hDbex)
		goto error;
	if (!hPattern) {
		hDbex->lastError = RPA_E_INVALID_PARAM;
		goto error;
	}
	if (hPattern->var.userdata4 == MATCH_CLASS_NAMEDMATCHPTR) {
		rpa_match_t *match = (rpa_match_t *)hPattern->var.v.ptr;
		name = match->name;
	}
	
error:
	return name;
}


const char *rpa_dbex_pattern_regex(rpa_dbex_handle hDbex, rpa_pattern_handle hPattern, int seq)
{
	int count = 0;
	const char *regex = ((void *)0);
	
	if (!hDbex)
		goto error;
	if (!hPattern) {
		hDbex->lastError = RPA_E_INVALID_PARAM;
		goto error;
	}
	if (hPattern->var.userdata4 == MATCH_CLASS_NAMEDMATCHPTR) {
		rpa_link_t *pos;
		rpa_mnode_t *mnode;
		rpa_match_list_t *match = (rpa_match_list_t *)hPattern->var.v.ptr;
		pos = rpa_list_first(&match->head);
		for (count = 0; count < seq && pos; count++) {
			pos = rpa_list_next(&match->head, pos);
		}
		if (pos) {
			mnode = rpa_list_entry(pos, rpa_mnode_t, mlink);
			regex = mnode->match->name;
		}
	} else if (hPattern->var.userdata4 == MATCH_CLASS_MATCHPTR) {
		rpa_match_t *match = (rpa_match_t *)hPattern->var.v.ptr;
		regex = match->name;
	}
	
error:
	return regex;
}


int rpa_dbex_strmatch(const char *str, const char *patterns)
{
	struct rpa_dbex_s dbex;
	int len = 0;
	int ret = 0;
	
	if (patterns == ((void *)0) || patterns == ((void *)0))
		return -1;
	len = r_strlen(str);
	rpa_dbex_init(&dbex, 16);
	rpa_dbex_open(&dbex);
	while (*patterns) {
		if ((ret = rpa_dbex_load_string(&dbex, patterns)) < 0)
			goto error;
		if (ret == 0)
			break;
		patterns += ret;
	}
	ret = rpa_stat_match_lite(&dbex.stat, rpa_dbex_default_pattern(&dbex), str, str, str + len);
	if (ret < 0)
		goto error;
	rpa_dbex_close_do(&dbex);
	rpa_dbex_cleanup(&dbex);
	return ret;
	
error:
	rpa_dbex_close_do(&dbex);
	rpa_dbex_cleanup(&dbex);
	return -1;
}


static int rpa_dbex_add_callback_work(rpa_dbex_handle hDbex, const char *namematch, unsigned int reason, rpa_match_callback func, void *userdata, int exact)
{
	int ret = 0, cbsize = 0, namesiz = 0;
	rpa_varlink_t *pVarLinkCallback;
	rpa_callbackdata_t *pCbData = (void*)0;
	
	if (namematch) {
		namesiz = r_strlen(namematch);
	}
	cbsize = sizeof(rpa_callbackdata_t) + namesiz + 1;
	if ((pCbData = (rpa_callbackdata_t *)r_malloc(cbsize)) == 0) {
		hDbex->lastError = RPA_E_OUTOFMEM;
		return -1;		
	}
	r_memset(pCbData, 0, cbsize);
	r_strncpy(pCbData->namematch, namematch, namesiz);
	pCbData->userdata = userdata;
	pCbData->reason = reason;
	pCbData->func = func;
	pCbData->exact = exact;
	pCbData->namematchsiz = namesiz;
	pVarLinkCallback = rpa_varlink_create(RPA_VAR_PTR, "callback");
	if (!pVarLinkCallback) {
		r_free(pCbData);
		hDbex->lastError = RPA_E_OUTOFMEM;
		return -1;		
	}
	pVarLinkCallback->var.userdata4 = (rpa_word_t)MATCH_CLASS_CALLBACKPTR;
	pVarLinkCallback->var.v.ptr = pCbData;
	pVarLinkCallback->var.finalize = rpa_var_finalize_ptr;
	rpa_list_addt(&hDbex->treehead, &pVarLinkCallback->lnk);
	rpa_list_addt(&hDbex->callbacks, &pVarLinkCallback->hlnk);
	return ret;	
}


int rpa_dbex_add_callback_exact(rpa_dbex_handle hDbex, const char *name, unsigned int reason, rpa_match_callback func, void *userdata)
{
	return rpa_dbex_add_callback_work(hDbex, name, reason, func, userdata, 1);
}


int rpa_dbex_add_callback(rpa_dbex_handle hDbex, const char *namematch, unsigned int reason, rpa_match_callback func, void *userdata)
{
	return rpa_dbex_add_callback_work(hDbex, namematch, reason, func, userdata, 0);
}


const char *rpa_dbex_callback_pattern(rpa_dbex_handle hDbex, rpa_callback_handle hCallback)
{
	if (!hDbex)
		goto error;
	if (!hCallback) {
		hDbex->lastError = RPA_E_INVALID_PARAM;
		goto error;
	}
	if (hCallback->var.userdata4 == MATCH_CLASS_CALLBACKPTR) {
		return ((rpa_callbackdata_t*)hCallback->var.v.ptr)->namematch;
	}
error:
	return ((void*)0);
}


void *rpa_dbex_callback_userdata(rpa_dbex_handle hDbex, rpa_callback_handle hCallback)
{
	rpa_callbackdata_t *pCbData = (void*)0;	
	if (!hDbex)
		goto error;
	if (!hCallback) {
		hDbex->lastError = RPA_E_INVALID_PARAM;
		goto error;
	}
	if (hCallback->var.userdata4 == MATCH_CLASS_CALLBACKPTR) {
		pCbData = (rpa_callbackdata_t *)hCallback->var.v.ptr;
		return (void*)pCbData->userdata;
	}
error:
	return ((void*)0);
}


int rpa_dbex_pattern_type(rpa_dbex_handle hDbex, rpa_pattern_handle hPattern)
{
	int type = RPA_PATTERN_SINGLE;

	if (!hDbex)
		goto error;
	if (!hPattern) {
		hDbex->lastError = RPA_E_INVALID_PARAM;
		goto error;
	}
	if (hPattern->var.userdata4 == MATCH_CLASS_NAMEDMATCHPTR) {
		rpa_match_t *match = (rpa_match_t *)hPattern->var.v.ptr;
		if (match->match_function_id == RPA_MATCHFUNC_NLIST_ALT)
			type = RPA_PATTERN_OR;
		else if (match->match_function_id == RPA_MATCHFUNC_NLIST_BESTALT)
			type = RPA_PATTERN_BEST;
	}
	
error:
	return type;
}


rpa_varlink_t *rpa_dbex_next_callback(rpa_dbex_handle hDbex, rpa_varlink_t *cur)
{
	rpa_link_t *pos;
	rpa_varlink_t *pVarLinkCallback;

	for (pos = rpa_list_next(&hDbex->callbacks, cur ? &cur->hlnk : &hDbex->callbacks); pos; pos = rpa_list_next(&hDbex->callbacks, pos)) {
		pVarLinkCallback = rpa_list_entry(pos, rpa_varlink_t, hlnk);
		if (pVarLinkCallback->var.userdata4 == MATCH_CLASS_CALLBACKPTR) {
			return pVarLinkCallback;
		}
	}
	return ((void *)0);
}


rpa_varlink_t *rpa_dbex_prev_callback(rpa_dbex_handle hDbex, rpa_varlink_t *cur)
{
	rpa_link_t *pos;
	rpa_varlink_t *pVarLinkCallback;

	for (pos = rpa_list_prev(&hDbex->callbacks, cur ? &cur->hlnk : &hDbex->callbacks); pos; pos = rpa_list_prev(&hDbex->callbacks, pos)) {
		pVarLinkCallback = rpa_list_entry(pos, rpa_varlink_t, hlnk);
		if (pVarLinkCallback->var.userdata4 == MATCH_CLASS_CALLBACKPTR) {
			return pVarLinkCallback;
		}
	}
	return ((void *)0);
}


rpa_varlink_t *rpa_dbex_first_callback(rpa_dbex_handle hDbex)
{
	return rpa_dbex_next_callback(hDbex, 0);
}


rpa_varlink_t *rpa_dbex_last_callback(rpa_dbex_handle hDbex)
{
	return rpa_dbex_prev_callback(hDbex, 0);	
}


int rpa_dbex_scan(rpa_dbex_handle hDbex, rpa_pattern_handle hPattern, const char *input, const char *start, const char *end, const char **where)
{
    if (!hDbex)
        return -1;
    if (!hPattern || !input || !start || !end) {
        hDbex->lastError = RPA_E_INVALID_PARAM;
        return -1;
    }

	return rpa_stat_scan(&hDbex->stat, hPattern, input, start, end, where);
}


int rpa_dbex_match(rpa_dbex_handle hDbex, rpa_pattern_handle hPattern, const char *input, const char *start, const char *end)
{
    if (!hDbex)
        return -1;
    if (!hPattern || !input || !start || !end) {
        hDbex->lastError = RPA_E_INVALID_PARAM;
        return -1;
    }

	return rpa_stat_match(&hDbex->stat, hPattern, input, start, end);
}


int rpa_dbex_parse(rpa_dbex_handle hDbex, rpa_pattern_handle hPattern, const char *input, const char *start, const char *end)
{
    if (!hDbex)
        return -1;
    if (!hPattern || !input || !start || !end) {
        hDbex->lastError = RPA_E_INVALID_PARAM;
        return -1;
    }

	return rpa_stat_parse(&hDbex->stat, hPattern, input, start, end);
}


int rpa_dbex_set_encoding(rpa_dbex_handle hDbex, unsigned int encoding)
{
    if (!hDbex)
		return -1;
    rpa_stat_set_encoding(&hDbex->stat, encoding);
    return 0;
}


int rpa_dbex_get_usedstack(rpa_dbex_handle hDbex)
{
    if (!hDbex)
        return -1;
    return rpa_stat_get_usedstack(&hDbex->stat);
}


int rpa_dbex_reset_usedstack(rpa_dbex_handle hDbex)
{
    if (!hDbex)
        return -1;
    return rpa_stat_reset_usedstack(&hDbex->stat);
}


int rpa_dbex_set_progress_callback(rpa_dbex_handle hDbex, rpa_progress_callback progress, void *userdata)
{
    if (!hDbex)
        return -1;
    return rpa_stat_set_progress_callback(&hDbex->stat, progress, userdata);
}


int rpa_dbex_abort(rpa_dbex_handle hDbex)
{
    if (!hDbex)
        return -1;
	return rpa_stat_abort(&hDbex->stat);
}


int rpa_dbex_dump_pattern(rpa_dbex_handle hDbex, rpa_pattern_handle hPattern)
{
    if (!hDbex)
        return -1;
	rpa_dump_pattern_tree(hPattern);
	return 0;
}


void *rpa_dbex_get_userdata(rpa_dbex_handle hDbex, unsigned int index)
{
    if (!hDbex)
        return 0;
	return rpa_stat_get_userdata(&hDbex->stat, index);
}


int rpa_dbex_set_userdata(rpa_dbex_handle hDbex, unsigned int index, void *ud)
{
    if (!hDbex)
        return -1;
	return rpa_stat_set_userdata(&hDbex->stat, index, ud);
}


const char *rpa_dbex_version()
{
	return RPA_VERSION_STRING;
}


const char *rpa_dbex_seversion()
{
	return rpa_dbex_search_version();
}
