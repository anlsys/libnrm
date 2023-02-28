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

/* fixtures for ringbuffer */
nrm_ringbuffer_t *ring = NULL;

void setup(void)
{
	int err;
	err = nrm_ringbuffer_create(&ring, 5, sizeof(double));
	ck_assert_int_eq(err, 0);
	ck_assert_ptr_nonnull(ring);
}

void teardown(void)
{
	nrm_ringbuffer_destroy(&ring);
	ck_assert_ptr_null(ring);
}

START_TEST(test_empty)
{
	/* do nothing, just make sure that the fixtures are ok */
}
END_TEST

START_TEST(test_invalid_create)
{
	int err;
	nrm_ringbuffer_t *r = NULL;

	err = nrm_ringbuffer_create(NULL, 5, sizeof(double));
	ck_assert_int_eq(err, -NRM_EINVAL);

	err = nrm_ringbuffer_create(&r, 0, sizeof(double));
	ck_assert_int_eq(err, -NRM_EINVAL);
	ck_assert_ptr_null(r);

	err = nrm_ringbuffer_create(&r, 5, 0);
	ck_assert_int_eq(err, -NRM_EINVAL);
	ck_assert_ptr_null(r);
}
END_TEST

START_TEST(test_length)
{
	int err;
	size_t sz;
	nrm_ringbuffer_t *r;

	err = nrm_ringbuffer_length(NULL, &sz);
	ck_assert_int_eq(err, -NRM_EINVAL);

	err = nrm_ringbuffer_create(&r, 5, sizeof(int));
	ck_assert_int_eq(err, 0);
	ck_assert_ptr_nonnull(r);

	err = nrm_ringbuffer_length(r, NULL);
	ck_assert_int_eq(err, -NRM_EINVAL);

	err = nrm_ringbuffer_length(r, &sz);
	ck_assert_int_eq(err, 0);
	ck_assert_int_eq(sz, 0);

	nrm_ringbuffer_destroy(&r);
	ck_assert_ptr_null(r);
}
END_TEST

START_TEST(test_basics)
{
	int err;
	double v = 42.0;
	void *p;
	double *ret;

	/* new fixture, should be empty */
	err = nrm_ringbuffer_isempty(ring);
	ck_assert_int_eq(err, 1);

	err = nrm_ringbuffer_isfull(ring);
	ck_assert_int_eq(err, 0);

	err = nrm_ringbuffer_push_back(ring, &v);
	ck_assert_int_eq(err, 0);

	err = nrm_ringbuffer_get(ring, 0, &p);
	ck_assert_int_eq(err, 0);
	ret = (double *)p;
	ck_assert_double_eq_tol(*ret, v, 0.1);

	err = nrm_ringbuffer_back(ring, &p);
	ck_assert_int_eq(err, 0);
	ret = (double *)p;
	ck_assert_double_eq_tol(*ret, v, 0.1);
}
END_TEST

Suite *ringbuffer_suite(void)
{
	Suite *s;
	TCase *tc;

	s = suite_create("ringbuffer");

	tc = tcase_create("nofixture");
	tcase_add_test(tc, test_invalid_create);
	tcase_add_test(tc, test_length);
	suite_add_tcase(s, tc);

	tc = tcase_create("basics");
	tcase_add_checked_fixture(tc, setup, teardown);
	tcase_add_test(tc, test_empty);
	tcase_add_test(tc, test_basics);
	suite_add_tcase(s, tc);

	return s;
}

int main(void)
{
	int failed;
	Suite *s;
	SRunner *sr;

	nrm_init(NULL, NULL);
	s = ringbuffer_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_ENV);
	failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	nrm_finalize();
	return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
