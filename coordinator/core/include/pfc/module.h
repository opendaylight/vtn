/*
 * Copyright (c) 2010-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_MODULE_H
#define	_PFC_MODULE_H

/*
 * Definitions for PFC module management.
 */

#include <pfc/base.h>
#include <pfc/synch.h>
#include <pfc/conf.h>
#include <pfc/event.h>
#include <pfc/moddefs.h>
#include <pfc/modconst.h>
#include <pfc/modevent.h>
#include <pfc/modipc_server.h>
#include <pfc/modipc_client.h>
#include <pfc/debug.h>

PFC_C_BEGIN_DECL

/*
 * Suffix of module filename.
 */
#define	PFC_MODULE_LIB_SUFFIX		"so"

/*
 * Suffix of module configuration file.
 */
#define	PFC_MODULE_CONF_SUFFIX		".conf"
#define	PFC_MODULE_CONF_SUFFIX_LEN	PFC_CONST_U(5)

/*
 * Prototypes.
 */
extern pfc_module_t	pfc_module_register(const pfc_modattr_t
					    *PFC_RESTRICT mattr,
					    pfc_cptr_t PFC_RESTRICT data);
extern pfc_module_t	pfc_module_c_register(const pfc_modattr_t
					      *PFC_RESTRICT mattr,
					      pfc_modfunc_t init,
					      pfc_modfunc_t fini);

extern void		__pfc_module_bootstrap(const char *name,
					       pfc_modboot_t func);

extern pfc_bool_t	pfc_module_is_started(void);
extern pfc_bool_t	pfc_module_hold(pfc_module_t module);
extern void		pfc_module_release(pfc_module_t module);
extern pfc_cfblk_t	__pfc_module_conf_getblock(pfc_module_t module,
						   const char *bname);
extern pfc_cfblk_t	__pfc_module_conf_getmap(pfc_module_t module,
						 const char *mname,
						 const char *key);
extern int		__pfc_module_conf_getmapkeys(pfc_module_t module,
						     const char *mname,
						     pfc_listm_t *keysp);
extern int		__pfc_module_conf_reload(pfc_module_t module);

extern int	__pfc_module_event_add(pfc_module_t module,
				       pfc_evhandler_t *PFC_RESTRICT idp,
				       pfc_evfunc_t handler,
				       pfc_ptr_t PFC_RESTRICT arg,
				       const pfc_evmask_t *PFC_RESTRICT evmask,
				       uint32_t priority,
				       const char *PFC_RESTRICT hname);
extern int	__pfc_module_event_post(pfc_module_t module,
					const char *target, pfc_evtype_t type);

extern int	__pfc_module_addhook(pfc_module_t module, pfc_modhook_t hook,
				     uint32_t pri);

/*
 * PFC_MODULE_BUILD is defined by the build system only for PFC module build.
 */
#ifdef	PFC_MODULE_BUILD

/*
 * static inline pfc_cfblk_t PFC_FATTR_ALWAYS_INLINE
 * pfc_module_conf_getblock(const char *bname)
 *	Return parameter block handle associated with the parameter block,
 *	which is defined in the module configuration file, specified by
 *	the name.
 *
 * Calling/Exit State:
 *	A valid block handle is returned on success.
 *	On error, PFC_CFBLK_INVALID is returned.
 */
static inline pfc_cfblk_t PFC_FATTR_ALWAYS_INLINE
pfc_module_conf_getblock(const char *bname)
{
	return __pfc_module_conf_getblock(PFC_MODULE_THIS_ID, bname);
}

/*
 * static inline pfc_cfblk_t PFC_FATTR_ALWAYS_INLINE
 * pfc_module_conf_getmap(const char *mname, const char *key)
 *	Return parameter map handle associated with the parameter map block,
 *	which is defined in the module configuration file, specified by
 *	the map name and key.
 *
 * Calling/Exit State:
 *	A valid block handle is returned on success.
 *	On error, PFC_CFBLK_INVALID is returned.
 */
static inline pfc_cfblk_t PFC_FATTR_ALWAYS_INLINE
pfc_module_conf_getmap(const char *mname, const char *key)
{
	return __pfc_module_conf_getmap(PFC_MODULE_THIS_ID, mname, key);
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_module_conf_getmapkeys(const char *mname, pfc_listm_t *keysp)
 *	Create a list model instance which contains all map keys of the
 *	parameter map specified by `mname' in the module configuration file.
 *
 * Calling/Exit State:
 *	Upon successful completion, the list model instance is set to the
 *	buffer pointed by `keysp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_module_conf_getmapkeys(const char *mname, pfc_listm_t *keysp)
{
	return __pfc_module_conf_getmapkeys(PFC_MODULE_THIS_ID, mname, keysp);
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_module_conf_reload(void)
 *	Reload module configuration file.
 *	Note that all parameter block handles obtained by
 *	pfc_module_conf_getblock() and pfc_module_conf_getmap() are discarded.
 *	You must re-obtain parameter block handles again.
 *
 * Calling/Exit State:
 *	Zero is returned on success.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_module_conf_reload(void)
{
	return __pfc_module_conf_reload(PFC_MODULE_THIS_ID);
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_module_event_add(pfc_evhandler_t *PFC_RESTRICT idp,
 *			pfc_evfunc_t handler, pfc_ptr_t PFC_RESTRICT arg,
 *			const pfc_evmask_t *PFC_RESTRICT evmask,
 *			uint32_t priority)
 *	Register module-specific event handler.
 *
 *	`handler' is a pointer to event handler, and `arg' is an arbitrary
 *	value passed to the handler. If a module-specific event is posted
 *	to this module, and its type is contained in the event mask specified
 *	by `evmask', the handler is called with specifying the event.
 *	If NULL is specified to `evmask', all events posted to this module
 *	are delivered to the handler.
 *
 * Calling/Exit State:
 *	Upon successful completion, event handler ID is set to `*idp',
 *	and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_module_event_add(pfc_evhandler_t *PFC_RESTRICT idp, pfc_evfunc_t handler,
		     pfc_ptr_t PFC_RESTRICT arg,
		     const pfc_evmask_t *PFC_RESTRICT evmask, uint32_t priority)
{
	return __pfc_module_event_add(PFC_MODULE_THIS_ID, idp, handler,
				      arg, evmask, priority, NULL);
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_module_event_add_named(pfc_evhandler_t *PFC_RESTRICT idp,
 *			      pfc_evfunc_t handler, pfc_ptr_t PFC_RESTRICT arg,
 *			      const pfc_evmask_t *PFC_RESTRICT evmask,
 *			      uint32_t priority, const char *PFC_RESTRICT hname)
 *	Register module-specific event handler, with specifying handler's name.
 *
 *	`handler' is a pointer to event handler, and `arg' is an arbitrary
 *	value passed to the handler. If a module-specific event is posted
 *	to this module, and its type is contained in the event mask specified
 *	by `evmask', the handler is called with specifying the event.
 *	If NULL is specified to `evmask', all events posted to this module
 *	are delivered to the handler.
 *
 *	`hname' is a user-defined name of event handler.
 *	Currently, it is used only for event delivery logging.
 *	If NULL is specified, module name is used as handler's name.
 *
 * Calling/Exit State:
 *	Upon successful completion, event handler ID is set to `*idp',
 *	and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_module_event_add_named(pfc_evhandler_t *PFC_RESTRICT idp,
			   pfc_evfunc_t handler, pfc_ptr_t PFC_RESTRICT arg,
			   const pfc_evmask_t *PFC_RESTRICT evmask,
			   uint32_t priority, const char *PFC_RESTRICT hname)
{
	return __pfc_module_event_add(PFC_MODULE_THIS_ID, idp, handler,
				      arg, evmask, priority, hname);
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_module_event_remove(pfc_evhandler_t id, const pfc_timespec_t *timeout)
 *	Remove the module-specific event handler added by the call of
 *	pfc_module_event_add().
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
pfc_module_event_remove(pfc_evhandler_t id, const pfc_timespec_t *timeout)
{
	return pfc_event_remove_handler(id, timeout);
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_module_event_post(const char *target, pfc_evtype_t type)
 *	Post a module-specific event of the type specified by `type'.
 *
 *	The event is delivered to only handlers added by the module specified
 *	by `target'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	ENOENT is returned if an invalid module name is specified to `target'.
 *	EPERM is returned if the module specified by `target' is not loaded.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_module_event_post(const char *target, pfc_evtype_t type)
{
	return __pfc_module_event_post(PFC_MODULE_THIS_ID, target, type);
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_module_addhook(pfc_modhook_t hook, uint32_t pri)
 *	Register module unload hook.
 *
 *	`hook' is a hook function to be called.
 *	`pri' is an uint32_t value which determines order of hook calls.
 *	Hook functions are called in ascending order of priority.
 *
 *	Module unload hook takes a boolean argument.
 *	PFC_TRUE is passed if the module is about to be unloaded in the PFC
 *	daemon shutdown sequence.
 *	PFC_FALSE is passed if the module is about to be unloaded by user
 *	request.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_module_addhook(pfc_modhook_t hook, uint32_t pri)
{
	return __pfc_module_addhook(PFC_MODULE_THIS_ID, hook, pri);
}

#endif	/* PFC_MODULE_BUILD */

PFC_C_END_DECL

#endif	/* !_PFC_MODULE_H */
