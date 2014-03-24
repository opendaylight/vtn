/*
 * Copyright (c) 2010-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * module.c - PFC module management.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include <unistd.h>
#include <pfc/hash.h>
#include <pfc/synch.h>
#include <pfc/atomic.h>
#include <pfc/conf.h>
#include <pfc/log.h>
#include <modcache.h>
#include <ipcsrv_impl.h>
#include "event_impl.h"
#include "module_impl.h"
#include "modevent_impl.h"

/*
 * Module table which manages all registered modules.
 */
static pmodule_t	*module_table;

/*
 * Number of registered modules.
 */
static uint32_t		module_count;

/*
 * Path to module directory.
 */
static const char	*module_dir;

/*
 * Length of module directory path.
 */
static size_t		module_dir_len;

/*
 * String length enough to store an absolute path to module related file.
 */
#define	PMODULE_PATH_MAX	(module_dir_len + PFC_MODULE_FNAME_MAX + 2)

/*
 * Path to module configuration file directory, and its length.
 */
static const char	*module_confdir;
static size_t		module_confdir_len;

/*
 * Global lock for module library loading.
 * Main purpose of this lock is guard error message returned by dlerror().
 */
static pfc_mutex_t	module_load_lock = PFC_MUTEX_INITIALIZER;

/*
 * Name of the module-specific event source.
 */
static const char	module_evsrc[] = "module_event";

/*
 * How long, in seconds, we should wait for completion of deletion of the
 * module-specific event source.
 */
#define	PMODULE_EVENT_DESTROY_TIMEOUT		PFC_CONST_U(30)

/*
 * How long, in seconds, we should wait for completion of deletion of the
 * module-specific event handlers.
 */
#define	PMODULE_EVENT_RMHANDLER_TIMEOUT		PFC_CONST_U(10)

/*
 * Event mask which contains all supported module-specific event types.
 */
#define	PMODULE_EVMASK_LOCAL					\
	(EVMASK_BIT(PFC_MODEVENT_TYPE_LOCAL_MAX + 1) - 1)
#define	PMODULE_EVMASK_SYSTEM			\
	EVMASK_BIT(PFC_MODEVENT_TYPE_RELOAD)
#define	PMODULE_EVMASK_VALID				\
	(PMODULE_EVMASK_LOCAL | PMODULE_EVMASK_SYSTEM)

/*
 * Event mask which represents invalid event types.
 */
#define	PMODULE_EVMASK_INVALID		(~PMODULE_EVMASK_VALID)

#define	PMODULE_EVMASK_IS_INVALID(mask)					\
	PFC_EXPECT_FALSE(EVMASK_TEST((mask), PMODULE_EVMASK_INVALID))

/*
 * IPC event handler ID registered by the module.
 */
typedef struct {
	pfc_ipcevhdlr_t		ieh_id;		/* IPC event handler's ID */
	pfc_rbnode_t		ieh_node;	/* Red-Black tree node */
} mod_ipcevhdlr_t;

#define	PMODULE_IPCEVHDLR_NODE2PTR(node)			\
	PFC_CAST_CONTAINER((node), mod_ipcevhdlr_t, ieh_node)

#define	PMODULE_IPCEVHDLR_KEY(id)	((pfc_cptr_t)(uintptr_t)(id))

/*
 * Initialize IPC event handler tree.
 */
#define	PMODULE_IPCEVHDLR_INIT(mod)					\
	pfc_rbtree_init(&(mod)->pm_ipcevhdlr, pfc_rbtree_uint32_compare, \
			pmodule_ipcevhdlr_getkey)

/*
 * Copy IPC event handler tree in the module, and clear it.
 * This macro needs to be used with holding the module lock.
 */
#define	PMODULE_IPCEVHDLR_CLEAR(mod, tree)		\
	do {						\
		*(tree) = (mod)->pm_ipcevhdlr;		\
		PMODULE_IPCEVHDLR_INIT(mod);		\
	} while (0)

/*
 * Module operations for modules written in C language.
 */
static int		pfc_module_c_init(pmodule_t *mod);
static pfc_bool_t	pfc_module_c_fini(pmodule_t *mod);
static int		pfc_module_c_ipc_register(pmodule_t *mod);

static const pfc_modops_t	module_c_ops = {
	.init		= pfc_module_c_init,
	.fini		= pfc_module_c_fini,
	.ipc_register	= pfc_module_c_ipc_register,
};

/*
 * Module operations associated with the module type.
 */
static pfc_modops_t const	*module_ops[PFC_MODTYPE_MAX + 1] = {
	&module_c_ops,		/* PFC_MODTYPE_C */
};

#define	PFC_MODTYPE_ASSERT(type)				\
	PFC_ASSERT((type) < PFC_ARRAY_CAPACITY(module_ops))

/*
 * Module unload hook entry.
 */
typedef struct {
	pfc_modhook_t	mhe_hook;		/* unload hook function */
	pfc_module_t	mhe_module;		/* module ID */
	uint32_t	mhe_priority;		/* priority */
	pfc_list_t	mhe_list;		/* link for global hook list */
} mod_hook_t;

#define	MOD_HOOK_LIST2PTR(list)					\
	PFC_CAST_CONTAINER((list), mod_hook_t, mhe_list)

/*
 * List of module unload hooks.
 */
typedef struct {
	pfc_list_t	mhl_head;		/* list of unload hooks */
	pfc_mutex_t	mhl_mutex;		/* mutex */
} mod_hooklist_t;

#define	MOD_HOOKLIST_LOCK(hlp)		pfc_mutex_lock(&(hlp)->mhl_mutex)
#define	MOD_HOOKLIST_UNLOCK(hlp)	pfc_mutex_unlock(&(hlp)->mhl_mutex)

static mod_hooklist_t	module_unload_hooks = {
	.mhl_head	= PFC_LIST_INITIALIZER(module_unload_hooks.mhl_head),
	.mhl_mutex	= PFC_MUTEX_INITIALIZER,
};

/*
 * Set true if all mandatory modules are started.
 */
static pfc_bool_t	module_started = PFC_FALSE;

/*
 * Internal prototypes.
 */
static void	pmodule_ctor(modch_ctx_t *PFC_RESTRICT ctx,
			     pmodule_t *PFC_RESTRICT mod, uint32_t index);

static pfc_ipcresp_t	pmodule_ipc_handler(pfc_ipcsrv_t *srv,
					    pfc_ipcid_t service,
					    pfc_ptr_t arg);

static char	*pmodule_library_path(pmodule_t *mod);
static int	pmodule_close_library(pmodule_t *mod, void *handle);
static void	pmodule_link_depends(modch_cdep_t *PFC_RESTRICT depends,
				     pmod_dep_t **PFC_RESTRICT headp,
				     uint16_t *PFC_RESTRICT nump);
static int	pmodule_register(const char *PFC_RESTRICT name,
				 pmodule_t *PFC_RESTRICT mod,
				 pfc_cptr_t PFC_RESTRICT data);
static pmodule_t	*pmodule_hold(pfc_module_t module);
static void		pmodule_release(pfc_module_t module);
static pfc_cptr_t	pmodule_getkey(pfc_rbnode_t *node);

static pfc_bool_t	pmodule_select_loaded(pmodule_t *mod);
static pfc_bool_t	pmodule_select_unloaded(pmodule_t *mod);
static pfc_bool_t	pmodule_select_running(pmodule_t *mod);

static int	pmodule_conf_load(pmodule_t *PFC_RESTRICT mod,
				  const char *PFC_RESTRICT path);
static int	pmodule_conf_reload(pmodule_t *mod);
static void	pmodule_conf_debug(pmodule_t *PFC_RESTRICT mod,
				   const char *PFC_RESTRICT action);
static void	pmodule_set_mandatory(pmodule_t *mod, pfc_bool_t mandatory);
static int	pmodule_select_wait(pmodule_t *mod, pmod_selector_t selector,
				    pfc_bool_t loading);
static int	pmodule_depends_wait(pmodule_t *mod, pmod_selector_t selector);
static int	pmodule_rev_depends_wait(pmodule_t *mod,
					 pmod_selector_t selector);

static int	pmodule_call_init(pmodule_t *mod);
static int	pmodule_call_fini(pmodule_t *mod);
static int	pmodule_call_boot(pmodule_t *mod);

static int	pmodule_load(pmodule_t *mod);
static int	pmodule_do_load(pmodule_t *mod);
static void	pmodule_dead(pmodule_t *mod);
static int	pmodule_start(pmodule_t *mod);
static int	pmodule_unload(pmodule_t *mod);

static int	pmodule_event_add(pfc_evhandler_t *PFC_RESTRICT idp,
				  pfc_refptr_t *domain, pfc_evfunc_t handler,
				  pfc_ptr_t PFC_RESTRICT arg,
				  const pfc_evmask_t *PFC_RESTRICT evmask,
				  uint32_t priority, pfc_refptr_t *rhname);
static int	pmodule_event_post(pfc_refptr_t *PFC_RESTRICT source,
				   const char *PFC_RESTRICT target,
				   pfc_evtype_t type);
static void	pmodule_event_dtor(pfc_ptr_t data);
static int	pmodule_event_clear(const char *PFC_RESTRICT name,
				    const pfc_timespec_t *PFC_RESTRICT timeout);

static void	pmodule_addhook(pfc_list_t *PFC_RESTRICT head,
				mod_hook_t *PFC_RESTRICT hep);
static void	pmodule_runhooks(pmodule_t *mod, pfc_bool_t isstop);
static void	pmodule_cleanhooks(pmodule_t *mod);

static void	pmodule_ipcevhdlr_dtor(pfc_rbnode_t *node, pfc_ptr_t arg);

static pfc_cptr_t	pmodule_ipcevhdlr_getkey(pfc_rbnode_t *node);

/*
 * Red-Black Tree which keeps pairs of module name and descriptor.
 *
 * Remarks:
 *	No lock is needed to serialize access to this tree because it is
 *	constructed while the PFC daemon is still single-threaded, and never
 *	updated by multiple threads.
 */
static pfc_rbtree_t	module_nametree =
	PFC_RBTREE_INITIALIZER((pfc_rbcomp_t)strcmp, pmodule_getkey);

/*
 * State change handler.
 */
typedef struct {
	pmod_state_t	pa_state;		/* target module state */
	int		(*pa_handler)(pmodule_t *mod);	/* handler */
} pmod_action_t;

static const pmod_action_t	module_state_actions[] = {
	{PMOD_STATE_LOADED, pmodule_load},
	{PMOD_STATE_RUNNING, pmodule_start},
	{PMOD_STATE_UNLOADED, pmodule_unload},
};

/*
 * String representation of module state.
 */
typedef struct {
	pmod_state_t	mss_state;		/* module state */
	const char	*mss_name;		/* name of state */
} pmod_state_str_t;

#define	PMOD_STATE_STR_DECL(state)	{ (state), #state + 11 }

static const pmod_state_str_t	module_state_label[] = {
	PMOD_STATE_STR_DECL(PMOD_STATE_UNLOADED),
	PMOD_STATE_STR_DECL(PMOD_STATE_INACTIVE),
	PMOD_STATE_STR_DECL(PMOD_STATE_DEAD),
	PMOD_STATE_STR_DECL(PMOD_STATE_LOADED),
	PMOD_STATE_STR_DECL(PMOD_STATE_RUNNING),
	PMOD_STATE_STR_DECL(PMOD_STATE_IN_INIT),
	PMOD_STATE_STR_DECL(PMOD_STATE_IN_FINI),
};

static const char	str_unavailable[] = " (unavailable)";

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pmodule_refptr_put(pfc_refptr_t **rpp)
 *	Put a refptr object in the buffer pointed by `rpp', and set NULL to
 *	the buffer.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pmodule_refptr_put(pfc_refptr_t **rpp)
{
	pfc_refptr_t	*rp = *rpp;

	if (rp != NULL) {
		pfc_refptr_put(rp);
		*rpp = NULL;
	}
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pmodule_conf_close(pmodule_t *mod)
 *	Close module configuration file handle associated with the given
 *	module.
 *
 * Remarks:
 *	This function must be called with holding module lock.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pmodule_conf_close(pmodule_t *mod)
{
	pfc_conf_t	conf = mod->pm_conf;

	if (conf != PFC_CONF_INVALID) {
		pfc_conf_close(conf);
		mod->pm_conf = PFC_CONF_INVALID;
	}
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pmodule_conf_destroy(pmodule_t *mod)
 *	Free up resources which keep module configuration file paths associated
 *	with the given module.
 *
 * Remarks:
 *	This function must be called with holding module lock.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pmodule_conf_destroy(pmodule_t *mod)
{
	pmodule_refptr_put(&mod->pm_cfpath);
	pmodule_refptr_put(&mod->pm_pubcfpath);
}

/*
 * static inline void
 * pmodule_state_change(pmodule_t *mod, pmod_state_t state)
 *	Change module state of the specified module.
 *
 * Remarks:
 *	The caller must call this function with holding module lock.
 */
static inline void
pmodule_state_change(pmodule_t *mod, pmod_state_t state)
{
	pmod_state_t	old = mod->pm_state;

	mod->pm_state = state;
	if (mod->pm_nwaiting != 0 && state != old) {
		PMODULE_BROADCAST(mod);
	}
}

/*
 * static inline void
 * pmodule_get(pmodule_t *mod)
 *	Acquire reference to the module.
 *
 * Remarks:
 *	The caller must call this function with holding module lock.
 */
static inline void
pmodule_get(pmodule_t *mod)
{
	mod->pm_refcnt++;
}

/*
 * static inline void
 * pmodule_put(pmodule_t *mod)
 *	Release reference to the module.
 *	If no one refers the module, module is unloaded.
 *
 * Remarks:
 *	The caller must hold module lock on calling.
 *	Note that it is released when a module library is unloaded, and
 *	it is acquired again on return.
 */
static inline void
pmodule_put(pmodule_t *mod)
{
	PFC_ASSERT(mod->pm_refcnt != 0);
	mod->pm_refcnt--;

	if (mod->pm_refcnt == 0) {
		/* Change the module state to INACTIVE. */
		pmodule_state_change(mod, PMOD_STATE_INACTIVE);
	}
}

/*
 * static inline pmodule_t *
 * pmodule_lookup(const char *name)
 *	Return module descriptor associated with the specified module name.
 *	NULL is returned if not found.
 *
 * Remarks:
 *	Unlike pmodule_hold(), this function doesn't update reference
 *	counter of the module descriptor.
 */
static inline pmodule_t *
pmodule_lookup(const char *name)
{
	pfc_rbnode_t	*node;
	pmodule_t	*mod;

	node = pfc_rbtree_get(&module_nametree, name);
	mod = (PFC_EXPECT_TRUE(node != NULL)) ? PMODULE_NODE2PTR(node) : NULL;

	return mod;
}

/*
 * static inline pmodule_t *
 * pmodule_lookup_byid(pfc_module_t module)
 *	Return module descriptor associated with the specified module ID.
 *	NULL is returned if not found.
 *
 * Remarks:
 *	Unlike pmodule_hold(), this function doesn't update reference
 *	counter of the module descriptor.
 */
static inline pmodule_t *
pmodule_lookup_byid(pfc_module_t module)
{
	if (PFC_EXPECT_FALSE(module == PFC_MODULE_INVALID ||
			     module >= module_count)) {
		return NULL;
	}

	return module_table + module;
}

/*
 * static inline int
 * pmodule_verify(pmodule_t *mod, pfc_bool_t loading)
 *	Verify that the module specified by the module descriptor can be
 *	changed its state.
 *
 *	`loading' must be true if the specified module is going to be loaded
 *	or started.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	If the module can't be changed its state, an error number which
 *	indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the module lock.
 */
static inline int
pmodule_verify(pmodule_t *mod, pfc_bool_t loading)
{
	pmod_state_t	state = mod->pm_state;
	uint32_t	flags = mod->pm_flags;

	if (loading) {
		if (PFC_EXPECT_FALSE(state == PMOD_STATE_DEAD)) {
			/* We can't change state of dead module. */
			return EBADF;
		}

		if (PFC_EXPECT_FALSE(flags & PMODF_SHUTDOWN)) {
			/* Module system has been shutdown. */
			return ESHUTDOWN;
		}
	}

	if (PFC_EXPECT_FALSE(flags & PMODF_CANCEL)) {
		/* Module operation has been canceled. */
		return ECANCELED;
	}

	if (PFC_EXPECT_FALSE(flags & PMODF_ERROR)) {
		/* Error state is not yet cleared. */
		return EBADF;
	}

	return 0;
}

/*
 * static inline int
 * pmodule_set_busy(pmodule_t *mod, pfc_bool_t loading)
 *	Ensure that the state of the specified module can be changed, and
 *	set busy flag to the specified module instance.
 *
 *	`loading' must be true if the specified module is going to be loaded
 *	or started.
 *
 * Calling/Exit State:
 *	Upon successful completion, busy flag is set to the module and
 *	zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The caller must call this function with holding module lock.
 */
static inline int
pmodule_set_busy(pmodule_t *mod, pfc_bool_t loading)
{
	uint32_t	flags;
	int	err = pmodule_verify(mod, loading);

	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	flags = mod->pm_flags;
	if (PFC_EXPECT_FALSE(flags & PMODF_BUSY)) {
		/* Another thread is changing state of this module. */
		return EBUSY;
	}

	mod->pm_flags = (flags | PMODF_BUSY);

	return 0;
}

/*
 * static inline void
 * pmodule_clear_busy(pmodule_t *mod)
 *	Clear busy flag in the specified module descriptor.
 *
 * Remarks:
 *	The caller must call this function with holding module lock.
 */
static inline void
pmodule_clear_busy(pmodule_t *mod)
{
	uint32_t	flags = mod->pm_flags;

	PFC_ASSERT(flags & PMODF_BUSY);
	mod->pm_flags = (flags & ~PMODF_BUSY);

	if (mod->pm_nwaiting != 0) {
		PMODULE_BROADCAST(mod);
	}
}

/*
 * static inline int
 * pmodule_event_clear_default(const char *name)
 *	Remove all module-specific event handlers associated with the domain
 *	specified by `name'.
 *
 *	This function always uses PMODULE_EVENT_RMHANDLER_TIMEOUT as timeout.
 */
static inline int
pmodule_event_clear_default(const char *name)
{
	pfc_timespec_t	timeout;

	timeout.tv_sec = PMODULE_EVENT_RMHANDLER_TIMEOUT;
	timeout.tv_nsec = 0;

	return pmodule_event_clear(name, &timeout);
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_module_bootstrap(void)
 *	Bootstrap code of the PFC module subsystem.
 */
void PFC_ATTR_HIDDEN
pfc_module_bootstrap(void)
{
	int	err;

	/* Create event source of module-specific event. */
	err = pfc_event_register(module_evsrc);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("Failed to create event source: %s",
			      strerror(err));
		/* NOTREACHED */
	}
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_module_fini(void)
 *	Finalize the PFC module subsystem.
 */
void PFC_ATTR_HIDDEN
pfc_module_fini(void)
{
	pmodule_t	*mod, *emod;
	pfc_timespec_t	timeout;
	int		err;

	/* Destroy module-specific event source. */
	timeout.tv_sec = PMODULE_EVENT_DESTROY_TIMEOUT;
	timeout.tv_nsec = 0;

	err = pfc_event_unregister(module_evsrc, &timeout);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to unregister event source: %s",
			      strerror(err));
		/* FALLTHROUGH */
	}

	/* Close module configuration files held by all modules. */
	emod = module_table + module_count;
	for (mod = module_table; mod < emod; mod++) {
		PMODULE_LOCK(mod);
		pmodule_conf_close(mod);
		pmodule_conf_destroy(mod);
		PMODULE_UNLOCK(mod);
	}
}

/*
 * void
 * pfc_module_init(const char *PFC_RESTRICT dir,
 *		   const char *PFC_RESTRICT confdir,
 *		   const uint8_t *PFC_RESTRICT addr, uint32_t size)
 *	Initialize PFC modules.
 *
 *	`dir' must be an absolute path to module directory.
 *	`confdir' must an absolute path to public module configuration file
 *	directory.
 *	`addr' and `size' must be an address and size of module cache contents
 *	obtained by the call of pfc_modcache_attach().
 */
void
pfc_module_init(const char *PFC_RESTRICT dir, const char *PFC_RESTRICT confdir,
		const uint8_t *PFC_RESTRICT addr, uint32_t size)
{
	modch_ctx_t	ctx;
	uint32_t	i, nmodules;
	pmodule_t	*mod;

	/* Set module directory. */
	PFC_ASSERT(dir != NULL);
	module_dir = dir;
	module_dir_len = strlen(dir);

	PFC_ASSERT(confdir != NULL);
	module_confdir = confdir;
	module_confdir_len = strlen(confdir);

	/* Initialize module cache reader context. */
	modch_ctx_init(&ctx, addr);

	nmodules = module_count = ctx.mx_nmodules;
	if (PFC_EXPECT_FALSE(nmodules == 0)) {
		/* No module is registered. */
		pfc_log_warn("No module is registered in the module cache.");
		module_table = NULL;

		return;
	}

	/* Allocate module table. */
	module_table = (pmodule_t *)malloc(sizeof(pmodule_t) * nmodules);
	if (PFC_EXPECT_FALSE(module_table == NULL)) {
		pfc_log_fatal("Failed to allocate module table: %u modules",
			      nmodules);
		/* NOTREACHED */
	}

	/* Initialize module descriptors for all registered modules. */
	for (i = 0, mod = module_table; i < nmodules; i++, mod++) {
		pmodule_ctor(&ctx, mod, i);
	}
}

/*
 * void
 * pfc_module_set_started(void)
 *	Set "started" flag which indicates all mandatory modules have been
 *	started.
 */
void
pfc_module_set_started(void)
{
	(void)pfc_atomic_swap_uint8(&module_started, PFC_TRUE);
}

/*
 * pfc_bool_t
 * pfc_module_is_started(void)
 *	Determine whether all mandatory modules are already started or not.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned only if all mandatory modules are already started.
 */
pfc_bool_t
pfc_module_is_started(void)
{
	return module_started;
}

/*
 * void
 * pfc_module_type_register(pfc_modtype_t type, const pfc_modops_t *ops)
 *	Register module operations for the specified module type.
 */
void
pfc_module_type_register(pfc_modtype_t type, const pfc_modops_t *ops)
{
	PFC_MODTYPE_ASSERT(type);
	PFC_ASSERT(module_ops[type] == NULL);

	module_ops[type] = ops;
}

/*
 * pfc_module_t
 * pfc_module_c_register(const pfc_modattr_t *PFC_RESTRICT mattr,
 *			 pfc_modfunc_t init, pfc_modfunc_t fini)
 *	Register a new PFC module whose type is PFC_MODTYPE_C.
 *	This function must be called once when the C module is going to be
 *	loaded into the PFC system.
 *
 * Calling/Exit State:
 *	Upon successful completion, a module ID associated with the specified
 *	module is returned. PFC_MODULE_INVALID is returned on failure.
 */
pfc_module_t
pfc_module_c_register(const pfc_modattr_t *PFC_RESTRICT mattr,
		      pfc_modfunc_t init, pfc_modfunc_t fini)
{
	pmodule_t	*mod;
	pfc_module_t	id;
	const char	*name = mattr->pma_name;
	int	err;

	PFC_ASSERT(init != NULL);
	PFC_ASSERT(fini != NULL);

	/* Search for a module descriptor associated with the module name. */
	mod = pmodule_lookup(name);
	if (PFC_EXPECT_FALSE(mod == NULL)) {
		return PFC_MODULE_INVALID;
	}

	PMODULE_LOCK(mod);

	if (PFC_EXPECT_FALSE(mod->pm_attr != NULL)) {
		/* Already registered. This should not happen. */
		PMODULE_UNLOCK(mod);

		return PFC_MODULE_INVALID;
	}

	/* Register this module. */
	mod->pm_attr = mattr;
	err = pmodule_register(name, mod, NULL);

	if (PFC_EXPECT_TRUE(err == 0)) {
		id = mod->pm_id;
		PMODULE_HANDLER_INIT(mod) = init;
		PMODULE_HANDLER_FINI(mod) = fini;
	}
	else {
		mod->pm_attr = NULL;
		id = PFC_MODULE_INVALID;
	}

	PMODULE_UNLOCK(mod);

	return id;
}

/*
 * pfc_module_t
 * pfc_module_register(const pfc_modattr_t *PFC_RESTRICT mattr,
 *		       pfc_cptr_t PFC_RESTRICT data)
 *	Register a new PFC module.
 *	This function must be called once when the module is going to be
 *	loaded into the PFC system. `data' is passed to the constructor
 *	defined by pfc_modops_t.
 *
 *	Currently, only libpfcxx uses this function to register C++ module.
 *
 * Calling/Exit State:
 *	Upon successful completion, a module ID associated with the specified
 *	module is returned. PFC_MODULE_INVALID is returned on failure.
 */
pfc_module_t
pfc_module_register(const pfc_modattr_t *PFC_RESTRICT mattr,
		    pfc_cptr_t PFC_RESTRICT data)
{
	pmodule_t	*mod;
	pfc_module_t	id;
	const char	*name = mattr->pma_name;
	int	err;

	/* Search for a module descriptor associated with the module name. */
	mod = pmodule_lookup(name);
	if (PFC_EXPECT_FALSE(mod == NULL)) {
		return PFC_MODULE_INVALID;
	}

	PMODULE_LOCK(mod);

	if (PFC_EXPECT_FALSE(mod->pm_attr != NULL)) {
		/* Already registered. This should not happen. */
		PMODULE_UNLOCK(mod);
		pfc_log_error("%s: Module attribute is not NULL.", name);

		return PFC_MODULE_INVALID;
	}

	/* Register this module. */
	mod->pm_attr = mattr;
	err = pmodule_register(name, mod, data);

	if (PFC_EXPECT_TRUE(err == 0)) {
		id = mod->pm_id;
	}
	else {
		id = PFC_MODULE_INVALID;
		mod->pm_attr = NULL;
	}

	PMODULE_UNLOCK(mod);

	return id;
}

/*
 * void
 * __pfc_module_bootstrap(const char *name, pfc_modboot_t func)
 *	Register module bootstrap function.
 *
 *	`name' must be the name of the target module.
 *	`func' must be a pointer to module bootstrap function.
 *	It  will be called just after the module specified by the name
 *	`name' is loaded.
 *
 *	Bootstrap function must return zero on successful return.
 *	If a non-zero value is returned, the module loading results in failure.
 */
void
__pfc_module_bootstrap(const char *name, pfc_modboot_t func)
{
	pmodule_t	*mod;

	/* Search for a module descriptor associated with the module name. */
	mod = pmodule_lookup(name);
	if (PFC_EXPECT_FALSE(mod == NULL)) {
		pfc_log_warn("%s: Attempt to register bootstrap function for "
			     "unknown module.", name);

		return;
	}

	PMODULE_LOCK(mod);
	pfc_log_verbose("%s: Register bootstrap function: %p", name, func);
	mod->pm_boot = func;
	PMODULE_UNLOCK(mod);
}

/*
 * pfc_ptr_t
 * pfc_module_call(const char *name, pmodcall_t handler, pfc_ptr_t arg)
 *	Retrieve module descriptor associated with the given module name,
 *	and call the specified callback with specifying module descriptor.
 *	The callback is called with holding the module lock.
 *
 * Calling/Exit State:
 *	A value returned by the callback is returned.
 *	If the specified module is not found, NULL is returned.
 */
pfc_ptr_t
pfc_module_call(const char *name, pmodcall_t handler, pfc_ptr_t arg)
{
	pmodule_t	*mod;
	pfc_ptr_t	ret;

	/* Search for a module descriptor associated with the module name. */
	mod = pmodule_lookup(name);
	if (PFC_EXPECT_FALSE(mod == NULL)) {
		/* Not found. */
		return NULL;
	}

	PMODULE_LOCK(mod);

	/* Call the specified callback. */
	ret = (*handler)(mod, arg);

	PMODULE_UNLOCK(mod);

	return ret;
}

/*
 * pmodule_t *
 * pfc_module_lookup(const char *name)
 *	Search for the module specified by the module name.
 *
 * Calling/Exit State:
 *	A pointer to module instance specified by the module name is returned.
 *	NULL is returned if not found.
 */
pmodule_t *
pfc_module_lookup(const char *name)
{
	return pmodule_lookup(name);
}

/*
 * uint32_t
 * pfc_module_count(void)
 *	Return the number of registered modules in the module cache.
 */
uint32_t
pfc_module_count(void)
{
	return module_count;
}

/*
 * pfc_bool_t
 * pfc_module_hold(pfc_module_t module)
 *	Hold the specified module to protect against module unloading
 *	temporarily.
 *
 * Calling/Exit State:
 *	Upon successful completion, PFC_TRUE is returned.
 *	PFC_FALSE is returned if the module could not be held.
 */
pfc_bool_t
pfc_module_hold(pfc_module_t module)
{
	return (PFC_EXPECT_TRUE(pmodule_hold(module) != NULL)) ? PFC_TRUE
		: PFC_FALSE;
}

/*
 * void
 * pfc_module_release(pfc_module_t module)
 *	Release the module.
 *	If no one holds the module, the module is unloaded.
 */
void
pfc_module_release(pfc_module_t module)
{
	pmodule_release(module);
}

/*
 * pfc_cfblk_t
 * __pfc_module_conf_getblock(pfc_module_t module, const char *bname)
 *	Return parameter block handle associated with the parameter block
 *	in the module configuration file. Block name is specified by `bname'.
 *
 * Calling/Exit State:
 *	A valid block handle is returned on success.
 *	On error, PFC_CFBLK_INVALID is returned.
 *
 * Remarks:
 *	This function is designed to be called only from the module
 *	associated with	`module'.
 */
pfc_cfblk_t
__pfc_module_conf_getblock(pfc_module_t module, const char *bname)
{
	pmodule_t	*mod = pmodule_lookup_byid(module);
	pfc_cfblk_t	block;

	PFC_ASSERT(mod != NULL);

	PMODULE_LOCK(mod);
	if (PFC_EXPECT_TRUE(PMODULE_IS_LOADED(mod) ||
			    PMODULE_IN_BOOTSTRAP(mod))) {
		block = pfc_conf_get_block(mod->pm_conf, bname);
	}
	else {
		/* This should not happen. */
		block = PFC_CFBLK_INVALID;
	}
	PMODULE_UNLOCK(mod);

	return block;
}

/*
 * pfc_cfblk_t
 * __pfc_module_conf_getmap(pfc_module_t module, const char *mname,
 *			    const char *key)
 *	Return parameter map block handle associated with the parameter map in
 *	the module configuration file. Map name is specified by `mname', and
 *	its key by `key'.
 *
 * Calling/Exit State:
 *	A valid block handle is returned on success.
 *	On error, PFC_CFBLK_INVALID is returned.
 *
 * Remarks:
 *	This function is designed to be called only from the module
 *	associated with	`module'.
 */
pfc_cfblk_t
__pfc_module_conf_getmap(pfc_module_t module, const char *mname,
			 const char *key)
{
	pmodule_t	*mod = pmodule_lookup_byid(module);
	pfc_cfblk_t	block;

	PFC_ASSERT(mod != NULL);

	PMODULE_LOCK(mod);
	if (PFC_EXPECT_TRUE(PMODULE_IS_LOADED(mod) ||
			    PMODULE_IN_BOOTSTRAP(mod))) {
		block = pfc_conf_get_map(mod->pm_conf, mname, key);
	}
	else {
		/* This should not happen. */
		block = PFC_CFBLK_INVALID;
	}
	PMODULE_UNLOCK(mod);

	return block;
}

/*
 * int
 * __pfc_module_conf_getmapkeys(pfc_module_t module, const char *mname,
 *				pfc_listm_t **keysp)
 *	Create a list model instance which contains all map keys of the
 *	parameter map specified by `mname' in the module configuration file.
 *
 * Calling/Exit State:
 *	Upon successful completion, the list model instance is set to the
 *	buffer pointed by `keysp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
__pfc_module_conf_getmapkeys(pfc_module_t module, const char *mname,
			     pfc_listm_t *keysp)
{
	pmodule_t	*mod = pmodule_lookup_byid(module);
	int		err;

	PFC_ASSERT(mod != NULL);

	PMODULE_LOCK(mod);
	if (PFC_EXPECT_TRUE(PMODULE_IS_LOADED(mod) ||
			    PMODULE_IN_BOOTSTRAP(mod))) {
		err = pfc_conf_get_mapkeys(mod->pm_conf, mname, keysp);
	}
	else {
		/* This should not happen. */
		err = ENODEV;
	}
	PMODULE_UNLOCK(mod);

	return err;
}

/*
 * int
 * __pfc_module_conf_reload(pfc_module_t module)
 *	Reload configuration file associated with the specified module.
 *
 * Calling/Exit State:
 *	Zero is returned on success.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function is designed to be called only from the module
 *	associated with	`module'.
 */
int
__pfc_module_conf_reload(pfc_module_t module)
{
	pmodule_t	*mod = pmodule_lookup_byid(module);
	int		err;

	PFC_ASSERT(mod != NULL);

	PMODULE_LOCK(mod);
	if (PFC_EXPECT_TRUE(PMODULE_IS_LOADED(mod))) {
		err = pmodule_conf_reload(mod);
	}
	else {
		/* This should not happen. */
		err = EBADF;
	}
	PMODULE_UNLOCK(mod);

	return err;
}

/*
 * pfc_bool_t
 * pfc_module_is_modevent(pfc_event_t event)
 *	Determine whether the given event is a module specific event or not.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned if the event specified by `event' is a module
 *	specific event. Otherwise PFC_FALSE is returned.
 */
pfc_bool_t
pfc_module_is_modevent(pfc_event_t event)
{
	event_t	*ev = PFC_EVENT_PTR(event);

	if (PFC_EXPECT_FALSE(ev == NULL)) {
		return PFC_FALSE;
	}

	return (ev->e_flags & EVF_MODEVENT) ? PFC_TRUE : PFC_FALSE;
}

/*
 * const char *
 * pfc_module_event_getsender(pfc_event_t event)
 *	Return a string which represents the sender of the given event.
 *
 * Calling/Exit State:
 *	If the given event is sent by an external process, NULL is returned.
 *	Otherwise the name of module who sends the given event is returned.
 *
 * Remarks:
 *	`event' must be a module-specific event.
 *	Specifying value that is not module-specific event results undefined
 *	behavior.
 */
const char *
pfc_module_event_getsender(pfc_event_t event)
{
	pfc_refptr_t	*name = PFC_EVENT_DATA(event, pfc_refptr_t *);

	PFC_ASSERT(pfc_module_is_modevent(event));

	if (name == NULL) {
		return NULL;
	}

	return pfc_refptr_string_value(name);
}

/*
 * int
 * __pfc_module_event_add(pfc_module_t module,
 *			  pfc_evhandler_t *PFC_RESTRICT idp,
 *			  pfc_evfunc_t handler, pfc_ptr_t PFC_RESTRICT arg,
 *			  const pfc_evmask_t *PFC_RESTRICT evmask,
 *			  uint32_t priority)
 *	Register module-specific event handler for the module specified by
 *	`module'.
 *	`hname' is a user-defined name of event handler.
 *	Currently, it is used only for event delivery logging.
 *	If NULL is specified, module name is used as handler's name.
 *
 * Calling/Exit State:
 *	Upon successful completion, event handler ID is set to `*idp',
 *	and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
__pfc_module_event_add(pfc_module_t module, pfc_evhandler_t *PFC_RESTRICT idp,
		       pfc_evfunc_t handler, pfc_ptr_t PFC_RESTRICT arg,
		       const pfc_evmask_t *PFC_RESTRICT evmask,
		       uint32_t priority, const char *PFC_RESTRICT hname)
{
	pmodule_t	*mod = pmodule_lookup_byid(module);
	pfc_refptr_t	*rhname, *rput;
	int		err;

	if (PFC_EXPECT_FALSE(mod == NULL)) {
		/* This should never happen. */
		return EBADF;
	}

	if (hname == NULL || *hname == '\0') {
		/* Use module name as handler's name. */
		rhname = mod->pm_name;
		rput = NULL;
	}
	else {
		/* Copy user-defined handler's name. */
		rhname = pfc_refptr_string_create(hname);
		if (PFC_EXPECT_FALSE(rhname == NULL)) {
			return ENOMEM;
		}
		rput = rhname;
	}

	/* Use module name as event delivery domain. */
	err = pmodule_event_add(idp, mod->pm_name, handler, arg, evmask,
				priority, rhname);
	if (rput != NULL) {
		pfc_refptr_put(rput);
	}

	return err;
}

/*
 * int
 * __pfc_module_event_post(pfc_module_t module, const char *target,
 *			   pfc_evtype_t type)
 *	Post a module-specific event to the module specified by `target'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
__pfc_module_event_post(pfc_module_t module, const char *target,
			pfc_evtype_t type)
{
	pmodule_t	*mod = pmodule_lookup_byid(module);
	pfc_refptr_t	*name;

	if (PFC_EXPECT_FALSE(mod == NULL)) {
		/* This should never happen. */
		return EBADF;
	}

	if (PFC_EXPECT_FALSE(target == NULL)) {
		/* Broadcasting an event is forbidden. */
		return EINVAL;
	}

	name = mod->pm_name;
	pfc_refptr_get(name);

	return pmodule_event_post(name, target, type);
}

/*
 * int
 * __pfc_module_ipcevent_addhdlr(pfc_module_t module,
 *				 pfc_ipcevhdlr_t *PFC_RESTRICT idp,
 *				 const char *PFC_RESTRICT channel,
 *				 pfc_ipcevfunc_t func,
 *				 const pfc_ipcevattr_t *PFC_RESTRICT attr,
 *				 const char *PFC_RESTRICT name)
 *	Add an IPC event handler which receives events generated by the IPC
 *	channel specified by `channel'.
 *
 *	See comments on pfc_module_ipcevent_add_handler() in pfc/module.h
 *	for more details.
 *
 * Calling/Exit State:
 *	Upon successful completion, an identifier of the added event handler
 *	is set to the buffer pointed by `idp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
__pfc_module_ipcevent_addhdlr(pfc_module_t module,
			      pfc_ipcevhdlr_t *PFC_RESTRICT idp,
			      const char *PFC_RESTRICT channel,
			      pfc_ipcevfunc_t func,
			      const pfc_ipcevattr_t *PFC_RESTRICT attr,
			      const char *PFC_RESTRICT name)
{
	pmodule_t	*mod = pmodule_lookup_byid(module);
	mod_ipcevhdlr_t	*mhp;
	pmod_state_t	state;
	pfc_bool_t	call_dtor = PFC_TRUE;
	int		err;

	if (PFC_EXPECT_FALSE(mod == NULL)) {
		/* This should never happen. */
		err = EBADF;
		goto error;
	}

	if (name == NULL) {
		/* Use module name as handler name. */
		name = PMODULE_NAME(mod);
	}

	/* Allocate a new IPC event handler entry. */
	mhp = (mod_ipcevhdlr_t *)malloc(sizeof(*mhp));
	if (PFC_EXPECT_FALSE(mhp == NULL)) {
		err = ENOMEM;
		goto error;
	}

	/* Check the module state. */
	PMODULE_LOCK(mod);
	state = mod->pm_state;
	if (PFC_EXPECT_FALSE(state != PMOD_STATE_IN_INIT &&
			     state != PMOD_STATE_RUNNING)) {
		err = EPERM;
		goto error_unlock;
	}

	/*
	 * Register the specified IPC event handler to the IPC event
	 * subsystem.
	 *
	 * Remarks:
	 *	`idp' specified by the caller must be passed to directly,
	 *	or C++ interface will not work.
	 */
	call_dtor = PFC_FALSE;
	err = pfc_ipcevent_add_handler(idp, channel, func, attr, name);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_unlock;
	}

	/* Preserve this handler's ID in the IPC handler tree. */
	mhp->ieh_id = *idp;
	PFC_ASSERT_INT(pfc_rbtree_put(&mod->pm_ipcevhdlr, &mhp->ieh_node), 0);
	PMODULE_UNLOCK(mod);

	return 0;

error_unlock:
	PMODULE_UNLOCK(mod);
	free(mhp);

error:
	if (call_dtor) {
		/*
		 * Call pfc_ipcevent_add_handler() with invalid parameter,
		 * just in order to call argument destructor.
		 */
		PFC_ASSERT_INT(pfc_ipcevent_add_handler(NULL, NULL, NULL,
							attr, NULL), EINVAL);
	}

	return err;
}

/*
 * int
 * __pfc_module_ipcevent_rmhdlr(pfc_module_t module, pfc_ipcevhdlr_t id)
 *	Remove the IPC event handler associated with the specified ID.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
__pfc_module_ipcevent_rmhdlr(pfc_module_t module, pfc_ipcevhdlr_t id)
{
	pmodule_t	*mod = pmodule_lookup_byid(module);
	mod_ipcevhdlr_t	*mhp;
	pfc_rbnode_t	*node;

	if (PFC_EXPECT_FALSE(mod == NULL)) {
		/* This should never happen. */
		return EBADF;
	}

	/* Remove the specified ID from the IPC event handler tree. */
	PMODULE_LOCK(mod);
	node = pfc_rbtree_remove(&mod->pm_ipcevhdlr,
				 PMODULE_IPCEVHDLR_KEY(id));
	PMODULE_UNLOCK(mod);

	if (PFC_EXPECT_FALSE(node == NULL)) {
		return ENOENT;
	}

	mhp = PMODULE_IPCEVHDLR_NODE2PTR(node);
	PFC_ASSERT(mhp->ieh_id == id);
	free(mhp);

	/* Remove this handler from the IPC event subsystem. */
	return pfc_ipcevent_remove_handler(id);
}

/*
 * int
 * __pfc_modevent_add(pfc_evhandler_t *PFC_RESTRICT idp,
 *		      const char *PFC_RESTRICT name, pfc_evfunc_t handler,
 *		      pfc_ptr_t PFC_RESTRICT arg,
 *		      const pfc_evmask_t *PFC_RESTRICT evmask,
 *		      uint32_t priority)
 *	Register module-specific event handler for PFC core component
 *	specified by `name'.
 *
 *	`name' must be a non-NULL pointer to the string which represents
 *	name of component, and it must starts with PFC_MODEVENT_CORE_PREFIX.
 *
 *	`hname' is a user-defined name of event handler.
 *	Currently, it is used only for event delivery logging.
 *	If NULL is specified, domain name `name' is used as handler's name.
 *
 * Calling/Exit State:
 *	Upon successful completion, event handler ID is set to `*idp',
 *	and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function is designed to be called by pfc_modevent_add().
 *	Never call this function directly.
 */
int
__pfc_modevent_add(pfc_evhandler_t *PFC_RESTRICT idp,
		   const char *PFC_RESTRICT name, pfc_evfunc_t handler,
		   pfc_ptr_t PFC_RESTRICT arg,
		   const pfc_evmask_t *PFC_RESTRICT evmask, uint32_t priority,
		   const char *PFC_RESTRICT hname)
{
	pfc_refptr_t	*domain, *rhname;
	int		err;

	PFC_ASSERT(PFC_MODULE_IS_CORE(name));

	domain = pfc_refptr_string_create(name);
	if (PFC_EXPECT_FALSE(domain == NULL)) {
		return ENOMEM;
	}

	if (hname == NULL || *hname == '\0') {
		/* Domain name will be used as handler's name. */
		rhname = NULL;
	}
	else {
		/* Copy user-defined handler's name. */
		rhname = pfc_refptr_string_create(hname);
		if (PFC_EXPECT_FALSE(rhname == NULL)) {
			err = ENOMEM;
			goto out;
		}
	}

	err = pmodule_event_add(idp, domain, handler, arg, evmask, priority,
				rhname);
	if (rhname != NULL) {
		pfc_refptr_put(rhname);
	}

out:
	pfc_refptr_put(domain);

	return err;
}

/*
 * int
 * __pfc_modevent_post(const char *name, const char *target, pfc_evtype_t type)
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
 *	This function is designed to be called by pfc_modevent_post().
 *	Never call this function directly.
 */
int
__pfc_modevent_post(const char *name, const char *target, pfc_evtype_t type)
{
	pfc_refptr_t	*domain;

	PFC_ASSERT(PFC_MODULE_IS_CORE(name));

	domain = pfc_refptr_string_create(name);
	if (PFC_EXPECT_FALSE(domain == NULL)) {
		return ENOMEM;
	}

	return pmodule_event_post(domain, target, type);
}

/*
 * int
 * __pfc_modevent_clear(const char *PFC_RESTRICT name,
 *			const pfc_timespec_t *PFC_RESTRICT timeout)
 *	Remove all module-specific event handlers associated with the domain
 *	specified by `name'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- The event handler may not be able to reuse even if this function
 *	  returns error, except for EINVAL.
 *
 *	- This function returns zero if no handler associated with the given
 *	  name is found.
 */
int
__pfc_modevent_clear(const char *PFC_RESTRICT name,
		     const pfc_timespec_t *PFC_RESTRICT timeout)
{
	PFC_ASSERT(PFC_MODULE_IS_CORE(name));

	return pmodule_event_clear(name, timeout);
}

/*
 * int
 * pfc_modevent_post_ex(const char *target, pfc_evtype_t type)
 *	Post a module-specific event to the module specified by `target'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function is used to generate module-specific event requested by
 *	an external process.
 */
int
pfc_modevent_post_ex(const char *target, pfc_evtype_t type)
{
	return pmodule_event_post(NULL, target, type);
}

/*
 * int
 * pfc_module_set_mandatory(const char *name, pfc_bool_t mandatory)
 *	Set mandatory flag to the module specified by module name.
 *	If `mandatory' is PFC_TRUE, mandatory flag is set to the module.
 *	If PFC_FALSE, mandatory flag is cleared.
 *
 *	If `mandatory' is PFC_TRUE, mandatory flags for all modules in
 *	the dependency list of the specified module are also turned on.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	ENOENT is returned if the module specified by name is not found.
 */
int
pfc_module_set_mandatory(const char *name, pfc_bool_t mandatory)
{
	pmodule_t	*mod = pmodule_lookup(name);

	if (PFC_EXPECT_FALSE(mod == NULL)) {
		return ENOENT;
	}

	pmodule_set_mandatory(mod, mandatory);

	return 0;
}

/*
 * void
 * pfc_module_set_mandatory_all(pfc_bool_t mandatory)
 *	Set mandatory flag to all registered modules.
 *	If `mandatory' is PFC_TRUE, mandatory flag is set, and cleared if
 *	PFC_FALSE.
 */
void
pfc_module_set_mandatory_all(pfc_bool_t mandatory)
{
	pmodule_t	*mod, *emod;

	emod = module_table + module_count;
	for (mod = module_table; mod < emod; mod++) {
		PMODULE_LOCK(mod);
		if (mandatory) {
			mod->pm_flags |= PMODF_MANDATORY;
		}
		else {
			mod->pm_flags &= ~PMODF_MANDATORY;
		}
		PMODULE_UNLOCK(mod);
	}
}

/*
 * int
 * pfc_module_select(pfc_listm_t list, pmod_selector_t selector,
 *		     pfc_bool_t reverse)
 *	Put all module descriptors which are selected by selector function
 *	to the specified list.
 *
 *	`selector' is a pointer to selector function. The selector will be
 *	called for each module descriptor. If the selector returns PFC_TRUE,
 *	the module descriptor will be put to the list. The error state of
 *	selected modules are all cleared.
 *
 *	`reverse' determines the order of module descriptors in the list.
 *	If `reverse' is PFC_FALSE, modules in the list are sorted in order of
 *	module load priority. If PFC_TRUE, module descriptors are sorted in
 *	reverse order.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *	Note that pfc_module_select() doesn't rollback changes to the list
 *	even on error.
 *
 * Remarks:
 *	Selector is called with holding the module lock.
 */
int
pfc_module_select(pfc_listm_t list, pmod_selector_t selector,
		  pfc_bool_t reverse)
{
	pmodule_t	*mod, *emod;

	emod = module_table + module_count;
	for (mod = module_table; mod < emod; mod++) {
		pfc_cptr_t	value;
		int	err;

		PMODULE_LOCK(mod);

		if (!(*selector)(mod)) {
			PMODULE_UNLOCK(mod);
			continue;
		}

		value = (pfc_cptr_t)mod;

		/* Clear error state. */
		mod->pm_flags &= ~(PMODF_ERROR | PMODF_CANCEL);
		PMODULE_UNLOCK(mod);

		if (reverse) {
			/* Push this module to the head of the list. */
			err = pfc_listm_push(list, value);
		}
		else {
			/* Push this module to the tail of the list. */
			err = pfc_listm_push_tail(list, value);
		}

		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}

	return 0;
}

/*
 * pfc_bool_t
 * pfc_module_select_all(pmodule_t *mod)
 *	Selector function which selects all modules.
 *
 * Remarks:
 *	This function must be called with holding the module lock.
 */
pfc_bool_t
pfc_module_select_all(pmodule_t *mod)
{
	return PFC_TRUE;
}

/*
 * pfc_bool_t
 * pfc_module_select_mandatory(pmodule_t *mod)
 *	Selector function which selects mandatory module.
 *
 * Remarks:
 *	This function must be called with holding the module lock.
 */
pfc_bool_t
pfc_module_select_mandatory(pmodule_t *mod)
{
	return (mod->pm_flags & PMODF_MANDATORY) ? PFC_TRUE : PFC_FALSE;
}

/*
 * int
 * pfc_module_iterate(pmod_iter_t iter, pfc_ptr_t arg)
 *	Iterate all registered modules.
 *
 *	`iter' is a pointer to iterator function. The iterator will be
 *	called for each module descriptor. The iteration continues until the
 *	iterator function returns non-zero value.
 *
 * Calling/Exit State:
 *	If the iterator returns non-zero value for at least one module,
 *	iterator's return value is returned.
 *	Zero is returned if the iterator returns zero for all modules.
 *
 * Remarks:
 *	The iterator is called with holding the module lock.
 */
int
pfc_module_iterate(pmod_iter_t iter, pfc_ptr_t arg)
{
	pmodule_t	*mod, *emod;
	int		err = 0;

	emod = module_table + module_count;
	for (mod = module_table; mod < emod; mod++) {
		PMODULE_LOCK(mod);
		err = (*iter)(mod, arg);
		PMODULE_UNLOCK(mod);
		if (err != 0) {
			break;
		}
	}

	return err;
}

/*
 * int
 * pfc_module_set_state(pmodule_t *mod, pmod_state_t state)
 *	Change the module state to the specified state.
 *	`state' is the target module state. Below are available target states:
 *
 *	PMOD_STATE_LOADED:	Load module and its configuration file.
 *	PMOD_STATE_RUNNING:	Load and start module.
 *	PMOD_STATE_UNLOADED:	Stop and unload module.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_module_set_state(pmodule_t *mod, pmod_state_t state)
{
	const pmod_action_t	*action, *eact;

	/* Search for a handler to change the module state. */
	eact = PFC_ARRAY_LIMIT(module_state_actions);
	for (action = module_state_actions; action < eact; action++) {
		if (action->pa_state == state) {
			return action->pa_handler(mod);
		}
	}

	return EINVAL;
}

/*
 * int
 * pfc_module_shutdown(pfc_listm_t list)
 *	Select modules to be unloaded on the system shutdown.
 *	Module descriptors associated with loaded modules are pushed to
 *	the `list' in order of unloading.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_module_shutdown(pfc_listm_t list)
{
	pmodule_t	*mod, *emod;

	emod = module_table + module_count;
	for (mod = module_table; mod < emod; mod++) {
		uint32_t	flags;
		pfc_bool_t	loaded;
		int	err;

		PMODULE_LOCK(mod);
		flags = mod->pm_flags;

		/* Set shutdown flag to all modules. */
		flags |= PMODF_SHUTDOWN;

		/* Clear error state. */
		flags &= ~(PMODF_ERROR | PMODF_CANCEL);

		mod->pm_flags = flags;
		loaded = (PMODULE_IS_LOADED(mod)) ? PFC_TRUE : PFC_FALSE;
		PMODULE_UNLOCK(mod);

		/* Push loaded modules to the list in reverse order. */
		if (loaded) {
			err = pfc_listm_push(list, (pfc_cptr_t)mod);
			if (PFC_EXPECT_FALSE(err != 0)) {
				return err;
			}
		}
	}

	return 0;
}

/*
 * void
 * pfc_module_cancel(void)
 *	Cancel all module operations currently running.
 *	Canceled module operation will return ECANCELED.
 */
void
pfc_module_cancel(void)
{
	pmodule_t	*mod, *emod;

	emod = module_table + module_count;
	for (mod = module_table; mod < emod; mod++) {
		/*
		 * Set the cancellation flag for all module descriptors,
		 * and wake up all waiter threads.
		 *
		 * We can set the cancellation flag irrespective of whether
		 * module operation is active because the cancellation flag
		 * will be always cleared on module selection.
		 */
		PMODULE_LOCK(mod);
		PMODULE_SET_CANCEL(mod);
		PMODULE_UNLOCK(mod);
	}
}

/*
 * int
 * __pfc_module_addhook(pfc_module_t module, pfc_modhook_t hook, uint32_t pri)
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
int
__pfc_module_addhook(pfc_module_t module, pfc_modhook_t hook, uint32_t pri)
{
	pmodule_t	*mod = pmodule_lookup_byid(module);
	mod_hooklist_t	*hlp = &module_unload_hooks;
	mod_hook_t	*hep;

	if (PFC_EXPECT_FALSE(mod == NULL)) {
		/* This should never happen. */
		return EBADF;
	}

	if (PFC_EXPECT_FALSE(hook == NULL)) {
		return EINVAL;
	}

	/* Allocate a new hook entry. */
	hep = (mod_hook_t *)malloc(sizeof(*hep));
	if (PFC_EXPECT_FALSE(hep == NULL)) {
		return ENOMEM;
	}

	hep->mhe_module = module;
	hep->mhe_priority = pri;
	hep->mhe_hook = hook;

	/* Link a new hook entry to the global hook list. */
	MOD_HOOKLIST_LOCK(hlp);
	pmodule_addhook(&hlp->mhl_head, hep);
	MOD_HOOKLIST_UNLOCK(hlp);

	return 0;
}

/*
 * void
 * pfc_module_runhooks(void)
 *	Run all module unload hooks, and invalidate them.
 *
 *	This function is called by the PFC daemon when it enters the shutdown
 *	sequence.
 *
 * Remarks:
 *	This function expects to be called by the PFC daemon on system
 *	shutdown.
 */
void
pfc_module_runhooks(void)
{
	pmodule_runhooks(NULL, PFC_TRUE);
}

/*
 * static int
 * pfc_module_c_init(pmodule_t *mod)
 *	Call initializer of the specified module.
 *	The module type must be PFC_MODTYPE_C.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	-1 is returned if the initializer returns PFC_FALSE.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
pfc_module_c_init(pmodule_t *mod)
{
	pfc_modfunc_t	init = PMODULE_HANDLER_INIT(mod);

	return (PFC_EXPECT_TRUE((*init)())) ? 0 : -1;
}

/*
 * static pfc_bool_t
 * pfc_module_c_fini(pmodule_t *mod)
 *	Call finalizer of the specified module.
 *	The module type must be PFC_MODTYPE_C.
 */
static pfc_bool_t
pfc_module_c_fini(pmodule_t *mod)
{
	pfc_modfunc_t	fini = PMODULE_HANDLER_FINI(mod);

	return (*fini)();
}

/*
 * static int
 * pfc_module_c_ipc_register(pmodule_t *mod)
 *	Register IPC service handler of the module.
 *	The module type must be PFC_MODTYPE_C.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
pfc_module_c_ipc_register(pmodule_t *mod)
{
	const pfc_modattr_t	*mattr = mod->pm_attr;
	int	err;

	if (!PMODULE_HAS_IPC(mattr)) {
		pfc_log_debug("%s: IPC service is not provided.",
			      PMODULE_NAME(mod));

		return 0;
	}

	if (PFC_EXPECT_FALSE(mattr->pma_ipchdlr == NULL)) {
		pfc_log_warn("%s: IPC service handler is NULL.",
			     PMODULE_NAME(mod));

		return EINVAL;
	}

	err = pfc_ipcsrv_add_handler_impl(mod->pm_name, mattr->pma_nipcs,
					  pmodule_ipc_handler,
					  (pfc_ptr_t)mattr, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("%s: Failed to register IPC service: %s",
			      PMODULE_NAME(mod), strerror(err));

		return err;
	}

	pfc_log_debug("%s: IPC service has been registered: nservices=%u",
		      PMODULE_NAME(mod), mattr->pma_nipcs);

	return 0;
}

/*
 * static void
 * pmodule_ctor(modch_ctx_t *PFC_RESTRICT ctx, pmodule_t *PFC_RESTRICT mod,
 *		uint32_t index)
 *	Constructor of module descriptor.
 *	Fetch the current module cache data from the cache reader context,
 *	and initialize module descriptor using it.
 */
static void
pmodule_ctor(modch_ctx_t *PFC_RESTRICT ctx, pmodule_t *PFC_RESTRICT mod,
	     uint32_t index)
{
	modch_cent_t	*mep = ctx->mx_module;
	modch_cdep_t	*depend = ctx->mx_depend, *rdepend = ctx->mx_rdepend;
	const char	*name = modch_string(ctx->mx_head, mep->mce_name);
	pfc_refptr_t	*rname;
	int	err;

	/* Create refptr string object which contains module name. */
	rname = pfc_refptr_string_create(name);
	if (PFC_EXPECT_FALSE(rname == NULL)) {
		pfc_log_fatal("%s: Failed to allocate buffer for module name.",
			      name);
		/* NOTREACHED */
	}

	/* Create module dependency link. */
	pmodule_link_depends(depend, &mod->pm_depends, &mod->pm_ndeps);

	/* Create reverse module dependency link. */
	pmodule_link_depends(rdepend, &mod->pm_rdepends, &mod->pm_nrdeps);

	PFC_ASSERT(index == (uint32_t)(mod - module_table));
	mod->pm_id = index;
	mod->pm_name = rname;
	mod->pm_boot = NULL;
	mod->pm_attr = NULL;
	mod->pm_handle = NULL;
	mod->pm_refcnt = 0;
	mod->pm_state = PMOD_STATE_UNLOADED;
	mod->pm_flags = 0;
	mod->pm_nwaiting = 0;
	mod->pm_conf = PFC_CONF_INVALID;
	mod->pm_cfpath = NULL;
	mod->pm_pubcfpath = NULL;
	PMODULE_OBJECT_TNAME(mod) = NULL;
	PMODULE_OBJECT_PTR(mod) = NULL;

	err = PFC_MUTEX_INIT(&mod->pm_mutex);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("%s: Failed to initialize module mutex: %s",
			      PMODULE_NAME(mod), strerror(err));
		/* NOTREACHED */
	}

	err = pfc_cond_init(&mod->pm_cond);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("%s: Failed to initialize condvar: %s",
			      PMODULE_NAME(mod), strerror(err));
		/* NOTREACHED */
	}

	/* Register a pair of module name and module descriptor. */
	err = pfc_rbtree_put(&module_nametree, &mod->pm_node);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("%s: Failed to register module descriptor: %s",
			      PMODULE_NAME(mod), strerror(err));
		/* NOTREACHED */
	}

	/* Hold module name while it is in module_nametree. */
	pfc_refptr_get(rname);

	/* Update cache reader context. */
	ctx->mx_module = mep + 1;
	ctx->mx_depend = depend + mod->pm_ndeps + 1;
	ctx->mx_rdepend = rdepend + mod->pm_nrdeps + 1;

	/* Initialize Red-Black tree for IPC event handlers. */
	PMODULE_IPCEVHDLR_INIT(mod);

	pfc_log_verbose("%u: Module descriptor has been created: %s",
			mod->pm_id, PMODULE_NAME(mod));
}

/*
 * static void
 * pmodule_link_depends(modch_cdep_t *PFC_RESTRICT depends,
 *			pmod_dep_t **PFC_RESTRICT headp,
 *			uint16_t *PFC_RESTRICT nump)
 *	Construct module dependency link.
 *	Created link elements are set to `*headp', and number of elements are
 *	set to `*nump'.
 */
static void
pmodule_link_depends(modch_cdep_t *PFC_RESTRICT depends,
		     pmod_dep_t **PFC_RESTRICT headp,
		     uint16_t *PFC_RESTRICT nump)
{
	modch_cdep_t	*dep;
	uint16_t	ndeps;

	/* Determine number of dependency entries. */
	for (dep = depends; dep->mcd_module != MODCH_DEP_NONE; dep++);
	PFC_ASSERT((uint32_t)(dep - depends) < PFC_MODULE_MAX);
	ndeps = (uint16_t)(dep - depends);

	if (ndeps == 0) {
		/* No dependency entries. */
		*headp = NULL;
	}
	else {
		pmod_dep_t	*pdeps;

		/* Allocate module dependency objects. */
		pdeps = (pmod_dep_t *)malloc(sizeof(*pdeps) * ndeps);
		if (PFC_EXPECT_FALSE(pdeps == NULL)) {
			pfc_log_fatal("Failed to allocate module dependency "
				      "entries.");
			/* NOTREACHED */
		}
		*headp = pdeps;

		for (dep = depends; dep < depends + ndeps; dep++, pdeps++) {
			pdeps->pd_module = dep->mcd_module;
			pdeps->pd_version = dep->mcd_version;
		}
		PFC_ASSERT(dep->mcd_module == MODCH_DEP_NONE);
	}

	*nump = ndeps;
}

/*
 * static pfc_ipcresp_t
 * pmodule_ipc_handler(pfc_ipcsrv_t *srv, pfc_ipcid_t service, pfc_ptr_t arg)
 *	IPC service handler of C module.
 *
 * Calling/Exit State:
 *	This function returns the response of the IPC service handler of
 *	this module.
 */
static pfc_ipcresp_t
pmodule_ipc_handler(pfc_ipcsrv_t *srv, pfc_ipcid_t service, pfc_ptr_t arg)
{
	const pfc_modattr_t	*mattr = (const pfc_modattr_t *)arg;

	return mattr->pma_ipchdlr(srv, service);
}

/*
 * static char *
 * pmodule_library_path(pmodule_t *mod)
 *	Construct library path associated with the specified module instance.
 *
 *	This function allocates PMODULE_PATH_MAX bytes of buffer to store
 *	an absolute path to the library, and a pointer to the buffer is
 *	returned.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer which contains absolute
 *	path to the module library file is returned.
 *	NULL is returned on failure.
 *
 * Remarks:
 *	The caller is responsible for releasing buffer using free().
 */
static char *
pmodule_library_path(pmodule_t *mod)
{
	const char	*name = PMODULE_NAME(mod);
	char		*path;
	const size_t	len = PMODULE_PATH_MAX;

	path = (char *)malloc(len);
	if (PFC_EXPECT_FALSE(path == NULL)) {
		pfc_log_error("%s: Failed to allocate module path buffer.",
			      name);

		return NULL;
	}

	snprintf(path, len, "%s/%s." PFC_MODULE_LIB_SUFFIX, module_dir, name);

	return path;
}

/*
 * static int
 * pmodule_close_library(pmodule_t *mod, void *handle)
 *	Close the module library handle.
 *	`handle' must be a library handle associated with the `mod', created
 *	by the call of dlopen().
 *
 * Calling/Exit State:
 *	Zero is returned if the library was actually closed.
 *	EBUSY is returned if dlclose() did not close the library.
 *	EIO is returned if dlclose() failed.
 *	ENOMEM is returned if no sufficient memory.
 *
 * Remarks:
 *	The caller must call this function with holding the module lock.
 *	Note that this function releases the lock, and acquires again on
 *	return.
 */
static int
pmodule_close_library(pmodule_t *mod, void *handle)
{
	const pfc_modattr_t	*mattr = mod->pm_attr;
	int	err = 0, ret;
	char	*path;
	void	*h;

	/*
	 * Module attribute pointer must be invalidated before unloading
	 * library file.
	 */
	mod->pm_attr = NULL;

	PMODULE_UNLOCK(mod);

	/* Prepare library path before dlclose(). */
	path = pmodule_library_path(mod);
	if (PFC_EXPECT_FALSE(path == NULL)) {
		err = ENOMEM;
		goto lock_again;
	}

	pfc_mutex_lock(&module_load_lock);

	/* Try to close library handle. */
	ret = dlclose(handle);
	if (PFC_EXPECT_FALSE(ret != 0)) {
		const char	*name = PMODULE_NAME(mod);
		char		*dlerr = dlerror();

		pfc_log_error("%s: dlclose() failed.", name);
		if (dlerr != NULL) {
			pfc_log_error("%s: dlerror(): %s", name, dlerr);
		}

		err = EIO;
		goto out;
	}

	/*
	 * The NODELETE flag may be set to the library handle by runtime
	 * linker. So we must ensure that the library is unloaded.
	 */
	h = dlopen(path, RTLD_GLOBAL | RTLD_NOW | RTLD_NOLOAD);
	if (PFC_EXPECT_FALSE(h != NULL)) {
		/* dlclose() did not unload the library. */
		pfc_log_warn("%s: dlclose() did not close the library.",
			     PMODULE_NAME(mod));
		(void)dlclose(h);
		PFC_ASSERT(h == handle);
		err = EBUSY;
	}

out:
	pfc_mutex_unlock(&module_load_lock);
	free(path);

lock_again:
	PMODULE_LOCK(mod);

	if (PFC_EXPECT_FALSE(err != 0)) {
		/* Restore module attribute pointer. */
		mod->pm_attr = mattr;
	}

	return err;
}

/*
 * static int
 * pmodule_register(const char *PFC_RESTRICT name, pmodule_t *PFC_RESTRICT mod,
 *		    pfc_cptr_t PFC_RESTRICT data)
 *	Register the given module descriptor.
 *
 * Remarks:
 *	This function must be called with holding the module lock.
 */
static int
pmodule_register(const char *PFC_RESTRICT name, pmodule_t *PFC_RESTRICT mod,
		 pfc_cptr_t PFC_RESTRICT data)
{
	const pfc_modops_t	*ops;
	const pfc_modattr_t	*mattr = mod->pm_attr;
	pfc_modtype_t	type = mattr->pma_type;
	uint8_t		sysversion = mattr->pma_sysversion;
	int		err;

	if (PFC_EXPECT_FALSE(sysversion  == 0 ||
			     sysversion > PFC_MODULE_SYSTEM_VERSION)) {
		pfc_log_error("%s: Module system version mismatch: %u, %u",
			      name, sysversion, PFC_MODULE_SYSTEM_VERSION);

		return EINVAL;
	}

	/* Call constructor defined by module operation. */
	PFC_MODTYPE_ASSERT(type);
	ops = module_ops[type];
	if (ops->ctor != NULL) {
		err = ops->ctor(mod, data);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_error("%s:%u: Module constructor failed: %s",
				      name, mod->pm_id, strerror(err));

			return err;
		}
	}

	if (PFC_EXPECT_FALSE(sysversion != PFC_MODULE_SYSTEM_VERSION)) {
		pfc_log_warn("%s: Old fashioned module: sysver=%u",
			     name, sysversion);
	}

	pfc_log_verbose("%s: Module has been registered.", name);

	return 0;
}

/*
 * static pmodule_t *
 * pmodule_hold(pfc_module_t module)
 *	Hold the specified module to protect against module unloading
 *	temporarily.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non NULL pointer to module instance
 *	associated with the specified module ID is returned. The returned
 *	module and all modules which depend on the specified module are held
 *	by pmodule_get(). The caller is responsible for release.
 *
 *	NULL is returned if the specified module could not be held.
 */
static pmodule_t *
pmodule_hold(pfc_module_t module)
{
	pmodule_t	*mod = pmodule_lookup_byid(module);
	pmod_dep_t	*dep, *edep;

	PFC_ASSERT(mod != NULL);

	/* Hold all modules which depends on the specified module. */
	edep = mod->pm_rdepends + mod->pm_nrdeps;
	for (dep = mod->pm_rdepends; dep < edep; dep++) {
		if (PFC_EXPECT_FALSE(pmodule_hold(dep->pd_module) == NULL)) {
			pmod_dep_t	*d;

			/* Rollback. */
			for (d = mod->pm_depends; d < dep; d++) {
				pmodule_release(d->pd_module);
			}

			return NULL;
		}
	}

	PMODULE_LOCK(mod);

	if (PFC_EXPECT_FALSE(!PMODULE_IS_LOADED(mod))) {
		PMODULE_UNLOCK(mod);

		for (dep = mod->pm_depends; dep < edep; dep++) {
			pmodule_release(dep->pd_module);
		}

		return NULL;
	}

	PFC_ASSERT(mod->pm_refcnt != 0);
	pmodule_get(mod);

	PMODULE_UNLOCK(mod);

	return mod;
}

/*
 * static void
 * pmodule_release(pfc_module_t module)
 *	Release modules held by the call of pmodule_hold().
 */
static void
pmodule_release(pfc_module_t module)
{
	pmodule_t	*mod = pmodule_lookup_byid(module);
	pmod_dep_t	*dep, *edep;

	PFC_ASSERT(mod != NULL);

	/* Release all modules which depends on the specified module. */
	edep = mod->pm_rdepends + mod->pm_nrdeps;
	for (dep = mod->pm_rdepends; dep < edep; dep++) {
		pmodule_release(dep->pd_module);
	}

	/* Release this module. */
	PMODULE_LOCK(mod);
	pmodule_put(mod);
	PMODULE_UNLOCK(mod);
}

/*
 * static pfc_cptr_t
 * pmodule_getkey(pfc_rbnode_t *node)
 *	Return the key of module descriptor specified by `node'.
 *	`node' must be a pointer to pm_node in pmodule_t.
 */
static pfc_cptr_t
pmodule_getkey(pfc_rbnode_t *node)
{
	pmodule_t	*mod = PMODULE_NODE2PTR(node);

	return (pfc_cptr_t)PMODULE_NAME(mod);
}

/*
 * static pfc_bool_t
 * pmodule_select_loaded(pmodule_t *mod)
 *	Selector function which selects the module if it is already loaded.
 *
 * Remarks:
 *	This function must be called with holding the module lock.
 */
static pfc_bool_t
pmodule_select_loaded(pmodule_t *mod)
{
	return (PMODULE_IS_LOADED(mod)) ? PFC_TRUE : PFC_FALSE;
}

/*
 * static pfc_bool_t
 * pmodule_select_unloaded(pmodule_t *mod)
 *	Selector function which selects the module if it is not loaded.
 *
 * Remarks:
 *	This function must be called with holding the module lock.
 */
static pfc_bool_t
pmodule_select_unloaded(pmodule_t *mod)
{
	return (PMODULE_IS_LOADED(mod)) ? PFC_FALSE : PFC_TRUE;
}

/*
 * static pfc_bool_t
 * pmodule_select_running(pmodule_t *mod)
 *	Selector function which selects the module if it is running.
 *
 * Remarks:
 *	This function must be called with holding the module lock.
 */
static pfc_bool_t
pmodule_select_running(pmodule_t *mod)
{
	return (mod->pm_state == PMOD_STATE_RUNNING) ? PFC_TRUE : PFC_FALSE;
}

/*
 * static int
 * pmodule_conf_load(pmodule_t *PFC_RESTRICT mod,
 *		     const char *PFC_RESTRICT path)
 *	Load module configuration file.
 *	`path' must be an absolute path of module configuration file.
 *
 * Calling/Exit State:
 *	Zero is returned on success.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding module lock.
 */
static int
pmodule_conf_load(pmodule_t *PFC_RESTRICT mod, const char *PFC_RESTRICT path)
{
	const pfc_modattr_t	*mattr = mod->pm_attr;
	const pfc_cfdef_t	*cfdef = mattr->pma_cfdef;
	pfc_refptr_t	*rpath, *public;
	int		err;

	if (cfdef == NULL) {
		/* This module has no configuration file. */
		return 0;
	}

	rpath = mod->pm_cfpath;
	if (rpath == NULL) {
		/*
		 * Preserve module configuration file path as refptr string.
		 *
		 * Remarks:
		 *	pm_cfpath keeps configuration file path even on error.
		 *	It may be available by reload.
		 */
		rpath = pfc_refptr_string_create(path);
		if (PFC_EXPECT_FALSE(rpath == NULL)) {
			pfc_log_error("%s: Failed to allocate buffer for "
				      "configuration file path.",
				      mattr->pma_name);

			return ENOMEM;
		}
		mod->pm_cfpath = rpath;
	}
	else {
		PFC_ASSERT(strcmp(path, pfc_refptr_string_value(rpath)) == 0);
	}

	public = mod->pm_pubcfpath;
	if (public == NULL) {
		char	*pubpath;
		size_t	len;

		/*
		 * Create path to public configuration file path.
		 *
		 * Remarks:
		 *	pm_pubcfpath keeps public configuration file path
		 *	even on error.
		 */
		len = PMODULE_NAME_LEN(mod) + PFC_MODULE_CONF_SUFFIX_LEN +
			module_confdir_len + 2;
		pubpath = malloc(len);
		if (PFC_EXPECT_FALSE(pubpath == NULL)) {
			goto error_public;
		}

		PFC_ASSERT_INT(snprintf(pubpath, len,
					"%s/%s" PFC_MODULE_CONF_SUFFIX,
					module_confdir, mattr->pma_name),
			       (int)len - 1);
		public = pfc_refptr_string_create(pubpath);
		free(pubpath);
		if (PFC_EXPECT_FALSE(public == NULL)) {
			goto error_public;
		}

		mod->pm_pubcfpath = public;
	}

	/*
	 * Open and parse configuration file.
	 * Public configuration file always precedes configuration file
	 * under module directory.
	 */
	err = pfc_conf_refopen2(&mod->pm_conf, public, rpath, cfdef);
	if (PFC_EXPECT_TRUE(err == 0)) {
		pmodule_conf_debug(mod, "loaded");
	}
	else {
		pfc_log_error("%s: Failed to load configuration file: err=%s",
			      mattr->pma_name, strerror(err));
	}

	return err;

error_public:
	pfc_log_error("%s: Failed to allocate buffer for public configuration "
		      "file path.", mattr->pma_name);

	return ENOMEM;
}

/*
 * static int
 * pmodule_conf_reload(pmodule_t *mod)
 *	Reload configuration file associated with the specified module
 *	instance.
 *
 * Calling/Exit State:
 *	Zero is returned on success.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding module lock.
 */
static int
pmodule_conf_reload(pmodule_t *mod)
{
	const pfc_modattr_t	*mattr;
	const pfc_cfdef_t	*cfdef;
	pfc_refptr_t	*rpath, *public;
	pfc_conf_t	prev, conf;
	int		err;

	PFC_ASSERT(mod != NULL);

	rpath = mod->pm_cfpath;
	public = mod->pm_pubcfpath;
	if (PFC_EXPECT_FALSE(rpath == NULL || public == NULL)) {
		/* No configuration file is associated with this module. */
		return 0;
	}

	mattr = mod->pm_attr;
	cfdef = mattr->pma_cfdef;
	PFC_ASSERT(cfdef != NULL);

	/* Preserve current handle. */
	prev = mod->pm_conf;
	if (prev != PFC_CONF_INVALID) {
		/* Reload configuration file. */
		err = pfc_conf_reload(prev);
		if (PFC_EXPECT_TRUE(err == 0)) {
			pmodule_conf_debug(mod, "reloaded");
		}
		else {
			pfc_log_error("%s: Failed to reload configuration "
				      "file: err=%s", mattr->pma_name,
				      strerror(err));
		}

		return err;
	}

	/* pfc_conf_refopen2() failed on module loading. Try again. */
	err = pfc_conf_refopen2(&conf, public, rpath, cfdef);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("%s: Failed to load configuration file: err=%s",
			      mattr->pma_name, strerror(err));

		return err;
	}

	/* Install new configuration file handle. */
	mod->pm_conf = conf;
	pmodule_conf_debug(mod, "loaded");

	return err;
}

/*
 * static void
 * pmodule_conf_debug(pmodule_t *PFC_RESTRICT mod,
 *		      const char *PFC_RESTRICT action)
 *	Output debugging log which indicates that the configuration file
 *	has been loaded.
 */
static void
pmodule_conf_debug(pmodule_t *PFC_RESTRICT mod,
		   const char *PFC_RESTRICT action)
{
	const char	*p_loaded, *s_loaded, *name;
	pfc_refptr_t	*primary, *secondary;
	pfc_conf_t	conf;

	pfc_log_burst_debug_begin();

	conf = mod->pm_conf;
	primary = mod->pm_pubcfpath;
	secondary = mod->pm_cfpath;
	PFC_ASSERT(primary != NULL && secondary != NULL);

	p_loaded = (pfc_conf_is_primary_loaded(conf))
		? "" : str_unavailable;
	s_loaded = (pfc_conf_is_secondary_loaded(conf))
		? "" : str_unavailable;
	name = PMODULE_NAME(mod);

	pfc_log_burst_write("%s: Configuration file has been %s.",
			    name, action);
	pfc_log_burst_write("%s: Primary: %s%s",
			    name, pfc_refptr_string_value(primary),
			    p_loaded);
	pfc_log_burst_write("%s: Secondary: %s%s",
			    name,  pfc_refptr_string_value(secondary),
			    s_loaded);
	pfc_log_burst_end();
}

/*
 * static void
 * pmodule_set_mandatory(pmodule_t *mod, pfc_bool_t mandatory)
 *	Change mandatory flag for the specified module.
 */
static void
pmodule_set_mandatory(pmodule_t *mod, pfc_bool_t mandatory)
{
	PMODULE_LOCK(mod);

	if (mandatory) {
		mod->pm_flags |= PMODF_MANDATORY;
	}
	else {
		mod->pm_flags &= ~PMODF_MANDATORY;
	}

	PMODULE_UNLOCK(mod);

	if (mandatory) {
		pmod_dep_t	*dep, *edep;

		/*
		 * All modules on which this module depends must also be
		 * mandatory.
		 */
		edep = mod->pm_depends + mod->pm_ndeps;
		for (dep = mod->pm_depends; dep < edep; dep++) {
			pmodule_t	*dmod;

			dmod = pmodule_lookup_byid(dep->pd_module);
			PFC_ASSERT(dmod != NULL);

			pmodule_set_mandatory(dmod, PFC_TRUE);
		}
	}
}

/*
 * static int
 * pmodule_select_wait(pmodule_t *mod, pmod_selector_t selector,
 *		       pfc_bool_t loading)
 *	Wait for the module to be selected by the specified selector.
 *	This function calls the specified selector with specifying the module.
 *	If the selector returns PFC_FALSE, the calling thread will be blocked
 *	until the selector returns PFC_TRUE.
 *
 *	`loading' must be true if the specified module is going to be loaded
 *	or started.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	EBADF is returned if an error has been detected.
 *	ESHUTDOWN is returned if the system is going to shutdown.
 *
 * Remarks:
 *	This function must be called with holding module lock.
 */
static int
pmodule_select_wait(pmodule_t *mod, pmod_selector_t selector,
		    pfc_bool_t loading)
{
	int	err = 0;

	while (1) {
		/* Catch exception bits. */
		err = pmodule_verify(mod, loading);
		if (PFC_EXPECT_FALSE(err != 0)) {
			break;
		}

		/* Call the selector. */
		if ((*selector)(mod)) {
			/* Selected. */
			break;
		}

		/* Wait for the state signal, and try again. */
		mod->pm_nwaiting++;
		PMODULE_WAIT(mod);
		mod->pm_nwaiting--;
	}

	return err;
}

/*
 * static int
 * pmodule_depends_wait(pmodule_t *mod, pmod_selector_t selector)
 *	Wait for all modules in the dependency list to be selected by the
 *	specified selector.
 *	If the module version is available, this function also checks whether
 *	all modules in the dependency list satisfy required module version.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	EBADF is returned if an error has been detected.
 *	ESHUTDOWN is returned if the system is going to shutdown.
 *	EINVAL is returned if the module version doesn't satisfy required
 *	version.
 *
 * Remarks:
 *	- This function acquires the lock for modules in the dependency list.
 *	  Be careful if you want to call this function with holding another
 *	  lock.
 *	- This function is designed to wait modules in the dependency list
 *	  to be loaded or started. So ESHUTDOWN is returned if the shutdown
 *	  flag is set in module descriptor.
 */
static int
pmodule_depends_wait(pmodule_t *mod, pmod_selector_t selector)
{
	pmod_dep_t	*dep, *edep;
	int	err = 0;

	edep = mod->pm_depends + mod->pm_ndeps;
	for (dep = mod->pm_depends; dep < edep; dep++) {
		const pfc_modattr_t	*mattr;
		pmodule_t	*dmod;

		dmod = pmodule_lookup_byid(dep->pd_module);
		PFC_ASSERT(dmod != NULL);

		PMODULE_LOCK(dmod);
		err = pmodule_select_wait(dmod, selector, PFC_TRUE);
		if (PFC_EXPECT_FALSE(err != 0)) {
			if (err != ESHUTDOWN && err != ECANCELED) {
				pfc_log_error("%s: Failed to select module %s:"
					      " %s", PMODULE_NAME(mod),
					      PMODULE_NAME(dmod),
					      strerror(err));
			}
			PMODULE_UNLOCK(dmod);
			break;
		}

		mattr = dmod->pm_attr;
		if (PFC_EXPECT_TRUE(mattr != NULL)) {
			uint8_t	dver = mattr->pma_version;

			if (PFC_EXPECT_FALSE(dver < dep->pd_version)) {
				pfc_log_error("%s requires %s version %u, "
					      "but %u.", PMODULE_NAME(mod),
					      PMODULE_NAME(dmod),
					      dep->pd_version, dver);
				err = EINVAL;
				PMODULE_UNLOCK(dmod);
				break;
			}
		}

		PMODULE_UNLOCK(dmod);
	}

	return err;
}

/*
 * static int
 * pmodule_rev_depends_wait(pmodule_t *mod, pmod_selector_t selector)
 *	Wait for all modules which depend on the specified module to be
 *	selected by the specified selector.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	EBADF is returned if an error has been detected.
 *	ESHUTDOWN is returned if the system is going to shutdown.
 *
 * Remarks:
 *	- This function acquires the lock for modules in the reverse dependency
 *	  list. Be careful if you want to call this function with holding
 *	  another lock.
 *	- This function is designed to be called on module unloading.
 *	  So this function never returns ESHUTDOWN even if the shutdown flag
 *	  is set in module descriptor.
 */
static int
pmodule_rev_depends_wait(pmodule_t *mod, pmod_selector_t selector)
{
	pmod_dep_t	*dep, *edep;
	int	err = 0;

	edep = mod->pm_rdepends + mod->pm_nrdeps;
	for (dep = mod->pm_rdepends; dep < edep; dep++) {
		pmodule_t	*dmod;

		dmod = pmodule_lookup_byid(dep->pd_module);
		PFC_ASSERT(dmod != NULL);

		PMODULE_LOCK(dmod);
		err = pmodule_select_wait(dmod, selector, PFC_FALSE);
		PMODULE_UNLOCK(dmod);
		if (PFC_EXPECT_FALSE(err != 0)) {
			break;
		}
	}

	return err;
}

/*
 * static int
 * pmodule_call_init(pmodule_t *mod)
 *	Call initializer of the specified module.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	-1 is returned if the initializer returns PFC_FALSE.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The caller must hold the module lock on calling.
 *	Note that it is released when module initializer is called, and
 *	it is acquired again on return.
 */
static int
pmodule_call_init(pmodule_t *mod)
{
	pfc_modtype_t	type;
	int		ret;
	const pfc_modattr_t	*mattr = mod->pm_attr;
	const pfc_modops_t	*ops;

	type = mattr->pma_type;
	PFC_MODTYPE_ASSERT(type);
	ops = module_ops[type];

	pfc_log_verbose("%s: Calling init().", mattr->pma_name);

	PFC_ASSERT(mod->pm_state == PMOD_STATE_LOADED);

	/*
	 * We don't need to use pmodule_state_change() because we know that
	 * no one wait for IN_INIT state.
	 */
	mod->pm_state = PMOD_STATE_IN_INIT;
	PMODULE_UNLOCK(mod);

	/* Call init(). */
	ret = ops->init(mod);

	if (PFC_EXPECT_FALSE(ret != 0)) {
		pfc_rbtree_t	ipcevhdlr;
		const char	*name = PMODULE_NAME(mod);

		/* Remove unload hooks registered by this module. */
		pmodule_cleanhooks(mod);

		/* Remove module-specific event handlers. */
		(void)pmodule_event_clear_default(name);

		/* Remove all IPC event handlers. */
		PMODULE_LOCK(mod);
		PMODULE_IPCEVHDLR_CLEAR(mod, &ipcevhdlr);
		PMODULE_UNLOCK(mod);

		pfc_rbtree_clear(&ipcevhdlr, pmodule_ipcevhdlr_dtor,
				 (pfc_ptr_t)name);
	}
	else {
		/* Register IPC service handler. */
		(void)ops->ipc_register(mod);
	}

	PMODULE_LOCK(mod);

	return ret;
}

/*
 * static int
 * pmodule_call_fini(pmodule_t *mod)
 *	Call finalizer of the specified module.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	-1 is returned if the finalizer returns PFC_FALSE.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The caller must hold the module lock on calling.
 *	Note that it is released when module finalizer is called, and
 *	it is acquired again on return.
 */
static int
pmodule_call_fini(pmodule_t *mod)
{
	pfc_modtype_t	type;
	pfc_bool_t	ret;
	pfc_rbtree_t	ipcevhdlr;
	int		err;
	const char	*name = PMODULE_NAME(mod);
	const pfc_modattr_t	*mattr = mod->pm_attr;
	const pfc_modops_t	*ops;

	type = mattr->pma_type;
	PFC_MODTYPE_ASSERT(type);
	ops = module_ops[type];

	pfc_log_verbose("%s: Calling fini().", mattr->pma_name);

	PFC_ASSERT(mod->pm_state == PMOD_STATE_RUNNING);

	/* Make all IPC event handlers invisible. */
	PMODULE_IPCEVHDLR_CLEAR(mod, &ipcevhdlr);

	/*
	 * We don't need to use pmodule_state_change() because we know that
	 * no one wait for IN_FINI state.
	 */
	mod->pm_state = PMOD_STATE_IN_FINI;
	PMODULE_UNLOCK(mod);

	/* Call module unload hooks registered by this module. */
	pmodule_runhooks(mod, PFC_FALSE);

	/*
	 * Remove all module-specific event handlers registered by this
	 * module.
	 */
	err = pmodule_event_clear_default(name);

	/* Remove IPC service handler. */
	pfc_ipcsrv_remove_handler(name);

	/* Remove IPC event handlers. */
	pfc_rbtree_clear(&ipcevhdlr, pmodule_ipcevhdlr_dtor, (pfc_ptr_t)name);

	/* Call fini(). */
	ret = ops->fini(mod);
	if (PFC_EXPECT_FALSE(!ret)) {
		err = -1;
	}

	PMODULE_LOCK(mod);

	return err;
}

/*
 * static int
 * pmodule_call_boot(pmodule_t *mod)
 *	Call bootstrap function of the specified module.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The caller must hold the module lock on calling.
 *	Note that it is released when module bootstrap function is called, and
 *	it is acquired again on return.
 */
static int
pmodule_call_boot(pmodule_t *mod)
{
	pfc_modboot_t	boot = mod->pm_boot;
	int		err;

	if (boot == NULL) {
		err = 0;
	}
	else {
		uint32_t	flags = mod->pm_flags;

		pfc_log_verbose("%s: Calling bootstrap function.",
				PMODULE_NAME(mod));
		PFC_ASSERT(!(flags & PMODF_BOOTSTRAP));
		mod->pm_flags = (flags | PMODF_BOOTSTRAP);
		PMODULE_UNLOCK(mod);

		err = (*boot)();
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_error("%s: Bootstrap function returned %d.",
				      PMODULE_NAME(mod), err);
		}

		PMODULE_LOCK(mod);
		mod->pm_flags &= ~PMODF_BOOTSTRAP;
	}

	return err;
}

/*
 * static int
 * pmodule_load(pmodule_t *mod)
 *	Load the PFC module associated with the specified module descriptor.
 *
 * Calling/Exit State:
 *	Upon successful completion, a module library associated with the
 *	specified module descriptor is loaded into the PFC system, and
 *	zero is returned. If the module has configuration file, it is also
 *	loaded.
 *
 *	An error number which indicates the cause of error is returned
 *	on failure.
 *
 * Remarks:
 *	- We don't assume that this function is called with specifying the
 *	  same module simultaneously. The caller must serialize the call of
 *	  pmodule_load() with specifying the same module.
 *
 *	- Although pmodule_load() checks state of modules in the dependency
 *	  list, it never changes them. The caller must load all modules in
 *	  the dependency list in advance.
 */
static int
pmodule_load(pmodule_t *mod)
{
	int	err;

	PFC_ASSERT(mod != NULL);

	/*
	 * Ensure that all modules on which this module depends are already
	 * loaded.
	 */
	err = pmodule_depends_wait(mod, pmodule_select_loaded);
	PMODULE_LOCK(mod);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/* Start transition of module state. */
	err = pmodule_set_busy(mod, PFC_TRUE);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/* Load the module. */
	err = pmodule_do_load(mod);
	pmodule_clear_busy(mod);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	PMODULE_UNLOCK(mod);

	return 0;

error:
	PMODULE_SET_ERROR(mod);
	PMODULE_UNLOCK(mod);

	return err;
}

/*
 * static int
 * pmodule_do_load(pmodule_t *mod)
 *	Load the PFC module associated with the specified module descriptor.
 *
 * Remarks:
 *	- This function never checks state of modules in the dependency list.
 *	- This function never set the module busy flag. The caller must do.
 *	- This function must be called with holding the module lock.
 *	  Note that the lock will be released while dlopen() is called.
 */
static int
pmodule_do_load(pmodule_t *mod)
{
	const char	*name;
	pmod_state_t	state;
	char	*path;
	void	*handle;
	size_t	len;
	int	err = 0;

	/* Construct an absolute path to module library file. */
	path = pmodule_library_path(mod);
	if (PFC_EXPECT_FALSE(path == NULL)) {
		return ENOMEM;
	}

	state = mod->pm_state;
	if (PMOD_STATE_IS_LOADED(state)) {
		/* Already loaded. */
		return 0;
	}

	name = PMODULE_NAME(mod);
	if (PFC_EXPECT_FALSE(state == PMOD_STATE_INACTIVE)) {
		/*
		 * The module is treated as unloaded, but the module library
		 * is still in memory. Only thing to do here is to reload
		 * module configuration file.
		 */
		err = pmodule_conf_reload(mod);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto out;
		}

		/* Call bootstrap function. */
		err = pmodule_call_boot(mod);
		if (PFC_EXPECT_TRUE(err == 0)) {
			pmodule_state_change(mod, PMOD_STATE_LOADED);
			pmodule_get(mod);
			pfc_log_verbose("%s: Module reopened.", name);
		}
		goto out;
	}

	PFC_ASSERT(mod->pm_handle == NULL);
	PFC_ASSERT(mod->pm_attr == NULL);

	/*
	 * The module lock must be released because it will be acquired by
	 * constructor of module.
	 */
	PMODULE_UNLOCK(mod);

	pfc_mutex_lock(&module_load_lock);

	/* Open the module library file. */
	handle = dlopen(path, RTLD_GLOBAL | RTLD_NOW);
	if (PFC_EXPECT_FALSE(handle == NULL)) {
		char	*dlerr = dlerror();

		pfc_log_error("%s: dlopen() failed: path = %s", name, path);
		if (dlerr != NULL) {
			pfc_log_error("%s: dlerror(): %s", name, dlerr);
		}
		err = EIO;

		pfc_mutex_unlock(&module_load_lock);
		PMODULE_LOCK(mod);
		goto out;
	}

	pfc_mutex_unlock(&module_load_lock);
	PMODULE_LOCK(mod);

	/* Ensure that the library has registered itself as a PFC module. */
	if (PFC_EXPECT_FALSE(mod->pm_attr == NULL)) {
		pfc_log_error("%s: Module is not registered.", name);

		err = pmodule_close_library(mod, handle);

		if (PFC_EXPECT_FALSE(err != 0)) {
			/*
			 * Mark this module dead even EBUSY case because the
			 * library file is invalid.
			 */
			pmodule_dead(mod);
		}

		err = ENOENT;
		goto out;
	}

	/* Load module configuration file. */
	len = PMODULE_PATH_MAX;
	snprintf(path, len, "%s/%s" PFC_MODULE_CONF_SUFFIX, module_dir, name);
	err = pmodule_conf_load(mod, path);
	if (PFC_EXPECT_FALSE(err != 0)) {
		int	e = pmodule_close_library(mod, handle);

		if (PFC_EXPECT_FALSE(e == EBUSY)) {
			mod->pm_handle = handle;
			pmodule_state_change(mod, PMOD_STATE_INACTIVE);
		}
		else if (PFC_EXPECT_FALSE(e != 0)) {
			pmodule_dead(mod);
		}
		goto out;
	}

	/* Call bootstrap function. */
	err = pmodule_call_boot(mod);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	/* We assume that no other thread tries to load this module. */
	PFC_ASSERT(mod->pm_handle == NULL);
	PFC_ASSERT(mod->pm_refcnt == 0);

	pfc_log_debug("%s: Module has been loaded.", name);
	mod->pm_refcnt = 1;
	mod->pm_handle = handle;

	pmodule_state_change(mod, PMOD_STATE_LOADED);

out:
	free(path);

	return err;
}

/*
 * static void
 * pmodule_dead(pmodule_t *mod)
 *	Change module state to DEAD.
 *	The caller must call this function with holding the module lock.
 */
static void
pmodule_dead(pmodule_t *mod)
{
	PFC_ASSERT(mod->pm_flags & PMODF_BUSY);
	pmodule_state_change(mod, PMOD_STATE_DEAD);
}

/*
 * static int
 * pmodule_start(pmodule_t *mod)
 *	Start the PFC module associated with the specified module descriptor.
 *
 * Calling/Exit State:
 *	Upon successful completion, the module associated with the specified
 *	module descriptor starts to run, and zero is returned.
 *
 *	An error number which indicates the cause of error is returned
 *	on failure. Note that EPROTO means init() entry returned PFC_FALSE.
 *
 * Remarks:
 *	- We don't assume that this function is called with specifying the
 *	  same module simultaneously. The caller must serialize the call of
 *	  pmodule_start() with specifying the same module.
 *
 *	- Although pmodule_start() checks state of modules in the dependency
 *	  list, it never changes them. The caller must load all modules in
 *	  the dependency list in advance.
 */
static int
pmodule_start(pmodule_t *mod)
{
	pmod_state_t	state;
	int		err;

	PFC_ASSERT(mod != NULL);

	/* Ensure that all modules on which this module depends are running. */
	err = pmodule_depends_wait(mod, pmodule_select_running);
	PMODULE_LOCK(mod);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	state = mod->pm_state;
	if (state == PMOD_STATE_RUNNING) {
		/* Already running. */
		goto out;
	}

	/* Start transition of module state. */
	err = pmodule_set_busy(mod, PFC_TRUE);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	if (!PMOD_STATE_IS_LOADED(state)) {
		/* Load the module. */
		err = pmodule_do_load(mod);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto error_busy;
		}
	}

	/* Call init() for this module. */
	err = pmodule_call_init(mod);
	if (PFC_EXPECT_TRUE(err == 0)) {
		pfc_log_debug("%s: Module has been started.",
			      PMODULE_NAME(mod));
		pmodule_state_change(mod, PMOD_STATE_RUNNING);
	}
	else {
		if (err < 0) {
			pfc_log_error("%s: init() has failed.",
				      PMODULE_NAME(mod));
			err = EPROTO;
		}
		else {
			pfc_log_error("%s: error occurred in init(): %s",
				      PMODULE_NAME(mod), strerror(err));
		}

		/* Revert module state. */
		pmodule_state_change(mod, PMOD_STATE_LOADED);
		goto error_busy;
	}

	pmodule_clear_busy(mod);
out:
	PMODULE_UNLOCK(mod);

	return 0;

error_busy:
	pmodule_clear_busy(mod);
error:
	PMODULE_SET_ERROR(mod);
	PMODULE_UNLOCK(mod);

	return err;
}

/*
 * static int
 * pmodule_unload(pmodule_t *mod)
 *	Unload the PFC module associated with the specified module descriptor.
 *
 * Calling/Exit State:
 *	Upon successful completion, the module associated with the specified
 *	module descriptor is unloaded, and zero is returned.
 *
 *	An error number which indicates the cause of error is returned
 *	on failure. Note that EPROTO means fini() entry returned PFC_FALSE.
 *
 * Remarks:
 *	- We don't assume that this function is called with specifying the
 *	  same module simultaneously. The caller must serialize the call of
 *	  pmodule_unload() with specifying the same module.
 *
 *	- Although pmodule_start() checks state of modules in the reverse
 *	  dependency list, it never changes them. The caller must unload
 *	  all modules in the reverse dependency list in advance.
 */
static int
pmodule_unload(pmodule_t *mod)
{
	pmod_state_t	state;
	int		err;

	PFC_ASSERT(mod != NULL);

	/*
	 * Ensure that all modules which depend on this module are not
	 * loaded.
	 */
	err = pmodule_rev_depends_wait(mod, pmodule_select_unloaded);
	PMODULE_LOCK(mod);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	state = mod->pm_state;

	if (!PMOD_STATE_IS_LOADED(state)) {
		/* Already unloaded. */
		goto out;
	}

	/* Start transition of module state. */
	err = pmodule_set_busy(mod, PFC_FALSE);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	if (state == PMOD_STATE_RUNNING) {
		/* Call fini() for this module. */
		err = pmodule_call_fini(mod);
		if (PFC_EXPECT_TRUE(err == 0)) {
			pfc_log_debug("%s: Module has been stopped.",
				      PMODULE_NAME(mod));
		}
		else {
			if (err < 0) {
				pfc_log_error("%s: fini() has failed.",
					      PMODULE_NAME(mod));
			}
			pmodule_dead(mod);
			err = EPROTO;

			/*
			 * We don't want to set error flag to this module
			 * because fini() error on module unloading should be
			 * ignored.
			 */
			goto clear_busy;
		}
	}

	/* Decrement reference to this module. */
	pmodule_state_change(mod, PMOD_STATE_INACTIVE);
	pmodule_put(mod);

clear_busy:
	pmodule_clear_busy(mod);

out:
	PMODULE_UNLOCK(mod);

	return err;

error:
	PMODULE_SET_ERROR(mod);
	PMODULE_UNLOCK(mod);

	return err;
}

/*
 * static int
 * pmodule_event_add(pfc_evhandler_t *PFC_RESTRICT idp,
 *		     pfc_refptr_t *domain, pfc_evfunc_t handler,
 *		     pfc_ptr_t PFC_RESTRICT arg,
 *		     const pfc_evmask_t *PFC_RESTRICT evmask,
 *		     uint32_t priority, pfc_refptr_t *rhname)
 *	Register module-specific event handler.
 *
 *	The domain of module-specific event delivery must be specified by
 *	the refptr string specified by `domain'.
 *
 *	`rhname' is a string refptr which represents user-defined name of
 *	event handler. Currently, it is used only for event delivery logging.
 *	NULL means an anonymous handler.
 *
 * Calling/Exit State:
 *	Upon successful completion, event handler ID is set to `*idp',
 *	and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
pmodule_event_add(pfc_evhandler_t *PFC_RESTRICT idp,
		  pfc_refptr_t *domain, pfc_evfunc_t handler,
		  pfc_ptr_t PFC_RESTRICT arg,
		  const pfc_evmask_t *PFC_RESTRICT evmask, uint32_t priority,
		  pfc_refptr_t *rhname)
{
	pfc_evmask_t	mask;

	/* Ensure that event mask bits are valid. */
	if (evmask == NULL) {
		mask = PMODULE_EVMASK_VALID;
		evmask = &mask;
	}
	else if (PMODULE_EVMASK_IS_INVALID(*evmask)) {
		return EINVAL;
	}

	return pfc_event_domain_add_handler(idp, module_evsrc, domain,
					    handler, arg, evmask, priority,
					    rhname);
}

/*
 * static int
 * pmodule_event_post(pfc_refptr_t *PFC_RESTRICT source,
 *		      const char *PFC_RESTRICT target, pfc_evtype_t type)
 *	Post a module-specific event.
 *
 *	`source' must be a pointer which represents who sends the event.
 *	NULL means an external process. A non-NULL pointer represents the
 *	module name.
 *
 *	`target' is a string which represents domain of the event delivery.
 *	NULL means the broadcast event. If a non-NULL pointer is specified,
 *	the event is delivered to only handlers associated with the specified
 *	domain.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The reference counter of `source' will be always decremented on return:
 *	  + On successful return, it will be decremented asynchronously by
 *	    event dispatch thread.
 *	  + On error return, it will be decremented immediately by this
 *	    function.
 */
static int
pmodule_event_post(pfc_refptr_t *PFC_RESTRICT source,
		   const char *PFC_RESTRICT target, pfc_evtype_t type)
{
	pfc_event_t	event;
	int		err;

	if (target == NULL) {
		/* Broadcast event must have system defined type. */
		if (PFC_EXPECT_FALSE(PFC_MODEVENT_TYPE_IS_LOCAL(type))) {
			err = EINVAL;
			goto error;
		}
	}
	else if (!PFC_MODULE_IS_CORE(target)) {
		pmodule_t	*tgt;
		pmod_state_t	state;

		/* The target must be a module. */
		tgt = pmodule_lookup(target);
		if (PFC_EXPECT_FALSE(tgt == NULL)) {
			err = ENOENT;
			goto error;
		}

		PMODULE_LOCK(tgt);
		state = tgt->pm_state;
		PMODULE_UNLOCK(tgt);

		if (PFC_EXPECT_FALSE(state != PMOD_STATE_RUNNING)) {
			/* The target module is not loaded. */
			err = ENODEV;
			goto error;
		}
	}

	if (PFC_EXPECT_FALSE(!PFC_MODEVENT_TYPE_IS_VALID(type))) {
		err = EINVAL;
		goto error;
	}

	/* Create a module-specific event. */
	err = pfc_event_create_flags(&event, type, source, pmodule_event_dtor,
				     EVF_MODEVENT);
	if (PFC_EXPECT_TRUE(err == 0)) {
		/* Post this event with specifying domain. */
		return pfc_event_domain_post(module_evsrc, target, event);
	}

error:
	if (source != NULL) {
		pfc_refptr_put(source);
	}

	return err;
}

/*
 * static void
 * pmodule_event_dtor(pfc_ptr_t data)
 *	Destructor of module-specific event instance.
 */
static void
pmodule_event_dtor(pfc_ptr_t data)
{
	if (data != NULL) {
		pfc_refptr_t	*ref = (pfc_refptr_t *)data;

		pfc_refptr_put(ref);
	}
}

/*
 * static int
 * pmodule_event_clear(const char *PFC_RESTRICT name,
 *		       const pfc_timespec_t *PFC_RESTRICT timeout)
 *	Remove all module-specific event handlers associated with the domain
 *	specified by `name'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
pmodule_event_clear(const char *PFC_RESTRICT name,
		    const pfc_timespec_t *PFC_RESTRICT timeout)
{
	int	err;

	err = pfc_event_domain_remove_handler(module_evsrc, name, timeout);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err == ENOENT) {
			/* No handler is found. We can ignore this error. */
			err = 0;
		}
		else {
			pfc_log_error("%s: Failed to remove module-specific "
				      "event handlers: %s",
				      name, strerror(err));
		}
	}

	return err;
}

/*
 * static void
 * pmodule_addhook(pfc_list_t *PFC_RESTRICT head, mod_hook_t *PFC_RESTRICT hep)
 *	Link the given module hook entry to the hook list.
 */
static void
pmodule_addhook(pfc_list_t *PFC_RESTRICT head, mod_hook_t *PFC_RESTRICT hep)
{
	pfc_list_t	*elem, *entry = &hep->mhe_list;
	mod_hook_t	*hook;
	uint32_t	pri = hep->mhe_priority;

	if (pfc_list_is_empty(head)) {
		pfc_list_push(head, entry);

		return;
	}

	/* Sort hooks in ascending order of priority. */
	hook = MOD_HOOK_LIST2PTR(head->pl_prev);
	if (hook->mhe_priority <= pri) {
		/* The given hook has the greatest priority. */
		pfc_list_push_tail(head, entry);

		return;
	}

#ifdef	PFC_VERBOSE_DEBUG
	pfc_list_init(entry);
#endif	/* PFC_VERBOSE_DEBUG */

	PFC_LIST_FOREACH(head, elem) {
		hook = MOD_HOOK_LIST2PTR(elem);
		if (hook->mhe_priority >= pri) {
			/* Link the given hook just before this hook. */
			pfc_list_push_tail(elem, entry);
			break;
		}
	}

	/* Above loop must link the given hook to the list. */
	PFC_ASSERT(!pfc_list_is_empty(entry));
}

/*
 * static void
 * pmodule_runhooks(pmodule_t *mod, pfc_bool_t isstop)
 *	Run module unload hooks associated with the given module, and remove
 *	them.
 *
 *	If `mod' is NULL, all unload hooks are invoked and then removed.
 *	`isstop' is an argument to be passed to each hooks.
 */
static void
pmodule_runhooks(pmodule_t *mod, pfc_bool_t isstop)
{
	mod_hooklist_t	*hlp = &module_unload_hooks;
	mod_hook_t	*hep;
	pfc_list_t	*elem, *next, hooks;

	MOD_HOOKLIST_LOCK(hlp);

	if (mod == NULL) {
		/* Remove all hooks. */
		pfc_list_move_all(&hlp->mhl_head, &hooks);
		pfc_list_init(&hlp->mhl_head);
	}
	else {
		pfc_module_t	module = mod->pm_id;

		/* Select hooks associated with the given module. */
		pfc_list_init(&hooks);
		PFC_LIST_FOREACH_SAFE(&hlp->mhl_head, elem, next) {
			hep = MOD_HOOK_LIST2PTR(elem);
			if (hep->mhe_module == module) {
				pfc_list_remove(elem);
				pfc_list_push_tail(&hooks, elem);
			}
		}
	}

	MOD_HOOKLIST_UNLOCK(hlp);

	/* Run selected hooks. */
	PFC_LIST_FOREACH_SAFE(&hooks, elem, next) {
		hep = MOD_HOOK_LIST2PTR(elem);

		pfc_log_burst_verbose_begin();
		mod = pmodule_lookup_byid(hep->mhe_module);
		PFC_ASSERT(mod != NULL);
		pfc_log_burst_write("%s: Calling unload hook: %p",
				    PMODULE_NAME(mod), hep->mhe_hook);
		pfc_log_burst_end();

		hep->mhe_hook(isstop);

		pfc_list_remove(elem);
		free(hep);
	}
}

/*
 * static void
 * pmodule_cleanhooks(pmodule_t *mod)
 *	Remove all unload hooks registered by the given module.
 */
static void
pmodule_cleanhooks(pmodule_t *mod)
{
	mod_hooklist_t	*hlp = &module_unload_hooks;
	pfc_module_t	module = mod->pm_id;
	pfc_list_t	*elem, *next;

	MOD_HOOKLIST_LOCK(hlp);

	PFC_LIST_FOREACH_SAFE(&hlp->mhl_head, elem, next) {
		mod_hook_t	*hep = MOD_HOOK_LIST2PTR(elem);

		if (hep->mhe_module == module) {
			pfc_list_remove(elem);
			free(hep);
		}
	}

	MOD_HOOKLIST_UNLOCK(hlp);
}

/*
 * static void
 * pmodule_ipcevhdlr_dtor(pfc_rbnode_t *node, pfc_ptr_t arg)
 *	Destructor of the IPC event handler entry.
 *	`node' must be a pointer to ieh_node in mod_ipcevhdlr_t.
 *	`arg' must be a pointer to module name.
 */
static void
pmodule_ipcevhdlr_dtor(pfc_rbnode_t *node, pfc_ptr_t arg)
{
	mod_ipcevhdlr_t	*mhp = PMODULE_IPCEVHDLR_NODE2PTR(node);
	pfc_ipcevhdlr_t	id = mhp->ieh_id;
	int		err;

	free(mhp);
	err = pfc_ipcevent_remove_handler(id);
	if (PFC_EXPECT_FALSE(err != 0)) {
		const char	*name = (const char *)arg;

		/* This should never happen. */
		pfc_log_error("%s: Failed to remove IPC event handler: "
			      "id=%u: %s", name, id, strerror(err));
		/* FALLTHROUGH */
	}
}

/*
 * static pfc_cptr_t
 * pmodule_ipcevhdlr_getkey(pfc_rbnode_t *node)
 *	Return the key of IPC event handler entry.
 *	`node' must be a pointer to ieh_node in mod_ipcevhdlr_t.
 */
static pfc_cptr_t
pmodule_ipcevhdlr_getkey(pfc_rbnode_t *node)
{
	mod_ipcevhdlr_t	*mhp = PMODULE_IPCEVHDLR_NODE2PTR(node);

	return PMODULE_IPCEVHDLR_KEY(mhp->ieh_id);
}
