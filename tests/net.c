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
#include <time.h>

#include "nrm.h"

#include "internal/nrmi.h"

/* fixtures for client and server */
zsock_t *client, *server;

void setup_pubsub(void)
{
	/* need to create the server first, otherwise the client will block
	 * indefinitely */
	ck_assert_int_eq(nrm_net_pub_init(&server), 0);
	ck_assert_ptr_nonnull(server);
	ck_assert_int_eq(nrm_net_bind_2(server, NRM_DEFAULT_UPSTREAM_URI,
	                                NRM_DEFAULT_UPSTREAM_PUB_PORT),
	                 0);
	ck_assert_int_eq(nrm_net_sub_init(&client), 0);
	ck_assert_ptr_nonnull(client);
	ck_assert_int_eq(
	        nrm_net_connect_and_wait(client, NRM_DEFAULT_UPSTREAM_URI,
	                                 NRM_DEFAULT_UPSTREAM_PUB_PORT),
	        0);
	ck_assert_int_eq(nrm_net_sub_set_topic(client, ""), 0);
	/* subscription is as annoying as usual, can't guarantee that the
	 * messages are showing up without waiting a bit
	 */
	sleep(1);
}

void teardown(void)
{
	zsock_destroy(&client);
	zsock_destroy(&server);
}

void setup_rpc(void)
{
	/* need to create the server first, otherwise the client will block
	 * indefinitely */
	ck_assert_int_eq(nrm_net_rpc_server_init(&server), 0);
	ck_assert_ptr_nonnull(server);
	ck_assert_int_eq(nrm_net_bind_2(server, NRM_DEFAULT_UPSTREAM_URI,
	                                NRM_DEFAULT_UPSTREAM_RPC_PORT),
	                 0);
	ck_assert_int_eq(nrm_net_rpc_client_init(&client), 0);
	ck_assert_ptr_nonnull(client);
	ck_assert_int_eq(
	        nrm_net_connect_and_wait(client, NRM_DEFAULT_UPSTREAM_URI,
	                                 NRM_DEFAULT_UPSTREAM_RPC_PORT),
	        0);
}

START_TEST(test_empty)
{
	/* do nothing, just make sure that the fixtures are ok */
}
END_TEST

START_TEST(test_send_onemsg_rpc)
{
	/* make sure we can send one message properly, and that it is received
	 */
	char sndbuf[] = "{'test':'value'}";
	ck_assert(!zsock_send(client, "s", sndbuf));
	char *recvbuf = NULL, *identity = NULL;
	ck_assert(!zsock_recv(server, "ss", &identity, &recvbuf));
	ck_assert_ptr_nonnull(recvbuf);
	ck_assert_ptr_nonnull(identity);
	ck_assert_str_eq(recvbuf, sndbuf);
	free(recvbuf);
	free(identity);
}
END_TEST

START_TEST(test_send_onemsg_pubsub)
{
	/* make sure we can send one message properly, and that it is received
	 */
	char sndbuf[] = "{'test':'value'}";
	ck_assert(!zsock_send(server, "s", sndbuf));
	char *recvbuf = NULL;
	ck_assert(!zsock_recv(client, "s", &recvbuf));
	ck_assert_ptr_nonnull(recvbuf);
	ck_assert_str_eq(recvbuf, sndbuf);
	free(recvbuf);
}
END_TEST

START_TEST(test_pub_init)
{
	zsock_t *pub = NULL;
	ck_assert(!nrm_net_pub_init(&pub));
	ck_assert_ptr_nonnull(pub);
	zsock_destroy(&pub);
}
END_TEST

START_TEST(test_sub_init)
{
	zsock_t *sub = NULL;
	ck_assert(!nrm_net_sub_init(&sub));
	ck_assert_ptr_nonnull(sub);
	zsock_destroy(&sub);
}
END_TEST

START_TEST(test_rpc_client_init)
{
	zsock_t *rpc = NULL;
	ck_assert(!nrm_net_rpc_client_init(&rpc));
	ck_assert_ptr_nonnull(rpc);
	zsock_destroy(&rpc);
}
END_TEST

START_TEST(test_rpc_server_init)
{
	zsock_t *rpc = NULL;
	ck_assert(!nrm_net_rpc_server_init(&rpc));
	ck_assert_ptr_nonnull(rpc);
	zsock_destroy(&rpc);
}
END_TEST

Suite *net_suite(void)
{
	Suite *s;

	s = suite_create("net");

	TCase *tc_init = tcase_create("inits");
	tcase_add_test(tc_init, test_pub_init);
	tcase_add_test(tc_init, test_sub_init);
	tcase_add_test(tc_init, test_rpc_client_init);
	tcase_add_test(tc_init, test_rpc_server_init);
	suite_add_tcase(s, tc_init);

	TCase *tc_pubsub = tcase_create("pubsub");
	tcase_add_checked_fixture(tc_pubsub, setup_pubsub, teardown);
	tcase_add_test(tc_pubsub, test_empty);
	tcase_add_test(tc_pubsub, test_send_onemsg_pubsub);
	suite_add_tcase(s, tc_pubsub);

	TCase *tc_rpc = tcase_create("rpc");
	tcase_add_checked_fixture(tc_rpc, setup_rpc, teardown);
	tcase_add_test(tc_rpc, test_empty);
	tcase_add_test(tc_rpc, test_send_onemsg_rpc);
	suite_add_tcase(s, tc_rpc);

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
