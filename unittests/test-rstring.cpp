// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

#include "gtest/gtest.h"
#include "rlib/rstring.h"
#include "rlib/rmem.h"

TEST(rstring_test, r_string_create_from_ansi)
{
	rstring_t *str = r_string_create_from_ansi("TEST");
	EXPECT_STREQ(str->str, "TEST");

	r_string_destroy(str);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}

TEST(rstring_test, r_string_create_from_str_len_less)
{
	rstring_t *str = r_string_create_from_str_len("TEST", 3);
	EXPECT_STREQ(str->str, "TES");
	EXPECT_EQ(str->size, 3);

	r_string_destroy(str);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}

TEST(rstring_test, r_string_create_from_str_len_more)
{
	rstring_t *str = r_string_create_from_str_len("TEST", 10);
	EXPECT_STREQ(str->str, "TEST");
	EXPECT_EQ(str->size, 4);

	r_string_destroy(str);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}

TEST(rstring_test, r_string_create_from_str_len_empty)
{
	rstring_t *str = r_string_create_from_str_len("TEST", 0);
	EXPECT_STREQ(str->str, "");
	EXPECT_EQ(str->size, 0);

	r_string_destroy(str);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}

TEST(rstring_test, r_string_create_from_double)
{
	rstring_t *str;

	str = r_string_create_from_double(9.5);
	EXPECT_STREQ(str->str, "9.500000");
	r_string_destroy(str);

	str = r_string_create_from_double(9.0);
	EXPECT_STREQ(str->str, "9.000000");
	r_string_destroy(str);

	str = r_string_create_from_double(-9.0);
	EXPECT_STREQ(str->str, "-9.000000");
	r_string_destroy(str);

	str = r_string_create_from_double(-90000000000.0);
	EXPECT_STREQ(str->str, "-90000000000.000000");
	r_string_destroy(str);

	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}


TEST(rstring_test, r_string_create_from_int)
{
	rstring_t *str;

	str = r_string_create_from_int(9);
	EXPECT_STREQ(str->str, "9");
	r_string_destroy(str);

	str = r_string_create_from_int(9000000000000000000);
	EXPECT_STREQ(str->str, "9000000000000000000");
	r_string_destroy(str);

	str = r_string_create_from_int(-9);
	EXPECT_STREQ(str->str, "-9");
	r_string_destroy(str);

	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}


TEST(rstring_test, r_string_create_from_str_len_null)
{
	rstring_t *str = r_string_create_from_str_len(NULL, 10);
	EXPECT_STREQ(str->str, "");
	EXPECT_EQ(str->size, 0);

	r_string_destroy(str);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}

TEST(rstring_test, r_string_create_from_string)
{
	rstring_t *str1 = r_string_create_from_ansi("TEST");
	rstring_t *str2 = r_string_create_from_string(str1);
	EXPECT_STREQ(str2->str, "TEST");
	EXPECT_EQ(str2->size, 4);

	r_string_destroy(str1);
	r_string_destroy(str2);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}

TEST(rstring_test, r_string_cat_len)
{
	rstring_t *str1 = r_string_create_from_ansi("TEST");

	r_string_cat_len(str1, "TEST", 2);
	EXPECT_STREQ(str1->str, "TESTTE");
	EXPECT_EQ(str1->size, 6);

	r_string_cat_len(str1, "TEST", 0);
	EXPECT_STREQ(str1->str, "TESTTE");
	EXPECT_EQ(str1->size, 6);

	r_string_destroy(str1);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}

TEST(rstring_test, r_string_cat)
{
	rstring_t *str1 = r_string_create_from_ansi("TEST");
	rstring_t *str2 = r_string_create_from_ansi("TEST");

	rstring_t *str3 = r_string_cat(str1, str2);
	EXPECT_STREQ(str3->str, "TESTTEST");
	EXPECT_EQ(str3->size, 8);

	r_string_destroy(str1);
	r_string_destroy(str2);
	r_string_destroy(str3);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}



TEST(rstring_test, r_string_copy)
{
	rstring_t *str1 = r_string_create_from_ansi("TEST");
	rstring_t *str2 = (rstring_t*)str1->obj.copy((robject_t*)str1);
	EXPECT_STREQ(str2->str, "TEST");
	EXPECT_EQ(str2->size, 4);

	r_string_destroy(str1);
	r_string_destroy(str2);
	EXPECT_EQ(r_debug_get_allocmem(), 0UL);
}
