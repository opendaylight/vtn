/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * uniqstr.c - List of unique strings.
 */

#include "unc_dmctl.h"

/*
 * Number of hash buckets.
 */
#define	UNIQSTR_NBUCKETS	PFC_CONST_U(32)

/*
 * int
 * uniqstr_create(pfc_listm_t *usp)
 *	Create a unique string list.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
uniqstr_create(pfc_listm_t *usp)
{
	int	err = pfc_hashlist_create_ref(usp, pfc_refptr_string_ops(),
					      UNIQSTR_NBUCKETS, 0);

	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Failed to create unique string list: %s",
		      strerror(err));
		/* FALLTHROUGH */
	}

	return err;
}

/*
 * int
 * uniqstr_append(pfc_listm_t us, const char *string)
 *	Append the given string to the tail of the unique string list.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	EEXIST is returned if the given string exists in the list.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
uniqstr_append(pfc_listm_t us, const char *string)
{
	pfc_refptr_t	*rstr = pfc_refptr_string_create(string);
	int		err;

	if (PFC_EXPECT_FALSE(rstr == NULL)) {
		error("Failed to copy string.");

		return ENOMEM;
	}

	err = pfc_listm_push_tail(us, (pfc_cptr_t)rstr);
	pfc_refptr_put(rstr);

	if (PFC_EXPECT_FALSE(err != 0 && err != EEXIST)) {
		error("Failed to append a string to unique string list: %s",
		      strerror(err));
		/* FALLTHROUGH */
	}

	return err;
}
