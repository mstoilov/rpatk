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

#ifndef _FASTATE_H_
#define _FASTATE_H_

#include "rtypes.h"
#include "rlib/robject.h"
#include "rlib/rmap.h"


#ifdef __cplusplus
extern "C" {
#endif

#define R_OBJECT_FASTATE 33

typedef struct rexstate_s rexstate_t;

typedef enum {
	REX_STATETYPE_NONE = 0,
	REX_STATETYPE_START = 1,
	REX_STATETYPE_ACCEPT = 2,
	REX_STATETYPE_REJECT = 3,
} rex_statetype_t;


struct rexstate_s {
	robject_t obj;
	rarray_t *etrans;
	rarray_t *trans;
	rarray_t *subset;
	unsigned long type;
	unsigned long uid;
};


robject_t *rex_state_init(robject_t *obj, unsigned int objtype, r_object_cleanupfun cleanup, unsigned long uid, unsigned long statetype);
rexstate_t *rex_state_create(unsigned long uid, long statetype);
void rex_state_destroy(rexstate_t *state);
void rex_state_addtransition_dst(rexstate_t *srcstate, unsigned long c, const rexstate_t *dststate);
void rex_state_addtransition(rexstate_t *state, unsigned int c, unsigned long dstuid);
void rex_state_addrangetransition(rexstate_t *state, unsigned int c1,  unsigned int c2, unsigned long dstuid);
void rex_state_addrangetransition_dst(rexstate_t *srcstate, unsigned int c1,  unsigned int c2, const rexstate_t *dststate);
void rex_state_addtransition_e(rexstate_t *state, unsigned long dstuid);
void rex_state_e_addtransition_e_dst(rexstate_t *srcstate, const rexstate_t *dststate);
rboolean rex_state_findsubstate(rexstate_t *state, unsigned long uid);
void rex_state_addsubstate(rexstate_t *state, unsigned long uid);
void rex_state_addsubstate_dst(rexstate_t *state, const rexstate_t *dststate);
long rex_state_next(rexstate_t *state, unsigned long input);
void rex_state_dump(rexstate_t *state);

/*
 * Virtual methods implementation
 */
void rex_state_cleanup(robject_t *obj);

#ifdef __cplusplus
}
#endif
#endif
