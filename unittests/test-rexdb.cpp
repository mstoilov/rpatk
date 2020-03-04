
#include "gtest/gtest.h"
#include "rlib/rstring.h"
#include "rlib/rmem.h"
#include "rex/rexstate.h"
#include "rex/rexdb.h"
#include "rex/rexnfasimulator.h"

TEST(rexdb_test, rex_db_create)
{
	rexdb_t *nfa = rex_db_create(REXDB_TYPE_NFA);
	rexstate_t *q0 = rex_state_create(1, REX_STATETYPE_START);
	rexstate_t *q1 = rex_state_create(2, REX_STATETYPE_ACCEPT);
	rexstate_t *q2 = rex_state_create(3, REX_STATETYPE_NONE);

	rex_state_addtransition_dst(q0, '0', '0', q0);
	rex_state_addtransition_e_dst(q0, q1);
	rex_state_addtransition_dst(q0, '1', '1', q2);
	rex_state_addtransition_dst(q2, '1', '1', q1);
	rex_db_insertstate(nfa, q0);
	rex_db_insertstate(nfa, q1);
	rex_db_insertstate(nfa, q2);

	rex_db_destroy(nfa);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}


TEST(rexdb_test, rex_nfa_simulator)
{
	rex_nfasimulator_t * si = rex_nfasimulator_create();
	rexdb_t *nfa = rex_db_create(REXDB_TYPE_NFA);
	rexstate_t *q0 = rex_state_create(1, REX_STATETYPE_START);
	rexstate_t *q1 = rex_state_create(2, REX_STATETYPE_ACCEPT);
	rexstate_t *q2 = rex_state_create(3, REX_STATETYPE_NONE);

	rex_state_addtransition_dst(q0, '0', '0', q0);
	rex_state_addtransition_e_dst(q0, q1);
	rex_state_addtransition_dst(q0, '1', '1', q2);
	rex_state_addtransition_dst(q2, '1', '1', q1);
	rex_db_insertstate(nfa, q0);
	rex_db_insertstate(nfa, q1);
	rex_db_insertstate(nfa, q2);

	EXPECT_EQ(rex_nfasimulator_run_s(si, nfa, 1, "11"), 1);
	EXPECT_EQ(rex_nfasimulator_run_s(si, nfa, 1, "011"), 2);
	EXPECT_EQ(rex_nfasimulator_run_s(si, nfa, 1, "0011"), 3);

	rex_db_destroy(nfa);
	rex_nfasimulator_destroy(si);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}
