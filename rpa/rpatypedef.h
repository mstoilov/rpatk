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

#ifndef _RPATYPEDEF_H_
#define _RPATYPEDEF_H_


#ifdef __cplusplus
extern "C" {
#endif


typedef struct rpa_dbex_s rpa_dbex_t;
typedef struct rpa_stat_s rpa_stat_t;
typedef struct rpa_match_s rpa_match_t;
typedef struct rpa_mnode_s rpa_mnode_t;
typedef struct rpa_mnode_callback_s rpa_mnode_callback_t;

typedef int (*RPA_MNODE_FUNCTION)(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);
typedef int (*RPA_MATCH_FUNCTION)(rpa_match_t *match, rpa_stat_t *stat, const char *input);
typedef int (*RPA_MATCH_CALLBACK)(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason);


#ifdef __cplusplus
}
#endif

#endif
