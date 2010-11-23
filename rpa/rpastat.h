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

#ifndef _RPASTAT_H_
#define _RPASTAT_H_

#include "rpaconfig.h"
#include "rpaclass.h"
#include "rpalist.h"
#include "rpautf.h"
#include "rpatypes.h"
#include "rpatypedef.h"
#include "rpadbex.h"
#include "rpacbrecord.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RPA_LOOPHASH_SIZE (1 << RPA_LOOPHASH_BITS)
#define RPA_LOOPHASH_MASK (RPA_LOOPHASH_SIZE - 1)
#define RPA_LOOPHASH(p) (((((rpa_word_t)(p)) >> 2) ^ (((rpa_word_t)(p)) >> 4))& RPA_LOOPHASH_MASK)


#define RPA_MCACHE_BITS 4
#define RPA_MCACHE_SIZE (1 << RPA_MCACHE_BITS)
#define RPA_MCACHE_MASK (RPA_MCACHE_SIZE - 1)
#define RPA_MCACHEHASH(p) ( ( (((rpa_word_t)(p)) >> 4) ^ (((rpa_word_t)(p)) >> 2) ) & RPA_MCACHE_MASK)
#define RPA_MCACHE_SET(_c_, _m_, _i_, _r_) do {(_c_)->match = (_m_); (_c_)->input = (_i_); (_c_)->ret = (_r_);} while (0)
#define RPA_MCACHE_CBSET(_c_, _m_, _i_, _r_, _o_) \
	do {(_c_)->match = (_m_); (_c_)->input = (_i_); (_c_)->ret = (_r_); (_c_)->cboffset = (_o_); } while (0)

typedef int (*RPA_CHECKSTACK_FUNCTION)(rpa_stat_t *stat);


typedef struct rpa_dloop_s {
	rpa_link_t lnk;
	rpa_match_t *match;
	const char *input;
	int size;
	rpa_mnode_t *mnode;
} rpa_dloop_t;


typedef struct rpa_mcache_s {
	rpa_match_t *match;
	const char *input;
	int ret;
	rpa_word_t cboffset;
} rpa_mcache_t;


struct rpa_stat_s {
	rpa_dbex_t *hDbex;
	/* 
	 * start and end members mark the beginning and the ending of the
	 * input stream.
	 */
	const char *start;
	const char *end;
	unsigned char fail;
	unsigned long usedstack;
	unsigned long maxstack;
	const unsigned char *stackmark;
	rpa_word_t highbound;
	rpa_mcache_t mcache[RPA_MCACHE_SIZE];
	rpa_head_t loophash[RPA_LOOPHASH_SIZE];
	rpa_head_t loopstack;
	rpa_dloop_t *currentloop;
	rpa_word_t ud[2];
	RPA_MATCH_FUNCTION *mtable;
	RPA_MNODE_FUNCTION *ntable;
	RPA_CHECKSTACK_FUNCTION checkstack;
	rpa_progress_callback progress;
	rpa_cbset_t cbset;
	void *progress_userdata;
	int (*getchar)(unsigned int *pwc, rpa_stat_t *stat, const char *input);
	const char *where;
	unsigned char usecache;	
};


int rpa_stat_getchar(unsigned int *pwc, rpa_stat_t *stat, const char *input);
int rpa_stat_utf8_getchar(unsigned int *pwc, rpa_stat_t *stat, const char *input);
int rpa_stat_utf16_getchar(unsigned int *pwc, rpa_stat_t *stat, const char *input);
int rpa_stat_byte_getchar(unsigned int *pwc, rpa_stat_t *stat, const char *input);
void rpa_stat_init(rpa_stat_t *stat);
void rpa_stat_cleanup(rpa_stat_t *stat);
int rpa_stat_checkstack_ascending(rpa_stat_t *stat);
int rpa_stat_checkstack_descending(rpa_stat_t *stat);
rpa_dloop_t *rpa_stat_current_loop(rpa_stat_t *stat);
void rpa_stat_cache_reset(rpa_stat_t *stat);
void rpa_stat_cache_cbreset(rpa_stat_t *stat, rpa_word_t offset);
int rpa_stat_match_lite(rpa_stat_handle hStat, rpa_pattern_handle hPattern, const char *input, const char *start, const char *end);

#ifdef __cplusplus
}
#endif

#endif
