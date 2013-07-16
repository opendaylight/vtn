/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * Definitions for libpfc_util/cloexec.c test sub command.
 */

#ifndef	_TEST_SUB_CLOEXEC_H
#define	_TEST_SUB_CLOEXEC_H

/*
 * Name of sub command.
 */
#define	SUB_CLOEXEC_NAME		"sub_cloexec"

/*
 * Path to sub_cloexec object directory.
 */
#define	SUB_CLOEXEC_OBJDIR		OBJDIR "/" SUB_CLOEXEC_NAME

/*
 * Path to sub_cloexec command.
 */
#define	SUB_CLOEXEC_PATH		SUB_CLOEXEC_OBJDIR "/" SUB_CLOEXEC_NAME

/*
 * Maximum size of pipe output.
 */
#define	SUB_CLOEXEC_MAX_OUTSIZE		64

/*
 * I/O timeout, in seconds.
 */
#define	SUB_CLOEXEC_IO_TIMEOUT		5

#endif	/* !_TEST_SUB_CLOEXEC_H */
