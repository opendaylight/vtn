/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * module.cc - PFC module management for C++ module.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <pfcxx/module.hh>
#include <pfc/rbtree.h>
#include <pfc/synch.h>
#include <pfc/debug.h>
#include <pfc/log.h>
#include <module_impl.h>
#include <ipcsrv_impl.h>
#include <ipcclnt_event.hh>
#include "event_impl.hh"

namespace pfc {
namespace core {

/*
 * Return module instance associated with the specified module descriptor.
 */
#define	PMODULE_INSTANCE(mod)                                   \
    (reinterpret_cast<Module *>(PMODULE_OBJECT_PTR(mod)))

/*
 * Module object entry which keep a pair of module class pointer type name and
 * module instance. This must be defined as POD because it is passed to
 * offsetof().
 */
extern "C" {
    typedef struct {
        const char	*mo_name;	/* type identifier */
        pfc_ptr_t	mo_object;	/* pointer to module instance */
        pfc_rbnode_t	mo_node;	/* Red-Black Tree node */
    } module_object_t;
}

#define	PMODULE_OBJECT_NODE2PTR(node)                           \
    PFC_CAST_CONTAINER((node), module_object_t, mo_node)

/*
 * Internal prototypes.
 */
static int		module_ctor(pmodule_t *mod, pfc_cptr_t data);
static void		module_dtor(pmodule_t *mod);
static int		module_init(pmodule_t *mod);
static pfc_bool_t	module_fini(pmodule_t *mod);
static int		module_ipc_register(pmodule_t *mod);
static pfc_ipcresp_t	module_ipc_handler(pfc_ipcsrv_t *srv,
                                           pfc_ipcid_t service, pfc_ptr_t arg);
static pfc_ptr_t	module_instance_callback(pmodule_t *mod, pfc_ptr_t arg);
static pfc_cptr_t	module_object_getkey(pfc_rbnode_t *node);

/*
 * Module operations for modules written in C++ language.
 */
static const pfc_modops_t  module_cxx_ops = {
    module_ctor,		/* ctor */
    module_dtor,		/* dtor */
    module_init,		/* init */
    module_fini,		/* fini */
    module_ipc_register,	/* ipc_register */
};

/*
 * Interface to register C++ module.
 */
struct module_cxx {
    const char		*cxx_tname;	/* type name of module object pointer */
    pfc_modfac_t	cxx_factory;	/* instance factory function */

    module_cxx(const char *tname, pfc_modfac_t factory)
        : cxx_tname(tname), cxx_factory(factory)
    {
    }
};

/*
 * Red-Black Tree which keeps pairs of name of module class pointer and module
 * instance. Any access to this tree must be done with holding the global
 * module object lock.
 */
static pfc_rbtree_t	module_objtree =
    PFC_RBTREE_INITIALIZER((pfc_rbcomp_t)strcmp, module_object_getkey);

/*
 * The global module object lock.
 */
static pfc_rwlock_t	module_objlock = PFC_RWLOCK_INITIALIZER;

#define	PMODULE_OBJECT_RDLOCK()		\
    PFC_ASSERT_INT(pfc_rwlock_rdlock(&module_objlock), 0)
#define	PMODULE_OBJECT_WRLOCK()		\
    PFC_ASSERT_INT(pfc_rwlock_wrlock(&module_objlock), 0)
#define	PMODULE_OBJECT_UNLOCK()		\
    PFC_ASSERT_INT(pfc_rwlock_unlock(&module_objlock), 0)

/*
 * static Module *
 * Module::getInstance(const char *name)
 *	Return the module instance associated with the given module name.
 *	NULL is returned if not found.
 */
Module *
Module::getInstance(const char *name)
{
    pfc_ptr_t ptr(pfc_module_call(name, module_instance_callback, NULL));

    return reinterpret_cast<Module *>(ptr);
}

/*
 * static Module *
 * getInstanceByType(const char *tname)
 *	Return the module instance associated with the given pointer type name.
 *	The given name must be the name of module class pointer.
 *	NULL is returned if not found.
 */
Module *
Module::getInstanceByType(const char *tname)
{
    PMODULE_OBJECT_RDLOCK();
    pfc_rbnode_t	*node(pfc_rbtree_get(&module_objtree, tname));

    Module	*m;
    if (PFC_EXPECT_TRUE(node != NULL)) {
        module_object_t	*mop(PMODULE_OBJECT_NODE2PTR(node));
        m = reinterpret_cast<Module *>(mop->mo_object);
    }
    else {
        m = reinterpret_cast<Module *>(NULL);
    }
    PMODULE_OBJECT_UNLOCK();

    return m;
}

/*
 * Module::Module(const pfc_modattr_t *attr)
 *	Constructor of abstract module class.
 */
Module::Module(const pfc_modattr_t *attr)
    : _attr(attr), _name(_attr->pma_name)
{
    pmodule_t	*mod(pfc_module_lookup(attr->pma_name));
    PFC_ASSERT(mod != NULL);

    _module = mod->pm_id;
}

/*
 * Module::~Module(void)
 *	Destructor of abstract module class.
 */
Module::~Module(void)
{
}

/*
 * int
 * Module::addEventHandler(pfc_evhandler_t &id, event_handler_t &handler,
 *			   const pfc_evmask_t *maskp, uint32_t priority,
 *			   const char *hname)
 *	Register module-specific event handler for this module.
 *
 * Calling/Exit State:
 *	Upon successful completion, event handler ID is set to `id',
 *	and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
Module::addEventHandler(pfc_evhandler_t &id, event_handler_t &handler,
                        const pfc_evmask_t *maskp, uint32_t priority,
                        const char *hname)
{
    /* Copy event handler object. */
    try {
        event_handler_t	*func(new event_handler_t(handler));
        pfc_evhandler_t	hid;

        int err(__pfc_module_event_add(getModuleId(), &hid, event_cxx_handler,
                                       func, maskp, priority, hname));
        if (PFC_EXPECT_FALSE(err != 0)) {
            delete func;
        }
        else {
            id = hid;
        }

        return err;
    }
    catch (const std::exception &ex) {
        pfc_log_error("%s: Failed to copy module-specific event handler: %s",
                      name(), ex.what());

        return ENOMEM;
    }
    PFCXX_CATCH_ALL() {
        pfc_log_error("%s: Failed to copy module-specific event handler.",
                      name());
    }

    return EINVAL;
}

/*
 * int
 * Module::postEvent(const char *target, pfc_evtype_t type)
 *	Post a module-specific event to the module specified by `target'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
Module::postEvent(const char *target, pfc_evtype_t type)
{
    return __pfc_module_event_post(getModuleId(), target, type);
}

/*
 * int
 * Module::addIpcEventHandler(const char *channel,
 *                            ipc::IpcEventHandler *handler,
 *                            const ipc::IpcEventAttr *attr)
 *      Add an IPC event handler which receives events generated by the IPC
 *      channel specified by `channel'.
 *
 *      `channel' is a pointer to IPC channel name, not IPC channel address.
 *      The IPC server's address part in `channel' is simply ignored.
 *      If NULL is specified, the channel name of the current default
 *      connection is used.
 *
 *      `attr' is a pointer to event attributes object which determines
 *      behavior of event handler. If NULL is specified to `attr', an event
 *      handler is added with default attributes.
 *
 * Calling/Exit State:
 *      Upon successful completion, zero is returned.
 *      Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *      `handler' is deleted on error except for EEXIST error.
 */
int
Module::addIpcEventHandler(const char *channel, ipc::IpcEventHandler *handler,
                           const ipc::IpcEventAttr *attr)
{
    if (PFC_EXPECT_FALSE(handler == NULL)) {
        return EINVAL;
    }

    ipc_evattr_t     newattr;
    pfc_ipcevfunc_t  wrapper;
    ipc::setup_wrapper(wrapper, newattr, handler, attr);

    const char       *name(handler->getName());
    pfc_ipcevhdlr_t  *idp(ipc::get_id_pointer(handler));

    return __pfc_module_ipcevent_addhdlr(_module, idp, channel, wrapper,
                                         PFC_IPCEVATTR_PTR(&newattr), name);
}

/*
 * int
 * Module::removeIpcEventHandler(pfc_ipcevhdlr_t id)
 *      Remove the IPC event handler associated with the specified ID.
 *
 * Calling/Exit State:
 *      Upon successful completion, zero is returned.
 *      Otherwise error number which indicates the cause of error is returned.
 */
int
Module::removeIpcEventHandler(pfc_ipcevhdlr_t id)
{
    return __pfc_module_ipcevent_rmhdlr(_module, id);
}

/*
 * pfc_ipcresp_t
 * Module::ipcService(ipc::ServerSession &sess, pfc_ipcid_t service)
 *	IPC service handler of the module.
 *	This method must be overridden if the module provides IPC services.
 *
 * Calling/Exit State:
 *	This method is always returns PFC_IPCRESP_FATAL.
 */
pfc_ipcresp_t
Module::ipcService(ipc::ServerSession &sess, pfc_ipcid_t service)
{
    pfc_log_error("%s: IPC service is not implemented.", name());

    return PFC_IPCRESP_FATAL;
}

/*
 * pfc_module_t
 * module_register(const pfc_modattr_t *mattr, pfc_modfac_t factory,
 *		   const char *tname)
 *	Register C++ module.
 */
pfc_module_t
module_register(const pfc_modattr_t *mattr, pfc_modfac_t factory,
                const char *tname)
{
    const module_cxx data(tname, factory);

    return pfc_module_register(mattr, reinterpret_cast<pfc_cptr_t>(&data));
}

/*
 * static int
 * module_ctor(pmodule_t *mod, pfc_cptr_t data)
 *	Constructor of the C++ module.
 *
 * Remarks:
 *	This function is called with holding the module lock.
 */
static int
module_ctor(pmodule_t *mod, pfc_cptr_t data)
{
    const module_cxx	*cxx(reinterpret_cast<const module_cxx *>(data));
    const char		*tname(cxx->cxx_tname);
    pfc_modfac_t	factory(cxx->cxx_factory);

    PFC_ASSERT(tname != NULL);
    PFC_ASSERT(factory != NULL);

    PMODULE_OBJECT_TNAME(mod) = tname;
    PMODULE_OBJECT_FACTORY(mod) = factory;
    PMODULE_OBJECT_PTR(mod) = NULL;

    return 0;
}

/*
 * static void
 * module_dtor(pmodule_t *mod)
 *	Destructor of the C++ module.
 *
 * Remarks:
 *	This function is called with holding the module lock.
 */
static void
module_dtor(pmodule_t *mod)
{
    Module	*m(PMODULE_INSTANCE(mod));

    if (PFC_EXPECT_TRUE(m != NULL)) {
        delete m;
        PMODULE_OBJECT_PTR(mod) = NULL;
    }
}

/*
 * static int
 * module_init(pmodule_t *mod)
 *	Call initializer of the specified module.
 *	The module type must be PFC_MODTYPE_CXX.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	-1 is returned if the initializer returns PFC_FALSE.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
module_init(pmodule_t *mod)
{
    pfc_modfac_t	factory(PMODULE_OBJECT_FACTORY(mod));

    /* Delete old module instance. */
    Module	*old(PMODULE_INSTANCE(mod));
    if (old != NULL) {
        delete old;
        PMODULE_OBJECT_PTR(mod) = NULL;
    }

    const pfc_modattr_t	*mattr(mod->pm_attr);

    /* Allocate a module object entry. */
    module_object_t	*mop(reinterpret_cast<module_object_t *>
                             (malloc(sizeof(module_object_t))));
    if (PFC_EXPECT_FALSE(mop == NULL)) {
        pfc_log_error("%s: failed to allocate module object entry.",
                      mattr->pma_name);

        return ENOMEM;
    }

    /* Instantiate module. */
    Module	*m;

    try {
        m = reinterpret_cast<Module *>((*factory)(mattr));
    }
    catch (const std::exception &ex) {
        pfc_log_error("%s: failed to instantiate module: %s",
                      mattr->pma_name, ex.what());
        free(mop);

        return ENOMEM;
    }
    PFCXX_CATCH_ALL() {
        pfc_log_error("%s: caught unexpected exception while creating module "
                      "instance.", PMODULE_NAME(mod));
        free(mop);

        return EFAULT;
    }

    if (PFC_EXPECT_FALSE(m == NULL)) {
        pfc_log_error("%s: NULL has been returned by factory function.",
                      PMODULE_NAME(mod));
        free(mop);

        return EINVAL;
    }

    /* Register module instance to typename hash. */
    mop->mo_name = PMODULE_OBJECT_TNAME(mod);
    mop->mo_object = m;

    PMODULE_OBJECT_WRLOCK();
    int	err(pfc_rbtree_put(&module_objtree, &mop->mo_node));
    PMODULE_OBJECT_UNLOCK();
    if (PFC_EXPECT_FALSE(err != 0)) {
        pfc_log_error("%s: failed to register object to object tree: %s",
                      mattr->pma_name, strerror(err));
        delete m;
        free(mop);

        return err;
    }

    PMODULE_OBJECT_PTR(mod) = m;

    /* Call init() method. */
    PFCXX_TRY_ON_RELEASE() {
        if (PFC_EXPECT_TRUE(m->init())) {
            return 0;
        }
    }
    PFCXX_CATCH_ON_RELEASE(ex) {
        pfc_log_error("%s: caught exception while calling init(): %s",
                      m->name(), ex.what());
    }
    PFCXX_CATCH_ALL_ON_RELEASE() {
        pfc_log_error("%s: caught unexpected exception while calling init()",
                      m->name());
    }

    PMODULE_OBJECT_WRLOCK();
    pfc_rbtree_remove_node(&module_objtree, &mop->mo_node);
    PMODULE_OBJECT_UNLOCK();
    PMODULE_OBJECT_PTR(mod) = NULL;
    delete m;
    free(mop);

    return -1;
}

/*
 * static pfc_bool_t
 * module_fini(pmodule_t *mod)
 *	Call finalizer of the specified module.
 *	The module type must be PFC_MODTYPE_CXX.
 */
static pfc_bool_t
module_fini(pmodule_t *mod)
{
    Module	*m(PMODULE_INSTANCE(mod));
    pfc_bool_t	ret(PFC_FALSE);

    PFC_ASSERT(m != NULL);

    PFCXX_TRY_ON_RELEASE() {
        ret = m->fini();
    }
    PFCXX_CATCH_ON_RELEASE(ex) {
        pfc_log_error("%s: caught exception while calling fini(): %s",
                      m->name(), ex.what());
    }
    PFCXX_CATCH_ALL_ON_RELEASE() {
        pfc_log_error("%s: caught unexpected exception while calling fini()",
                      m->name());
    }

    /*
     * Remove the module instance from module_objtree irrespective of what
     * fini() has returned.
     */
    PMODULE_OBJECT_WRLOCK();
    pfc_rbnode_t	*node(pfc_rbtree_remove(&module_objtree,
                                                PMODULE_OBJECT_TNAME(mod)));
    PMODULE_OBJECT_UNLOCK();
    if (PFC_EXPECT_FALSE(node == NULL)) {
        /* This should never happen. */
        pfc_log_warn("%s: not found in module object tree.", m->name());
    }
    else {
        module_object_t	*mop(PMODULE_OBJECT_NODE2PTR(node));
        free(mop);
    }

    return ret;
}

/*
 * static int
 * module_ipc_register(pmodule_t *mod)
 *	Register IPC service handler of the module.
 *	The module type must be PFC_MODTYPE_CXX.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
module_ipc_register(pmodule_t *mod)
{
    const pfc_modattr_t	*mattr(mod->pm_attr);

    if (!PMODULE_HAS_IPC(mattr)) {
        pfc_log_debug("%s: IPC service is not provided", PMODULE_NAME(mod));

        return 0;
    }

    PFC_ASSERT(mattr->pma_ipchdlr == NULL);

    int	err(pfc_ipcsrv_add_handler_impl(mod->pm_name, mattr->pma_nipcs,
                                        module_ipc_handler,
                                        PMODULE_OBJECT_PTR(mod), NULL));

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
 * static pfc_ipcresp_t
 * module_ipc_handler(pfc_ipcsrv_t *srv, pfc_ipcid_t service, pfc_ptr_t arg)
 *	IPC service handler of C++ module.
 *
 * Calling/Exit State:
 *	This function returns the response of the IPC service handler of
 *	this module.
 */
static pfc_ipcresp_t
module_ipc_handler(pfc_ipcsrv_t *srv, pfc_ipcid_t service, pfc_ptr_t arg)
{
    Module	*m(reinterpret_cast<Module *>(arg));
    ipc::ServerSession	sess(srv);

    PFCXX_TRY_ON_RELEASE() {
        return m->ipcService(sess, service);
    }
    PFCXX_CATCH_ON_RELEASE(ex) {
        pfc_log_error("%s: Unexpected exception while calling IPC service: %s",
                      m->name(), ex.what());
    }
    PFCXX_CATCH_ALL_ON_RELEASE() {
        pfc_log_error("%s: Unexpected exception while calling IPC service.",
                      m->name());
    }

    return PFC_IPCRESP_FATAL;
}

/*
 * static pfc_ptr_t
 * module_instance_callback(pmodule_t *mod, pfc_ptr_t arg PFC_ATTR_UNUSED)
 *	Return the module instance associated with the given module descriptor.
 *	This function is called via pfc_module_call().
 */
static pfc_ptr_t
module_instance_callback(pmodule_t *mod, pfc_ptr_t arg PFC_ATTR_UNUSED)
{
    if (PFC_EXPECT_FALSE(!PMODULE_IS_LOADED(mod))) {
        /* Not loaded. */
        return NULL;
    }

    return reinterpret_cast<pfc_ptr_t>(PMODULE_OBJECT_PTR(mod));
}

/*
 * static pfc_cptr_t
 * module_object_getkey(pfc_rbnode_t *node)
 *	Return the key of module object entry specified by `node'.
 *	`node' must be a pointer to mo_node in module_object_t.
 */
static pfc_cptr_t
module_object_getkey(pfc_rbnode_t *node)
{
    module_object_t	*mop(PMODULE_OBJECT_NODE2PTR(node));

    return mop->mo_name;
}

/*
 * static void PFC_FATTR_INIT
 * module_libinit(void)
 *	Initialize PFC C++ module subsystem.
 */
static void PFC_FATTR_INIT
module_libinit(void)
{
    pfc_module_type_register(PFC_MODTYPE_CXX, &module_cxx_ops);
}

}	// core
}	// pfc
