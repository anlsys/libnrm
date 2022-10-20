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

START_TEST(test_basics)
{
	nrm_hash_t *hash = NULL;
	int a = 42;
	int err;
	nrm_string_t fortytwo = nrm_string_fromchar("fortytwo");
	ck_assert_str_eq("fortytwo", fortytwo);
	err = nrm_hash_add(&hash, fortytwo, &a);
	ck_assert_int_eq(err, 0);
	ck_assert_ptr_nonnull(hash);

	void *ret = NULL;
	err = nrm_hash_find(hash, fortytwo, &ret);
	ck_assert_int_eq(err, 0);
	ck_assert_ptr_eq(ret, &a);

	size_t len;
	err = nrm_hash_size(hash, &len);
	ck_assert_int_eq(err, 0);
	ck_assert_int_eq(len, 1);

	nrm_hash_destroy(&hash);
	ck_assert_ptr_null(hash);
	nrm_string_decref(fortytwo);
}

START_TEST(test_iter)
{
	nrm_hash_t *hash = NULL;
	int a = 42, b = 43, c = 44;
	int err;
	nrm_string_t fortytwo = nrm_string_fromchar("fortytwo");
	nrm_string_t fortythree = nrm_string_fromchar("fortythree");
	nrm_string_t fortyfour = nrm_string_fromchar("fortyfour");
	ck_assert_str_eq("fortytwo", fortytwo);
	err = nrm_hash_add(&hash, fortytwo, &a);
	ck_assert_int_eq(err, 0);
	err = nrm_hash_add(&hash, fortythree, &b);
	ck_assert_int_eq(err, 0);
	err = nrm_hash_add(&hash, fortyfour, &c);
	ck_assert_int_eq(err, 0);

	nrm_hash_foreach(hash, iter)
	{
		int *j = (int *)nrm_hash_iterator_get(iter);
		if (j != &a && j != &b && j != &c)
			ck_abort();
		*j = *j + 1;
	}
	ck_assert_int_eq(a, 43);
	ck_assert_int_eq(b, 44);
	ck_assert_int_eq(c, 45);

	size_t len;
	err = nrm_hash_size(hash, &len);
	ck_assert_int_eq(err, 0);
	ck_assert_int_eq(len, 3);

	nrm_hash_destroy(&hash);
	ck_assert_ptr_null(hash);
	nrm_string_decref(fortytwo);
	nrm_string_decref(fortythree);
	nrm_string_decref(fortyfour);
}

Suite *hash_suite(void)
{
	Suite *s;

	s = suite_create("hash");

	TCase *tc_basics = tcase_create("basics");
	tcase_add_test(tc_basics, test_basics);
	tcase_add_test(tc_basics, test_iter);
	suite_add_tcase(s, tc_basics);

	return s;
}

int main(void)
{
	int failed;
	Suite *s;
	SRunner *sr;

	nrm_init(NULL, NULL);
	nrm_log_init(stderr, "tests/utils/hash");
	nrm_log_setlevel(NRM_LOG_DEBUG);
	s = hash_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	nrm_finalize();
	return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
