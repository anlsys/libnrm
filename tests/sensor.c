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

#include "nrm-internal.h"

/* fixtures for pair of sensor receiver and emitter */
struct nrm_sensor_emitter_ctxt *sensor;
struct nrm_sensor_receiver_ctxt *receiver;

void setup_both(void)
{
	/* need to create the server first, otherwise the client will block
	 * indefinitely */

	sensor = nrm_sensor_emitter_create();
	ck_assert_ptr_nonnull(sensor);
	receiver = nrm_sensor_receiver_create();
	ck_assert_ptr_nonnull(receiver);

	ck_assert(!nrm_sensor_receiver_start(receiver));
	ck_assert(!nrm_sensor_emitter_start(sensor, "test-sensor"));
}

void teardown_both(void)
{
	ck_assert(!nrm_sensor_emitter_exit(sensor));
	ck_assert(!nrm_sensor_receiver_exit(receiver));
	nrm_sensor_emitter_destroy(sensor);
	nrm_sensor_receiver_destroy(receiver);
}

START_TEST(test_send_progress)
{
	char *identity, *buf;
	nrm_scope_t *scope = nrm_scope_create();
	ck_assert_ptr_nonnull(scope);
	ck_assert(!nrm_scope_threadprivate(scope));
	ck_assert(!nrm_sensor_emitter_send_progress(sensor, 42, scope));
	ck_assert(!nrm_sensor_receiver_recv(receiver, &identity, &buf));
	ck_assert_ptr_nonnull(identity);
	ck_assert_ptr_nonnull(buf);
	free(identity);
	free(buf);
	nrm_scope_destroy(scope);
}
END_TEST

Suite *sensor_suite(void)
{
	Suite *s;
	TCase *tc_dc;

	s = suite_create("sensor");

	tc_dc = tcase_create("normal_workflow");
	tcase_add_checked_fixture(tc_dc, setup_both, teardown_both);
	tcase_add_test(tc_dc, test_send_progress);
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
