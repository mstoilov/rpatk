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

#ifndef _RPABITMAP_H_
#define _RPABITMAP_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "rtypes.h"
#include "rpa/rpadbex.h"
#include "rpa/rparecord.h"

typedef ruword rpabitmap_t;

#define RPA_BITMAP_SIZE (sizeof(rpabitmap_t))
#define RPA_BITMAP_BITS (RPA_BITMAP_SIZE * 8)
#define RPA_RECORD2BITMAP(__r__) (&(__r__)->userdata)
#define RPA_BITMAP_SETBIT(__pbmp__, __b__) do { *(__pbmp__) |= ((rpabitmap_t)1) << (__b__); } while (0)
#define RPA_BITMAP_GETBIT(__pbmp__, __b__) (*(__pbmp__) & (((rpabitmap_t)1) << (__b__)) ? 1 : 0)
#define RPA_BITMAP_CLRBIT(__pbmp__, __b__) do { *(__pbmp__) &= ~(((rpabitmap_t)1) << (__b__)); } while (0)
#define RPA_BITMAP_CLRALL(__pbmp__) do { *(__pbmp__) = (rpabitmap_t)0; } while (0)
#define RPA_BITMAP_SETALL(__pbmp__) do { *(__pbmp__) = (rpabitmap_t)-1; } while (0)
#define RPA_BITMAP_ORBITS(__pbmp1__, __pbmp2__) do { *(__pbmp1__) |= *(__pbmp2__); } while (0)
#define RPA_BITMAP_SETVAL(__pbmp__, __v__) do { *(__pbmp__) = __v__; } while (0)
#define RPA_BITMAP_GETVAL(__pbmp__) *(__pbmp__)


//#define RPA_BITMAP_SETBIT(__r__, __b__) do { (__r__)->userdata |= ((ruword)1) << (__b__); } while (0)
//#define RPA_BITMAP_GETBIT(__r__, __b__) ((__r__)->userdata & (((ruword)1) << (__b__)) ? 1 : 0)
//#define RPA_BITMAP_CLRBIT(__r__, __b__) do { (__r__)->userdata &= ~(((ruword)1) << (__b__)); } while (0)
//#define RPA_BITMAP_CLRALL(__r__) do { (__r__)->userdata = (ruword)0; } while (0)
//#define RPA_BITMAP_SETALL(__r__) do { (__r__)->userdata = (ruword)-1; } while (0)
//#define RPA_BITMAP_ORBITS(__r__, __c__) do { (__r__)->userdata |= (__c__)->userdata; } while (0)
//#define RPA_BITMAP_SETVAL(__r__, __v__) do { (__r__)->userdata = __v__; } while (0)
//#define RPA_BITMAP_GETVAL(__r__) ((__r__)->userdata)
//#define RPA_BMAP_GETBIT(__bitmap__, __bit__) ((__bitmap__) & (((rpabitmap_t)1) << (__bit__)) ? 1 : 0)


typedef struct rpa_bitmapcompiler_s {
	rpadbex_t *dbex;
	ruint32 beginchar;
	ruint32 endchar;
} rpa_bitmapcompiler_t;

long rpa_bitmap_set(rarray_t *records, long rec, rpointer userdata);


#ifdef __cplusplus
}
#endif

#endif
