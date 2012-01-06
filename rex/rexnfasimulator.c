#include "rlib/rmem.h"
#include "rlib/rutf.h"
#include "rex/rexnfasimulator.h"
#include "rex/rextransition.h"


rex_nfasimulator_t *rex_nfasimulator_create()
{
	rex_nfasimulator_t *si;

	si = (rex_nfasimulator_t*) r_malloc(sizeof(*si));
	si->newstates = r_array_create(sizeof(unsigned long));
	si->oldstates = r_array_create(sizeof(unsigned long));
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


static void rex_nfasimulator_addstate(rex_nfasimulator_t *si, rexdb_t *a, long uid)
{
	long i;
	rexstate_t *s;
	rex_transition_t *t;
	rboolean on = TRUE;

	r_array_push(si->newstates, uid, unsigned long);
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
	long i;

	if (r_array_length(si->newstates)) {
		r_array_sort(si->newstates, unsigned long);
		for (i = 0; i < r_array_length(si->newstates); i++)
			r_printf("%ld, ", r_array_index(si->newstates, i, unsigned long));
		r_printf("\n");
	}

}

long rex_nfasimulator_run(rex_nfasimulator_t *si, rexdb_t *a, long uid, const char *str, const char *end)
{
	rexstate_t *s;
	rex_transition_t *t;
	ruint32 wc;
	rboolean off = FALSE;
	rboolean mapval;
	int wcsize;
	long i, ret = 0;
	unsigned long suid;
	rex_accept_t acc, *pacc;

	r_array_setlength(si->oldstates, 0);
	r_array_setlength(si->newstates, 0);
	r_array_setlength(si->accepts, 0);
	r_array_setlength(si->onmap, r_array_length(a->states));
	for (i = 0; i < r_array_length(a->states); i++)
		r_array_replace(si->onmap, i, &off);
	rex_nfasimulator_addstate(si, a, uid);

#if 1
	rex_nfasimulator_dumpnewstates(si);
#endif

	while (r_array_length(si->newstates)) {
		while (r_array_length(si->newstates)) {
			suid = r_array_pop(si->newstates, unsigned long);
			r_array_push(si->oldstates, suid, unsigned long);
			r_array_replace(si->onmap, suid, &off);
			s = rex_db_getstate(a, suid);
			if (s->type == REX_STATETYPE_ACCEPT) {
				acc.state = suid;
				acc.count = si->count;
				acc.inputsize = ret;
				acc.userdata = s->userdata;
				r_array_add(si->accepts, &acc);
			}
		}

		wcsize = r_utf8_mbtowc(&wc, (const unsigned char*)str + ret, (const unsigned char*)end);
		if (wcsize <= 0)
			goto end;
		ret += wcsize;
		while (r_array_length(si->oldstates)) {
			suid = r_array_pop(si->oldstates, unsigned long);
			s = rex_db_getstate(a, suid);
			for (i = 0; i < r_array_length(s->trans); i++) {
				t = (rex_transition_t *)r_array_slot(s->trans, i);
				if ((t->type == REX_TRANSITION_INPUT && t->lowin == wc) || (t->type == REX_TRANSITION_RANGE && t->lowin <=  wc && wc <= t->highin)) {
					mapval = r_array_index(si->onmap, t->dstuid, rboolean);
					if (mapval == 0)
						rex_nfasimulator_addstate(si, a, t->dstuid);
				}
			}
		}
#if 1
		rex_nfasimulator_dumpnewstates(si);
#endif
	}

end:
	if (r_array_length(si->accepts)) {
		pacc = (rex_accept_t*)r_array_slot(si->accepts, r_array_length(si->accepts) - 1);
		return pacc->inputsize;
	}
	return 0;
}


void rex_nfasimulator_start(rex_nfasimulator_t *si, rexdb_t *db, long uid)
{
	long i;
	unsigned long suid;
	rboolean off = FALSE;

	si->count = 0;
	si->inputsize = 0;
	r_array_setlength(si->oldstates, 0);
	r_array_setlength(si->newstates, 0);
	r_array_setlength(si->onmap, r_array_length(db->states));
	for (i = 0; i < r_array_length(db->states); i++)
		r_array_replace(si->onmap, i, &off);
	rex_nfasimulator_addstate(si, db, uid);

#if 1
	rex_nfasimulator_dumpnewstates(si);
#endif

	while (r_array_length(si->newstates)) {
		suid = r_array_pop(si->newstates, unsigned long);
		r_array_push(si->oldstates, suid, unsigned long);
		r_array_replace(si->onmap, suid, &off);
	}
}


long rex_nfasimulator_next(rex_nfasimulator_t *si, rexdb_t *db, ruint32 wc, int wcsize)
{
	rexstate_t *s;
	rex_transition_t *t;
	rboolean off = FALSE;
	rboolean mapval;
	long i;
	unsigned long suid;

	if (!r_array_length(si->oldstates))
		return 0;
	si->count += 1;
	si->inputsize += wcsize;
	while (r_array_length(si->oldstates)) {
		suid = r_array_pop(si->oldstates, unsigned long);
		s = rex_db_getstate(db, suid);
		for (i = 0; i < r_array_length(s->trans); i++) {
			t = (rex_transition_t *)r_array_slot(s->trans, i);
			if ((t->type == REX_TRANSITION_INPUT && t->lowin == wc) || (t->type == REX_TRANSITION_RANGE && t->lowin <=  wc && wc <= t->highin)) {
				mapval = r_array_index(si->onmap, t->dstuid, rboolean);
				if (mapval == 0)
					rex_nfasimulator_addstate(si, db, t->dstuid);
			}
		}
	}
#if 1
	rex_nfasimulator_dumpnewstates(si);
#endif

	while (r_array_length(si->newstates)) {
		suid = r_array_pop(si->newstates, unsigned long);
		r_array_push(si->oldstates, suid, unsigned long);
		r_array_replace(si->onmap, suid, &off);
		s = rex_db_getstate(db, suid);
		if (s->type == REX_STATETYPE_ACCEPT) {
			rex_accept_t acc;
			acc.count = si->count;
			acc.inputsize = si->inputsize;
			acc.state = suid;
			acc.userdata = s->userdata;
			r_array_add(si->accepts, &acc);

		}
	}

	return r_array_length(si->oldstates);
}
