// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

#include "gtest/gtest.h"
#include "rlib/rstring.h"
#include "rlib/rhash.h"
#include "rlib/rmem.h"

TEST(rhash_test, r_hash_create)
{
	rhash_t *h = r_hash_create(4, r_hash_strequal ,r_hash_strhash);

	r_hash_destroy(h);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}


TEST(rhash_test, r_hash_insert)
{
	rhash_t *h = r_hash_create(4, r_hash_strequal ,r_hash_strhash);

	r_hash_insert_index(h, "one", 1);
	r_hash_insert_index(h, "two", 2);
	r_hash_insert_index(h, "three", 3);
	r_hash_insert_index(h, "four", 4);

	EXPECT_EQ(r_hash_lookup_index(h, "one"), 1);
	EXPECT_EQ(r_hash_lookup_index(h, "two"), 2);
	EXPECT_EQ(r_hash_lookup_index(h, "three"), 3);
	EXPECT_EQ(r_hash_lookup_index(h, "four"), 4);
	EXPECT_EQ(r_hash_lookup_index(h, "five"), R_HASH_INVALID_INDEXVAL);

	r_hash_destroy(h);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}
