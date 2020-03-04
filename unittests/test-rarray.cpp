// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

#include "gtest/gtest.h"
#include "rlib/rarray.h"
#include "rlib/rmem.h"

TEST(rarray_test, r_array_create)
{
	rarray_t *a = r_array_create(sizeof(int));

	EXPECT_NE((unsigned long)a, 0UL);
	r_array_destroy(a);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}

TEST(rarray_test, r_array_add)
{
	rarray_t *a = r_array_create(sizeof(int));
	int data[] = {0, 1, 2, 3, 4};

	EXPECT_EQ(r_array_empty(a), 1);

	for (auto i : data) {
		r_array_add(a, &i);
	}

	EXPECT_EQ(r_array_empty(a), 0);
	EXPECT_EQ(r_array_length(a), 5);
	EXPECT_EQ(*(int*)r_array_slot(a, 0), 0);
	EXPECT_EQ(*(int*)r_array_slot(a, 1), 1);
	EXPECT_EQ(*(int*)r_array_slot(a, 2), 2);
	EXPECT_EQ(*(int*)r_array_slot(a, 3), 3);
	r_array_destroy(a);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}

TEST(rarray_test, r_array_empty)
{
	rarray_t *a = r_array_create(sizeof(int));
	int data[] = {0, 1, 2, 3, 4};

	EXPECT_EQ(r_array_empty(a), 1);

	for (auto i : data) {
		r_array_add(a, &i);
	}
	EXPECT_EQ(r_array_empty(a), 0);
	r_array_destroy(a);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}

TEST(rarray_test, r_array_length)
{
	rarray_t *a = r_array_create(sizeof(int));
	int data[] = {0, 1, 2, 3, 4};

	EXPECT_EQ(r_array_length(a), 0);
	for (auto i : data) {
		r_array_add(a, &i);
	}
	EXPECT_EQ(r_array_length(a), 5);
	r_array_destroy(a);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}

TEST(rarray_test, r_array_insert)
{
	rarray_t *a = r_array_create(sizeof(int));
	int data[] = {0, 1, 2, 3, 4};

	for (auto i : data) {
		r_array_insert(a, 0, &i);
	}

	EXPECT_EQ(*(int*)r_array_slot(a, 0), 4);
	EXPECT_EQ(*(int*)r_array_slot(a, 1), 3);
	EXPECT_EQ(*(int*)r_array_slot(a, 2), 2);
	EXPECT_EQ(*(int*)r_array_slot(a, 3), 1);
	r_array_destroy(a);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}

TEST(rarray_test, r_array_slot)
{
	rarray_t *a = r_array_create(sizeof(int));
	int data[] = {0, 1, 2, 3, 4};

	for (auto i : data) {
		r_array_add(a, &i);
	}

	EXPECT_EQ(*(int*)r_array_slot(a, 0), 0);
	EXPECT_EQ(*(int*)r_array_slot(a, 1), 1);
	EXPECT_EQ(*(int*)r_array_slot(a, 2), 2);
	EXPECT_EQ(*(int*)r_array_slot(a, 3), 3);

	r_array_insert(a, 2, &data[4]);
	EXPECT_EQ(r_array_length(a), 6);
	EXPECT_EQ(*(int*)r_array_slot(a, 0), 0);
	EXPECT_EQ(*(int*)r_array_slot(a, 1), 1);
	EXPECT_EQ(*(int*)r_array_slot(a, 2), 4);
	EXPECT_EQ(*(int*)r_array_slot(a, 3), 2);
	EXPECT_EQ(*(int*)r_array_slot(a, 4), 3);

	r_array_insert(a, 8, &data[4]);
	EXPECT_EQ(r_array_length(a), 9);
	EXPECT_EQ(r_array_index(a, 0, int), 0);
	EXPECT_EQ(r_array_index(a, 1, int), 1);
	EXPECT_EQ(r_array_index(a, 2, int), 4);
	EXPECT_EQ(r_array_index(a, 3, int), 2);
	EXPECT_EQ(r_array_index(a, 4, int), 3);
	EXPECT_EQ(r_array_index(a, 8, int), 4);
	EXPECT_EQ(r_array_last(a, int), 4);
	r_array_push(a, 12, int);
	EXPECT_EQ(r_array_pop(a, int), 12);
	EXPECT_EQ(r_array_length(a), 9);

	r_array_destroy(a);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}

TEST(rarray_test, r_array_index)
{
	rarray_t *a = r_array_create(sizeof(int));
	int data[] = {0, 1, 2, 3, 4};

	for (auto i : data) {
		r_array_add(a, &i);
	}
	EXPECT_EQ(r_array_index(a, 0, int), 0);
	EXPECT_EQ(r_array_index(a, 1, int), 1);
	EXPECT_EQ(r_array_index(a, 2, int), 2);
	EXPECT_EQ(r_array_index(a, 3, int), 3);
	EXPECT_EQ(r_array_index(a, 4, int), 4);

	r_array_destroy(a);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}

TEST(rarray_test, r_array_push_pop)
{
	rarray_t *a = r_array_create(sizeof(int));
	int data[] = {0, 1, 2, 3, 4};

	for (auto i : data) {
		r_array_push(a, i, int);
	}

	EXPECT_EQ(r_array_length(a), 5);
	EXPECT_EQ(r_array_pop(a, int), 4);
	EXPECT_EQ(r_array_pop(a, int), 3);
	EXPECT_EQ(r_array_pop(a, int), 2);
	EXPECT_EQ(r_array_pop(a, int), 1);
	EXPECT_EQ(r_array_pop(a, int), 0);
	EXPECT_EQ(r_array_length(a), 0);
	r_array_destroy(a);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}

TEST(rarray_test, r_array_copy)
{
	rarray_t *a = r_array_create(sizeof(int));
	rarray_t *b = NULL;

	int data[] = {0, 1, 2, 3, 4};

	for (auto i : data) {
		r_array_push(a, i, int);
	}
	b = (rarray_t*)r_array_copy((const robject_t*)a);

	EXPECT_EQ(a->obj.type, b->obj.type);
	EXPECT_EQ(a->obj.size, b->obj.size);
	EXPECT_EQ(a->obj.cleanup, b->obj.cleanup);
	EXPECT_EQ(a->obj.copy, b->obj.copy);
	EXPECT_EQ(r_array_length(b), 5);
	EXPECT_EQ(r_array_pop(b, int), 4);
	EXPECT_EQ(r_array_pop(b, int), 3);
	EXPECT_EQ(r_array_pop(b, int), 2);
	EXPECT_EQ(r_array_pop(b, int), 1);
	EXPECT_EQ(r_array_pop(b, int), 0);
	EXPECT_EQ(r_array_length(b), 0);
	r_array_destroy(a);
	r_array_destroy(b);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}
