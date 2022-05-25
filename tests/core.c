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

START_TEST(test_version_values)
{
	ck_assert_int_eq(nrm_version_major, NRM_VERSION_MAJOR);
	ck_assert_int_eq(nrm_version_minor, NRM_VERSION_MINOR);
	ck_assert_int_eq(nrm_version_patch, NRM_VERSION_PATCH);
	ck_assert_str_eq(nrm_version_revision, NRM_VERSION_REVISION);
	ck_assert_str_eq(nrm_version_string, NRM_VERSION_STRING);
}
END_TEST

START_TEST(test_init_null)
{
	ck_assert(!nrm_init(NULL, NULL));
	nrm_finalize();
}
END_TEST

Suite *core_suite(void)
{
	Suite *s;
	TCase *tc_init;
	TCase *tc_version;

	s = suite_create("core");

	tc_init = tcase_create("init");
	tcase_add_test(tc_init, test_init_null);
	suite_add_tcase(s, tc_init);

	tc_version = tcase_create("version");
	tcase_add_test(tc_version, test_version_values);
	suite_add_tcase(s, tc_version);

	return s;
}

int main(void)
{
	int failed;
	Suite *s;
	SRunner *sr;

	s = core_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
