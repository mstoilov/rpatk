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

#ifndef _RPAMATCHRANGELIST_H_
#define _RPAMATCHRANGELIST_H_

#include <stdarg.h>
#include "rpalist.h"
#include "rpamatchlist.h"


#ifdef __cplusplus
extern "C" {
#endif


rpa_match_t * rpa_match_rangelist_create(const char *name, rpa_matchfunc_t match_function_id);
rpa_match_t * rpa_match_rangelist_create_namesize(
	const char *name,
	unsigned int namesiz,
	rpa_matchfunc_t match_function_id);

#ifdef __cplusplus
}
#endif

#endif
