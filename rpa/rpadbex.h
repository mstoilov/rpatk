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

#ifndef _RPADBEX_H_
#define _RPADBEX_H_


#ifdef __cplusplus
extern "C" {
#endif

#define RPA_ENCODING_ICASE 1
#define RPA_ENCODING_BYTE 0
#define RPA_ENCODING_ICASE_BYTE (RPA_ENCODING_BYTE | RPA_ENCODING_ICASE)
#define RPA_ENCODING_UTF8 2
#define RPA_ENCODING_ICASE_UTF8 (RPA_ENCODING_UTF8 | RPA_ENCODING_ICASE)
#define RPA_ENCODING_UTF16LE 4
#define RPA_ENCODING_ICASE_UTF16LE (RPA_ENCODING_UTF16LE | RPA_ENCODING_ICASE)

#define RPA_PATTERN_SINGLE 0
#define RPA_PATTERN_BEST 1
#define RPA_PATTERN_OR 2


/* 
 * Callback reasons, the callback will set the reason for the callback
 * RPA_REASON_END and RPA_REASON_MATCHED can be combined and delivered
 * in the same call, if there is a match.
 */
#define RPA_REASON_START 	(1 << 5)
#define RPA_REASON_END	 	(1 << 6)
#define RPA_REASON_MATCHED	(1 << 7)
#define RPA_REASON_ALL		(RPA_REASON_START|RPA_REASON_END|RPA_REASON_MATCHED)


typedef struct rpa_stat_s *rpa_stat_handle;
typedef struct rpa_dbex_s *rpa_dbex_handle;
typedef struct rpa_varlink_s *rpa_group_handle;
typedef struct rpa_varlink_s *rpa_pattern_handle;
typedef struct rpa_varlink_s *rpa_callback_handle;
typedef void (*rpa_progress_callback)(void *userdata, const char *input, const char *start, const char *end);
typedef int (*rpa_match_callback)(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end);


rpa_dbex_handle rpa_dbex_create(void);
rpa_dbex_handle rpa_dbex_create_with_hash(unsigned int uHashEntries);
void rpa_dbex_destroy(rpa_dbex_handle hDbex);
unsigned int rpa_dbex_get_error(rpa_dbex_handle hDbex);
int rpa_dbex_add_callback(rpa_dbex_handle hDbex, const char *namematch, unsigned int reason, rpa_match_callback func, void *userdata);
int rpa_dbex_open(rpa_dbex_handle hDbex);
void rpa_dbex_close(rpa_dbex_handle hDbex);
int rpa_dbex_load(rpa_dbex_handle hDbex, const char *patterns, unsigned int size);
int rpa_dbex_load_string(rpa_dbex_handle hDbex, const char *patterns);
int rpa_dbex_pattern_type(rpa_dbex_handle hDbex, rpa_pattern_handle hPattern);
rpa_pattern_handle rpa_dbex_get_pattern(rpa_dbex_handle hDbex, const char *name);
rpa_pattern_handle rpa_dbex_default_pattern(rpa_dbex_handle hDbex);
rpa_pattern_handle rpa_dbex_next_pattern(rpa_dbex_handle hDbex, rpa_pattern_handle hCur);
rpa_pattern_handle rpa_dbex_prev_pattern(rpa_dbex_handle hDbex, rpa_pattern_handle hCur);
rpa_pattern_handle rpa_dbex_first_pattern(rpa_dbex_handle hDbex);
rpa_pattern_handle rpa_dbex_last_pattern(rpa_dbex_handle hDbex);
rpa_callback_handle rpa_dbex_next_callback(rpa_dbex_handle hDbex, rpa_callback_handle hCur);
rpa_callback_handle rpa_dbex_prev_callback(rpa_dbex_handle hDbex, rpa_callback_handle hCur);
rpa_callback_handle rpa_dbex_first_callback(rpa_dbex_handle hDbex);
rpa_callback_handle rpa_dbex_last_callback(rpa_dbex_handle hDbex);
const char *rpa_dbex_callback_pattern(rpa_dbex_handle hDbex, rpa_callback_handle hCallback);
void *rpa_dbex_callback_userdata(rpa_dbex_handle hDbex, rpa_callback_handle hCallback);
const char *rpa_dbex_pattern_name(rpa_dbex_handle hDbex, rpa_pattern_handle hPattern);
const char *rpa_dbex_pattern_regex(rpa_dbex_handle hDbex, rpa_pattern_handle hPattern, int seq);
int rpa_dbex_strmatch(const char *str, const char *patterns);
int rpa_dbex_scan(rpa_dbex_handle hDbex, rpa_pattern_handle hPattern, const char *input, const char *start, const char *end, const char **where);
int rpa_dbex_match(rpa_dbex_handle hDbex, rpa_pattern_handle hPattern, const char *input, const char *start, const char *end);
int rpa_dbex_parse(rpa_dbex_handle hDbex, rpa_pattern_handle hPattern, const char *input, const char *start, const char *end);
int rpa_dbex_set_encoding(rpa_dbex_handle hDbex, unsigned int encoding);
int rpa_dbex_get_usedstack(rpa_dbex_handle hDbex);
int rpa_dbex_reset_usedstack(rpa_dbex_handle hDbex);
int rpa_dbex_set_progress_callback(rpa_dbex_handle hDbex, rpa_progress_callback progress, void *userdata);
int rpa_dbex_abort(rpa_dbex_handle hDbex);
int rpa_dbex_dump_pattern(rpa_dbex_handle hDbex, rpa_pattern_handle hPattern);
void *rpa_dbex_get_userdata(rpa_dbex_handle hDbex, unsigned int index);
int rpa_dbex_set_userdata(rpa_dbex_handle hDbex, unsigned int index, void *ud);
const char *rpa_dbex_version();
const char *rpa_dbex_seversion();

rpa_stat_handle rpa_stat_create(rpa_dbex_handle hDbex);
void rpa_stat_destroy(rpa_stat_handle hStat);
int rpa_stat_set_encoding(rpa_stat_handle hStat, unsigned int encoding);
int rpa_stat_reset_usedstack(rpa_stat_handle hStat);
int rpa_stat_get_usedstack(rpa_stat_handle hStat);
int rpa_stat_set_maxstack(rpa_stat_handle hStat, unsigned long size);
int rpa_stat_scan(rpa_stat_handle hStat, rpa_pattern_handle hPattern, const char *input, const char *start, const char *end, const char **where);
int rpa_stat_match(rpa_stat_handle hStat, rpa_pattern_handle hPattern, const char *input, const char *start, const char *end);
int rpa_stat_parse(rpa_stat_handle hStat, rpa_pattern_handle hPattern, const char *input, const char *start, const char *end);
int rpa_stat_set_progress_callback(rpa_stat_handle hStat, rpa_progress_callback progress, void *userdata);
int rpa_stat_abort(rpa_stat_handle hStat);
void *rpa_stat_get_userdata(rpa_stat_handle hStat, unsigned int index);
int rpa_stat_set_userdata(rpa_stat_handle hStat, unsigned int index, void *ud);


#ifdef __cplusplus
}
#endif

#endif
