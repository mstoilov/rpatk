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

#include "rlib/rmem.h"
#include "rlib/rutf.h"
#include "rex/rexnfasimulator.h"
#include "rex/rextransition.h"


rex_nfasimulator_t *rex_nfasimulator_create()
{
	rex_nfasimulator_t *si;

	si = (rex_nfasimulator_t*) r_malloc(sizeof(*si));
	si->newstates = r_array_create(sizeof(size_t));
	si->oldstates = r_array_create(sizeof(size_t));
	si->accepts = r_array_create(sizeof(rex_accept_t));
	si->onmap = r_array_create(sizeof(rboolean));
	return si;
}


void rex_nfasimulator_destroy(rex_nfasimulator_t *si)
{
	if (si) {
		r_array_destroy(si->newstates);
		r_array_destroy(si->oldstates);
		r_array_destroy(si->accepts);
		r_array_destroy(si->onmap);
		r_free(si);
	}
}

/**
 * Add the uid to the si->newstates set and also add all
 * states that are reachable via empty transitions.
 */
static void rex_nfasimulator_addstate(rex_nfasimulator_t *si, rexdb_t *a, size_t uid)
{
	size_t i;
	rexstate_t *s;
	rex_transition_t *t;
	rboolean on = TRUE;

	r_array_push(si->newstates, uid, size_t);
	r_array_replace(si->onmap, uid, &on);
	s = rex_db_getstate(a, uid);
	for (i = 0; i < r_array_length(s->etrans); i++) {
		t = (rex_transition_t *)r_array_slot(s->etrans, i);
		if (r_array_index(si->onmap, t->dstuid, rboolean) == 0) {
			rex_nfasimulator_addstate(si, a, t->dstuid);
		}
	}
}


void rex_nfasimulator_dumpnewstates(rex_nfasimulator_t *si)
{
	size_t i;

	if (r_array_length(si->newstates)) {
		r_array_sort(si->newstates, size_t);
		for (i = 0; i < r_array_length(si->newstates); i++)
			r_printf("%ld, ", r_array_index(si->newstates, i, size_t));
		r_printf("\n");
	}

}


size_t rex_nfasimulator_run_s(rex_nfasimulator_t *si, rexdb_t *nfa, size_t uid, const char *str)
{
	return rex_nfasimulator_run(si, nfa, uid, str, r_strlen(str));
}


void rex_nfasimulator_start(rex_nfasimulator_t *si, rexdb_t *db, size_t uid)
{
	size_t i;
	size_t suid;
	rboolean off = FALSE;

	si->count = 0;
	si->inputsize = 0;
	r_array_setlength(si->accepts, 0);
	r_array_setlength(si->oldstates, 0);
	r_array_setlength(si->newstates, 0);
	r_array_setlength(si->onmap, r_array_length(db->states));
	for (i = 0; i < r_array_length(db->states); i++)
		r_array_replace(si->onmap, i, &off);
	rex_nfasimulator_addstate(si, db, uid);
#if 0
	rex_nfasimulator_dumpnewstates(si);
#endif
	while (r_array_length(si->newstates)) {
		suid = r_array_pop(si->newstates, size_t);
		r_array_push(si->oldstates, suid, size_t);
		r_array_replace(si->onmap, suid, &off);
	}
}


size_t rex_nfasimulator_run(rex_nfasimulator_t *si, rexdb_t *nfa, size_t uid, const char *str, size_t size)
{
	int inc;
	ruint32 wc;
	const char *ptr = str;
	const char *end = str + size;

	rex_nfasimulator_start(si, nfa, uid);
	while ((inc = r_utf8_mbtowc(&wc, (const unsigned char*)ptr, (const unsigned char*)end)) > 0) {
		if (rex_nfasimulator_next(si, nfa, wc, inc) == 0)
			break;
		ptr += inc;
	}

	return r_array_length(si->accepts);
}


size_t rex_nfasimulator_next(rex_nfasimulator_t *si, rexdb_t *db, ruint32 wc, size_t wcsize)
{
	rexstate_t *s;
	rex_transition_t *t;
	rboolean off = FALSE;
	rboolean mapval;
	size_t i;
	size_t suid;

	if (!r_array_length(si->oldstates))
		return 0;
	si->count += 1;
	si->inputsize += wcsize;

	/*
	 * Find all states that are reachable through input (wc) transition
	 * and add it to the si->newstates list.
	 */
	while (r_array_length(si->oldstates)) {
		suid = r_array_pop(si->oldstates, size_t);
		s = rex_db_getstate(db, suid);
		for (i = 0; i < r_array_length(s->trans); i++) {
			t = (rex_transition_t *)r_array_slot(s->trans, i);
			if (t->lowin <=  wc && wc <= t->highin) {
				mapval = r_array_index(si->onmap, t->dstuid, rboolean);
				if (mapval == 0) {
					/*
					 * Only add the state if it hasn't already been added.
					 *
					 */
					rex_nfasimulator_addstate(si, db, t->dstuid);
				}
			}
		}
	}
#if 0
	rex_nfasimulator_dumpnewstates(si);
#endif

	while (r_array_length(si->newstates)) {
		suid = r_array_pop(si->newstates, size_t);
		r_array_push(si->oldstates, suid, size_t);
		r_array_replace(si->onmap, suid, &off);
		s = rex_db_getstate(db, suid);
		if (s->type == REX_STATETYPE_ACCEPT) {
			rex_accept_t acc;
			acc.count = si->count;
			acc.inputsize = si->inputsize;
			acc.state = s;
			r_array_add(si->accepts, &acc);

		}
	}

	return r_array_length(si->oldstates);
}
