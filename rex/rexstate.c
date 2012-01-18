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
#include <stdio.h>
#include <ctype.h>
#include "rlib/rmem.h"
#include "rex/rextransition.h"
#include "rex/rexstate.h"


void rex_subset_addnewsubstate(rarray_t *subset, unsigned long ss_uid, unsigned long ss_type, void *ss_userdata)
{
	rexsubstate_t *ssptr;
	rexsubstate_t subst;
	long min, max, mid;

	min = 0;
	max = min + r_array_length(subset);
	while (max > min) {
		mid = (min + max)/2;
		ssptr = (rexsubstate_t *)r_array_slot(subset, mid);
		if (ss_uid == ssptr->ss_uid) {
			return;
		} else if (ss_uid >= ssptr->ss_uid) {
			min = mid + 1;
		} else {
			max = mid;
		}
	}
	subst.ss_uid = ss_uid;
	subst.ss_type = ss_type;
	subst.ss_userdata = ss_userdata;
	r_array_insert(subset, min, &subst);
}


void rex_state_cleanup(robject_t *obj)
{
	rexstate_t *state = (rexstate_t*) obj;

	r_array_destroy(state->trans);
	r_array_destroy(state->etrans);
	r_array_destroy(state->subset);
	r_object_cleanup(obj);
}


robject_t *rex_state_init(robject_t *obj, unsigned int objtype, r_object_cleanupfun cleanup, unsigned long uid, unsigned long statetype)
{
	rexstate_t *state = (rexstate_t*)obj;

	r_object_init(obj, objtype, cleanup, NULL);
	state->trans = r_array_create(sizeof(rex_transition_t));
	state->etrans = r_array_create(sizeof(rex_transition_t));
	state->subset = r_array_create(sizeof(rexsubstate_t));
	state->type = statetype;
	state->uid = uid;
	return (robject_t*)state;
}


rexstate_t *rex_state_create(unsigned long uid, long statetype)
{
	rexstate_t *state;
	state = (rexstate_t*)r_object_create(sizeof(*state));
	rex_state_init((robject_t*)state, R_OBJECT_FASTATE, rex_state_cleanup, uid, statetype);
	return state;
}


void rex_state_destroy(rexstate_t *state)
{
	r_object_destroy((robject_t*)state);
}


void rex_state_addtransition_e(rexstate_t *state, unsigned long dstuid)
{
	rex_transitions_add_e(state->etrans, state->uid, dstuid);
}


void rex_state_addtransition_e_dst(rexstate_t *srcstate, const rexstate_t *dststate)
{
	rex_state_addtransition_e(srcstate, dststate->uid);
}


void rex_state_addtransition(rexstate_t *state, rexchar_t c, unsigned long dstuid)
{
	rex_transitions_add(state->trans, c, c, state->uid, dstuid);
}


void rex_state_addtransition_dst(rexstate_t *srcstate, rexchar_t c, const rexstate_t *dststate)
{
	rex_state_addtransition(srcstate, c, dststate->uid);
}


void rex_state_addrangetransition(rexstate_t *state, rexchar_t c1, rexchar_t c2, unsigned long dstuid)
{
	rex_transitions_add(state->trans, c1, c2, state->uid, dstuid);
}


void rex_state_addrangetransition_dst(rexstate_t *srcstate, rexchar_t c1,  rexchar_t c2, const rexstate_t *dststate)
{
	rex_state_addrangetransition(srcstate, c1, c2, dststate->uid);
}


#if 0
void rex_state_normalizetransitions(rexstate_t *state)
{
	long i, j;
	rex_transition_t *itrans, *jtrans;
	rarray_t *trans = state->trans;

startover:
	for (i = 0; i < r_array_length(trans); i++) {
		itrans = (rex_transition_t *)r_array_slot(trans, i);
		if (itrans->lowin == itrans->highin)
			itrans->type = REX_TRANSITION_INPUT;
		for (j = 0; j < r_array_length(trans); j++) {
			if (i == j) {
				/*
				 * Same transition.
				 */
				continue;
			}
			jtrans = (rex_transition_t *)r_array_slot(trans, j);
			if (itrans->dstuid != jtrans->dstuid) {
				/*
				 * These two can never be merged.
				 */
				continue;
			}
			if (jtrans->lowin >= itrans->lowin && jtrans->lowin <= itrans->highin) {
				/*
				 * Overlapping regions
				 * Merge jtrans into itrans and delete jtrans.
				 */
				if (jtrans->highin > itrans->highin)
					itrans->highin = jtrans->highin;
				if (itrans->lowin != itrans->highin)
					itrans->type = REX_TRANSITION_RANGE;
				r_array_delete(trans, j);
				goto startover;
			}
			if (itrans->highin != REX_CHAR_MAX && jtrans->lowin == itrans->highin + 1) {
				/*
				 * Adjacent regions
				 * Merge jtrans into itrans and delete jtrans.
				 */
				itrans->highin = jtrans->highin;
				if (itrans->lowin != itrans->highin)
					itrans->type = REX_TRANSITION_RANGE;
				r_array_delete(trans, j);
				goto startover;
			}
		}
	}
}
#endif


void rex_state_addnewsubstate(rexstate_t *state, unsigned long ss_uid, unsigned long ss_type, void *ss_userdata)
{
	rex_subset_addnewsubstate(state->subset, ss_uid, ss_type, ss_userdata);
}


rexsubstate_t *rex_state_findsubstate(rexstate_t *state, unsigned long ss_uid)
{
	rexsubstate_t *ssptr;
	long min, max, mid;

	min = 0;
	max = min + r_array_length(state->subset);
	while (max > min) {
		mid = (min + max)/2;
		ssptr = (rexsubstate_t *)r_array_slot(state->subset, mid);
		if (ss_uid == ssptr->ss_uid) {
			return ssptr;
		} else if (ssptr->ss_uid >= ss_uid) {
			min = mid + 1;
		} else {
			max = mid;
		}
	}
	return NULL;
}


void rex_state_addsubstate(rexstate_t *state, const rexstate_t *substate)
{
	rex_state_addnewsubstate(state, substate->uid, substate->type, substate->userdata);
}


void rex_state_setuserdata(rexstate_t *state, void *userdata)
{
	state->userdata = userdata;
}


long rex_state_next(rexstate_t *state, unsigned long input)
{
	rex_transition_t *t;
	long i = 0;

	for (i = 0; i < r_array_length(state->trans); i++) {
		t = (rex_transition_t *)r_array_slot(state->trans, i);
		if (t->lowin == input)
			return t->dstuid;
	}
	return -1;
}


void rex_state_dump(rexstate_t *state)
{
	long index;
	char buf[240];
	int bufsize = sizeof(buf) - 1;
	int n = 0;
	rex_transition_t *t;

	if (!state)
		return;
	fprintf(stdout, "State %ld", state->uid);
	if (rex_subset_length(state->subset)) {
		fprintf(stdout, " (");
		for (index = 0; index < rex_subset_length(state->subset); index++) {
			fprintf(stdout, "%ld", rex_subset_slot(state->subset, index)->ss_uid);
			if (rex_subset_slot(state->subset, index)->ss_type == REX_STATETYPE_ACCEPT)
				fprintf(stdout, "{accept}");
			fprintf(stdout, ",");
		}
		fprintf(stdout, ")");
	}
	fprintf(stdout, ": ");
	if (state->type == REX_STATETYPE_ACCEPT) {
		fprintf(stdout, " REX_STATETYPE_ACCEPT ");
		if (state->userdata) {
			fprintf(stdout, " : %s", (const char*)state->userdata);
		}
	} else if (state->type == REX_STATETYPE_DEAD) {
		fprintf(stdout, " REX_STATETYPE_DEAD ");
	} else if (state->type == REX_STATETYPE_START) {
		fprintf(stdout, " REX_STATETYPE_START ");
	}
	fprintf(stdout, "\n");

	for (index = 0; index < r_array_length(state->etrans); index++) {
		t = (rex_transition_t *)r_array_slot(state->etrans, index);
		if (t->type == REX_TRANSITION_EMPTY) {
			fprintf(stdout, "    epsilon -> %ld\n", t->dstuid);
		}
	}
	for (index = 0; index < r_array_length(state->trans); index++) {
		t = (rex_transition_t *)r_array_slot(state->trans, index);
		n = 0;
		if (t->type == REX_TRANSITION_EMPTY) {
			fprintf(stdout, "    epsilon -> %ld\n", t->dstuid);
		} else if (t->type == REX_TRANSITION_RANGE) {
			if (isprint(t->lowin) && !isspace(t->lowin) && isprint(t->highin) && !isspace(t->highin))
				n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "        [%c - %c] ", t->lowin, t->highin);
			else
				n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "        [0x%X - 0x%X] ", t->lowin, t->highin);
		} else {
			if (isprint(t->lowin) && !isspace(t->lowin))
				n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "        '%c' ", t->lowin);
			else
				n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "        0x%X ", t->lowin);
		}
		r_memset(buf + n, ' ', bufsize - n);
		n = 40;
		n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "-> %ld", t->dstuid);
		fprintf(stdout, "%s\n", buf);
	}
	if (!r_array_length(state->etrans) && !r_array_length(state->trans))
		fprintf(stdout, "        (none)\n");
	fprintf(stdout, "\n");

}

