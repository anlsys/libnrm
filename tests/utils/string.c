/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#include "nrm.h"
#include <check.h>
#include <stdlib.h>
#include <time.h>

START_TEST(test_fromchar)
{
	nrm_string_t s = nrm_string_fromchar("test");
	ck_assert_ptr_nonnull(s);
	ck_assert_str_eq(s, "test");
	size_t len = nrm_string_strlen(s);
	ck_assert_int_eq(len, 4);
	nrm_string_decref(s);
}
END_TEST

START_TEST(test_frombuf)
{
	nrm_string_t s = nrm_string_frombuf("test", 4);
	ck_assert_ptr_nonnull(s);
	ck_assert_str_eq(s, "test");
	size_t len = nrm_string_strlen(s);
	ck_assert_int_eq(len, 4);
	nrm_string_decref(s);
}
END_TEST

START_TEST(test_fromprintf)
{
	nrm_string_t s = nrm_string_fromprintf("test.%d", 4);
	ck_assert_ptr_nonnull(s);
	ck_assert_str_eq(s, "test.4");
	size_t len = nrm_string_strlen(s);
	ck_assert_int_eq(len, 6);
	nrm_string_decref(s);
}
END_TEST

Suite *string_suite(void)
{
	Suite *s;

	s = suite_create("string");

	TCase *tc_init = tcase_create("creation");
	tcase_add_test(tc_init, test_fromchar);
	tcase_add_test(tc_init, test_frombuf);
	tcase_add_test(tc_init, test_fromprintf);
	suite_add_tcase(s, tc_init);

	return s;
}

int main(void)
{
	int failed;
	Suite *s;
	SRunner *sr;

	nrm_init(NULL, NULL);
	nrm_log_init(stderr, "tests/utils/string");
	nrm_log_setlevel(NRM_LOG_DEBUG);
	s = string_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	nrm_finalize();
	return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
