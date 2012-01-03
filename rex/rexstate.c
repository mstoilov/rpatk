#include <stdio.h>
#include "rex/rextransition.h"
#include "rex/rexstate.h"


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
	state->subset = r_array_create(sizeof(unsigned long));
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
	rex_transition_t ntrans;

	ntrans.lowin = 0;
	ntrans.highin = 0;
	ntrans.type = REX_TRANSITION_EMPTY;
	ntrans.dstuid = dstuid;
	ntrans.srcuid = state->uid;
	r_array_add(state->etrans, &ntrans);
}


void rex_state_addtransition_e_dst(rexstate_t *srcstate, const rexstate_t *dststate)
{
	rex_state_addtransition_e(srcstate, dststate->uid);
}


void rex_state_addtransition(rexstate_t *state, unsigned int c, unsigned long dstuid)
{
	rex_transition_t *t;
	rex_transition_t ntrans;
	long min, max, mid;

	ntrans.lowin = c;
	ntrans.highin = c;
	ntrans.type = REX_TRANSITION_INPUT;
	ntrans.dstuid = dstuid;
	ntrans.srcuid = state->uid;
	min = 0;
	max = min + r_array_length(state->trans);
	while (max > min) {
		mid = (min + max)/2;
		t = (rex_transition_t *)r_array_slot(state->trans, mid);
		if (c >= t->lowin) {
			min = mid + 1;
		} else {
			max = mid;
		}
	}
	r_array_insert(state->trans, min, &ntrans);
}


void rex_state_addtransition_dst(rexstate_t *srcstate, unsigned long c, const rexstate_t *dststate)
{
	rex_state_addtransition(srcstate, c, dststate->uid);
}


void rex_state_addrangetransition(rexstate_t *state, unsigned int c1,  unsigned int c2, unsigned long dstuid)
{
	rex_transition_t *t;
	rex_transition_t ntrans;
	long min, max, mid;

	ntrans.lowin = c1;
	ntrans.highin = c2;
	ntrans.type = REX_TRANSITION_RANGE;
	ntrans.dstuid = dstuid;
	ntrans.srcuid = state->uid;
	min = 0;
	max = min + r_array_length(state->trans);
	while (max > min) {
		mid = (min + max)/2;
		t = (rex_transition_t *)r_array_slot(state->trans, mid);
		if (c1 >= t->lowin) {
			min = mid + 1;
		} else {
			max = mid;
		}
	}
	r_array_insert(state->trans, min, &ntrans);
}


void rex_state_addrangetransition_dst(rexstate_t *srcstate, unsigned int c1,  unsigned int c2, const rexstate_t *dststate)
{
	rex_state_addrangetransition(srcstate, c1, c2, dststate->uid);
}


void rex_state_addsubstate(rexstate_t *state, unsigned long uid)
{
	unsigned long substate;
	long min, max, mid;

	min = 0;
	max = min + r_array_length(state->subset);
	while (max > min) {
		mid = (min + max)/2;
		substate = r_array_index(state->subset, mid, unsigned long);
		if (uid == substate) {
			return;
		} else if (uid >= substate) {
			min = mid + 1;
		} else {
			max = mid;
		}
	}
	r_array_insert(state->subset, min, &uid);
}


rboolean rex_state_findsubstate(rexstate_t *state, unsigned long uid)
{
	unsigned long substate;
	long min, max, mid;

	min = 0;
	max = min + r_array_length(state->subset);
	while (max > min) {
		mid = (min + max)/2;
		substate = r_array_index(state->subset, mid, unsigned long);
		if (uid == substate) {
			return TRUE;
		} else if (uid >= substate) {
			min = mid + 1;
		} else {
			max = mid;
		}
	}
	return FALSE;
}


void rex_state_addsubstate_dst(rexstate_t *state, const rexstate_t *dststate)
{
	rex_state_addsubstate(state, dststate->uid);
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
	rex_transition_t *t;

	fprintf(stdout, "State %ld", state->uid);
	if (r_array_length(state->subset)) {
		fprintf(stdout, "(");
		for (index = 0; index < r_array_length(state->subset); index++) {
			fprintf(stdout, "%ld,", r_array_index(state->subset, index, unsigned long));
		}
		fprintf(stdout, ")");
	}
	fprintf(stdout, ":\n");

	for (index = 0; index < r_array_length(state->etrans); index++) {
		t = (rex_transition_t *)r_array_slot(state->etrans, index);
		if (t->type == REX_TRANSITION_EMPTY) {
			fprintf(stdout, "    epsilon -> %ld\n", t->dstuid);
		}
	}
	for (index = 0; index < r_array_length(state->trans); index++) {
		t = (rex_transition_t *)r_array_slot(state->trans, index);
		if (t->type == REX_TRANSITION_EMPTY) {
			fprintf(stdout, "    epsilon -> %ld\n", t->dstuid);
		} else {
			fprintf(stdout, "          %c -> %ld\n", t->lowin, t->dstuid);
		}
	}
}

