/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_MODULE_IMPL_H
#define	_PFC_LIBPFC_MODULE_IMPL_H

/*
 * Internal definitions for module management.
 */

#include <pfc/module.h>
#include <pfc/hash.h>
#include <pfc/rbtree.h>
#include <pfc/clock.h>
#include <pfc/conf.h>
#include <pfc/synch.h>
#include <pfc/refptr.h>
#include <pfc/list.h>
#include <pfc/listmodel.h>

PFC_C_BEGIN_DECL

/*
 * Number of hash buckets for the hash table which manages PFC module.
 */
#define	PFC_MODULE_HASH_NBUCKETS	31

/*
 * Module state bits.
 */
#define	PMODST_LOADED		0x1	/* module is loaded */
#define	PMODST_SERIAL(num)	((num) << 4)

/*
 * Module state.
 */
typedef enum {
	/*
	 * The module is not loaded into the PFC system.
	 * This is initial state of module descriptor.
	 */
	PMOD_STATE_UNLOADED	= 0,

	/* The module is unloaded, but the library is still in memory. */
	PMOD_STATE_INACTIVE	= PMODST_SERIAL(1),

	/* This module is dead. */
	PMOD_STATE_DEAD		= PMODST_SERIAL(2),

	/* The module is loaded, but not yet running. */
	PMOD_STATE_LOADED	= (PMODST_SERIAL(3) | PMODST_LOADED),

	/* The module is now running. */
	PMOD_STATE_RUNNING	= (PMODST_SERIAL(4) | PMODST_LOADED),

	/* Module's init() is now calling. */
	PMOD_STATE_IN_INIT	= (PMODST_SERIAL(5) | PMODST_LOADED),

	/* Module's fini() is now calling. */
	PMOD_STATE_IN_FINI	= (PMODST_SERIAL(6) | PMODST_LOADED),
} pmod_state_t;

/*
 * Module dependency.
 */
typedef struct {
	pfc_module_t		pd_module;	/* module ID */
	uint8_t			pd_version;	/* module version */
	uint8_t			pd_pad;
} pmod_dep_t;

/*
 * Common module descriptor.
 */
typedef struct {
	const pfc_modattr_t	*pm_attr;	/* module description */
	pfc_refptr_t		*pm_name;	/* module name */
	pfc_modboot_t		pm_boot;	/* bootstrap function */
	pfc_rbnode_t		pm_node;	/* Red-Black Tree node */
	void			*pm_handle;	/* DSO handle */
	pfc_mutex_t		pm_mutex;	/* module mutex */
	pfc_cond_t		pm_cond;	/* condition variable */
	pmod_dep_t		*pm_depends;	/* dependencies */
	pmod_dep_t		*pm_rdepends;	/* reverse dependencies */
	uint16_t		pm_ndeps;	/* number of dependencies */
	uint16_t		pm_nrdeps;	/* number of reverse deps */
	uint32_t		pm_refcnt;	/* reference counter */
	volatile pmod_state_t	pm_state;	/* module state */
	volatile uint32_t	pm_flags;	/* flags */
	volatile uint32_t	pm_nwaiting;	/* number of waiter threads */
	pfc_module_t		pm_id;		/* module ID */
	pfc_conf_t		pm_conf;	/* configuration file handle */
	pfc_refptr_t		*pm_cfpath;	/* configuration file path */
	pfc_refptr_t		*pm_pubcfpath;	/* public conf file path */
	pfc_rbtree_t		pm_ipcevhdlr;	/* IPC event handlers */

	union {
		/* C module data */
		struct {
			pfc_modfunc_t	init;	/* initializer */
			pfc_modfunc_t	fini;	/* finalizer */
		} handler;

		/* C++ module data */
		struct {
			const char	*tname;		/* type identifier */
			pfc_modfac_t	factory;	/* factory function */
			void		*ptr;		/* pointer to object */
		} object;
	} pm_u;
} pmodule_t;

#define	PMODULE_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), pmodule_t, pm_node)

#define	PMODULE_HANDLER_INIT(mod)	((mod)->pm_u.handler.init)
#define	PMODULE_HANDLER_FINI(mod)	((mod)->pm_u.handler.fini)
#define	PMODULE_OBJECT_TNAME(mod)	((mod)->pm_u.object.tname)
#define	PMODULE_OBJECT_FACTORY(mod)	((mod)->pm_u.object.factory)
#define	PMODULE_OBJECT_PTR(mod)		((mod)->pm_u.object.ptr)

#define	PMODULE_NAME(mod)	pfc_refptr_string_value((mod)->pm_name)
#define	PMODULE_NAME_LEN(mod)	pfc_refptr_string_length((mod)->pm_name)

/*
 * Module lock/unlock macros.
 */
#define	PMODULE_LOCK(mod)	pfc_mutex_lock(&(mod)->pm_mutex)
#define	PMODULE_UNLOCK(mod)	pfc_mutex_unlock(&(mod)->pm_mutex)

/*
 * Condition variable macros.
 */
#define	PMODULE_WAIT(mod)					\
	pfc_cond_wait(&(mod)->pm_cond, &(mod)->pm_mutex)
#define	PMODULE_TIMEDWAIT_ABS(mod, abstime)				\
	pfc_cond_timedwait_abs(&(mod)->pm_cond, &(mod)->pm_mutex, (abstime))
#define	PMODULE_BROADCAST(mod)		pfc_cond_broadcast(&(mod)->pm_cond)

/*
 * Module status test macros.
 */
#define	PMOD_STATE_IS_LOADED(state)	((state) & PMODST_LOADED)
#define	PMODULE_IS_LOADED(mod)		PMOD_STATE_IS_LOADED((mod)->pm_state)

/*
 * Module flags.
 */
#define	PMODF_MANDATORY		PFC_CONST_U(0x1)	/* mandatory module */
#define	PMODF_ERROR		PFC_CONST_U(0x2)	/* error detected */
#define	PMODF_SHUTDOWN		PFC_CONST_U(0x4)	/* module shutdown */
#define	PMODF_BUSY		PFC_CONST_U(0x8)	/* busy flag */
#define	PMODF_CANCEL		PFC_CONST_U(0x10)	/* operation cancel */
#define	PMODF_BOOTSTRAP		PFC_CONST_U(0x20)	/* calling boot */

#define	PMODULE_SET_FLAG(mod, flag)		\
	do {					\
		(mod)->pm_flags |= (flag);	\
		if ((mod)->pm_nwaiting != 0) {	\
			PMODULE_BROADCAST(mod);	\
		}				\
	} while (0)

#define	PMODULE_SET_ERROR(mod)		PMODULE_SET_FLAG(mod, PMODF_ERROR)
#define	PMODULE_SET_CANCEL(mod)		PMODULE_SET_FLAG(mod, PMODF_CANCEL)

/*
 * Determine whether the module bootstrap function is being called or not.
 */
#define	PMODULE_IN_BOOTSTRAP(mod)	((mod)->pm_flags & PMODF_BOOTSTRAP)

/*
 * Determine whether the module supports IPC service.
 */
#define	PMODULE_HAS_IPC(mattr)						\
	((mattr)->pma_sysversion >= 2 && (mattr)->pma_nipcs != 0)

/*
 * Module operations.
 */
typedef struct {
	/*
	 * Create module information.
	 * Note that this method is called with holding the module lock.
	 *
	 * Upon successful completion, 0 must be returned.
	 * Otherwise, error number defined in errno.h must be returned.
	 */
	int		(*ctor)(pmodule_t *mod, pfc_cptr_t arg);

	/*
	 * Destructor of the module, which will be called just before it is
	 * unloaded from memory.
	 * Any resource allocated by the constructor should be released.
	 * Note that this method is called with holding the module lock.
	 */
	void		(*dtor)(pmodule_t *mod);

	/*
	 * Call initializer of the module.
	 *
	 * Upon successful completion, zero is returned.
	 * -1 is returned if the initializer returns PFC_FALSE.
	 * Otherwise error number which indicates the cause of error is
	 * returned.
	 */
	int		(*init)(pmodule_t *mod);

	/*
	 * Call finalizer of the module.
	 */
	pfc_bool_t	(*fini)(pmodule_t *mod);

	/*
	 * Register IPC service of the module.
	 *
	 * Upon successful completion, zero is returned.
	 * Otherwise error number which indicates the cause of error is
	 * returned.
	 */
	int		(*ipc_register)(pmodule_t *mod);
} pfc_modops_t;

/*
 * Prototype of pmodule callback function.
 */
typedef pfc_ptr_t	(*pmodcall_t)(pmodule_t *mod, pfc_ptr_t arg);

/*
 * Prototype of pmodule selector function.
 */
typedef pfc_bool_t	(*pmod_selector_t)(pmodule_t *mod);

/*
 * Prototype of pmodule iterator function.
 */
typedef int		(*pmod_iter_t)(pmodule_t *mod, pfc_ptr_t arg);

#ifdef	_PFC_LIBPFC_BUILD

/*
 * Internal prototypes.
 */
extern void		pfc_module_bootstrap(void);
extern void		pfc_module_fini(void);

#endif	/* _PFC_LIBPFC_BUILD */

/*
 * Prototypes.
 */
extern void		pfc_module_init(const char *PFC_RESTRICT dir,
					const char *PFC_RESTRICT confdir,
					const uint8_t *PFC_RESTRICT addr,
					uint32_t size);
extern void		pfc_module_set_started(void);
extern void		pfc_module_type_register(pfc_modtype_t type,
						 const pfc_modops_t *ops);
extern pfc_ptr_t	pfc_module_call(const char *name, pmodcall_t handler,
					pfc_ptr_t arg);
extern pmodule_t	*pfc_module_lookup(const char *name);
extern uint32_t		pfc_module_count(void);
extern int		pfc_module_set_mandatory(const char *name,
						 pfc_bool_t mandatory);
extern void		pfc_module_set_mandatory_all(pfc_bool_t mandatory);
extern int		pfc_module_select(pfc_listm_t list,
					  pmod_selector_t selector,
					  pfc_bool_t reverse);
extern int		pfc_module_iterate(pmod_iter_t iter, pfc_ptr_t arg);
extern pfc_bool_t	pfc_module_select_all(pmodule_t *mod);
extern pfc_bool_t	pfc_module_select_mandatory(pmodule_t *mod);
extern int		pfc_module_set_state(pmodule_t *mod,
					     pmod_state_t state);
extern int		pfc_module_shutdown(pfc_listm_t list);
extern void		pfc_module_cancel(void);

extern void		pfc_module_runhooks(void);

PFC_C_END_DECL

#endif	/* !_PFC_LIBPFC_MODULE_IMPL_H */
