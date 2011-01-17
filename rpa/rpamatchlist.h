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

#ifndef _RPAMATCHLIST_H_
#define _RPAMATCHLIST_H_

#include <stdarg.h>
#include "rpalist.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct rpa_match_list_s {
	rpa_match_t base;
	rpa_head_t head;
	void *dataptr;
} rpa_match_list_t;


typedef struct rpa_match_nlist_s {
	rpa_match_list_t base;
	unsigned char loopy;
	void *callback;
} rpa_match_nlist_t;

rpa_match_t * rpa_match_list_create(const char *name, rpa_matchfunc_t match_function_id);
rpa_match_t * rpa_match_list_create_namesize(
	const char *name,
	unsigned int namesiz,
	rpa_matchfunc_t match_function_id);
rpa_match_t *rpa_match_list_init(
	rpa_match_t *match, 
	const char *name, 
	unsigned int namesiz,
	rpa_class_methods_t *vptr,
	rpa_matchfunc_t match_function_id);
void rpa_match_list_cleanup(rpa_match_t *match);

rpa_match_t *rpa_match_list_init_dataptr(
	rpa_match_t *match, 
	const char *name, 
	unsigned int namesiz,
	rpa_class_methods_t *vptr,
	rpa_matchfunc_t match_function_id);
void rpa_match_list_cleanup_dataptr(rpa_match_t *match);
int rpa_match_list_scan(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_list(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_list_alt(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_list_noalt(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_list_and(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_list_minus(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_list_at(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_list_icase(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_list_alt_icase(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_list_and_icase(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_list_minus_icase(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_list_contain(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_list_not(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_list_utf8not(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_list_u16not(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_list_best_alt(rpa_match_t *match, rpa_stat_t *stat, const char *input);


rpa_match_t * rpa_match_nlist_create_namesize(
	const char *name,
	unsigned int namesiz,
	rpa_matchfunc_t match_function_id);


#ifdef __cplusplus
}
#endif

#endif
