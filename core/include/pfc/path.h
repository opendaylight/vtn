/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_PATH_H
#define	_PFC_PATH_H

/*
 * This header file contains default file path for PFC core system.
 */

#include <pfc/base.h>

/*
 * Runtime directory path.
 */
#define	PFC_RUNDIR_PATH		PFC_LOCALSTATEDIR "/run"

/*
 * Database directory path.
 */
#define	PFC_DBDIR_PATH		PFC_LOCALSTATEDIR "/db"

/*
 * Daemon process name.
 */
#define	PFC_PFCD_NAME		"uncd"

/*
 * Default configuration file path.
 */
#define	PFC_PFCD_CONF_PATH	PFC_SYSCONFDIR "/" PFC_PFCD_NAME ".conf"

/*
 * Default PID file path.
 */
#define	PFC_PFCD_PID_PATH	PFC_RUNDIR_PATH "/" PFC_PFCD_NAME ".pid"

/*
 * Default working directory for pfcd.
 */
#define	PFC_PFCD_WORKDIR_PATH	PFC_RUNDIR_PATH "/" PFC_PFCD_NAME

/*
 * Default database directory for pfcd.
 */
#define	PFC_PFCD_DBDIR_PATH	PFC_DBDIR_PATH "/" PFC_PFCD_NAME

/*
 * Default stderr logging directory.
 */
#define	PFC_PFCD_ERRLOG_PATH	PFC_PFCD_WORKDIR_PATH "/stderr"

/*
 * Directory name which contains control channels.
 * This directory is located under working directory.
 */
#define	PFC_PFCD_CHANNEL_NAME		"channel"
#define	PFC_PFCD_CHANNEL_NAMELEN	7

/*
 * Name of daemon controller port.
 * This file is located under channel directory.
 */
#define	PFC_PFCD_CTRL_NAME		"ctrl"
#define	PFC_PFCD_CTRL_NAMELEN		4

/*
 * Name of module cache file.
 */
#define	PFC_MODCACHE_NAME		"module.cache"
#define	PFC_MODCACHE_NAMELEN		12

/*
 * Directory name which contains message log files.
 * This directory is located under working directory.
 */
#define	PFC_PFCD_LOG_NAME		"log"
#define	PFC_PFCD_LOG_NAMELEN		3

/*
 * Name of message log file.
 */
#define	PFC_MSGLOG_NAME			"pfcd_message.log"
#define	PFC_MSGLOG_NAMELEN		16

/*
 * Directory name which contains monitor log files.
 * This directory is located under working directory.
 */
#define	PFC_PFCD_MONITOR_NAME		"monitor"
#define	PFC_PFCD_MONITOR_NAMELEN	7

/*
 * Name of monitor log file.
 */
#define	PFC_MONLOG_NAME			"pfc_monitor.log"
#define	PFC_MONLOG_NAMELEN		15

/*
 * Name of log level file.
 */
#define PFC_LOGLVL_NAME			"pfc_loglevel"
#define PFC_LOGLVL_NAMELEN		12

#endif	/* !_PFC_PATH_H */
