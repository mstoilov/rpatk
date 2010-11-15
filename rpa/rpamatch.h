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

#ifndef _RPAMATCH_H_
#define _RPAMATCH_H_

#include <stdarg.h>
#include "rpaclass.h"
#include "rpalist.h"
#include "rpatypes.h"
#include "rpawordstack.h"
#include "rpatypedef.h"

#ifdef __cplusplus
extern "C" {
#endif

/* These are the regular expression (regex) occurence flags:
 * RPA_MATCH_NONE     - no flags, the pattern will be matched only once
 * RPA_MATCH_MULTIPLE - (regex +) multiple flag means the pattern will be matched at least once,
 *     but if it occurs more then once the consecutive occurrances will also be matched. If the regex
 *     has something like .+ (match anything) it will match absolutely anything until the end of the
 *     buffer is reached.
 * RPA_MATCH_OPTIONAL - (regex ?) optional flag means the pattern might not be matched. If the pattern
 *     is not encountered, the matching algorithm moves on to the next pattern.
 * RPA_MATCH_OPTIONAL | RPA_MATCH_MULTIPLE - (regex *) The pattern might be encountered once, more than
 *     once or not at all.
 * 
 * Short description of regex metachars:
 * ? - zero or one  (RPA_MATCH_OPTIONAL)
 * + - one or more  (RPA_MATCH_MULTIPLE)
 * * - zero or more (RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE)
 */
#define RPA_MATCH_NONE 0
#define RPA_MATCH_MULTIPLE (1 << 0)
#define RPA_MATCH_OPTIONAL (1 << 1)
#define RPA_MATCH_MULTIOPT (RPA_MATCH_MULTIPLE | RPA_MATCH_OPTIONAL)
#define RPA_MATCH_MASK RPA_MATCH_MULTIOPT
#define RPA_MNODE_CALLBACK (1 << 2)
#define RPA_MNODE_LOOP (1 << 3)
#define RPA_MNODEFUNC_MASK ((1 << 3) - 1)
#define RPA_MNODE_SYNCRONOUS (1 << 4)
#define RPA_MNODE_NOCONNECT (1 << 5)
/* This definition is used to initialize a const on the stack that will be used
 * for stack usage measurement. The content of the const is not important and it
 * is not used in the measurement algorithm. It might be used for debugging
 * purposes though.
 */
#define RPA_STACK_MARK	'M'
#define RPA_MIN_PROGRESS 1024

typedef enum rpa_matchfunc_s {
	RPA_MATCHFUNC_NONE = 0,
	RPA_MATCHFUNC_LIST,
	RPA_MATCHFUNC_LIST_ALT,
	RPA_MATCHFUNC_LIST_NOALT,	
	RPA_MATCHFUNC_LIST_AND,
	RPA_MATCHFUNC_LIST_MINUS,
	RPA_MATCHFUNC_LIST_NOT,
	RPA_MATCHFUNC_LIST_CONTAIN,
	RPA_MATCHFUNC_LIST_AT,
	RPA_MATCHFUNC_NLIST_ALT,
	RPA_MATCHFUNC_NLIST_BESTALT,
	RPA_MATCHFUNC_NEWLINE,
	RPA_MATCHFUNC_CHREQANY,
	RPA_MATCHFUNC_LSTCHR,
	RPA_MATCHFUNC_ABORT,
	RPA_MATCHFUNC_FAIL,
	RPA_MATCHFUNC_VAL_CHREQ,
	RPA_MATCHFUNC_VAL_CHRNOTEQ,
	RPA_MATCHFUNC_RANGE_CHRINRANGE,
	RPA_MATCHFUNC_RANGE_CHRNOTINRANGE,
	RPA_MATCHFUNC_STR,
	RPA_MATCHFUNC_STR_CHRINSTR,
	RPA_MATCHFUNC_STR_CHRNOTINSTR,
	RPA_MATCHFUNC_SCAN,
} rpa_matchfunc_t;


enum rpa_match_class_e {
	MATCH_CLASS_NONE = 0,
	MATCH_CLASS_MATCHPTR = 1,
	MATCH_CLASS_NAMEDMATCHPTR,
	MATCH_CLASS_MNODEPTR,
	MATCH_CLASS_CALLBACKPTR,
	MATCH_CLASS_DATAPTR,
};


struct rpa_match_s {
	rpa_class_t cls;
	char *name;
	unsigned int namesiz;
	unsigned int match_function_id;
};


rpa_match_t *rpa_match_init(
	rpa_match_t *match, 
	const char *name,
	unsigned int namesize,
	rpa_matchfunc_t match_function_id,
	rpa_class_methods_t *vptr);
rpa_match_t *rpa_match_create_namesize(
	const char *name,
	unsigned int namesiz,
	rpa_matchfunc_t match_function_id);
rpa_match_t *rpa_match_create(
	const char *name,
	rpa_matchfunc_t match_function_id);
void rpa_match_setup_name(
	rpa_match_t *match, 
	const char *name,
	unsigned int namesiz);
void rpa_match_cleanup(rpa_match_t *match);
void rpa_match_clear_cache(rpa_match_t *match);
void rpa_match_set_mathfunc(rpa_match_t *match, rpa_matchfunc_t match_function_id);
int rpa_match_exec(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_exec_nocache(rpa_match_t *match, rpa_stat_t *stat, const char *input);

int rpa_match_newline(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_lstchr(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_abort(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_fail(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_chreqany(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_scan_byte(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_scan_byte_icase(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_scan_utf8(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_scan_utf8_icase(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_scan_utf16(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_scan_utf16_icase(rpa_match_t *match, rpa_stat_t *stat, const char *input);

rpa_word_t rpa_sdbm_hash_3(rpa_word_t c1, rpa_word_t c2, rpa_word_t c3);
#ifdef __cplusplus
}
#endif

#endif
