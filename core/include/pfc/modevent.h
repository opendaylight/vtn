/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_MODEVENT_H
#define	_PFC_MODEVENT_H

/*
 * Definitions for module-specific event.
 */

#include <string.h>
#include <pfc/base.h>
#include <pfc/event.h>
#include <pfc/refptr.h>
#include <pfc/modconst.h>

PFC_C_BEGIN_DECL

/*
 * Event type from 0 to 23 are defined as module-local event type.
 */
#define	PFC_MODEVENT_TYPE_LOCAL_MIN	PFC_CONST_U(0)
#define	PFC_MODEVENT_TYPE_LOCAL_MAX	PFC_CONST_U(23)

/*
 * The meaning of types from 24 to 31 are defined by the system.
 */

/* Configuration reload event. */
#define	PFC_MODEVENT_TYPE_RELOAD	PFC_CONST_U(24)

#define	PFC_MODEVENT_TYPE_SYSTEM_MIN	PFC_MODEVENT_TYPE_RELOAD
#define	PFC_MODEVENT_TYPE_SYSTEM_MAX	PFC_MODEVENT_TYPE_RELOAD

/*
 * Determine whether the specified module-specific event type is module local
 * type or not.
 */
#define	PFC_MODEVENT_TYPE_IS_LOCAL(type)	\
	((type) <= PFC_MODEVENT_TYPE_LOCAL_MAX)

/*
 * Determine whether the specified module-specific event type is valid or not.
 */
#define	PFC_MODEVENT_TYPE_IS_VALID(type)		\
	((type) <= PFC_MODEVENT_TYPE_SYSTEM_MAX)

/*
 * Prototypes.
 */
extern pfc_bool_t	pfc_module_is_modevent(pfc_event_t event);
extern const char	*pfc_module_event_getsender(pfc_event_t event);

#ifdef	PFC_MODULE_CORE_THIS

/*
 * Prototypes only for core component.
 */
extern int	__pfc_modevent_add(pfc_evhandler_t *PFC_RESTRICT idp,
				   const char *PFC_RESTRICT name,
				   pfc_evfunc_t handler,
				   pfc_ptr_t PFC_RESTRICT arg,
				   const pfc_evmask_t *PFC_RESTRICT evmask,
				   uint32_t priority,
				   const char *PFC_RESTRICT hname);
extern int	__pfc_modevent_post(const char *name, const char *target,
				    pfc_evtype_t type);
extern int	__pfc_modevent_clear(const char *PFC_RESTRICT name,
				     const pfc_timespec_t *PFC_RESTRICT
				     timeout);

/*
 * int
 * pfc_modevent_add(pfc_evhandler_t *PFC_RESTRICT idp, pfc_evfunc_t handler,
 *		    pfc_ptr_t PFC_RESTRICT arg,
 *		    const pfc_evmask_t *PFC_RESTRICT evmask, uint32_t priority)
 * int
 * pfc_modevent_add_named(pfc_evhandler_t *PFC_RESTRICT idp,
 *			  pfc_evfunc_t handler, pfc_ptr_t PFC_RESTRICT arg,
 *			  const pfc_evmask_t *PFC_RESTRICT evmask,
 *			  uint32_t priority, const char *PFC_RESTRICT hname)
 *	Register module-specific event handler.
 *
 *	This function is used to register module-specific event handler for
 *	PFC core component.
 *
 *	pfc_modevent_add_named() is used to set user-specified name to the
 *	event handler.
 *
 * Calling/Exit State:
 *	Upon successful completion, event handler ID is set to `*idp',
 *	and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	These functions require PFC_LOG_IDENT to be defined.
 */
#define	pfc_modevent_add(idp, handler, arg, evmask, priority)		\
	__pfc_modevent_add(idp, PFC_MODULE_CORE_THIS, handler, arg,	\
			   evmask, priority, PFC_LOG_IDENT)

#define	pfc_modevent_add_named(idp, handler, arg, evmask, priority, hname) \
	__pfc_modevent_add(idp, PFC_MODULE_CORE_THIS, handler, arg,	\
			   evmask, priority, hname)

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_modevent_remove(pfc_evhandler_t id, const pfc_timespec_t *timeout)
 *	Remove the module-specific event handler added by the call of
 *	pfc_modevent_add().
 *
 *	`id' must be an ID of module-specific event handler.
 *	If the handler specified by `id' is being called, the calling thread
 *	will block until the handler returns. If it does not return within
 *	the period specified by `timeout', ETIMEDOUT is returned.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The event handler may not be able to reuse even if this function
 *	returns error, except for EINVAL.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_modevent_remove(pfc_evhandler_t id, const pfc_timespec_t *timeout)
{
	return pfc_event_remove_handler(id, timeout);
}

/*
 * int
 * pfc_modevent_post(const char *target, pfc_evtype_t type)
 *	Post a module-specific event to the module specified by `target'.
 *
 *	This function is used to post a module-specific event from PFC core
 *	component.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function requires PFC_LOG_IDENT to be defined.
 */
#define	pfc_modevent_post(target, type)					\
	__pfc_modevent_post(PFC_MODULE_CORE_THIS, target, type)

/*
 * int
 * pfc_modevent_clear(const pfc_timespec_t *timeout)
 *	Remove all module-specific event handlers associated with the
 *	PFC core component of the calling thread.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function requires PFC_LOG_IDENT to be defined.
 *
 *	- The event handler may not be able to reuse even if this function
 *	  returns error, except for EINVAL.
 */
#define	pfc_modevent_clear(timeout)				\
	__pfc_modevent_clear(PFC_MODULE_CORE_THIS, timeout)

#endif	/* PFC_MODULE_CORE_THIS */

PFC_C_END_DECL

#endif	/* !_PFC_MODEVENT_H */
