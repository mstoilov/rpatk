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

#ifndef _RPADBEXPRIV_H_
#define _RPADBEXPRIV_H_

#include "rpalist.h"
#include "rpadbex.h"
#include "rpavarlink.h"
#include "rpamatch.h"
#include "rpamatchval.h"
#include "rpastat.h"
#include "rpaparser.h"
#include "rpavm.h"
#include "rpatypedef.h"

#define RPA_MATCH_NEWLINE_CLASSID	(('\0' << 24) | ('n' << 16) | ('l' << 8) | ('n' << 0))
#define RPA_MATCH_LASTCHR_CLASSID	(('\0' << 24) | ('c' << 16) | ('s' << 8) | ('l' << 0))
#define RPA_NAMEHASH_SIZE 4096
#define RPA_VALHASH_SIZE 128
#define RPA_PREALLOC_VAL_COUNT 128

#define VM_CREATE_VAL_MATCHPTR 0
#define VM_CREATE_LIST_MATCHPTR 1
#define VM_CREATE_NLIST_MATCHPTR 2
#define VM_GET_NLIST_MATCHPTR 3
#define VM_CREATE_NEWLINE_MATCHPTR 4
#define VM_CREATE_ANYCHAR_MATCHPTR 5
#define VM_CREATE_STR_MATCHPTR 6
#define VM_CREATE_RANGE_MATCHPTR 7
#define VM_CREATE_MNODE 8
#define VM_CREATE_MNODE_CALLBACK 9
#define VM_VARLINK_PTR 10
#define VM_ADD_MNODE_TO_LIST 11
#define VM_SET_MATCH_FUNCTION 12
#define VM_DUMP_TREE 13
#define VM_SET_MATCH_NAME 14
#define VM_SET_DEFAULT_PATTERN 15
#define VM_SETUP_LIST 16
#define VM_ADD_MATCH_TO_DATAPTR 17
#define VM_CREATE_HLIST_MATCHPTR 18
#define VM_CREATE_FUNLAST_MATCHPTR 19
#define VM_CREATE_FUNABORT_MATCHPTR 20
#define VM_CREATE_FUNFAIL_MATCHPTR 21
#define VM_SET_STRVAL 22
#define VM_NOOP_23 23 
#define VM_ADD_MNODE_TO_LIST_DATAPTR 24
#define VM_GET_MATCH_FUNCTION 25
#define VM_RESET_LIST 26
#define VM_CREATE_RANGELIST_MATCHPTR 27


struct rpa_dbex_s {
	rpa_head_t treehead;
	rpa_head_t *namehash;
	unsigned int namehashEntries;
	rpa_head_t callbacks;
	rpa_head_t callbackmnodes;
	rpa_stat_t stat;
	unsigned int rwlock;
	rpa_pattern_handle defaultPattern;
	unsigned int lastError;
	rpa_parser_t *parser;
	rpa_vm_t *vm;
	rpa_varlink_t *pVarLinkValMatchPtr[RPA_PREALLOC_VAL_COUNT];
	void *dataptr;
};


typedef struct rpa_callbackdata_s {
	unsigned int reason;
	unsigned int namematchsiz;
	rpa_match_callback func;
	int exact;
	void *userdata;
	char namematch[1];
} rpa_callbackdata_t;


rpa_varlink_t *rpa_dbex_next_callback(rpa_dbex_handle hDbex, rpa_varlink_t *cur);
rpa_varlink_t *rpa_dbex_prev_callback(rpa_dbex_handle hDbex, rpa_varlink_t *cur);
int rpa_dbex_build(rpa_dbex_handle hDbex, const char *input);
void rpa_dbex_cleanup_buildstack(rpa_dbex_handle hDbex);
rpa_mnode_t *rpa_dbex_build_tree(rpa_dbex_handle hDbex);
int rpa_dbex_check_parser(rpa_dbex_handle hDbex);
int rpa_dbex_check_vm(rpa_dbex_handle hDbex);
rpa_varlink_t *rpa_dbex_create_mnode(rpa_dbex_handle hDbex, rpa_match_t *match, unsigned int flags);
int rpa_dbex_int_ascii_matchptr_array(rpa_dbex_handle hDbex);
rpa_dbex_handle rpa_dbex_init(rpa_dbex_handle hDbex, unsigned int namehashEntries);
void rpa_dbex_close_do(rpa_dbex_handle hDbex);
void rpa_dbex_cleanup(rpa_dbex_handle hDbex);
void rpa_dbex_connect_callbacks(rpa_dbex_handle hDbex);
#endif
