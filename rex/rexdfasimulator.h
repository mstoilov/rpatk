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

#ifndef _REXDFASIMULATOR_H_
#define _REXDFASIMULATOR_H_

#include "rtypes.h"
#include "rlib/robject.h"
#include "rlib/rarray.h"
#include "rex/rexdb.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct rex_dfasimulator_s {
	rarray_t *accepts;
	size_t inputsize;
	size_t count;
	rexstate_t *next;
} rex_dfasimulator_t;


rex_dfasimulator_t *rex_dfasimulator_create();
void rex_dfasimulator_destroy(rex_dfasimulator_t *si);
size_t rex_dfasimulator_run(rex_dfasimulator_t *si, rexdb_t *nfa, size_t uid, const char *str, size_t size);
void rex_dfasimulator_start(rex_dfasimulator_t *si, rexdb_t *db, size_t uid);
size_t rex_dfasimulator_next(rex_dfasimulator_t *si, rexdb_t *db, ruint32 wc, int wcsize);


#ifdef __cplusplus
}
#endif

#endif
