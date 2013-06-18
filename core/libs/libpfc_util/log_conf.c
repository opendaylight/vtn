/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * log_conf.c - PFC logging system configuration.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pfc/base.h>
#include <pfc/log.h>
#include <pfc/conf.h>
#include <pfc/debug.h>
#include "log_impl.h"

/*
 * Configuration parameter names.
 */
static const char	conf_log_syslog[] = "log_syslog";
static const char	conf_log_facility[] = "log_facility";
static const char	conf_message_rotate[] = "message_rotate";
static const char	conf_message_size[] = "message_size";

/*
 * Default log facility.
 */
#define	LOG_FACILITY_DEFAULT		PFC_LOGFACL_LOCAL0

/*
 * Default value of message_rotate.
 */
#define LOG_MSG_ROTATE_DEFAULT		10U

/*
 * Default value of message_size.
 */
#define LOG_MSG_SIZE_DEFAULT		10000000U	/* 10MB */

/*
 * Internal prototypes.
 */
static void	logconf_init_common(pfc_log_conf_t *PFC_RESTRICT cfp,
				    pfc_cfblk_t blk,
				    const char *PFC_RESTRICT ident,
				    pfc_log_fatal_t handler);
static char	*logconf_alloc_path(const char *PFC_RESTRICT base, size_t blen,
				    const char *PFC_RESTRICT name, size_t nlen);

/*
 * void
 * pfc_logconf_init(pfc_log_conf_t *PFC_RESTRICT cfp, pfc_cfblk_t blk,
 *		    const char *PFC_RESTRICT ident, pfc_log_fatal_t handler)
 *	Initialize log configuration.
 *
 *	This function setup the specified `cfp'.
 *	cfp is used for the argument to pfc_log_sysinit().
 *
 *	`blk' must have the following configuration parameter.
 *	Typically, blk is the options block in pfcd.conf.
 *		- log_syslog
 *		- log_facility
 *		- message_rotate
 *		- message_size
 *		- log_level
 */
void
pfc_logconf_init(pfc_log_conf_t *PFC_RESTRICT cfp, pfc_cfblk_t blk,
		 const char *PFC_RESTRICT ident, pfc_log_fatal_t handler)
{
	pfc_bool_t	log_syslog;

	/*
	 * Determine whether to use only the syslog for recording logs.
	 * If "log_syslog" is false, pfc_log_xxx() record logs to the local
	 * file. Default is "false".
	 */
	log_syslog = pfc_conf_get_bool(blk, conf_log_syslog, PFC_FALSE);
	if (log_syslog) {
		cfp->plc_type = LOG_TYPE_SYSLOG;
	} else {
		cfp->plc_type = LOG_TYPE_FILE;
	}

	cfp->plc_level.plvc_level = PFC_LOGLVL_NONE;
	cfp->plc_output = NULL;
	cfp->plc_rcount = pfc_conf_get_uint32(blk, conf_message_rotate,
					      LOG_MSG_ROTATE_DEFAULT);
	cfp->plc_rsize = (size_t)pfc_conf_get_uint32(blk, conf_message_size,
						     LOG_MSG_SIZE_DEFAULT);
	logconf_init_common(cfp, blk, ident, handler);
}

/*
 * void
 * pfc_logconf_early(pfc_log_conf_t *PFC_RESTRICT cfp, pfc_cfblk_t blk,
 *		     const char *PFC_RESTRICT ident, FILE *PFC_RESTRICT out,
 *		     pfc_log_level_t level, pfc_log_fatal_t handler)
 *	Initialize log configuration for early logging.
 *	This function is called by pfc_log_init().
 */
void
pfc_logconf_early(pfc_log_conf_t *PFC_RESTRICT cfp, pfc_cfblk_t blk,
		  const char *PFC_RESTRICT ident, FILE *PFC_RESTRICT out,
		  pfc_log_level_t level, pfc_log_fatal_t handler)
{
	PFC_ASSERT(out != NULL);
	cfp->plc_type = LOG_TYPE_STREAM;

	cfp->plc_level.plvc_level = (pfc_log_level_is_valid(level) == 0)
		? level : PFC_LOGLVL_NONE;
	cfp->plc_output = out;
	logconf_init_common(cfp, blk, ident, handler);
}

/*
 * static void
 * logconf_init_common(pfc_log_conf_t *PFC_RESTRICT cfp, pfc_cfblk_t blk,
 *		       const char *PFC_RESTRICT ident, pfc_log_fatal_t handler)
 *	Initialize log configuration.
 */
static void
logconf_init_common(pfc_log_conf_t *PFC_RESTRICT cfp, pfc_cfblk_t blk,
		    const char *PFC_RESTRICT ident, pfc_log_fatal_t handler)
{
	const char	*faname;

	cfp->plc_ident = ident;
	PFC_ASSERT(ident != NULL);
	cfp->plc_handler = handler;
	cfp->plc_logdir = NULL;
	cfp->plc_logpath = NULL;
	cfp->plc_lvlpath = NULL;
	cfp->plc_cfblk = blk;

	cfp->plc_level.plvc_deflevel = LOG_LEVEL_DEFAULT;
	cfp->plc_facility = LOG_FACILITY_DEFAULT;

	faname = pfc_conf_get_string(blk, conf_log_facility, NULL);
	(void)pfc_log_facility_get(faname, &cfp->plc_facility);
}

/*
 * void
 * pfc_logconf_setsyslog(pfc_log_conf_t *PFC_RESTRICT cfp, pfc_bool_t sysonly)
 *	Set logging type.
 *	If `sysonly' is true, all logs are dumped to syslog.
 */
void
pfc_logconf_setsyslog(pfc_log_conf_t *PFC_RESTRICT cfp, pfc_bool_t sysonly)
{
	PFC_ASSERT(cfp->plc_type != LOG_TYPE_STREAM);
	if (sysonly) {
		cfp->plc_type = LOG_TYPE_SYSLOG;
	} else {
		cfp->plc_type = LOG_TYPE_FILE;
	}
}

/*
 * void
 * pfc_logconf_setrotate(pfc_log_conf_t *PFC_RESTRICT cfp,
 *			 uint32_t rcount, size_t rsize)
 *	Set rotation parameters of system log file.
 */
void
pfc_logconf_setrotate(pfc_log_conf_t *PFC_RESTRICT cfp,
		      uint32_t rcount, size_t rsize)
{
	cfp->plc_rcount = rcount;
	cfp->plc_rsize = rsize;
}

/*
 * void
 * pfc_logconf_setlevel(pfc_log_conf_t *cfp, pfc_log_level_t lvl)
 *	Set logging level.
 *
 *	If `lvl' is invalid, configuration parameter is not changed.
 */
void
pfc_logconf_setlevel(pfc_log_conf_t *cfp, pfc_log_level_t lvl)
{
	if (pfc_log_level_is_valid(lvl) == 0) {
		cfp->plc_level.plvc_level = lvl;
	}
}

/*
 * void
 * pfc_logconf_setdeflevel(pfc_log_conf_t *cfp, pfc_log_level_t lvl)
 *	Set default logging level, which is used if no default parameter
 *	is defined by the loglevel file and configuration file.
 *
 *	If `lvl' is invalid, configuration parameter is not changed.
 */
void
pfc_logconf_setdeflevel(pfc_log_conf_t *cfp, pfc_log_level_t lvl)
{
	if (pfc_log_level_is_valid(lvl) == 0) {
		cfp->plc_level.plvc_deflevel = lvl;
	}
}

/*
 * void
 * pfc_logconf_setpath(pfc_log_conf_t *PFC_RESTRICT cfp,
 *		    const char *PFC_RESTRICT base, size_t blen,
 *		    const char *PFC_RESTRICT dname, size_t dlen,
 *		    const char *PFC_RESTRICT fname, size_t flen)
 *	Set path name to the system log file.
 */
void
pfc_logconf_setpath(pfc_log_conf_t *PFC_RESTRICT cfp,
		    const char *PFC_RESTRICT base, size_t blen,
		    const char *PFC_RESTRICT dname, size_t dlen,
		    const char *PFC_RESTRICT fname, size_t flen)
{
	size_t	dirlen;

	if (PFC_EXPECT_FALSE(cfp->plc_logdir != NULL)) {
		free((void *)cfp->plc_logdir);
	}
	if (PFC_EXPECT_FALSE(cfp->plc_logpath != NULL)) {
		free((void *)cfp->plc_logpath);
	}

	if (dname == NULL) {
		/* Ignore log directory name. */
		cfp->plc_logdir = strdup(base);
		PFC_ASSERT(cfp->plc_logdir != NULL);
		dirlen = blen;
	}
	else {
		cfp->plc_logdir = logconf_alloc_path(base, blen, dname, dlen);
		dirlen = blen + dlen + 1;
	}
	cfp->plc_logpath = logconf_alloc_path(cfp->plc_logdir, dirlen,
					      fname, flen);
}

/*
 * void
 * pfc_logconf_setlvlpath(pfc_log_conf_t *PFC_RESTRICT cfp,
 *		       const char *PFC_RESTRICT base, size_t blen,
 *		       const char *PFC_RESTRICT fname, size_t flen)
 *	Set path name to the log level file.
 */
void
pfc_logconf_setlvlpath(pfc_log_conf_t *PFC_RESTRICT cfp,
		       const char *PFC_RESTRICT base, size_t blen,
		       const char *PFC_RESTRICT fname, size_t flen)
{
	if (PFC_EXPECT_FALSE(cfp->plc_lvlpath != NULL)) {
		free((void *)cfp->plc_lvlpath);
	}

	cfp->plc_lvlpath = logconf_alloc_path(base, blen, fname, flen);
}

/*
 * void
 * pfc_logconf_setfacility(pfc_log_conf_t *cfp, pfc_log_facl_t facility)
 *	Set logging facility.
 *
 *	This function does nothing if `facility' is not a valid logging
 *	facility.
 */
void
pfc_logconf_setfacility(pfc_log_conf_t *cfp, pfc_log_facl_t facility)
{
	int	err = pfc_log_facility_is_valid(facility);

	if (PFC_EXPECT_TRUE(err == 0)) {
		cfp->plc_facility = facility;
	}
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_logconf_destroy(pfc_log_conf_t *cfp)
 *	Destroy log configuration object.
 */
void PFC_ATTR_HIDDEN
pfc_logconf_destroy(pfc_log_conf_t *cfp)
{
	if (cfp->plc_logdir != NULL) {
		free((void *)cfp->plc_logdir);
	}
	if (cfp->plc_logpath != NULL) {
		free((void *)cfp->plc_logpath);
	}
	if (cfp->plc_lvlpath != NULL) {
		free((void *)cfp->plc_lvlpath);
	}
}

/*
 * static char *
 * logconf_alloc_path(const char *PFC_RESTRICT base, size_t blen,
 *		      const char *PFC_RESTRICT name, size_t nlen)
 *	Allocate the path name buffer.
 */
static char *
logconf_alloc_path(const char *PFC_RESTRICT base, size_t blen,
		   const char *PFC_RESTRICT name, size_t nlen)
{
	char	*path;
	size_t	sz = blen + nlen + 2;

	path = (char *)malloc(sz);
	PFC_ASSERT(path != NULL);
	PFC_ASSERT_INT(snprintf(path, sz, "%s/%s", base, name), (int)(sz - 1));

	return path;
}
