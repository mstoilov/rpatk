/*
 *  Regular Pattern Analyzer Toolkit (RPA/Tk)
 *  Copyright (c) 2009-2012 Martin Stoilov
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

#ifndef _RMEM_H_
#define _RMEM_H_


#include "rtypes.h"

#ifdef __cplusplus
extern "C" {
#endif


rpointer r_malloc(size_t size);
rpointer r_realloc(rpointer ptr, size_t size);
rpointer r_calloc(size_t nmemb, size_t size);
void r_free(rpointer ptr);
rpointer r_zmalloc(size_t size);
rpointer r_memset(rpointer s, int32_t c, size_t n);
rpointer r_memcpy(rpointer dest, rconstpointer src, size_t n);
rpointer r_memmove(rpointer dest, rconstpointer src, size_t n);
size_t r_debug_get_allocmem();
size_t r_debug_get_maxmem();
void r_debug_reset_maxmem();


#ifdef __cplusplus
}
#endif

#endif
