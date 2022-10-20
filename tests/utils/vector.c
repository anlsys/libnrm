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
	nrm_vector_t *vector = NULL;
	int a = 42;
	int err;
	nrm_string_t fortytwo = nrm_string_fromchar("fortytwo");
	ck_assert_str_eq("fortytwo", fortytwo);

	err = nrm_vector_push_back(vector, fortytwo);
	ck_assert_int_eq(err, 0);
	ck_assert_ptr_nonnull(vector);

	void *ret = NULL;
	size_t pos;
	err = nrm_vector_find(vector, fortytwo, comp(), pos);
	ck_assert_int_eq(err, 0);
	ck_assert_ptr_eq(ret, &a);

	size_t len;
	err = nrm_vector_length(vector, &len);
	ck_assert_int_eq(err, 0);
	ck_assert_int_eq(len, 1);

	nrm_vector_destroy(&vector);
	ck_assert_ptr_null(vector);
	nrm_string_decref(fortytwo);
}

START_TEST(test_iter)
{
	nrm_vector_t *vector = NULL;
	int a = 42, b = 43, c = 44;
	int err;
	nrm_string_t fortytwo = nrm_string_fromchar("fortytwo");
	nrm_string_t fortythree = nrm_string_fromchar("fortythree");
	nrm_string_t fortyfour = nrm_string_fromchar("fortyfour");
	ck_assert_str_eq("fortytwo", fortytwo);
	err = nrm_vector_push_back(vector, fortytwo);
	ck_assert_int_eq(err, 0);
	err = nrm_vector_push_back(vector, fortythree);
	ck_assert_int_eq(err, 0);
	err = nrm_vector_push_back(vector, fortyfour);
	ck_assert_int_eq(err, 0);

	nrm_vector_foreach(vector, iterator)
	{
		int *j = (int *)nrm_vector_iterator_get(iterator);
		if (j != &a && j != &b && j != &c)
			ck_abort();
		*j = *j + 1;
	}
	ck_assert_int_eq(a, 43);
	ck_assert_int_eq(b, 44);
	ck_assert_int_eq(c, 45);

	size_t len;
	err = nrm_vector_length(vector, &len);
	ck_assert_int_eq(err, 0);
	ck_assert_int_eq(len, 3);

	nrm_vector_destroy(&vector);
	ck_assert_ptr_null(vector);
	nrm_string_decref(fortytwo);
	nrm_string_decref(fortythree);
	nrm_string_decref(fortyfour);
}

Suite *vector_suite(void)
{
	Suite *s;

	s = suite_create("vector");

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
	nrm_log_init(stderr, "tests/utils/vector");
	nrm_log_setlevel(NRM_LOG_DEBUG);
	s = vector_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	nrm_finalize();
	return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
