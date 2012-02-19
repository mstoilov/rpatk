/*
 *  Regular Pattern Analyzer Toolkit (RPA/Tk)
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

#ifndef _RMEM_H_
#define _RMEM_H_


#include "rtypes.h"

#ifdef __cplusplus
extern "C" {
#endif


rpointer r_malloc(unsigned long size);
rpointer r_realloc(rpointer ptr, unsigned long size);
rpointer r_calloc(unsigned long nmemb, unsigned long size);
void r_free(rpointer ptr);
rpointer r_zmalloc(unsigned long size);
rpointer r_memset(rpointer s, int c, unsigned long n);
rpointer r_memcpy(rpointer dest, rconstpointer src, unsigned long n);
rpointer r_memmove(rpointer dest, rconstpointer src, unsigned long n);
unsigned long r_debug_get_allocmem();
unsigned long r_debug_get_maxmem();
void r_debug_reset_maxmem();


#ifdef __cplusplus
}
#endif

#endif
