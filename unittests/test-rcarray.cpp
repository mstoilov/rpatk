// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

#include "gtest/gtest.h"
#include "rlib/rcarray.h"
#include "rlib/rmem.h"

TEST(rcarray_test, r_carray_create)
{
	rcarray_t *a = r_carray_create(sizeof(int));

	EXPECT_NE((unsigned long)a, 0UL);
	r_carray_destroy(a);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}

TEST(rcarray_test, r_carray_add)
{
	rcarray_t *a = r_carray_create(sizeof(int));
	int data[] = {0, 1, 2, 3, 4};

	EXPECT_EQ(r_carray_empty(a), 1);

	for (auto i : data) {
		r_carray_add(a, &i);
	}

	EXPECT_EQ(r_carray_empty(a), 0);
	EXPECT_EQ(r_carray_length(a), 5);
	EXPECT_EQ(*(int*)r_carray_slot(a, 0), 0);
	EXPECT_EQ(*(int*)r_carray_slot(a, 1), 1);
	EXPECT_EQ(*(int*)r_carray_slot(a, 2), 2);
	EXPECT_EQ(*(int*)r_carray_slot(a, 3), 3);
	r_carray_destroy(a);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}

TEST(rcarray_test, r_carray_empty)
{
	rcarray_t *a = r_carray_create(sizeof(int));
	int data[] = {0, 1, 2, 3, 4};

	EXPECT_EQ(r_carray_empty(a), 1);

	for (auto i : data) {
		r_carray_add(a, &i);
	}
	EXPECT_EQ(r_carray_empty(a), 0);
	r_carray_destroy(a);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}

TEST(rcarray_test, r_carray_length)
{
	rcarray_t *a = r_carray_create(sizeof(int));
	int data[] = {0, 1, 2, 3, 4};

	EXPECT_EQ(r_carray_length(a), 0);
	for (auto i : data) {
		r_carray_add(a, &i);
	}
	EXPECT_EQ(r_carray_length(a), 5);
	r_carray_destroy(a);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}


TEST(rcarray_test, r_carray_slot)
{
	rcarray_t *a = r_carray_create(sizeof(int));
	int data[] = {0, 1, 2, 3, 4};

	for (auto i : data) {
		r_carray_add(a, &i);
	}

	EXPECT_EQ(*(int*)r_carray_slot(a, 0), 0);
	EXPECT_EQ(*(int*)r_carray_slot(a, 1), 1);
	EXPECT_EQ(*(int*)r_carray_slot(a, 2), 2);
	EXPECT_EQ(*(int*)r_carray_slot(a, 3), 3);

	r_carray_destroy(a);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}

TEST(rcarray_test, r_carray_index)
{
	rcarray_t *a = r_carray_create(sizeof(int));
	int data[] = {0, 1, 2, 3, 4};

	for (auto i : data) {
		r_carray_add(a, &i);
	}
	EXPECT_EQ(r_carray_index(a, 0, int), 0);
	EXPECT_EQ(r_carray_index(a, 1, int), 1);
	EXPECT_EQ(r_carray_index(a, 2, int), 2);
	EXPECT_EQ(r_carray_index(a, 3, int), 3);
	EXPECT_EQ(r_carray_index(a, 4, int), 4);

	r_carray_destroy(a);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}


TEST(rcarray_test, r_carray_copy)
{
	rcarray_t *a = r_carray_create(sizeof(int));
	rcarray_t *b = NULL;

	int data[] = {0, 1, 2, 3, 4};

	for (int i = 0; i < 1000; i++) {
		r_carray_add(a, &i);
	}
	b = (rcarray_t*)r_carray_copy((const robject_t*)a);

	EXPECT_EQ(a->obj.type, b->obj.type);
	EXPECT_EQ(a->obj.size, b->obj.size);
	EXPECT_EQ(a->obj.cleanup, b->obj.cleanup);
	EXPECT_EQ(a->obj.copy, b->obj.copy);
	EXPECT_EQ(r_carray_length(b), 1000);
	EXPECT_EQ(r_carray_index(b, 800, int), 800);

	r_carray_destroy(a);
	r_carray_destroy(b);

	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}
