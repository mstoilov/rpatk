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

#ifndef _RPASEARCH_H_
#define _RPASEARCH_H_

#include "rpamatch.h"
#include "rpadbexpriv.h"

#ifdef __cplusplus
extern "C" {
#endif

int rpa_dump_list_dataptr(rpa_match_t *match, char *buffer, unsigned int size);
int rpa_dbex_add_mnode_to_list_dataptr(rpa_dbex_handle hDbex, rpa_match_t *match, rpa_mnode_t *mnode);
void rpa_dbex_setup_list(rpa_dbex_handle hDbex, rpa_match_t *match);
int rpa_dbex_reset_list_dataptr(rpa_dbex_handle hDbex, rpa_match_t *match);
int rpase_stat_set_encoding(rpa_stat_t *stat, unsigned int encoding);
const char *rpa_dbex_search_version();


#ifdef __cplusplus
}
#endif

#endif
