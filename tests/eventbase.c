/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#include <check.h>
#include <stdlib.h>

#include "nrm.h"
#include "internal/nrmi.h"

/* fixtures for eventbase */
nrm_eventbase_t *eventbase;

void setup(void)
{
	eventbase = nrm_eventbase_create(5);
	ck_assert_ptr_nonnull(eventbase);
}

void teardown(void)
{
	nrm_eventbase_destroy(&eventbase);
	ck_assert_ptr_null(eventbase);
}

START_TEST(test_create)
{
	eventbase = nrm_eventbase_create(5);
	size_t outperiods;
	ck_assert_int_eq(nrm_eventbase_get_maxperiods(eventbase), 5);
	ck_assert_ptr_null(eventbase->hash);
}
END_TEST

START_TEST(test_push_event_last_value)
{
	int ret_push, ret_last;
	nrm_scope_t *scope = nrm_scope_create("nrm.scope.eventbasetest");
	nrm_string_t uuid = nrm_string_fromchar("test-uuid");
	nrm_time_t now;
	nrm_time_gettime(&now);
	double value = 1234;
	double *last_value;

	// testing push_event
	ret_push = nrm_eventbase_push_event(eventbase, uuid, scope, now, value);
	ck_assert_int_eq(ret_push, 0);

	// now checking last_value. new scope
	nrm_scope_t *nscope = nrm_scope_create("nrm.scope.eventbasetest2");
	ret_last = nrm_eventbase_last_value(eventbase, uuid, nscope, &last_value);
	ck_assert_float_eq(&last_value, 0.0);
	ck_assert_int_eq(ret_last, -NRM_EINVAL);


	&last_value = 1234;

	// check last_value. s2s == NULL
	new_eventbase = nrm_eventbase_create(4);
	ret_last = nrm_eventbase_last_value(new_eventbase, uuid, scope, &last_value);
	ck_assert_float_eq(&last_value, 0.0);
	ck_assert_int_eq(ret_last, -NRM_EINVAL);

}
END_TEST

START_TEST(test_tick)
{
	int ret;
	nrm_time_t now;
	nrm_time_gettime(&now);

	ret = nrm_eventbase_tick(eventbase, now);
	ck_assert_int_eq(ret, 0);
}
END_TEST


START_TEST(test_destroy)
{
	nrm_eventbase_destroy(&eventbase);
}
END_TEST

Suite *eventbase_suite(void)
{
	Suite *s;
	TCase *tc_dc;

	s = suite_create("eventbase");

	tc_dc = tcase_create("normal_workflow");
	tcase_add_checked_fixture(tc_dc, setup, teardown);
	tcase_add_test(tc_dc, test_create);
	tcase_add_test(tc_dc, test_push_event_last_value);
	tcase_add_test(tc_dc, test_tick);
	tcase_add_test(tc_dc, test_destroy);
	suite_add_tcase(s, tc_dc);

	return s;
}

int main(void)
{
	int failed;
	Suite *s;
	SRunner *sr;

	nrm_init(NULL, NULL);
	s = sensor_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_ENV);
	failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	nrm_finalize();
	return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
