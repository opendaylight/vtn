/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_THREAD_H
#define	_PFC_THREAD_H

/*
 * Public definitions for PFC thread implementation.
 */

#include <pfc/base.h>
#include <pfc/clock.h>
#include <pthread.h>
#include <errno.h>

PFC_C_BEGIN_DECL

/*
 * Thread identifier.
 */
typedef uint32_t	pfc_thread_t;

/*
 * Special thread ID which means invalid thread.
 */
#define	PFC_THREAD_INVALID	0U

/*
 * Prototype for thread function.
 */
typedef void		*(*pfc_thfunc_t)(void *arg);

/*
 * The key of PFC thread specific data.
 */
typedef uint32_t	pfc_tsd_key_t;

/*
 * Invalid TSD key.
 */
#define	PFC_TSD_KEY_INVALID	PFC_CONST_U(0)

/*
 * Prototype for destructor of PFC thread specific data.
 */
typedef void		(*pfc_tsd_dtor_t)(pfc_ptr_t value);

/*
 * The type of identifier used by pfc_thread_once().
 */
typedef pthread_once_t	pfc_once_t;

/*
 * Prototype for initializer used by pfc_thread_once().
 */
typedef void		(*pfc_once_init_t)(void);

/*
 * Initial value of identifier used by pfc_thread_once().
 */
#define	PFC_THREAD_ONCE_INIT	PTHREAD_ONCE_INIT

/*
 * Thread exit status which indicates it is cancelled.
 */
#define	PFC_THREAD_CANCELED	((void *)-1)

/*
 * Flags for pfc_thread_create() / pfc_thread_createat()
 */
#define	PFC_THREAD_DETACHED	PFC_CONST_U(0x1)

/*
 * Prototypes.
 */
extern int	pfc_thread_create(pfc_thread_t *PFC_RESTRICT thread,
				  pfc_thfunc_t func, void *PFC_RESTRICT arg,
				  uint32_t flags);
extern int	pfc_thread_createat(pfc_thread_t *PFC_RESTRICT thread,
				    const char *PFC_RESTRICT poolname,
				    pfc_thfunc_t func, void *PFC_RESTRICT arg,
				    uint32_t flags);
extern int	pfc_thread_join(pfc_thread_t thread, void **statusp);
extern int	pfc_thread_timedjoin(pfc_thread_t thread,
				     void *PFC_RESTRICT *statusp,
				     const pfc_timespec_t *PFC_RESTRICT to);
extern int	pfc_thread_detach(pfc_thread_t thread);
extern void	pfc_thread_exit(void *status)  PFC_FATTR_NORETURN;

extern pfc_thread_t	pfc_thread_self(void);

extern int	pfc_thread_key_create(pfc_tsd_key_t *PFC_RESTRICT keyp,
				      pfc_tsd_dtor_t dtor);
extern int	pfc_thread_key_delete(pfc_tsd_key_t key);
extern int	pfc_thread_setspecific(pfc_tsd_key_t key, pfc_cptr_t value);
extern pfc_ptr_t	pfc_thread_getspecific(pfc_tsd_key_t key);

extern int	pfc_thread_once(pfc_once_t *PFC_RESTRICT control,
				pfc_once_init_t init_routine);

/*
 * The following definitions are only for internal use, not public.
 */
typedef void		(*__pfc_tsd_dtor_t)(pfc_ptr_t value, pfc_ptr_t arg);

extern int	__pfc_thread_key_create(pfc_tsd_key_t *PFC_RESTRICT keyp,
					__pfc_tsd_dtor_t dtor,
					pfc_ptr_t PFC_RESTRICT arg);

PFC_C_END_DECL

#endif	/* !_PFC_THREAD_H */
