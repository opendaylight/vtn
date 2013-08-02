/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * thread_exit.c - Terminate logical thread.
 *
 * Remarks:
 *	This file must be compiled with C++ exception handling support,
 *	because pthread_exit() in glibc throws internal exception for
 *	thread cancellation.
 */

#include "thread_impl.h"

/*
 * void PFC_FATTR_NORETURN
 * pfc_thread_exit(void *status)
 *	Terminate calling thread.
 *
 * Remarks:
 *	Use of this function is strongly discouraged because it terminates
 *	current physical thread in the pool. Although it is not harmless,
 *	it costs more CPU time than returning thread function.
 */
void PFC_FATTR_NORETURN
pfc_thread_exit(void *status)
{
	tpool_job_t	*current = tpool_job_current;

	if (PFC_EXPECT_TRUE(current != NULL)) {
		tpool_thread_t	*ttp;

		ttp = (tpool_thread_t *)pthread_getspecific(pfc_thread_key);
		if (PFC_EXPECT_TRUE(ttp != NULL)) {
			TPOOL_THREAD_LOCK(ttp);
			ttp->tt_job = NULL;
			TPOOL_THREAD_UNLOCK(ttp);
		}
		pfc_thread_cleanup(current, status);
	}

	pthread_exit(NULL);
}
