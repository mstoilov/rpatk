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

#ifndef _RPABITMAP_H_
#define _RPABITMAP_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "rtypes.h"
#include "rpa/rpadbex.h"
#include "rpa/rparecord.h"


#define RPA_BITMAP_SIZE (sizeof(rword))
#define RPA_BITMAP_BITS (RPA_BITMAP_SIZE*8)
#define RPA_BITMAP_SETBIT(__r__, __b__) do { (__r__)->userdata |= ((rword)1) << (__b__); } while (0)
#define RPA_BITMAP_GETBIT(__r__, __b__) (__r__)->userdata & (((rword)1) << (__b__)) ? 1 : 0
#define RPA_BITMAP_CLRBIT(__r__, __b__) do { (__r__)->userdata &= ~(((rword)1) << (__b__)); } while (0)
#define RPA_BITMAP_CLRALL(__r__) do { (__r__)->userdata = (rword)0; } while (0)
#define RPA_BITMAP_ORBITS(__r__, __c__) do { (__r__)->userdata |= (__c__)->userdata; } while (0)


typedef struct rpa_bitmapcompiler_s {
	rpadbex_t *dbex;
	ruint32 beginchar;
	ruint32 endchar;
} rpa_bitmapcompiler_t;

void rpa_dbex_buildbitmapinfo(rpadbex_t *dbex);
void rpa_dbex_buildbitmapinfo_for_rule(rpadbex_t *dbex, rparule_t rid);


#ifdef __cplusplus
}
#endif

#endif
