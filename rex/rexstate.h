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

#ifndef _REXSTATE_H_
#define _REXSTATE_H_

#include "rtypes.h"
#include "rlib/robject.h"
#include "rlib/rmap.h"
#include "rex/rextransition.h"


#ifdef __cplusplus
extern "C" {
#endif

#define R_OBJECT_FASTATE 33


typedef struct rexstate_s rexstate_t;

typedef enum {
	REX_STATETYPE_NONE = 0,
	REX_STATETYPE_START = 1,
	REX_STATETYPE_ACCEPT = 2,
	REX_STATETYPE_DEAD = 3,
	REX_STATETYPE_ZOMBIE = 4,
} rex_statetype_t;

struct rexstate_s {
	robject_t obj;
	rarray_t *etrans;
	rarray_t *trans;
	rarray_t *subset;
	unsigned long type;
	unsigned long uid;
	void *userdata;
};

typedef struct rexsubstate_s {
	unsigned long ss_type;
	void *ss_userdata;
} rexsubstate_t;


robject_t *rex_state_init(robject_t *obj, unsigned int objtype, r_object_cleanupfun cleanup, unsigned long uid, unsigned long statetype);
rexstate_t *rex_state_create(unsigned long uid, long statetype);
void rex_state_destroy(rexstate_t *state);
rex_transition_t *rex_state_addtransition_e_dst(rexstate_t *srcstate, const rexstate_t *dststate);
rex_transition_t *rex_state_addtransition_e(rexstate_t *state, unsigned long dstuid);
rex_transition_t *rex_state_addtransition(rexstate_t *state, rexchar_t c1, rexchar_t c2, unsigned long dstuid);
rex_transition_t *rex_state_addtransition_dst(rexstate_t *srcstate, rexchar_t c1, rexchar_t c2, const rexstate_t *dststate);
void rex_state_addnewsubstate(rexstate_t *state, unsigned long ss_uid);
void rex_state_addsubstate(rexstate_t *state, const rexstate_t *substate);
long rex_state_next(rexstate_t *state, unsigned long input);
void rex_state_setuserdata(rexstate_t *state, void *userdata);

/*
 * Virtual methods implementation
 */
void rex_state_cleanup(robject_t *obj);

void rex_subset_addnewsubstate(rarray_t *subset, unsigned long ss_uid);
#define rex_subset_length(__set__) r_array_length(__set__)
#define rex_subset_clear(__set__) r_array_setlength(__set__, 0)
#define rex_subset_push(__set__, __uid__) do {unsigned long __u__ = (__uid__); r_array_add(__set__, &__u__); } while (0)
#define rex_subset_pop(__set__) r_array_pop(__set__, unsigned long)
#define rex_subset_index(__set__, __i__) (r_array_index(__set__, __i__, unsigned long))

#ifdef __cplusplus
}
#endif
#endif
