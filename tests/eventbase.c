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
nrm_scope_t *scope;
nrm_string_t sen_uuid;
nrm_time_t now;

void setup(void)
{
	eventbase = nrm_eventbase_create(5);
	ck_assert_ptr_nonnull(eventbase);
	scope = nrm_scope_create("nrm.scope.eventbasetest");
	sen_uuid = nrm_string_fromchar("test-uuid");
	nrm_time_gettime(&now);
}

void teardown(void)
{
	nrm_eventbase_destroy(&eventbase);
	nrm_scope_destroy(scope);
	ck_assert_ptr_null(scope);
	ck_assert_ptr_null(eventbase);
	nrm_string_decref(sen_uuid);
}

START_TEST(test_empty)
{
	/* do nothing, just make sure that the fixtures are ok */
}
END_TEST

START_TEST(test_create)
{
	eventbase = nrm_eventbase_create(5);
	size_t outperiods;
	ck_assert_int_eq(nrm_eventbase_get_maxperiods(eventbase), 5);
}
END_TEST

START_TEST(test_push_tick_last_normal)
{
	int ret_push, ret_tick, ret_last;
	double value = 1234, new_val = 1000;
	double *last_value;

	// testing push_event normal
	ret_push = nrm_eventbase_push_event(eventbase, sen_uuid, scope, now, value);
	ck_assert_int_eq(ret_push, 0);

	// testing tick normal
	ret_tick = nrm_eventbase_tick(eventbase, now);
	ck_assert_int_eq(ret_tick, 0);

	// test last_value normal
	ret_last = nrm_eventbase_last_value(eventbase, sen_uuid, nrm_scope_uuid(scope), last_value);
	ck_assert_int_eq(*last_value, 1234);

	// testing push_event normal, accumulation
	ret_push = nrm_eventbase_push_event(eventbase, sen_uuid, scope, now, new_val);
	ck_assert_int_eq(ret_push, 0);

	// testing tick normal, accumulation
	ret_tick = nrm_eventbase_tick(eventbase, now);
	ck_assert_int_eq(ret_tick, 0);

	// test last_value normal, accumulation
	ret_last = nrm_eventbase_last_value(eventbase, sen_uuid, nrm_scope_uuid(scope), last_value);
	ck_assert_int_eq(*last_value, 2234);

	free(last_value);

}
END_TEST

START_TEST(test_push_tick_last_edge_cases)
{
	int ret_push, ret_tick, ret_last;
	double value = 1234;
	double *last_value;

	// now checking last_value. new scope, so value should be set 0.0
	nrm_scope_t *nscope = nrm_scope_create("nrm.scope.eventbasetest2");
	ret_last = nrm_eventbase_last_value(eventbase, sen_uuid, nrm_scope_uuid(nscope), last_value);
	ck_assert_double_eq(*last_value, 0.0);
	ck_assert_int_eq(ret_last, -NRM_EINVAL);

	// check last_value. s2s == NULL
	nrm_eventbase_t *new_eventbase = nrm_eventbase_create(4);
	ret_last = nrm_eventbase_last_value(new_eventbase, sen_uuid, nrm_scope_uuid(scope), last_value);
	ck_assert_double_eq(*last_value, 0.0);
	ck_assert_int_eq(ret_last, -NRM_EINVAL);

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
	tcase_add_test(tc_dc, test_empty);
	tcase_add_test(tc_dc, test_create);
	tcase_add_test(tc_dc, test_push_tick_last_normal);
	tcase_add_test(tc_dc, test_push_tick_last_edge_cases);
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
	s = eventbase_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_ENV);
	failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	nrm_finalize();
	return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
