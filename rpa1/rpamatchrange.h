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

#ifndef _RPAMATCHRANGE_H_
#define _RPAMATCHRANGE_H_


#ifdef __cplusplus
extern "C" {
#endif


typedef struct rpa_match_range_s {
	rpa_match_t base;
	unsigned int low;
	unsigned int high;
	unsigned int icaselow;
	unsigned int icasehigh;
} rpa_match_range_t;

void rpa_match_range_cleanup(rpa_match_t *match);
//void rpa_match_range_destroy(rpa_match_t *match);
rpa_match_t *rpa_match_range_init(
	rpa_match_t *match, 
	const char *name,
	unsigned int namesiz,
	rpa_class_methods_t *vptr,
	rpa_matchfunc_t match_function_id,
	unsigned long low,
	unsigned long high);
rpa_match_t * rpa_match_range_create_namesize(
	const char *name,
	unsigned int namesiz,
	rpa_matchfunc_t match_function_id,
	unsigned long low,
	unsigned long high);
rpa_match_t * rpa_match_range_create(
	const char *name,
	rpa_matchfunc_t match_function_id,
	unsigned long low,
	unsigned long high);
int rpa_match_range_chrinrange(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_range_chrnotinrange(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_range_chrinrange_icase(rpa_match_t *match, rpa_stat_t *stat, const char *input);
int rpa_match_range_chrnotinrange_icase(rpa_match_t *match, rpa_stat_t *stat, const char *input);

#ifdef __cplusplus
}
#endif

#endif
