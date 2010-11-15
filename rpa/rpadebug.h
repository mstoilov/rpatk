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

#if !defined(RPADEBUG_H)
#define RPADEBUG_H


#include "rpadbex.h"

#define RPA_BREAK __asm__ ("int $3")
#ifdef __cplusplus
extern "C" {
#endif

typedef struct rpa_varlink_s *rpa_varlink_ptr;

void rpa_dump_pattern_tree(rpa_pattern_handle pattern);
void rpa_dump_stack(rpa_dbex_handle hDbex);
void rpa_varlink_dump(rpa_varlink_ptr pVarLink);
long rpa_get_alloc_mem();
long rpa_get_alloc_maxmem();


#ifdef __cplusplus
}
#endif

#endif
