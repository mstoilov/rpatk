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

#ifndef _REXSTATE_H_
#define _REXSTATE_H_

#include "rtypes.h"
#include "rlib/robject.h"
#include "rlib/rstring.h"
#include "rex/rextransition.h"


#ifdef __cplusplus
extern "C" {
#endif

#define R_OBJECT_FASTATE 33

typedef struct rexstate_s rexstate_t;

struct rexstate_s {
	robject_t obj;
	rarray_t *etrans;		/* Epsilon transitions */
	rarray_t *trans;		/* Transitions */
	rarray_t *subset;		/* Sub-states associated with this state */
	rex_statetype_t type;	/* REX_STATETYPE_START, REX_STATETYPE_ACCEPT, ... */
	size_t uid;				/* The index of this state in the array of states in rexdb_t */
	rexuserdata_t userdata;	/* User defined data associated with the state */
};

typedef struct rexsubstate_s {
	size_t ss_uid;
	size_t ss_type;
	rexuserdata_t ss_userdata;
} rexsubstate_t;

typedef struct rex_accept_s {
	rexstate_t *state;
	size_t inputsize;
	size_t count;
} rex_accept_t;

/*
 * Initialize a new object of type rexstate_t. rexstate_t inherits the robject_t type
 * so we must call the r_object_init to initialize the base object.
 *
 * This function is normally called by the @ref rex_state_create function.
 */

void rex_state_init(robject_t *obj, ruint32 objtype, r_object_cleanupfun cleanup, r_object_copyfun copy, size_t uid, rex_statetype_t statetype);

/**
 * Create a new rexstate_t with the specified uid and type.
 * The available types are:
 *
 */
rexstate_t *rex_state_create(size_t uid, rex_statetype_t statetype);

/**
 * Destroy @ref rexstate_t object created with @ref rex_state_create
 *
 */
void rex_state_destroy(rexstate_t *state);

/**
 * Add empty transition to the srcstate pointing to dststate.
 *
 */
rex_transition_t *rex_state_addtransition_e_dst(rexstate_t *srcstate, const rexstate_t *dststate);

/**
 * Add empty transition to the srcstate pointing to dstuid.
 *
 */
rex_transition_t *rex_state_addtransition_e(rexstate_t *state, size_t dstuid);

/**
 * Add transition to state for inputs [c1-c2], pointing to dstuid. If the input
 * is a single character than c1 and c2 must be the same.
 *
 */
rex_transition_t *rex_state_addtransition(rexstate_t *state, rexchar_t c1, rexchar_t c2, size_t dstuid);

/**
 * Add transition to state for inputs [c1-c2], pointing to dststate. If the input
 * is a single character than c1 and c2 must be the same.
 *
 */
rex_transition_t *rex_state_addtransition_dst(rexstate_t *srcstate, rexchar_t c1, rexchar_t c2, const rexstate_t *dststate);

/**
 * Add a substate to the state.
 *
 * Substates become relevent when we compile a DFA from NFA. Then one or many NFA
 * states form a subset of states in a single DFA state. Adding the substats to
 * a DFA state helps keep track of which NFA states are included in the DFA state's subset.
 */
void rex_state_addsubstate(rexstate_t *state, size_t uid, size_t type, rexuserdata_t userdata);
void rex_state_addsubstate_dst(rexstate_t *state, const rexstate_t *substate);

/**
 * Set the userdata.
 */
void rex_state_setuserdata(rexstate_t *state, rexuserdata_t userdata);

/**
 * Virtual methods implementation
 */
void rex_state_cleanup(robject_t *obj);

/**
 * Add new sub-state uid to a an array.
 */
void rex_subset_addnewsubstate(rarray_t *subset, size_t ss_uid);


#define rex_subset_length(__set__) r_array_length(__set__)
#define rex_subset_clear(__set__) r_array_setlength(__set__, 0)
#define rex_subset_push(__set__, __uid__) do {size_t __u__ = (__uid__); r_array_add(__set__, &__u__); } while (0)
#define rex_subset_pop(__set__) r_array_pop(__set__, size_t)
#define rex_subset_index(__set__, __i__) (r_array_index(__set__, __i__, size_t))

#ifdef __cplusplus
}
#endif
#endif
