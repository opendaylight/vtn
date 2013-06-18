/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * resource.c - Resource limit management.
 */

#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <pfc/conf.h>
#include "pfcd.h"

/*
 * Internal resource limit.
 */
#ifdef	PFC_LP64

#define	PFCD_RLIMIT_AS_MIN		0x10000000ULL		/* 256M */
#define	PFCD_RLIMIT_AS_MAX		0x800000000000ULL	/* 128T */
#define	PFCD_RLIMIT_AS_DEFAULT		RLIM_INFINITY

#define	PFCD_RLIMIT_CORE_MIN		0ULL			/* 0 */
#define	PFCD_RLIMIT_CORE_MAX		0x800000000000ULL	/* 128T */
#define	PFCD_RLIMIT_CORE_DEFAULT	RLIM_INFINITY

#define	PFCD_RLIMIT_DATA_MIN		0x08000000ULL		/* 128M */
#define	PFCD_RLIMIT_DATA_MAX		0x800000000000ULL	/* 128T */
#define	PFCD_RLIMIT_DATA_DEFAULT	RLIM_INFINITY

#define	PFCD_RLIMIT_STACK_MIN		0x10000ULL		/* 64K */
#define	PFCD_RLIMIT_STACK_MAX		0x10000000ULL		/* 256M */
#define	PFCD_RLIMIT_STACK_DEFAULT	0x800000ULL		/* 8M */

#else	/* !PFC_LP64 */

#define	PFCD_RLIMIT_AS_MIN		0x10000000UL		/* 256M */
#define	PFCD_RLIMIT_AS_MAX		0x80000000UL		/* 2G */
#define	PFCD_RLIMIT_AS_DEFAULT		RLIM_INFINITY

#define	PFCD_RLIMIT_CORE_MIN		0UL			/* 0 */
#define	PFCD_RLIMIT_CORE_MAX		0x80000000UL		/* 2G */
#define	PFCD_RLIMIT_CORE_DEFAULT	RLIM_INFINITY

#define	PFCD_RLIMIT_DATA_MIN		0x08000000UL		/* 128M */
#define	PFCD_RLIMIT_DATA_MAX		0x80000000UL		/* 2G */
#define	PFCD_RLIMIT_DATA_DEFAULT	RLIM_INFINITY

#define	PFCD_RLIMIT_STACK_MIN		0x10000ULL		/* 64K */
#define	PFCD_RLIMIT_STACK_MAX		0x10000000ULL		/* 256M */
#define	PFCD_RLIMIT_STACK_DEFAULT	0x800000ULL		/* 8M */

#endif	/* PFC_LP64 */

#define	PFCD_RLIMIT_NOFILE_MIN		128UL
#define	PFCD_RLIMIT_NOFILE_MAX		0x100000UL	/* 1M */
#define	PFCD_RLIMIT_NOFILE_DEFAULT	5000UL

/*
 * Tunable resources.
 */
typedef struct {
	const char	*rl_name;		/* parameter name */
	int		rl_resource;		/* resource ID */
	int		rl_flags;		/* flags */
	rlim_t		rl_min;			/* lower internal limit */
	rlim_t		rl_max;			/* upper internal limit */
	rlim_t		rl_default;		/* default value */
} pfcd_rlim_t;

/* Flags for rl_flags */
#define	PFCD_RLIMF_INFINITY	0x1U	/* RLIM_INFINITY is supported */
#define	PFCD_RLIMF_PAGESIZE	0x2U	/* round up to pagesize */

#define	__PFCD_RLIMIT_DECL(name, id, flags)		\
	{						\
		.rl_name	= (name),		\
		.rl_resource	= (id),			\
		.rl_flags	= (flags),		\
		.rl_min		= PFCD_##id##_MIN,	\
		.rl_max		= PFCD_##id##_MAX,	\
		.rl_default	= PFCD_##id##_DEFAULT,	\
	}

#define	PFCD_RLIMIT_DECL(name, id)		\
	__PFCD_RLIMIT_DECL(name, id, 0)
#define	PFCD_RLIMIT_INF_DECL(name, id)			\
	__PFCD_RLIMIT_DECL(name, id, PFCD_RLIMF_INFINITY)
#define	PFCD_RLIMIT_VSPACE_DECL(name, id)				\
	__PFCD_RLIMIT_DECL(name, id, PFCD_RLIMF_INFINITY | PFCD_RLIMF_PAGESIZE)

/*
 * Supported resource limits.
 */
static const pfcd_rlim_t	pfcd_resources[] = {
	PFCD_RLIMIT_VSPACE_DECL("as_size", RLIMIT_AS),
	PFCD_RLIMIT_INF_DECL("core_size", RLIMIT_CORE),
	PFCD_RLIMIT_VSPACE_DECL("data_size", RLIMIT_DATA),
	PFCD_RLIMIT_DECL("open_files", RLIMIT_NOFILE),
	PFCD_RLIMIT_VSPACE_DECL("stack_size", RLIMIT_STACK),
};

/*
 * Name of parameter block which defines resource limit.
 */
static const char	pfc_rlim_confname[] = "rlimit";

/*
 * Resource IDs to be configured as infinity.
 */
typedef struct {
	const char	*rli_name;		/* symbolic name */
	int		rli_resource;		/* resource ID */
} pfcd_rlim_id_t;

static const pfcd_rlim_id_t	pfcd_inf_resources[] = {
	{ "cpu_time", RLIMIT_CPU },
	{ "file_size", RLIMIT_FSIZE },
};

/*
 * Internal prototypes.
 */
static void	resource_configure(pfc_cfblk_t conf, const pfcd_rlim_t *rp);
static void	resource_set_infinity(const pfcd_rlim_id_t *idp);

/*
 * void
 * resource_init(void)
 *	Initialize system resource limit.
 */
void
resource_init(void)
{
	const pfcd_rlim_t	*rp;
	const pfcd_rlim_id_t	*idp;
	pfc_cfblk_t	conf;

	conf = pfc_sysconf_get_block(pfc_rlim_confname);

	/* Configure tunable resource limits. */
	for (rp = pfcd_resources; rp < PFC_ARRAY_LIMIT(pfcd_resources); rp++) {
		resource_configure(conf, rp);
	}

	/*
	 * Set resource limit defined in pfcd_inf_resources array to
	 * infinity.
	 */
	for (idp = pfcd_inf_resources;
	     idp < PFC_ARRAY_LIMIT(pfcd_inf_resources); idp++) {
		resource_set_infinity(idp);
	}
}

/*
 * static void
 * resource_configure(pfc_cfblk_t conf, const pfcd_rlim_t *rp)
 *	Configure the specified resource limit.
 */
static void
resource_configure(pfc_cfblk_t conf, const pfcd_rlim_t *rp)
{
	struct rlimit	rl;
	pfc_ulong_t	value;
	int	ret, id = rp->rl_resource;

	/* Read resource limit from configuration file. */
	value = pfc_conf_get_ulong(conf, rp->rl_name, rp->rl_default);

	if (rp->rl_flags & PFCD_RLIMF_PAGESIZE) {
		size_t	pagesize = pfc_get_pagesize();

		if (value < ((pfc_ulong_t)~0UL) - pagesize) {
			/* Round up to the pagesize. */
			value = PFC_POW2_ROUNDUP(value, pagesize);
		}
	}

	if (value < rp->rl_min) {
		pfc_log_warn("%s: Force to use lower limit: 0x%lx:0x%lx",
			     rp->rl_name, value, rp->rl_min);
		value = rp->rl_min;
	}
	else if (value > rp->rl_max) {
		if (rp->rl_flags & PFCD_RLIMF_INFINITY) {
			/* Use infinity value. */
			value = RLIM_INFINITY;
		}
		else {
			value = rp->rl_max;
			pfc_log_warn("%s: Force to use upper limit: 0x%lx",
				     rp->rl_name, value);
		}
	}

	/* Get current limit. */
	if (PFC_EXPECT_FALSE(getrlimit(id, &rl) != 0)) {
		pfc_log_warn("%s: Failed to get resource limit: %s",
			     rp->rl_name, strerror(errno));

		return;
	}

	if (rl.rlim_cur == value) {
		/* Nothing to do. */
		return;
	}

	/* Set new limit. */
	rl.rlim_cur = value;
	if (value == RLIM_INFINITY || value > rl.rlim_max) {
		rl.rlim_max = value;
	}

	ret = setrlimit(id, &rl);
	if (value == RLIM_INFINITY) {
		if (PFC_EXPECT_TRUE(ret == 0)) {
			pfc_log_debug("%s: Set rlimit to infinity.",
				      rp->rl_name);
		}
		else {
			pfc_log_warn("%s: Failed to set resource limit to "
				     "infinity: %s", rp->rl_name,
				     strerror(errno));
		}
	}
	else {
		if (PFC_EXPECT_TRUE(ret == 0)) {
			pfc_log_debug("%s: Set rlimit to 0x%lx.",
				      rp->rl_name, value);
		}
		else {
			pfc_log_warn("%s: Failed to set resource limit to "
				     "0x%lx: %s", rp->rl_name,
				     value, strerror(errno));
		}
	}
}

/*
 * static void
 * resource_set_infinity(const pfcd_rlim_id_t *idp)
 *	Set the specified system resource limit as infinity.
 */
static void
resource_set_infinity(const pfcd_rlim_id_t *idp)
{
	struct rlimit	rl;
	int	id = idp->rli_resource;

	if (PFC_EXPECT_FALSE(getrlimit(id, &rl) != 0)) {
		pfc_log_warn("%s: Failed to get resource limit: %s",
			     idp->rli_name, strerror(errno));

		return;
	}

	if (rl.rlim_cur != RLIM_INFINITY) {
		rl.rlim_cur = rl.rlim_max = RLIM_INFINITY;

		if (PFC_EXPECT_TRUE(setrlimit(id, &rl) == 0)) {
			pfc_log_debug("%s: Set rlimit to infinity.",
				      idp->rli_name);
		}
		else {
			pfc_log_warn("%s: Failed to set resource limit to "
				     "infinity: %s", idp->rli_name,
				     strerror(errno));
		}
	}
}
