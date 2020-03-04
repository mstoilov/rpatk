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
#include "rex/rexdfasimulator.h"
#include "rex/rextransition.h"


rex_dfasimulator_t *rex_dfasimulator_create()
{
	rex_dfasimulator_t *si;

	si = (rex_dfasimulator_t*) r_malloc(sizeof(*si));
	si->accepts = r_array_create(sizeof(rex_accept_t));
	return si;
}


void rex_dfasimulator_destroy(rex_dfasimulator_t *si)
{
	if (si) {
		r_array_destroy(si->accepts);
		r_free(si);
	}
}


size_t rex_dfasimulator_run(rex_dfasimulator_t *si, rexdb_t *dfa, size_t uid, const char *input, size_t size)
{
	ruint32 wc;
	int inc = 0;
	size_t ret = 0;
	const char *end = input + size;

	rex_dfasimulator_start(si, dfa, uid);
	while ((inc = r_utf8_mbtowc(&wc, (const unsigned char*)input, (const unsigned char*)end)) > 0) {
		input += inc;
		if (rex_dfasimulator_next(si, dfa, wc, inc) <= 0)
			break;
	}
	if (r_array_length(si->accepts) > 0) {
		rex_accept_t *acc = (rex_accept_t *)r_array_lastslot(si->accepts);
		ret = acc->inputsize;
	}
	return ret;
}


void rex_dfasimulator_start(rex_dfasimulator_t *si, rexdb_t *db, size_t uid)
{
	si->count = 0;
	si->inputsize = 0;
	si->next = rex_db_getstate(db, uid);
	r_array_setlength(si->accepts, 0);
}


size_t rex_dfasimulator_next(rex_dfasimulator_t *si, rexdb_t *db, ruint32 wc, int wcsize)
{
	rex_transition_t *t;

	si->count += 1;
	si->inputsize += wcsize;
	if (!si->next || si->next->type == REX_STATETYPE_DEAD)
		return 0;
	t = rex_transitions_find(si->next->trans, wc);
	si->next = rex_db_getstate(db, t->dstuid);
	if (si->next->type == REX_STATETYPE_ACCEPT) {
		rex_accept_t acc;
		acc.count = si->count;
		acc.inputsize = si->inputsize;
		acc.state = si->next;
		r_array_add(si->accepts, &acc);
	}
	return si->next->uid;
}
