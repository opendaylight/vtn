/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_UTIL_LOG_IMPL_H
#define	_PFC_LIBPFC_UTIL_LOG_IMPL_H

/*
 * Internal definitions for logging system.
 */

#include <stdio.h>
#include <pfc/base.h>
#include <pfc/conf.h>
#include <pfc/log.h>

PFC_C_BEGIN_DECL

/*
 * Destination types.
 */
#define	LOG_TYPE_STREAM			PFC_CONST_U(1)
#define	LOG_TYPE_SYSLOG			PFC_CONST_U(2)
#define	LOG_TYPE_FILE			PFC_CONST_U(3)

/*
 * Default logging level.
 */
#define	LOG_LEVEL_DEFAULT		PFC_LOGLVL_INFO

/*
 * Error location number in file rotation function.
 */
#define	LOG_ERRLOC_APPEND		1U
#define	LOG_ERRLOC_FSTAT		2U
#define	LOG_ERRLOC_TRUNCATE		3U
#define	LOG_ERRLOC_CREATE		4U
#define	LOG_ERRLOC_CREATE_ROTATE	5U
#define	LOG_ERRLOC_FDOPEN		6U
#define	LOG_ERRLOC_ROTATION		7U
#define	LOG_ERRLOC_ROTATION_LAST	8U
#define	LOG_ERRLOC_DIR_LSTAT		9U
#define	LOG_ERRLOC_DIR_CHMOD		10U
#define	LOG_ERRLOC_DIR_MKDIR		11U
#define	LOG_ERRLOC_LSTAT		12U
#define	LOG_ERRLOC_BUSY			13U

/*
 * Prototypes.
 */
#ifdef	_PFC_LIBPFC_UTIL_BUILD
extern int	pfc_log_level_is_valid(pfc_log_level_t level);
extern int	pfc_log_facility_is_valid(pfc_log_facl_t facility);
extern int	pfc_log_facility_get(const char *PFC_RESTRICT name,
				     pfc_log_facl_t *PFC_RESTRICT fap);
extern void	pfc_logconf_destroy(pfc_log_conf_t *cfp);
#endif	/* _PFC_LIBPFC_UTIL_BUILD */

PFC_C_END_DECL

#endif	/* !_PFC_LIBPFC_UTIL_LOG_IMPL_H */
