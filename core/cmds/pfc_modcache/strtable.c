/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * strtable.c - String table generator.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pfc/hash.h>
#include "modcache_impl.h"

/*
 * String table.
 */
struct strtable {
	pfc_hash_t	s_hash;		/* string hash table */
	uint32_t	s_size;		/* size of string table */
};

/*
 * Number of string hash buckets.
 */
#define	STRTABLE_HASH_NBUCKETS		31U

/*
 * void
 * strtable_create(strtable_t **tblpp)
 *	Create string table instance.
 *
 * Calling/Exit State:
 *	Upon successful completion, pointer to string table instance is set to
 *	`*tblpp' and zero is returned.
 *
 *	Program exits on error.
 */
void
strtable_create(strtable_t **tblpp)
{
	strtable_t	*tblp;
	int	err;

	tblp = (strtable_t *)malloc(sizeof(strtable_t));
	if (PFC_EXPECT_FALSE(tblp == NULL)) {
		fatal("Failed to allocate string table instance.");
		/* NOTREACHED */
	}

	err = pfc_strhash_create(&tblp->s_hash, NULL, STRTABLE_HASH_NBUCKETS,
				 PFC_HASH_NOLOCK);
	if (PFC_EXPECT_FALSE(err != 0)) {
		fatal("Failed to create string hash table: %s",
		      strerror(err));
		/* NOTREACHED */
	}

	/* The first entry of the string table is an empty string. */
	tblp->s_size = 1;

	*tblpp = tblp;
}

/*
 * uint32_t
 * strtable_add(strtable_t *PFC_RESTRICT tblp, pfc_refptr_t *PFC_RESTRICT rstr)
 *	Add the specified string to the string table.
 *
 * Calling/Exit State:
 *	Upon successful completion, string table offset assigned to the
 *	specified string is returned.
 *
 *	Program exits on error.
 */
uint32_t
strtable_add(strtable_t *PFC_RESTRICT tblp, pfc_refptr_t *PFC_RESTRICT rstr)
{
	pfc_hash_t	hash = tblp->s_hash;
	pfc_cptr_t	value;
	uint32_t	off;
	int	err;

	/* Look up the specified string in the string hash. */
	err = pfc_hash_get_kref(hash, rstr, &value);
	if (PFC_EXPECT_FALSE(err == 0)) {
		return (uint32_t)(uintptr_t)value;
	}

	/* Assign new string entry. */
	off = tblp->s_size;
	tblp->s_size = off + pfc_refptr_string_length(rstr) + 1;
	err = pfc_hash_put_kref(hash, rstr, (pfc_cptr_t)(uintptr_t)off);
	if (PFC_EXPECT_FALSE(err != 0)) {
		fatal("Failed to put a string to hash table: %s",
		      strerror(err));
		/* NOTREACHED */
	}

	debug_printf(3, "%s => string table %u",
		     pfc_refptr_string_value(rstr), off);

	return off;
}

/*
 * const char *
 * strtable_finalize(strtable_t *PFC_RESTRICT tblp,
 *		     uint32_t *PFC_RESTRICT sizep)
 *	Finalize the specified string table.
 *
 *	This function creates the contents of string table section, and
 *	destroys the specified string table instance.
 *
 * Calling/Exit State:
 *	Upon successful completion, the size of string table section is set
 *	to `*sizep', and a pointer to byte array that contains string table
 *	section data is returned. Returned pointer should be freed by free(3).
 *
 *	Program exits on error.
 */
const char *
strtable_finalize(strtable_t *PFC_RESTRICT tblp, uint32_t *PFC_RESTRICT sizep)
{
	char		*buf;
	pfc_hash_t	hash;
	pfc_hashiter_t	it;
	pfc_refptr_t	*key;
	pfc_cptr_t	value;
	int	err;

	buf = (char *)malloc(tblp->s_size);
	if (PFC_EXPECT_FALSE(buf == NULL)) {
		fatal("Failed to allocate string table data.");
		/* NOTREACHED */
	}

	/* The first entry must be an empty string. */
	*buf = '\0';

	/* Copy strings to the string table. */
	hash = tblp->s_hash;
	it = pfc_hashiter_get(hash);
	if (PFC_EXPECT_FALSE(it == NULL)) {
		fatal("Failed to create hash iterator.");
		/* NOTREACHED */
	}

	while ((err = pfc_hashiter_next(it, (pfc_cptr_t *)&key, &value)) == 0) {
		uint32_t	off = (uint32_t)(uintptr_t)value, len;
		const char	*src;
		char	*dst;

		PFC_ASSERT(off > 0 && off < tblp->s_size);
		len = pfc_refptr_string_length(key) + 1;
		src = pfc_refptr_string_value(key);
		dst = buf + off;
		memcpy(dst, src, len);
	}
	PFC_ASSERT(err == ENOENT);

	*sizep = tblp->s_size;

	/* Destroy string table instance. */
	pfc_hash_destroy(hash);
	free(tblp);

	return (const char *)buf;
}
