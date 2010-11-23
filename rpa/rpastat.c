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

#include "rmem.h"
#include "rpamatch.h"
#include "rpamnode.h"
#include "rpamatchval.h"
#include "rpamatchrange.h"
#include "rpamatchstr.h"
#include "rpamatchlist.h"
#include "rpadbex.h"
#include "rpastat.h"
#include "rpavarlink.h"
#include "rpaconfig.h"
#include "rstring.h"

#include "rpasearch.h"


static RPA_MATCH_FUNCTION mtable_byte[] = {
	(void*) 0,
	rpa_match_list,					/* RPA_MATCHFUNC_LIST */
	rpa_match_list_alt,				/* RPA_MATCHFUNC_LIST_ALT */
	rpa_match_list_noalt, 			/* RPA_MATCHFUNC_LIST_NOALT */
	rpa_match_list_and,				/* RPA_MATCHFUNC_LIST_AND */
	rpa_match_list_minus,			/* RPA_MATCHFUNC_LIST_MINUS */
	rpa_match_list_not,				/* RPA_MATCHFUNC_LIST_NOT */
	rpa_match_list_contain,			/* RPA_MATCHFUNC_LIST_CONTAIN */
	rpa_match_list_at,				/* RPA_MATCHFUNC_LIST_AT */
	rpa_match_list_alt,				/* RPA_MATCHFUNC_NLIST_ALT */
	rpa_match_list_best_alt,		/* RPA_MATCHFUNC_NLIST_BESTALT */
	rpa_match_newline,				/* RPA_MATCHFUNC_NEWLINE */
	rpa_match_chreqany,				/* RPA_MATCHFUNC_CHREQANY */
	rpa_match_lstchr,				/* RPA_MATCHFUNC_LSTCHR */
	rpa_match_abort,				/* RPA_MATCHFUNC_ABORT */
	rpa_match_fail,					/* RPA_MATCHFUNC_FAIL */
	rpa_match_val_chreq,			/* RPA_MATCHFUNC_VAL_CHREQ */
	rpa_match_val_chrnoteq,			/* RPA_MATCHFUNC_VAL_CHRNOTEQ */
	rpa_match_range_chrinrange,		/* RPA_MATCHFUNC_RANGE_CHRINRANGE */
	rpa_match_range_chrnotinrange,	/* RPA_MATCHFUNC_RANGE_CHRNOTINRANGE */
	rpa_match_str,					/* RPA_MATCHFUNC_STR */
	rpa_match_str_chrinstr,			/* RPA_MATCHFUNC_STR_CHRINSTR */
	rpa_match_str_chrnotinstr,		/* RPA_MATCHFUNC_STR_CHRNOTINSTR */
	rpa_match_scan_byte,			/* RPA_MATCHFUNC_SCAN */
};


static RPA_MATCH_FUNCTION mtable_icase_byte[] = {
	(void*) 0,
	rpa_match_list_icase,					/* RPA_MATCHFUNC_LIST */
	rpa_match_list_alt_icase,				/* RPA_MATCHFUNC_LIST_ALT */
	rpa_match_list_noalt, 					/* RPA_MATCHFUNC_LIST_NOALT */
	rpa_match_list_and_icase,				/* RPA_MATCHFUNC_LIST_AND */
	rpa_match_list_minus_icase,				/* RPA_MATCHFUNC_LIST_MINUS */
	rpa_match_list_not,						/* RPA_MATCHFUNC_LIST_NOT */
	rpa_match_list_contain,					/* RPA_MATCHFUNC_LIST_CONTAIN */
	rpa_match_list_at,						/* RPA_MATCHFUNC_LIST_AT */	
	rpa_match_list_alt,						/* RPA_MATCHFUNC_NLIST_ALT */
	rpa_match_list_best_alt,				/* RPA_MATCHFUNC_NLIST_BESTALT */
	rpa_match_newline,						/* RPA_MATCHFUNC_NEWLINE */
	rpa_match_chreqany,						/* RPA_MATCHFUNC_CHREQANY */
	rpa_match_lstchr,						/* RPA_MATCHFUNC_LSTCHR */
	rpa_match_abort,						/* RPA_MATCHFUNC_ABORT */
	rpa_match_fail,							/* RPA_MATCHFUNC_FAIL */
	rpa_match_val_chreq_icase,				/* RPA_MATCHFUNC_VAL_CHREQ */
	rpa_match_val_chrnoteq_icase,			/* RPA_MATCHFUNC_VAL_CHRNOTEQ */
	rpa_match_range_chrinrange_icase,		/* RPA_MATCHFUNC_RANGE_CHRINRANGE */
	rpa_match_range_chrnotinrange_icase,	/* RPA_MATCHFUNC_RANGE_CHRNOTINRANGE */
	rpa_match_str_icase,					/* RPA_MATCHFUNC_STR */
	rpa_match_str_chrinstr,					/* RPA_MATCHFUNC_STR_CHRINSTR */
	rpa_match_str_chrnotinstr,				/* RPA_MATCHFUNC_STR_CHRNOTINSTR */
	rpa_match_scan_byte_icase,				/* RPA_MATCHFUNC_SCAN */
};


static RPA_MATCH_FUNCTION mtable_utf8[] = {
	(void*) 0,
	rpa_match_list,					/* RPA_MATCHFUNC_LIST */
	rpa_match_list_alt,				/* RPA_MATCHFUNC_LIST_ALT */
	rpa_match_list_noalt, 			/* RPA_MATCHFUNC_LIST_NOALT */
	rpa_match_list_and,				/* RPA_MATCHFUNC_LIST_AND */
	rpa_match_list_minus,			/* RPA_MATCHFUNC_LIST_MINUS */
	rpa_match_list_not,				/* RPA_MATCHFUNC_LIST_NOT */
	rpa_match_list_contain,			/* RPA_MATCHFUNC_LIST_CONTAIN */
	rpa_match_list_at,				/* RPA_MATCHFUNC_LIST_AT */
	rpa_match_list_alt,				/* RPA_MATCHFUNC_NLIST_ALT */
	rpa_match_list_best_alt,		/* RPA_MATCHFUNC_NLIST_BESTALT */
	rpa_match_newline,				/* RPA_MATCHFUNC_NEWLINE */
	rpa_match_chreqany,				/* RPA_MATCHFUNC_CHREQANY */
	rpa_match_lstchr,				/* RPA_MATCHFUNC_LSTCHR */
	rpa_match_abort,				/* RPA_MATCHFUNC_ABORT */
	rpa_match_fail,					/* RPA_MATCHFUNC_FAIL */
	rpa_match_val_chreq,			/* RPA_MATCHFUNC_VAL_CHREQ */
	rpa_match_val_chrnoteq,			/* RPA_MATCHFUNC_VAL_CHRNOTEQ */
	rpa_match_range_chrinrange,		/* RPA_MATCHFUNC_RANGE_CHRINRANGE */
	rpa_match_range_chrnotinrange,	/* RPA_MATCHFUNC_RANGE_CHRNOTINRANGE */
	rpa_match_str,					/* RPA_MATCHFUNC_STR */
	rpa_match_str_chrinstr,			/* RPA_MATCHFUNC_STR_CHRINSTR */
	rpa_match_str_chrnotinstr,		/* RPA_MATCHFUNC_STR_CHRNOTINSTR */
	rpa_match_scan_utf8,			/* RPA_MATCHFUNC_SCAN */
};


static RPA_MATCH_FUNCTION mtable_icase_utf8[] = {
	(void*) 0,
	rpa_match_list_icase,					/* RPA_MATCHFUNC_LIST */
	rpa_match_list_alt_icase,				/* RPA_MATCHFUNC_LIST_ALT */
	rpa_match_list_noalt, 					/* RPA_MATCHFUNC_LIST_NOALT */
	rpa_match_list_and_icase,				/* RPA_MATCHFUNC_LIST_AND */
	rpa_match_list_minus_icase,				/* RPA_MATCHFUNC_LIST_MINUS */
	rpa_match_list_not,						/* RPA_MATCHFUNC_LIST_NOT */
	rpa_match_list_contain,					/* RPA_MATCHFUNC_LIST_CONTAIN */
	rpa_match_list_at,						/* RPA_MATCHFUNC_LIST_AT */	
	rpa_match_list_alt,						/* RPA_MATCHFUNC_NLIST_ALT */
	rpa_match_list_best_alt,				/* RPA_MATCHFUNC_NLIST_BESTALT */
	rpa_match_newline,						/* RPA_MATCHFUNC_NEWLINE */
	rpa_match_chreqany,						/* RPA_MATCHFUNC_CHREQANY */
	rpa_match_lstchr,						/* RPA_MATCHFUNC_LSTCHR */
	rpa_match_abort,						/* RPA_MATCHFUNC_ABORT */
	rpa_match_fail,							/* RPA_MATCHFUNC_FAIL */
	rpa_match_val_chreq_icase,				/* RPA_MATCHFUNC_VAL_CHREQ */
	rpa_match_val_chrnoteq_icase,			/* RPA_MATCHFUNC_VAL_CHRNOTEQ */
	rpa_match_range_chrinrange_icase,		/* RPA_MATCHFUNC_RANGE_CHRINRANGE */
	rpa_match_range_chrnotinrange_icase,	/* RPA_MATCHFUNC_RANGE_CHRNOTINRANGE */
	rpa_match_str_icase,					/* RPA_MATCHFUNC_STR */
	rpa_match_str_chrinstr,					/* RPA_MATCHFUNC_STR_CHRINSTR */
	rpa_match_str_chrnotinstr,				/* RPA_MATCHFUNC_STR_CHRNOTINSTR */
	rpa_match_scan_utf8_icase,				/* RPA_MATCHFUNC_SCAN */	
};


static RPA_MATCH_FUNCTION mtable_utf16[] = {
	(void*) 0,
	rpa_match_list,					/* RPA_MATCHFUNC_LIST */
	rpa_match_list_alt,				/* RPA_MATCHFUNC_LIST_ALT */
	rpa_match_list_noalt, 			/* RPA_MATCHFUNC_LIST_NOALT */
	rpa_match_list_and,				/* RPA_MATCHFUNC_LIST_AND */
	rpa_match_list_minus,			/* RPA_MATCHFUNC_LIST_MINUS */
	rpa_match_list_not,				/* RPA_MATCHFUNC_LIST_NOT */
	rpa_match_list_contain,			/* RPA_MATCHFUNC_LIST_CONTAIN */
	rpa_match_list_at,				/* RPA_MATCHFUNC_LIST_AT */
	rpa_match_list_alt,				/* RPA_MATCHFUNC_NLIST_ALT */
	rpa_match_list_best_alt,		/* RPA_MATCHFUNC_NLIST_BESTALT */
	rpa_match_newline,				/* RPA_MATCHFUNC_NEWLINE */
	rpa_match_chreqany,				/* RPA_MATCHFUNC_CHREQANY */
	rpa_match_lstchr,				/* RPA_MATCHFUNC_LSTCHR */
	rpa_match_abort,				/* RPA_MATCHFUNC_ABORT */
	rpa_match_fail,					/* RPA_MATCHFUNC_FAIL */
	rpa_match_val_chreq,			/* RPA_MATCHFUNC_VAL_CHREQ */
	rpa_match_val_chrnoteq,			/* RPA_MATCHFUNC_VAL_CHRNOTEQ */
	rpa_match_range_chrinrange,		/* RPA_MATCHFUNC_RANGE_CHRINRANGE */
	rpa_match_range_chrnotinrange,	/* RPA_MATCHFUNC_RANGE_CHRNOTINRANGE */
	rpa_match_str,					/* RPA_MATCHFUNC_STR */
	rpa_match_str_chrinstr,			/* RPA_MATCHFUNC_STR_CHRINSTR */
	rpa_match_str_chrnotinstr,		/* RPA_MATCHFUNC_STR_CHRNOTINSTR */
	rpa_match_scan_utf16,			/* RPA_MATCHFUNC_SCAN */
};


static RPA_MATCH_FUNCTION mtable_icase_utf16[] = {
	(void*) 0,
	rpa_match_list_icase,					/* RPA_MATCHFUNC_LIST */
	rpa_match_list_alt_icase,				/* RPA_MATCHFUNC_LIST_ALT */
	rpa_match_list_noalt, 					/* RPA_MATCHFUNC_LIST_NOALT */
	rpa_match_list_and_icase,				/* RPA_MATCHFUNC_LIST_AND */
	rpa_match_list_minus_icase,				/* RPA_MATCHFUNC_LIST_MINUS */
	rpa_match_list_not,						/* RPA_MATCHFUNC_LIST_NOT */
	rpa_match_list_contain,					/* RPA_MATCHFUNC_LIST_CONTAIN */
	rpa_match_list_at,						/* RPA_MATCHFUNC_LIST_AT */
	rpa_match_list_alt,						/* RPA_MATCHFUNC_NLIST_ALT */
	rpa_match_list_best_alt,				/* RPA_MATCHFUNC_NLIST_BESTALT */
	rpa_match_newline,						/* RPA_MATCHFUNC_NEWLINE */
	rpa_match_chreqany,						/* RPA_MATCHFUNC_CHREQANY */
	rpa_match_lstchr,						/* RPA_MATCHFUNC_LSTCHR */
	rpa_match_abort,						/* RPA_MATCHFUNC_ABORT */
	rpa_match_fail,							/* RPA_MATCHFUNC_FAIL */
	rpa_match_val_chreq_icase,				/* RPA_MATCHFUNC_VAL_CHREQ */
	rpa_match_val_chrnoteq_icase,			/* RPA_MATCHFUNC_VAL_CHRNOTEQ */
	rpa_match_range_chrinrange_icase,		/* RPA_MATCHFUNC_RANGE_CHRINRANGE */
	rpa_match_range_chrnotinrange_icase,	/* RPA_MATCHFUNC_RANGE_CHRNOTINRANGE */
	rpa_match_str_icase,					/* RPA_MATCHFUNC_STR */
	rpa_match_str_chrinstr,					/* RPA_MATCHFUNC_STR_CHRINSTR */
	rpa_match_str_chrnotinstr,				/* RPA_MATCHFUNC_STR_CHRNOTINSTR */
	rpa_match_scan_utf16_icase,				/* RPA_MATCHFUNC_SCAN */
};


static RPA_MNODE_FUNCTION ntable_match[] = {
	rpa_mnode_plain,
	rpa_mnode_multiple,
	rpa_mnode_optional,
	rpa_mnode_multiopt,

/*	callback */
	rpa_mnode_callback_plain,
	rpa_mnode_callback_multiple,
	rpa_mnode_callback_optional,
	rpa_mnode_callback_multiopt,

};


static RPA_MNODE_FUNCTION ntable_parse[] = {
	rpa_mnode_p_plain,
	rpa_mnode_p_multiple,
	rpa_mnode_p_optional,
	rpa_mnode_p_multiopt,

/*	callback */
	rpa_mnode_p_callback_plain,
	rpa_mnode_p_callback_multiple,
	rpa_mnode_p_callback_optional,
	rpa_mnode_p_callback_multiopt,
};


int rpa_stat_utf8_getchar(unsigned int *pwc, rpa_stat_t *stat, const char *input)
{
	return rpa_utf8_mbtowc(pwc, (const unsigned char*)input, (const unsigned char*)stat->end);
}


int rpa_stat_byte_getchar(unsigned int *pwc, rpa_stat_t *stat, const char *input)
{
	if (input >= stat->end) {
		*pwc = (unsigned int)0;
		return 0;
	}
	*pwc = *((const unsigned char*)input);
	return 1;
	
}


int rpa_stat_utf16_getchar(unsigned int *pwc, rpa_stat_t *stat, const char *input)
{
	return rpa_utf16_mbtowc(pwc, (const unsigned char*)input, (const unsigned char*)stat->end);
}


int rpa_stat_getchar(unsigned int *pwc, rpa_stat_t *stat, const char *input)
{
	return stat->getchar(pwc, stat, input);
}


void rpa_stat_init(rpa_stat_t *stat)
{
	int i;
	unsigned char s1 = 'a';
	unsigned char s2 = 'b';
	
	r_memset(stat, 0, sizeof(*stat));
	for (i = 0; i < RPA_LOOPHASH_SIZE; i++)
		rpa_list_init(&stat->loophash[i]);
	rpa_list_init(&stat->loopstack);
	stat->ntable = ntable_match;
	rpa_stat_set_encoding(stat, RPA_ENCODING_UTF8);
	rpa_cbset_init(&stat->cbset);
	stat->maxstack = RPA_MAXSTACK_SIZE;
	if (&s1 < &s2)
		stat->checkstack = rpa_stat_checkstack_ascending;
	else
		stat->checkstack = rpa_stat_checkstack_descending;
}


void rpa_stat_cleanup(rpa_stat_t *stat)
{
	rpa_cbset_cleanup(&stat->cbset);
}


int rpa_stat_set_encoding(rpa_stat_t *stat, unsigned int encoding)
{
	if (rpase_stat_set_encoding(stat, encoding) >= 0)
		return 0;
	
	if (encoding == RPA_ENCODING_BYTE) {
		stat->mtable = mtable_byte;
		stat->getchar = rpa_stat_byte_getchar;
	} else if (encoding == RPA_ENCODING_ICASE_BYTE) {
		stat->mtable = mtable_icase_byte;
		stat->getchar = rpa_stat_byte_getchar;
	} else if (encoding == RPA_ENCODING_UTF8) {
		stat->mtable = mtable_utf8;
		stat->getchar = rpa_stat_utf8_getchar;
	} else if (encoding == RPA_ENCODING_ICASE_UTF8) {
		stat->mtable = mtable_icase_utf8;
		stat->getchar = rpa_stat_utf8_getchar;
	} else if (encoding == RPA_ENCODING_UTF16LE) {
		stat->mtable = mtable_utf16;
		stat->getchar = rpa_stat_utf16_getchar;
	} else if (encoding == RPA_ENCODING_ICASE_UTF16LE) {
		stat->mtable = mtable_icase_utf16;
		stat->getchar = rpa_stat_utf16_getchar;
	} else {
		return -1;
	}
	return 0;
}


rpa_stat_handle rpa_stat_create(rpa_dbex_handle hDbex)
{
	rpa_stat_t *stat;
	
	if (!hDbex)
		return (void*)0;
	stat = (rpa_stat_t *)r_malloc(sizeof(*stat));
	if (!stat)
		return ((void*)0);
	r_memset(stat, 0, sizeof(*stat));
	rpa_stat_init(stat);
	return stat;
}


void rpa_stat_destroy(rpa_stat_handle hStat)
{
	if (hStat)
		rpa_stat_cleanup(hStat);
	r_free(hStat);
}


int rpa_stat_scan(rpa_stat_handle hStat, rpa_pattern_handle hPattern, const char *input, const char *start, const char *end, const char **where)
{
	const unsigned char smk = RPA_STACK_MARK;
	rpa_match_t *match;
	int ret;
	
	if (!hStat)
		return -1;
	if (!hPattern || !input || !start || !end || input < start) {
		/*
		 * Set last error
		 */
		return -1;
	}
	hStat->ntable = ntable_match;
	hStat->start = start;
	hStat->end = end;
	hStat->stackmark = &smk;
	match = (rpa_match_t *)hPattern->var.v.ptr;
	rpa_stat_cache_reset(hStat);
	ret = hStat->mtable[RPA_MATCHFUNC_SCAN](match, hStat, input);
	if (ret && where)	
		*where = hStat->where;
	return ret;
}


int rpa_stat_match(rpa_stat_handle hStat, rpa_pattern_handle hPattern, const char *input, const char *start, const char *end)
{
	const unsigned char smk = RPA_STACK_MARK;
	rpa_match_t *match;
	int ret;
	
	if (!hStat)
		return -1;
	if (!hPattern || !input || !start || !end || input < start) {
		/*
		 * Set last error
		 */
		return -1;
	}
	hStat->ntable = ntable_match;
	hStat->start = start;
	hStat->end = end;
	hStat->fail = 0;
	hStat->stackmark = &smk;
	rpa_stat_cache_reset(hStat);
	match = (rpa_match_t *)hPattern->var.v.ptr;
	ret = hStat->mtable[match->match_function_id](match, hStat, input);
	return ret;
}


int rpa_stat_match_lite(rpa_stat_handle hStat, rpa_pattern_handle hPattern, const char *input, const char *start, const char *end)
{
	const unsigned char smk = RPA_STACK_MARK;
	rpa_match_t *match;
	int ret;
	
	if (!hStat)
		return -1;
	if (!hPattern || !input || !start || !end || input < start) {
		/*
		 * Set last error
		 */
		return -1;
	}
	hStat->ntable = ntable_match;
	hStat->start = start;
	hStat->end = end;
	hStat->fail = 0;
	hStat->stackmark = &smk;
	match = (rpa_match_t *)hPattern->var.v.ptr;
	ret = hStat->mtable[match->match_function_id](match, hStat, input);
	return ret;
}


static int rpa_stat_play_cbset(rpa_stat_t *stat, const char *input, unsigned int size)
{
	rpa_cbset_t *cbset = &stat->cbset;
	rpa_word_t off;
	int ret;

	for (off = 1; off <= cbset->off; off++) {
		rpa_cbrecord_t *cbrec = &cbset->data[off];
		ret = rpa_mnode_exec_callback(cbrec->mnode, stat, cbrec->input, cbrec->size, RPA_REASON_START);
		if (!ret)
			return 0;
		ret = rpa_mnode_exec_callback(cbrec->mnode, stat, cbrec->input, cbrec->size, RPA_REASON_MATCHED | RPA_REASON_END);
		if (!ret)
			return 0;
	}
	return size;
}


int rpa_stat_parse(rpa_stat_handle hStat, rpa_pattern_handle hPattern, const char *input, const char *start, const char *end)
{
	const unsigned char smk = RPA_STACK_MARK;
	rpa_match_t *match;
	int ret;
	
	if (!hStat)
		return -1;
	if (!hPattern || !input || !start || !end || input < start) {
		/*
		 * Set last error
		 */
		return -1;
	}
	hStat->ntable = ntable_parse;
	hStat->start = start;
	hStat->end = end;
	hStat->fail = 0;
	hStat->stackmark = &smk;
	match = (rpa_match_t *)hPattern->var.v.ptr;
	rpa_stat_cache_reset(hStat);
	ret = hStat->mtable[match->match_function_id](match, hStat, input);
	if (ret) {
		ret = rpa_stat_play_cbset(hStat, input, ret);
	}
	return ret;
}


int rpa_stat_get_usedstack(rpa_stat_handle stat)
{
	if (!stat)
		return -1;
		
	return stat->usedstack;
}


int rpa_stat_reset_usedstack(rpa_stat_handle hStat)
{
	if (!hStat)
		return -1;
	hStat->usedstack = 0;
	return 0;
}


int rpa_stat_set_maxstack(rpa_stat_handle hStat, unsigned long size)
{
	if (!hStat)
		return -1;
	hStat->maxstack = size;
	return 0;
}


int rpa_stat_checkstack_ascending(rpa_stat_t *stat)
{
	unsigned long usedstack;
	unsigned char *curstackmark = (unsigned char*)&stat;

	if (curstackmark < stat->stackmark)
		return 1;
	usedstack = (int)(curstackmark - stat->stackmark);
	if (usedstack > stat->usedstack)
			stat->usedstack = usedstack;
#ifndef RPANOSTACKCHEK
	if (stat->usedstack > stat->maxstack)
		return 0;
#endif
	return 1;
}


int rpa_stat_checkstack_descending(rpa_stat_t *stat)
{
	unsigned long usedstack;
	unsigned char *curstackmark = (unsigned char*)&stat;

	if (curstackmark > stat->stackmark)
		return 1;
	usedstack = (int)(stat->stackmark - curstackmark);
	if (usedstack > stat->usedstack)
			stat->usedstack = usedstack;
#ifndef RPANOSTACKCHEK
	if (stat->usedstack > stat->maxstack)
		return 0;
#endif
	return 1;
}


int rpa_stat_set_progress_callback(rpa_stat_handle hStat, rpa_progress_callback progress, void *userdata)
{
	hStat->progress = progress;
	hStat->progress_userdata = userdata;
	return 0;
}


int rpa_stat_abort(rpa_stat_handle hStat)
{
	hStat->end = 0;
	return 0;
}


void *rpa_stat_get_userdata(rpa_stat_handle stat, unsigned int index)
{
	if (index >= (sizeof(stat->ud)/sizeof(stat->ud[0])))
		return (void*)-1;
	return (void*)stat->ud[index];
}


int rpa_stat_set_userdata(rpa_stat_handle stat, unsigned int index, void *ud)
{
	if (index >= (sizeof(stat->ud)/sizeof(stat->ud[0])))
		return -1;
	stat->ud[index] = (rpa_word_t)ud;
	return 0;
}


rpa_dloop_t *rpa_stat_current_loop(rpa_stat_t *stat)
{
	rpa_link_t *pos;

	if ((pos = rpa_list_last(&stat->loopstack))) {
		return rpa_list_entry(pos, rpa_dloop_t, lnk);
	}
	return (void*)0;
}

void rpa_stat_cache_reset(rpa_stat_t *stat)
{
	int i;

	stat->highbound = 0;
	stat->usecache = 1;
	for (i = 0; i < RPA_MCACHE_SIZE; i++) {
		stat->mcache[i].match = NULL;
	}
}


void rpa_stat_cache_cbreset(rpa_stat_t *stat, rpa_word_t offset)
{
	int i;
	
	if (offset <= stat->highbound) {
		for (i = 0; i < RPA_MCACHE_SIZE; i++) {
			if (stat->mcache[i].cboffset >= offset)
				stat->mcache[i].match = NULL;
		}
	}
	stat->highbound = offset;
}

