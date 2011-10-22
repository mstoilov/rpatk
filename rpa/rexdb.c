#include <stdio.h>
#include "rlib/rmem.h"
#include "rlib/rutf.h"
#include "rpa/rextransition.h"
#include "rpa/rexdb.h"
#include "rpa/rexnfasimulator.h"


void rex_db_cleanup(robject_t *obj)
{
	long i;
	rexdb_t *rexdb = (rexdb_t*) obj;

	for (i = 0; i < r_array_length(rexdb->states); i++)
		rex_state_destroy(r_array_index(rexdb->states, i, rexstate_t*));
	r_array_destroy(rexdb->states);
	r_object_cleanup(obj);
}


robject_t *rex_db_init(robject_t *obj, unsigned int objtype, r_object_cleanupfun cleanup, rexdb_type_t type)
{
	rexdb_t *rexdb = (rexdb_t*) obj;

	r_object_init(obj, objtype, cleanup, NULL);
	rexdb->states = r_array_create(sizeof(rexstate_t*));
	rexdb->type = type;
	return (robject_t*)rexdb;
}


rexdb_t *rex_db_create(rexdb_type_t type)
{
	rexdb_t *rexdb;

	rexdb = (rexdb_t*)r_object_create(sizeof(*rexdb));
	rex_db_init((robject_t*)rexdb, R_OBJECT_ATON, rex_db_cleanup, type);
	return rexdb;
}


void rex_db_destroy(rexdb_t *rexdb)
{
	r_object_destroy((robject_t*)rexdb);
}


long rex_db_createstate(rexdb_t *rexdb, rex_statetype_t type)
{
	unsigned long uid = 0;
	rexstate_t *state;

	state = rex_state_create(-1, type);
	uid = r_array_add(rexdb->states, &state);
	state->uid = uid;
	return uid;
}


long rex_db_insertstate(rexdb_t *rexdb, rexstate_t *s)
{
	long uid = r_array_insert(rexdb->states, s->uid, &s);
	if (s->type == REX_STATETYPE_START)
		rexdb->start = s;
	return uid;
}


rexstate_t *rex_db_getstate(rexdb_t *rexdb, long uid)
{
	if (uid >= 0 && uid < r_array_length(rexdb->states))
		return r_array_index(rexdb->states, uid, rexstate_t*);
	return NULL;
}


int rex_db_addrangetrasition(rexdb_t *rexdb, unsigned int c1, unsigned int c2, unsigned long srcuid, unsigned long dstuid)
{
	rexstate_t *s = rex_db_getstate(rexdb, srcuid);

	if (s == NULL)
		return -1;
	rex_state_addrangetransition(s, c1, c2, dstuid);
	return 0;
}


int rex_db_addtrasition(rexdb_t *rexdb, unsigned int input, unsigned long srcuid, unsigned long dstuid)
{
	rexstate_t *s = rex_db_getstate(rexdb, srcuid);

	if (s == NULL)
		return -1;
	rex_state_addtransition(s, input, dstuid);
	return 0;
}


int rex_db_addtrasition_e(rexdb_t *rexdb, unsigned long srcuid, unsigned long dstuid)
{
	rexstate_t *s = rex_db_getstate(rexdb, srcuid);

	if (rexdb->type == REXDB_TYPE_DFA || s == NULL)
		return -1;
	rex_state_addtransition_e(s, dstuid);
	return 0;
}


int rex_db_simulate_nfa(rexdb_t *rexdb, long uid, const char *str, const char *end)
{
	rex_transition_t *t;
	rexstate_t *state = rex_db_getstate(rexdb, uid);
	long i;
	int ret, wcsize;
	ruint32 wc;

#if 1
	fprintf(stdout, "state: %ld, string: %s\n", uid, str);
#endif
	if (state->type == REX_STATETYPE_ACCEPT)
		return 0;

	for (i = 0; *str && i < r_array_length(state->trans); i++) {
		t = (rex_transition_t *)r_array_slot(state->trans, i);
		wcsize = r_utf8_mbtowc(&wc, (const unsigned char*)str, (const unsigned char*)end);
		if (t->lowin == wc) {
			ret = rex_db_simulate_nfa(rexdb, t->dstuid, str + wcsize, end);
			if (ret >= 0)
				return ret + wcsize;
		}
	}

	/*
	 * Look for e transitions
	 */
	for (i = 0; i < r_array_length(state->etrans); i++) {
		t = (rex_transition_t *)r_array_slot(state->etrans, i);
		ret = rex_db_simulate_nfa(rexdb, t->dstuid, str, end);
		if (ret >= 0)
			return ret;
	}

	return -1;
}


int rex_db_simulate_nfa2(rexdb_t *a, long uid, const char *str, const char *end)
{
	rex_nfasimulator_t *si;
	int ret;

	si = rex_nfasimulator_create();
	ret = rex_nfasimulator_run(si, a, uid, str, end);
	if (ret) {
		long i;
		rex_accept_t*pacc;
		for (i = 0; i < r_array_length(si->accepts); i++) {
			pacc = (rex_accept_t*)r_array_slot(si->accepts, i);
			r_printf("State: %ld, inputsize: %ld\n", pacc->state, pacc->inputsize);
		}


	}
	rex_nfasimulator_destroy(si);
	return ret;
}
