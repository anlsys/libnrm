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

/* fixture for creating a pair of down client and down server */
struct nrm_net_ctxt down_client;
struct nrm_net_ctxt down_server;

void setup_down(void)
{
	/* need to create the server first, otherwise the client will block
	 * indefinitely */
	ck_assert(!nrm_net_down_server_init(&down_server,
					    NRM_DEFAULT_DOWNSTREAM_URI));
	ck_assert(!nrm_net_down_client_init(&down_client,
					    NRM_DEFAULT_DOWNSTREAM_URI));
}

void teardown_down(void)
{
	ck_assert(!nrm_net_fini(&down_client));
	ck_assert(!nrm_net_fini(&down_server));
}

START_TEST(test_net_down_empty)
{
	/* do nothing, just make sure that the fixtures are ok */
}
END_TEST


START_TEST(test_net_down_send_onemsg)
{
	/* make sure we can send one message properly, and that it is received
	 */
	char sndbuf[] = "{'test':'value'}";
	ck_assert_int_eq(nrm_net_send(&down_client, sndbuf, sizeof(sndbuf), 0),
			 sizeof(sndbuf));
	char *recvbuf = NULL, *identity = NULL;
	ck_assert(!nrm_net_recv_multipart(&down_server, &identity, &recvbuf));
	ck_assert_ptr_nonnull(recvbuf);
	ck_assert_ptr_nonnull(identity);
	ck_assert_str_eq(recvbuf, sndbuf);
}
END_TEST


Suite *net_suite(void)
{
	Suite *s;
	TCase *tc_dc;

	s = suite_create("net");

	tc_dc = tcase_create("downstream");
	tcase_add_checked_fixture(tc_dc, setup_down, teardown_down);
	tcase_add_test(tc_dc, test_net_down_empty);
	tcase_add_test(tc_dc, test_net_down_send_onemsg);
	suite_add_tcase(s, tc_dc);

	return s;
}

int main(void)
{
	int failed;
	Suite *s;
	SRunner *sr;

	nrm_init(NULL, NULL);
	s = net_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_ENV);
	failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	nrm_finalize();
	return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

