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

#include "internal/nrmi.h"

#include "nrm-internal.h"

/* fixtures for eventbase */
nrm_eventbase_t eventbase;

void setup(void)
{
	eventbase = nrm_eventbase_create(5);
	ck_assert(eventbase != NULL);
}

void teardown(void)
{
	nrm_eventbase_destroy(eventbase);
	assert(eventbase == NULL);
}

START_TEST(test_create)
{
	eventbase = nrm_eventbase_create(5);
	ck_assert(eventbase->maxperiods == 5);
	ck_assert(eventbase->hash == NULL);
}
END_TEST

START_TEST(test_push_event)
{
	int ret;
	nrm_scope_t *scope = nrm_scope_create("nrm.scope.eventbasetest");
	nrm_string_t uuid = nrm_string_fromchar("test-uuid");
	nrm_time_t now;
	nrm_time_gettime(&now);
	double value = 1234;

	ret = nrm_eventbase_push_event(eventbase, uuid, scope, now, value);
	ck_assert_int_eq(ret, 0);
}
END_TEST

START_TEST(test_tick)
{
}
END_TEST

START_TEST(test_last_value)
{
}
END_TEST

START_TEST(test_destroy)
{
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
	tcase_add_test(tc_dc, test_push_event);
	tcase_add_test(tc_dc, test_tick);
	tcase_add_test(tc_dc, test_last_value);
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
