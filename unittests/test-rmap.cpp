// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

#include "gtest/gtest.h"
#include "rlib/rstring.h"
#include "rlib/rmap.h"
#include "rlib/rmem.h"

TEST(rmap_test, r_map_create)
{
	rmap_t *m = r_map_create(sizeof(int), 5);

	r_map_destroy(m);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}

TEST(rmap_test, r_map_add)
{
	rmap_t *m = r_map_create(sizeof(int), 5);

	int one = 1, two = 2, three = 3;

	r_map_add_s(m, "one", &one);
	r_map_add_s(m, "two", &two);
	r_map_add_s(m, "three", &three);


	r_map_destroy(m);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}

TEST(rmap_test, r_map_lookup)
{
	rmap_t *m = r_map_create(sizeof(int), 5);

	int one = 1, two = 2, three = 3;

	r_map_add_s(m, "one", &one);
	r_map_add_s(m, "two", &two);
	r_map_add_s(m, "three", &three);

	EXPECT_NE(r_map_value(m, r_map_lookup_s(m, "one")), (void*)NULL);
	EXPECT_EQ(r_map_value(m, r_map_lookup_s(m, "five")), (void*)NULL);

	if (r_map_value(m, r_map_lookup_s(m, "one")) != NULL)
		EXPECT_EQ(*((int*)r_map_value(m, r_map_lookup_s(m, "one"))), 1);


	r_map_destroy(m);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}

TEST(rmap_test, r_map_delete)
{
	rmap_t *m = r_map_create(sizeof(int), 5);

	int one = 1, two = 2, three = 3;

	size_t idx_one = r_map_add_s(m, "one", &one);
	size_t idx_two = r_map_add_s(m, "two", &two);
	size_t idx_three = r_map_add_s(m, "three", &three);

	EXPECT_EQ(r_map_lookup_s(m, "one"), idx_one);
	EXPECT_EQ(r_map_lookup_s(m, "two"), idx_two);
	EXPECT_EQ(r_map_lookup_s(m, "three"), idx_three);
	EXPECT_NE(r_map_value(m, r_map_lookup_s(m, "one")), nullptr);
	EXPECT_EQ(r_map_value(m, r_map_lookup_s(m, "five")), nullptr);
	if (r_map_value(m, r_map_lookup_s(m, "one")) != NULL)
		EXPECT_EQ(*((int*)r_map_value(m, r_map_lookup_s(m, "one"))), 1);

	r_map_delete(m, idx_one);
	EXPECT_EQ(r_map_lookup_s(m, "one"), (size_t)-1);
	idx_one = r_map_add_s(m, "one", &one);
	EXPECT_EQ(r_map_lookup_s(m, "one"), idx_one);

	r_map_destroy(m);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}
