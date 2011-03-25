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

#if !defined(RPAVARLINK_H)
#define RPAVARLINK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rpalist.h"
#include "rpavar.h"


#define USE_UD1 (1<<0)
#define USE_UD2 (1<<1)
#define USE_UD3 (1<<2)
#define USE_UD4 (1<<3)


typedef struct rpa_varlink_s {
	rpa_list_t lnk;
	rpa_list_t hlnk;	
	rpa_var_t var;
} rpa_varlink_t;


rpa_varlink_t *rpa_varlink_find_last_matchptr(rpa_head_t *head);
rpa_varlink_t *rpa_varlink_find_last_named_matchptr(rpa_head_t *head);
rpa_varlink_t *rpa_dbex_find_named_matchptr(rpa_head_t *head, const char *name);
rpa_varlink_t *rpa_varlink_create(unsigned char type, const char *name);
void rpa_varlink_destroy(rpa_varlink_t *pVarLink);
void rpa_varlink_destroy_all(rpa_head_t *head);


#ifdef __cplusplus
}
#endif

#endif

