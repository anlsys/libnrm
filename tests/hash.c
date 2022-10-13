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
#include <stdlib.h>

int main(void)
{
	nrm_init(NULL, NULL);

	nrm_hash_t *hash_table = NULL;

	nrm_hash_add(&hash_table, "first");
	nrm_hash_add(&hash_table, "second");
	nrm_hash_add(&hash_table, "third");
	nrm_hash_add(&hash_table, "a");
	nrm_hash_add(&hash_table, "b");
	nrm_hash_add(&hash_table, "c");

	printf("Hash table of size: %d\n", nrm_hash_size(hash_table));

	nrm_hash_t *wanted = NULL;

	nrm_hash_find(hash_table, &wanted, "second");
	nrm_hash_print_uuid(wanted);
	nrm_hash_set_uuid(&wanted, "newsecond");
	nrm_hash_delete_element(hash_table, wanted);

	nrm_hash_iterator_t *iter = NULL;
	nrm_hash_iterator_create(&iter);
	nrm_hash_t *tmp = NULL;
	nrm_hash_t *el = NULL;
	nrm_hash_iter(el, hash_table, iter, tmp)
	{
		nrm_hash_print_uuid(el);
	}

	nrm_hash_delete_table(hash_table);

	nrm_finalize();

	return 0;
}
