#include <stdio.h>
#include <string.h>

#include "rlib/rutf.h"
#include "rlib/rarray.h"
#include "rlib/rmem.h"
#include "rexdb.h"
#include "rexstate.h"
#include "rexnfasimulator.h"
#include "rexcompiler.h"

static void init_ababb(rexdb_t *nfa)
{
	rexstate_t *s0 = rex_state_create(0, REX_STATETYPE_START);
	rexstate_t *s1 = rex_state_create(1, REX_STATETYPE_NONE);
	rexstate_t *s2 = rex_state_create(2, REX_STATETYPE_NONE);
	rexstate_t *s3 = rex_state_create(3, REX_STATETYPE_NONE);
	rexstate_t *s4 = rex_state_create(4, REX_STATETYPE_NONE);
	rexstate_t *s5 = rex_state_create(5, REX_STATETYPE_NONE);
	rexstate_t *s6 = rex_state_create(6, REX_STATETYPE_NONE);
	rexstate_t *s7 = rex_state_create(7, REX_STATETYPE_NONE);
	rexstate_t *s8 = rex_state_create(8, REX_STATETYPE_NONE);
	rexstate_t *s9 = rex_state_create(9, REX_STATETYPE_NONE);
	rexstate_t *s10 = rex_state_create(10, REX_STATETYPE_ACCEPT);

	rex_db_insertstate(nfa, s0);
	rex_db_insertstate(nfa, s1);
	rex_db_insertstate(nfa, s2);
	rex_db_insertstate(nfa, s3);
	rex_db_insertstate(nfa, s4);
	rex_db_insertstate(nfa, s5);
	rex_db_insertstate(nfa, s6);
	rex_db_insertstate(nfa, s7);
	rex_db_insertstate(nfa, s8);
	rex_db_insertstate(nfa, s9);
	rex_db_insertstate(nfa, s10);

	rex_state_addtransition_e_dst(s0, s1);
	rex_state_addtransition_e_dst(s0, s7);
	rex_state_addtransition_e_dst(s1, s2);
	rex_state_addtransition_e_dst(s1, s4);
	rex_state_addtransition_e_dst(s3, s6);
	rex_state_addtransition_e_dst(s6, s1);
	rex_state_addtransition_e_dst(s5, s6);
	rex_state_addtransition_e_dst(s6, s7);

	rex_state_addtransition_dst(s2, 'a', 'a', s3);
//	rex_state_addtransition_dst(s4, 'b', 'z', s5);
	rex_state_addtransition_dst(s4, 'b', 'b', s5);
	rex_state_addtransition_dst(s7, 'a', 'a', s8);
	rex_state_addtransition_dst(s8, 'b', 'b', s9);
	rex_state_addtransition_dst(s9, 'b', 'b', s10);

}


int main(int argc, char *argv[])
{
	int i, inc;
	ruint32 wc;
	long startstate = -1;
	rexfragment_t *frag = NULL;
	const char *ptr, *in, *end;
	const char *name;
	rex_nfasimulator_t *si = rex_nfasimulator_create();
	rexdb_t *nfa = rex_db_create(REXDB_TYPE_NFA);
	rexcompiler_t *co = rex_compiler_create(nfa);


	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-e") == 0) {
			if (++i < argc) {
				name = argv[i];
			}
			if (++i < argc) {
				frag = rex_compiler_addexpression_s(co, frag, argv[i], (void*)name);
				if (frag) {
					startstate = frag->start;
				}
			}
		} else if (strcmp(argv[i], "-D") == 0) {
			int j;
			for (j = 0; j < r_array_length(nfa->states); j++) {
				rex_state_dump(rex_db_getstate(nfa, j));
			}
		} else if (strcmp(argv[i], "-ababb") == 0) {
			init_ababb(nfa);
			startstate = 0;
		} else {
			if (startstate < 0)
				return 1;
			ptr = in = argv[i];
			end = in + strlen(in);
//			ret = rex_nfasimulator_run(si, nfa, 0, in, end);
			rex_nfasimulator_start(si, nfa, startstate);
			while ((inc = r_utf8_mbtowc(&wc, (const unsigned char*)ptr, (const unsigned char*)end)) > 0) {
				rex_nfasimulator_next(si, nfa, wc, inc);
				ptr += inc;
			}
			if (r_array_length(si->accepts)) {
				rex_accept_t *acc = (rex_accept_t *)r_array_lastslot(si->accepts);
				if (acc->userdata)
					fprintf(stdout, "%s: ", (const char*)acc->userdata);
				fwrite(in, 1, acc->inputsize, stdout);
				fprintf(stdout, "\n");
			}
		}
	}

	if (frag) {
		rex_fragment_destroy(frag);
	}
	rex_db_destroy(nfa);
	rex_nfasimulator_destroy(si);
	rex_compiler_destroy(co);
	fprintf(stdout, "memory: %ld KB (leaked %ld Bytes)\n", (long)r_debug_get_maxmem()/1000, (long)r_debug_get_allocmem());

	return 0;
}
