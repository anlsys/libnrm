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

#include "nrm-internal.h"

#include <check.h>
#include <stdlib.h>

START_TEST(test_net_down_client)
{
}
END_TEST

Suite *net_suite(void)
{
	Suite *s;
	TCase *tc_dc;

	s = suite_create("net");

	tc_dc = tcase_create("down_client");
	tcase_add_test(tc_dc, test_net_down_client);
	suite_add_tcase(s, tc_dc);

	return s;
}

int main(void)
{
	int failed;
	Suite *s;
	SRunner *sr;

	s = net_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

