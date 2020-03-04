// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

#include "gtest/gtest.h"
#include "rlib/robject.h"

TEST(robject_test, r_object_create)
{
	robject_t *obj = r_object_create(sizeof(*obj));

	EXPECT_NE((unsigned long)obj, 0UL);
}
