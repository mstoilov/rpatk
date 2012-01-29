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

#ifndef _REXDB_H_
#define _REXDB_H_

#include "rtypes.h"
#include "rlib/robject.h"
#include "rlib/rarray.h"
#include "rex/rexstate.h"


#ifdef __cplusplus
extern "C" {
#endif

#define R_OBJECT_REXDB 35

typedef enum {
	REXDB_TYPE_DFA = 0,
	REXDB_TYPE_NFA = 1
} rexdb_type_t;


typedef struct rexdb_s {
	robject_t obj;
	rexdb_type_t type;
	rarray_t *states;
	rarray_t *substates;	/* Used only if the type is REXDB_TYPE_DFA */
	rexstate_t *start;
} rexdb_t;


robject_t *rex_db_init(robject_t *obj, unsigned int objtype, r_object_cleanupfun cleanup, rexdb_type_t type);
rexdb_t *rex_db_create(rexdb_type_t type);
rexdb_t *rex_db_createdfa(rexdb_t *nfa, unsigned long start);
void rex_db_destroy(rexdb_t *rexdb);
long rex_db_createstate(rexdb_t *rexdb, rex_statetype_t type);
long rex_db_insertstate(rexdb_t *rexdb, rexstate_t *s);
long rex_db_findstate(rexdb_t *a, const rarray_t *subset);
rexstate_t *rex_db_getstate(rexdb_t *rexdb, long uid);
rexsubstate_t *rex_db_getsubstate(rexdb_t *rexdb, unsigned long uid);
rex_transition_t * rex_db_addrangetrasition(rexdb_t *rexdb, rexchar_t c1, rexchar_t c2, unsigned long srcuid, unsigned long dstuid);
rex_transition_t * rex_db_addtrasition_e(rexdb_t *rexdb, unsigned long srcuid, unsigned long dstuid);
int rex_db_simulate_nfa(rexdb_t *rexdb, long uid, const char *str, const char *end);
int rex_db_simulate_nfa2(rexdb_t *a, long uid, const char *str, const char *end);
long rex_db_compile(rexdb_t *rexdb, const char *str, unsigned int size);
void rex_db_dumpstate(rexdb_t *rexdb, unsigned long uid);
const char *rex_db_version();


/*
 * Virtual methods implementation
 */
void rex_db_cleanup(robject_t *obj);

#ifdef __cplusplus
}
#endif
#endif
