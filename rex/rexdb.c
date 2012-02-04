#include <stdio.h>
#include <ctype.h>
#include "rlib/rmem.h"
#include "rlib/rutf.h"
#include "rex/rextransition.h"
#include "rex/rexnfasimulator.h"
#include "rex/rexdfaconv.h"
#include "rex/rexcompiler.h"

struct rexdfa_ctx {
	unsigned long nstates;
	unsigned long ntrnas;
	unsigned long nsubstates;
	unsigned long naccsubstates;
};


rexdb_t *rex_db_createdfa(rexdb_t *nfa, unsigned long start)
{
	rexdfaconv_t *conv = rex_dfaconv_create();
	rexdb_t *dfa = rex_dfaconv_run(conv, nfa, start);
	rex_dfaconv_destroy(conv);
	return dfa;
}


void rex_db_cleanup(robject_t *obj)
{
	long i;
	rexdb_t *rexdb = (rexdb_t*) obj;

	for (i = 0; i < r_array_length(rexdb->states); i++)
		rex_state_destroy(r_array_index(rexdb->states, i, rexstate_t*));
	r_array_destroy(rexdb->states);
	r_array_destroy(rexdb->substates);
	rex_compiler_destroy(rexdb->co);
	r_object_cleanup(obj);
}


robject_t *rex_db_init(robject_t *obj, unsigned int objtype, r_object_cleanupfun cleanup, rexdb_type_t type)
{
	rexdb_t *rexdb = (rexdb_t*) obj;

	r_object_init(obj, objtype, cleanup, NULL);
	rexdb->co = NULL;
	rexdb->states = r_array_create(sizeof(rexstate_t*));
	rexdb->substates = r_array_create(sizeof(rexsubstate_t));
	rexdb->type = type;
	if (type == REXDB_TYPE_DFA) {
		/*
		 * Create the dead state.
		 */
		long uid = rex_db_createstate(rexdb , REX_STATETYPE_DEAD);
		rex_state_addtransition(rex_db_getstate(rexdb, uid), 0, REX_CHAR_MAX, uid);
	}
	if (type == REXDB_TYPE_NFA) {
		rexdb->co = rex_compiler_create(rexdb);
	}
	return (robject_t*)rexdb;
}


rexdb_t *rex_db_create(rexdb_type_t type)
{
	rexdb_t *rexdb;

	rexdb = (rexdb_t*)r_object_create(sizeof(*rexdb));
	rex_db_init((robject_t*)rexdb, R_OBJECT_REXDB, rex_db_cleanup, type);
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


long rex_db_findstate(rexdb_t *a, const rarray_t *subset)
{
	long i, j;
	rexstate_t *s;

	for (i = 0; i < r_array_length(a->states); i++) {
		s = r_array_index(a->states, i, rexstate_t*);
		if (r_array_length(s->subset) && r_array_length(s->subset) == r_array_length(subset)) {
			for (j = 0; j < r_array_length(subset); j++) {
				if (rex_subset_index(s->subset, j) != rex_subset_index(subset, j))
					break;
			}
			if (j == r_array_length(subset))
				return s->uid;
		}
	}
	return -1;
}


rex_transition_t *rex_db_addrangetrasition(rexdb_t *rexdb, rexchar_t c1, rexchar_t c2, unsigned long srcuid, unsigned long dstuid)
{
	rexstate_t *s = rex_db_getstate(rexdb, srcuid);

	if (s == NULL)
		return NULL;
	return rex_state_addtransition(s, c1, c2, dstuid);
}


rex_transition_t *rex_db_addtrasition_e(rexdb_t *rexdb, unsigned long srcuid, unsigned long dstuid)
{
	rexstate_t *s = rex_db_getstate(rexdb, srcuid);

	if (rexdb->type == REXDB_TYPE_DFA || s == NULL)
		return NULL;
	return rex_state_addtransition_e(s, dstuid);
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


rexsubstate_t *rex_db_getsubstate(rexdb_t *rexdb, unsigned long uid)
{
	if (uid < r_array_length(rexdb->substates)) {
		return (rexsubstate_t *)r_array_slot(rexdb->substates, uid);
	}
	return NULL;
}


long rex_db_addexpression(rexdb_t *rexdb, unsigned long prev, const char *str, unsigned int size, rexuserdata_t userdata)
{
	if (rexdb->type != REXDB_TYPE_NFA)
		return -1;
	return rex_compiler_addexpression(rexdb->co, rexdb, prev, str, size, userdata);
}


long rex_db_addexpression_s(rexdb_t *rexdb, unsigned long prev, const char *str, rexuserdata_t userdata)
{
	if (rexdb->type != REXDB_TYPE_NFA)
		return -1;
	return rex_compiler_addexpression_s(rexdb->co, rexdb, prev, str, userdata);
}


void rex_db_dumpstate(rexdb_t *rexdb, unsigned long uid)
{
	long index;
	char buf[240];
	int bufsize = sizeof(buf) - 1;
	int n = 0;
	rex_transition_t *t;
	rexsubstate_t *ss;
	rexstate_t *state = rex_db_getstate(rexdb, uid);

	if (!state)
		return;
	fprintf(stdout, "State %ld", state->uid);
	if (rex_subset_length(state->subset)) {
		fprintf(stdout, " (");
		for (index = 0; index < rex_subset_length(state->subset); index++) {
			fprintf(stdout, "%ld", rex_subset_index(state->subset, index));
			if ((ss = rex_db_getsubstate(rexdb, rex_subset_index(state->subset, index))) != NULL) {
				if (ss->ss_type == REX_STATETYPE_ACCEPT)
					fprintf(stdout, "*");
			}
			fprintf(stdout, ",");
		}
		fprintf(stdout, ")");
	}
	fprintf(stdout, ": ");
	if (state->type == REX_STATETYPE_ACCEPT) {
		fprintf(stdout, " REX_STATETYPE_ACCEPT ");
	} else if (state->type == REX_STATETYPE_DEAD) {
		fprintf(stdout, " REX_STATETYPE_DEAD ");
	} else if (state->type == REX_STATETYPE_START) {
		fprintf(stdout, " REX_STATETYPE_START ");
	}
	fprintf(stdout, "\n");

	for (index = 0; index < r_array_length(state->etrans); index++) {
		n = 0;
		t = (rex_transition_t *)r_array_slot(state->etrans, index);
		if (t->type == REX_TRANSITION_EMPTY) {
			n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "        epsilon ");
			r_memset(buf + n, ' ', bufsize - n);
			n = 40;
			n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "-> %ld", t->dstuid);
			fprintf(stdout, "%s\n", buf);
		}
	}
	for (index = 0; index < r_array_length(state->trans); index++) {
		t = (rex_transition_t *)r_array_slot(state->trans, index);
		n = 0;
		if (t->type == REX_TRANSITION_EMPTY) {
			fprintf(stdout, "    epsilon -> %ld\n", t->dstuid);
		} else if (t->lowin != t->highin) {
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


long rex_db_numtransitions(rexdb_t *rexdb)
{
	long i;
	long ret = 0;

	for (i = 0; i < r_array_length(rexdb->states); i++) {
		rexstate_t *state = rex_db_getstate(rexdb, i);
		ret += r_array_length(state->trans);
	}
	return ret;
}


long rex_db_numstates(rexdb_t *rexdb)
{
	return r_array_length(rexdb->states);
}


long rex_db_numsubstates(rexdb_t *rexdb)
{
	long i;
	long ret = 0;

	for (i = 0; i < r_array_length(rexdb->states); i++) {
		rexstate_t *state = rex_db_getstate(rexdb, i);
		ret += rex_subset_length(state->subset);
	}
	return ret;
}


long rex_db_numaccsubstates(rexdb_t *rexdb)
{
	long i, j;
	long ret = 0;

	for (i = 0; i < r_array_length(rexdb->states); i++) {
		rexstate_t *state = rex_db_getstate(rexdb, i);
		for (j = 0; j < rex_subset_length(state->subset); j++) {
			rexsubstate_t *substate = rex_db_getsubstate(rexdb, rex_subset_index(state->subset, j));
			if (substate->ss_type == REX_STATETYPE_ACCEPT)
				ret += 1;
		}
	}
	return ret;
}


const char *rex_db_version()
{
	return "1.0";
}


static void rex_db_filldfastate(rexdb_t *db, rexdfa_t *dfa, struct rexdfa_ctx *ctx, rexstate_t *state)
{
	long i;
	rex_transition_t *t = NULL;
	rexdfs_t *s = &dfa->states[ctx->nstates++];
	s->type = state->type;
	s->trans = ctx->ntrnas;
	s->ntrans = r_array_length(state->trans);
	for (i = 0; i < s->ntrans; i++) {
		t = (rex_transition_t *)r_array_slot(state->trans, i);
		dfa->trans[s->trans + i].lowin = t->lowin;
		dfa->trans[s->trans + i].highin = t->highin;
		dfa->trans[s->trans + i].state = t->dstuid;
	}
	ctx->ntrnas += s->ntrans;
	s->substates = ctx->nsubstates;
	s->nsubstates = rex_subset_length(state->subset);
	for (i = 0; i < s->nsubstates; i++) {
		unsigned long uid = rex_subset_index(state->subset, i);
		rexsubstate_t *substate = rex_db_getsubstate(db, uid);
		dfa->substates[s->substates + i].uid = uid;
		dfa->substates[s->substates + i].type = substate->ss_type;
		dfa->substates[s->substates + i].userdata = substate->ss_userdata;
	}
	ctx->nsubstates += s->nsubstates;
	s->accsubstates = ctx->naccsubstates;
	s->naccsubstates = 0L;
	for (i = 0; i < s->nsubstates; i++) {
		unsigned long uid = rex_subset_index(state->subset, i);
		rexsubstate_t *substate = rex_db_getsubstate(db, uid);
		if (substate->ss_type == REX_STATETYPE_ACCEPT) {
			dfa->accsubstates[s->accsubstates + s->naccsubstates].uid = uid;
			dfa->accsubstates[s->accsubstates + s->naccsubstates].type = substate->ss_type;
			dfa->accsubstates[s->accsubstates + s->naccsubstates].userdata = substate->ss_userdata;
			s->naccsubstates++;
		}
	}
	ctx->naccsubstates += s->naccsubstates;
}


rexdfa_t *rex_db_todfa(rexdb_t *db)
{
	long i;
	rexdfa_t *dfa;
	struct rexdfa_ctx ctx;
	unsigned long nstates = rex_db_numstates(db);
	unsigned long ntrans = rex_db_numtransitions(db);
	unsigned long nsubstates = rex_db_numsubstates(db);
	unsigned long naccsubstates = rex_db_numaccsubstates(db);
	dfa = rex_dfa_create(nstates, ntrans, naccsubstates, nsubstates);
	r_memset(&ctx, 0, sizeof(ctx));

	for (i = 0; i < r_array_length(db->states); i++) {
		rexstate_t *state = rex_db_getstate(db, i);
		rex_db_filldfastate(db, dfa, &ctx, state);
	}
	R_ASSERT(ctx.nstates == nstates);
	R_ASSERT(ctx.ntrnas == ntrans);
	R_ASSERT(ctx.nsubstates == nsubstates);
	R_ASSERT(ctx.naccsubstates == naccsubstates);
	return dfa;
}
