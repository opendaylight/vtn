/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * libpfc_util.c - libpfc_util initialization.
 */

#include <string.h>
#include <pthread.h>
#include <pfc/util.h>
#include "util_impl.h"
#include "refptr_impl.h"
#include "hash_impl.h"
#include "conf_impl.h"
#include "tid_impl.h"

/*
 * Substring of valgrind library path.
 */
static const char	valgrind_keyword[] = "vgpreload";

/*
 * Determine whether the library is loaded via valgrind or not.
 */
pfc_bool_t	pfc_valgrind_mode PFC_ATTR_HIDDEN;

extern void	pfc_log_libinit(void);

/*
 * Internal prototypes.
 */
static void	libpfc_util_fork_prepare(void);
static void	libpfc_util_fork_parent(void);
static void	libpfc_util_fork_child(void);

/*
 * static void PFC_FATTR_INIT
 * libpfc_util_libinit(void)
 *	Constructor of libpfc_util library.
 */
static void PFC_FATTR_INIT
libpfc_util_libinit(void)
{
	char	*preload;
	int	err;

#ifdef	PFC_USE_GLIBC
	/*
	 * GNU libc registers atfork handlers for malloc() at the first call
	 * of malloc(). The malloc() prepare fork handler acquires internal
	 * malloc() locks in order to serialize malloc()/free() call while a
	 * thread in the process calls fork().
	 *
	 * The prepare fork handlers are called in the reverse order in which
	 * they were registered by calls of pthread_atfork(). If malloc()
	 * have never been called here, the malloc() prepare fork handler will
	 * be called before the libpfc_util prepare fork handler.
	 * It may cause deadlock due to lock order violation because the
	 * libpfc_util prepare fork handler acquires internal locks with
	 * holding internal malloc() locks. So we must guarantee that the
	 * malloc() atfork handlers are established.
	 *
	 * Note that the latest gcc knows what malloc() and free() are.
	 * The latest gcc's optimizer will ignore such code as
	 * "free(malloc(1))".
	 */
	{
		void *volatile	ptr = malloc(1);
		free(ptr);
	}
#endif	/* PFC_USE_GLIBC */

	/*
	 * Determine whether the library is loaded via valgrind or not.
	 * If valgrind library is pre-loaded, we assume that we're on the
	 * valgrind world.
	 */
	preload = getenv("LD_PRELOAD");
	if (preload != NULL && strstr(preload, valgrind_keyword) != NULL) {
		pfc_valgrind_mode = PFC_TRUE;
	}

	/* Initialize PFC logging architecture. */
	pfc_log_libinit();

	/* Initialize system configuration. */
	sysconf_init();

	/*
	 * Call initializer of reference pointer and hash table.
	 */
	pfc_refptr_libinit();

	/* Register prefork handler. */
	err = pthread_atfork(libpfc_util_fork_prepare, libpfc_util_fork_parent,
			     libpfc_util_fork_child);
	if (PFC_EXPECT_FALSE(err != 0)) {
		fprintf(stderr, "libpfc_util: Failed to register prefork "
			"handler: %s\n", strerror(err));
		abort();
		/* NOTREACHED */
	}
}

/*
 * static void PFC_FATTR_FINI
 * libpfc_util_libinit(void)
 *	Destructor of libpfc_util library.
 */
static void PFC_FATTR_FINI
libpfc_util_libfini(void)
{
	/*
	 * Call destructor of configuration file parser.
	 * Note that this call will destroy system configuration file handle.
	 */
	pfc_conf_libfini();
}

/*
 * static void
 * libpfc_util_fork_prepare(void)
 *	fork(2) handler which will be called just before fork(2) on parent
 *	process.
 */
static void
libpfc_util_fork_prepare(void)
{
	pfc_conf_fork_prepare();

	/*
	 * pfc_refptr_fork_prepare() acquires the mutex for refptr string.
	 * So it must be called after configuration file fork(2) handler
	 * because configuration file APIs may create refptr string with
	 * holding locks.
	 */
	pfc_refptr_fork_prepare();
}

/*
 * static void
 * libpfc_util_fork_parent(void)
 *	fork(2) handler which will be called just before returning from fork(2)
 *	on parent process.
 */
static void
libpfc_util_fork_parent(void)
{
	pfc_refptr_fork_parent();
	pfc_conf_fork_parent();
}

/*
 * static void
 * libpfc_util_fork_child(void)
 *	fork(2) handler which will be called just before returning from fork(2)
 *	on child process.
 */
static void
libpfc_util_fork_child(void)
{
	libpfc_tid_fork_child();
	pfc_refptr_fork_child();
	pfc_conf_fork_child();
}
