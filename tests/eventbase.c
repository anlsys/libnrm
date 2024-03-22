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

/* fixtures for eventbase */
nrm_eventbase_t *eventbase;
nrm_scope_t *scope;
nrm_sensor_t *sensor;
nrm_time_t now;

void setup(void)
{
	eventbase = nrm_eventbase_create(5);
	scope = nrm_scope_create("nrm.scope.eventbasetest");
	sensor = nrm_sensor_create("nrm.sensor.eventbasetest");
	nrm_time_gettime(&now);

	ck_assert_ptr_nonnull(eventbase);
	ck_assert_ptr_nonnull(scope);
	ck_assert_ptr_nonnull(sensor);
}

void teardown(void)
{
	nrm_eventbase_destroy(&eventbase);
	nrm_scope_destroy(scope);
	nrm_sensor_destroy(&sensor);

	ck_assert_ptr_null(sensor);
	ck_assert_ptr_null(eventbase);
}

START_TEST(test_empty)
{
	/* do nothing, just make sure that the fixtures are ok */
}
END_TEST

START_TEST(test_push_tick_last_normal)
{
	int err;
	double value = 1234, new_val = 1000;

	nrm_string_t sensor_uuid = nrm_sensor_uuid(sensor);

	nrm_time_t future;
	nrm_time_gettime(&future);

	// testing push_event normal
	err = nrm_eventbase_push_event(eventbase, sensor_uuid, scope, now,
	                               value);
	ck_assert_int_eq(err, 0);

	// test pull nothing
	nrm_timeserie_t *ts;
	err = nrm_eventbase_pull_timeserie(eventbase, sensor_uuid, scope,
	                                   future, &ts);
	ck_assert_int_eq(err, 0);
	nrm_vector_t *e = nrm_timeserie_get_events(ts);
	size_t numevents;
	nrm_vector_length(e, &numevents);
	ck_assert_int_eq(numevents, 0);
	nrm_timeserie_destroy(&ts);

	// testing tick normal
	err = nrm_eventbase_tick(eventbase, now);
	ck_assert_int_eq(err, 0);

	// testing pull normal
	err = nrm_eventbase_pull_timeserie(eventbase, sensor_uuid, scope, now,
	                                   &ts);
	ck_assert_int_eq(err, 0);
	e = nrm_timeserie_get_events(ts);
	nrm_vector_length(e, &numevents);
	ck_assert_int_eq(numevents, 1);

	nrm_event_t *event;
	nrm_vector_get_withtype(nrm_event_t, e, 0, event);
	ck_assert_double_eq_tol(event->value, 1234, 0.1);
	nrm_timeserie_destroy(&ts);

	// normal push
	err = nrm_eventbase_push_event(eventbase, sensor_uuid, scope, future,
	                               new_val);
	ck_assert_int_eq(err, 0);

	nrm_time_t further;
	nrm_time_gettime(&further);

	// second push
	err = nrm_eventbase_push_event(eventbase, sensor_uuid, scope, further,
	                               new_val);
	ck_assert_int_eq(err, 0);

	// test ts is growing
	err = nrm_eventbase_pull_timeserie(eventbase, sensor_uuid, scope, now,
	                                   &ts);
	ck_assert_int_eq(err, 0);
	e = nrm_timeserie_get_events(ts);
	nrm_vector_length(e, &numevents);
	ck_assert_int_eq(numevents, 3);
	nrm_timeserie_destroy(&ts);
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
	tcase_add_test(tc_dc, test_push_tick_last_normal);
	suite_add_tcase(s, tc_dc);

	return s;
}

int main(void)
{
	int failed;
	Suite *s;
	SRunner *sr;

	nrm_init(NULL, NULL);
	nrm_log_init(stderr, "tests/eventbase");
	s = eventbase_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_ENV);
	failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	nrm_finalize();
	return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
