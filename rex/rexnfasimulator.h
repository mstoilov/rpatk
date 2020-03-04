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

#ifndef _REXNFASIMULATOR_H_
#define _REXNFASIMULATOR_H_

#include "rtypes.h"
#include "rlib/robject.h"
#include "rlib/rarray.h"
#include "rex/rexdb.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct rex_nfasimulator_s {
	rarray_t *accepts;			/* Keep track of all accept states we encounter. */
	rarray_t *newstates;		/* Newstates array */
	rarray_t *oldstates;		/* Old states, from here we find all possible new states and add them to the newstates array.*/
	rarray_t *onmap;			/* Keep track of which states have already been added to the newstates array */
	size_t inputsize;			/* input size in bytes */
	size_t count;				/* input count in chars, if the chars are multi-byte utf8
								 * count and inputsize will be different.
								 * For ASCII chars, count and inputsize will be the same.
								 */
} rex_nfasimulator_t;


rex_nfasimulator_t *rex_nfasimulator_create();
void rex_nfasimulator_destroy(rex_nfasimulator_t *si);
size_t rex_nfasimulator_run(rex_nfasimulator_t *si, rexdb_t *nfa, size_t uid, const char *str, size_t size);
size_t rex_nfasimulator_run_s(rex_nfasimulator_t *si, rexdb_t *nfa, size_t uid, const char *str);
void rex_nfasimulator_start(rex_nfasimulator_t *si, rexdb_t *db, size_t uid);
size_t rex_nfasimulator_next(rex_nfasimulator_t *si, rexdb_t *db, ruint32 wc, size_t wcsize);


#ifdef __cplusplus
}
#endif

#endif
