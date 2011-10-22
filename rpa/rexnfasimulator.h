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

#ifndef _REXNFASIMULATOR_H_
#define _REXNFASIMULATOR_H_

#include "rtypes.h"
#include "rlib/robject.h"
#include "rlib/rarray.h"
#include "rpa/rexdb.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct rex_accept_s {
	unsigned long state;
	long inputsize;
} rex_accept_t;


typedef struct rex_nfasimulator_s {
	rarray_t *accepts;
	rarray_t *newstates;
	rarray_t *oldstates;
	rarray_t *onmap;
} rex_nfasimulator_t;


rex_nfasimulator_t *rex_nfasimulator_create();
void rex_nfasimulator_destroy(rex_nfasimulator_t *si);
long rex_nfasimulator_run(rex_nfasimulator_t *si, rexdb_t *db, long uid, const char *start, const char *end);
void rex_nfasimulator_start(rex_nfasimulator_t *si, rexdb_t *db, long uid);
long rex_nfasimulator_next(rex_nfasimulator_t *si, rexdb_t *db, ruint32 wc, rboolean *acc);


#ifdef __cplusplus
}
#endif

#endif
