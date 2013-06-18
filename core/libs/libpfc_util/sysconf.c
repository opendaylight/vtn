/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * sysconf.c - System configuration management.
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pfc/util.h>
#include <pfc/bitpos.h>
#include <pfc/debug.h>

/*
 * Page size, offset, mask, and log2(pagesize).
 */
size_t		pfc_pagesize PFC_ATTR_HIDDEN;
size_t		pfc_pageoffset PFC_ATTR_HIDDEN;
size_t		pfc_pagemask PFC_ATTR_HIDDEN;
uint32_t	pfc_pageshift PFC_ATTR_HIDDEN;

/*
 * Maximum number of groups to which one user can belong.
 */
uint32_t	pfc_ngroups_max PFC_ATTR_HIDDEN;

#define	SYSCONF_READ(param, var, type)				\
	do {							\
		long	__v;					\
								\
		__v = sysconf(param);				\
		if (PFC_EXPECT_FALSE(__v == -1)) {		\
			fprintf(stderr, "sysconf_init: "	\
				"sysconf(%s) failed: %s\n",	\
				#param, strerror(errno));	\
			abort();				\
			/* NOTREACHED */			\
		}						\
		(var) = (type)__v;				\
	} while (0)

/*
 * void PFC_ATTR_HIDDEN
 * sysconf_init(void)
 *	Cache system configurations on library initialization.
 */
void PFC_ATTR_HIDDEN
sysconf_init(void)
{
	int	shift;

	SYSCONF_READ(_SC_PAGESIZE, pfc_pagesize, size_t);
	SYSCONF_READ(_SC_NGROUPS_MAX, pfc_ngroups_max, uint32_t);

	PFC_ASSERT(pfc_pagesize != 0 && PFC_IS_POW2(pfc_pagesize));
	PFC_ASSERT(pfc_ngroups_max != 0);

	pfc_pageoffset = pfc_pagesize - 1;
	pfc_pagemask = ~pfc_pageoffset;

	shift = pfc_highbit_ulong((pfc_ulong_t)pfc_pagesize);
	PFC_ASSERT(shift >= 0 && pfc_pagesize == (1UL << shift));
	pfc_pageshift = shift;
}

/*
 * size_t
 * pfc_get_pagesize(void)
 *	Return system page size in bytes.
 */
size_t
pfc_get_pagesize(void)
{
	return pfc_pagesize;
}

/*
 * uint32_t
 * pfc_get_online_cpus(void)
 *	Return number of CPUs which is currently online.
 */
uint32_t
pfc_get_online_cpus(void)
{
#ifdef	_SC_NPROCESSORS_ONLN
	long	value;

	/* This is Linux-specific. */
	value = sysconf(_SC_NPROCESSORS_ONLN);
	if (PFC_EXPECT_FALSE(value <= 0)) {
		value = 1U;
	}

	return (uint32_t)value;
#else	/* !_SC_NPROCESSORS_ONLN */
	return 1U;
#endif	/* _SC_NPROCESSORS_ONLN */
}

/*
 * uint32_t
 * pfc_get_ngroups_max(void)
 *	Return the maximum number of groups to which one user can belong.
 */
uint32_t
pfc_get_ngroups_max(void)
{
	return pfc_ngroups_max;
}
