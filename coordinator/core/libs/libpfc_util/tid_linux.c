/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * tid_linux.c - System thread ID management. (Linux specific)
 */

/*
 * Suppress declaration of inlined pfc_gettid().
 */
#define	__PFC_UTIL_DONT_DEFINE_GETTID

#include "tid_impl.h"

/*
 * Define entity of thread ID cache.
 */
__thread pfc_tid_t	tid_cache PFC_ATTR_HIDDEN = PFC_TID_INVALID;

/*
 * pfc_tid_t
 * pfc_gettid(void)
 *	Return identifier of system thread for the calling thread.
 */
pfc_tid_t
pfc_gettid(void)
{
	return __pfc_gettid();
}
