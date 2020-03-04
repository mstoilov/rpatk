
#include "gtest/gtest.h"
#include "rlib/rstring.h"
#include "rlib/rmem.h"
#include "rex/rexstate.h"

TEST(rexstate_test, rex_state_create)
{
	rexstate_t *q0 = rex_state_create(0, REX_STATETYPE_DEAD);
	rex_state_addtransition_e(q0, 1);
	rex_state_addtransition(q0, 'a', 'a', 1);
	rex_state_addtransition(q0, 'd', 'f', 2);

	rex_state_destroy(q0);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}


TEST(rexstate_test, rex_state_copy)
{
	rexstate_t *q0 = rex_state_create(1, REX_STATETYPE_START);
	rexstate_t *q1 = NULL;

	rex_state_addtransition_e(q0, 1);
	rex_state_addtransition(q0, 'a', 'a', 1);
	rex_state_addtransition(q0, 'd', 'f', 2);
	rex_state_addsubstate(q0, 10, REX_STATETYPE_START, 0);
	rex_state_addsubstate(q0, 15, REX_STATETYPE_DEAD, 0);
	rex_state_addsubstate(q0, 13, REX_STATETYPE_NONE, 0);
	rex_state_addsubstate(q0, 14, REX_STATETYPE_NONE, 0);


	q1 = (rexstate_t *)q0->obj.copy((robject_t*)q0);

	EXPECT_EQ(q1->obj.type, q0->obj.type);
	EXPECT_EQ(q1->obj.size, q0->obj.size);
	EXPECT_EQ(q1->obj.cleanup, q0->obj.cleanup);
	EXPECT_EQ(q1->obj.copy, q0->obj.copy);
	EXPECT_EQ(q1->type, q0->uid);
	EXPECT_EQ(q1->uid, q0->uid);
	EXPECT_EQ(q1->etrans->len, q1->etrans->len);
	EXPECT_EQ(q1->trans->len, q1->trans->len);
	EXPECT_EQ(((rex_transition_t*)r_array_slot(q1->trans, 0))->lowin, ((rex_transition_t*)r_array_slot(q0->trans, 0))->lowin);
	EXPECT_EQ(((rex_transition_t*)r_array_slot(q1->trans, 0))->highin, ((rex_transition_t*)r_array_slot(q0->trans, 0))->highin);
	EXPECT_EQ(r_array_length(q1->subset), 4);
	EXPECT_EQ(((rexsubstate_t*)r_array_slot(q1->subset, 0))->ss_uid, 10);
	EXPECT_EQ(((rexsubstate_t*)r_array_slot(q1->subset, 1))->ss_uid, 13);
	EXPECT_EQ(((rexsubstate_t*)r_array_slot(q1->subset, 2))->ss_uid, 14);
	EXPECT_EQ(((rexsubstate_t*)r_array_slot(q1->subset, 3))->ss_uid, 15);
	EXPECT_EQ(((rexsubstate_t*)r_array_slot(q1->subset, 0))->ss_type, ((rexsubstate_t*)r_array_slot(q0->subset, 0))->ss_type);
	rex_state_destroy(q1);
	rex_state_destroy(q0);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}
