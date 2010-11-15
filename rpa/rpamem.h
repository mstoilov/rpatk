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

#ifndef _RPAMEM_H_
#define _RPAMEM_H_


#ifdef __cplusplus
extern "C" {
#endif

extern long int g_rpa_allocmem;
extern long int g_rpa_maxmem;


void *rpa_malloc(unsigned long size);
void *rpa_zmalloc(unsigned long size);
void rpa_free(void *ptr);
void *rpa_realloc(void *ptr, unsigned long size);
void *rpa_memset(void *s, int c, unsigned long n);
void *rpa_memcpy(void *dest, const void *src, unsigned long n);

#ifdef __cplusplus
}
#endif

#endif
