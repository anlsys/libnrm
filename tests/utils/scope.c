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

#include "internal/messages.h"
#include "internal/nrmi.h"

START_TEST(test_from_json)
{

	json_t *valid_cpu_scope = json_loads("{\"cpu\": [0]}", 0, NULL);
	nrm_scope_t *scope = nrm_scope_create("test");
	nrm_scope_from_json(scope, valid_cpu_scope);
	assert(nrm_bitmap_isset(&scope->maps[NRM_SCOPE_TYPE_CPU], 0));
}
END_TEST

Suite *scope_suite(void)
{
	Suite *s;

	s = suite_create("scope");

	TCase *tc_json = tcase_create("json");
	tcase_add_test(tc_json, test_from_json);
	suite_add_tcase(s, tc_json);

	return s;
}

int main(void)
{
	int failed;
	Suite *s;
	SRunner *sr;

	nrm_init(NULL, NULL);
	nrm_log_init(stderr, "tests/scope");
	nrm_log_setlevel(NRM_LOG_DEBUG);
	s = scope_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	nrm_finalize();
	return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
