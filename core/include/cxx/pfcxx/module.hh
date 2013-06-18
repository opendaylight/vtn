/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFCXX_MODULE_HH
#define	_PFCXX_MODULE_HH

/*
 * Definitions for C++ module management.
 */

#include <new>
#include <string>
#include <typeinfo>
#include <exception>
#include <boost/noncopyable.hpp>
#include <pfc/module.h>
#include <pfc/log.h>
#include <pfcxx/conf.hh>
#include <pfcxx/event.hh>
#include <pfcxx/ipc_server.hh>
#include <pfcxx/ipc_client.hh>

namespace pfc {
namespace core {

/*
 * Abstract class for module instance.
 * Each module has one Module instance.
 */
class Module
    : boost::noncopyable
{
public:
    Module(const pfc_modattr_t *attr);
    virtual ~Module(void);

    /*
     * Return the module instance associated with the given module name.
     * NULL is returned if not found.
     */
    static Module	*getInstance(const char *name);

    /*
     * Return the module instance associated with the given pointer type name.
     * NULL is returned if not found.
     */
    static Module	*getInstanceByType(const char *tname);

    /*
     * Return PFC_TRUE if the given event object is a module specific event.
     */
    static inline pfc_bool_t
    isModuleEvent(Event *eobj)
    {
        if (PFC_EXPECT_FALSE(eobj == NULL)) {
            return PFC_FALSE;
        }

        return pfc_module_is_modevent(eobj->getEvent());
    }

    /*
     * Return a string which represents the sender of the given event object.
     *
     * `eobj' must be a pointer to a module specific event object.
     * Specifying a non module specific event object results undefined
     * behavior.
     */
    static inline const char *
    getEventSender(Event *eobj)
    {
        return pfc_module_event_getsender(eobj->getEvent());
    }

    /*
     * Start the module service.
     *
     * This method is called once after the module is loaded into the PFC
     * system. If this method returns PFC_FALSE, the module load is failed.
     */
    virtual pfc_bool_t init(void) = 0;

    /*
     * Stop the module service.
     *
     * This method is called once just before the module is unloaded from
     * the PFC system. If this function returns PFC_FALSE, the module is not
     * unloaded from the memory, but its service is stopped.
     */
    virtual pfc_bool_t fini(void) = 0;

    /*
     * Get pointer to the module instance, and set it into the specified
     * pointer. NULL is set if the module is not found.
     */
    template <class T>
    inline void getInstance(T *&ptr)
    {
        Module	*m(Module::getInstanceByType(typeid(ptr).name()));

        ptr = dynamic_cast<T *>(m);
    }

    /*
     * Return module name in C-styled format.
     */
    inline const char *name(void) const
    {
        return _attr->pma_name;
    }

    /*
     * Return module name.
     */
    inline std::string getName(void) const
    {
        return _name;
    }

    /*
     * Return module identifier.
     */
    inline pfc_module_t getModuleId(void) const
    {
        return _module;
    }

    /*
     * Return module version.
     */
    inline uint8_t getVersion(void) const
    {
        return _attr->pma_version;
    }

    /*
     * Return module system version.
     */
    inline uint8_t getSystemVersion(void) const
    {
        return _attr->pma_sysversion;
    }

    /*
     * Return basic attributes of the module.
     * This method is provided only for PFC module subsystem.
     */
    inline const pfc_modattr_t *getAttribute(void) const
    {
        return _attr;
    }

    /*
     * Create a list model instance which contains all map keys of the
     * parameter map specified by `mname' in the module configuration file.
     * Zero is returned on success, error number on failure.
     */
    inline int
    getConfMapKeys(const char *mname, pfc_listm_t &keys)
    {
        return __pfc_module_conf_getmapkeys(_module, mname, &keys);
    }

    inline int
    getConfMapKeys(const std::string &mname, pfc_listm_t &keys)
    {
        return __pfc_module_conf_getmapkeys(_module, mname.c_str(), &keys);
    }

    /*
     * Reload configuration file for this module.
     * Zero is returned on success, error number on failure.
     * Do nothing if this module doesn't use module configuration file.
     */
    inline int
    reloadConf(void)
    {
        return __pfc_module_conf_reload(_module);
    }

    /*
     * Add an event handler which receives module-specific events posted to
     * this module.
     * The handler added by this method will receive all events posted to
     * this module.
     *
     * If `hname' is specified, it is used as event handler's name.
     */
    inline int
    addEventHandler(pfc_evhandler_t &id, event_handler_t &handler,
                    uint32_t priority = Event::DEFAULT_PRIORITY,
                    const char *hname = PFC_LOG_IDENT)
    {
        return addEventHandler(id, handler, NULL, priority, hname);
    }

    /*
     * Add an event handler which receives module-specific events posted to
     * this module.
     * The handler added by this method will receive events which has the
     * event type specified by the event mask bits.
     *
     * If `hname' is specified, it is used as event handler's name.
     */
    inline int
    addEventHandler(pfc_evhandler_t &id, event_handler_t &handler,
                    EventMask &mask,
                    uint32_t priority = Event::DEFAULT_PRIORITY,
                    const char *hname = PFC_LOG_IDENT)
    {
        return addEventHandler(id, handler, mask.getMask(), priority, hname);
    }

    /*
     * Remove the event handler associated with the specified handler ID.
     */
    inline int
    removeEventHandler(pfc_evhandler_t id,
                       const pfc_timespec_t *timeout = NULL)
    {
        return pfc_event_remove_handler(id, timeout);
    }

    /*
     * Post a module-specific event of the type specified by `type'.
     * The event is delivered to only handlers added by the module specified
     * by `target'.
     */
    int	postEvent(const char *target, pfc_evtype_t type);

    /*
     * Post a module-specific event of the type specified by `type'.
     * This method specifies the target module by std::string.
     */
    inline int
    postEvent(const std::string &target, pfc_evtype_t type)
    {
        return postEvent(target.c_str(), type);
    }

    /*
     * Add an IPC event handler which receives events generated by the IPC
     * channel specified by `channel'.
     */
    int addIpcEventHandler(const char *channel, ipc::IpcEventHandler *handler,
                           const ipc::IpcEventAttr *attr = NULL);

    /*
     * Remove the IPC event handler associated with the specified ID.
     */
    int removeIpcEventHandler(pfc_ipcevhdlr_t id);

    /*
     * Remove the specified IPC event handler.
     */
    inline int
    removeIpcEventHandler(ipc::IpcEventHandler *handler)
    {
        if (PFC_EXPECT_FALSE(handler == NULL)) {
            return EINVAL;
        }

        return removeIpcEventHandler(handler->getId());
    }

    /*
     * IPC service handler.
     * This method must be overridden if the module provides IPC services.
     *
     * `sess' is an instance of IPC server session. Argument sent by the client
     * can be obtained via this instance, and arbitrary data can be sent to
     * the client via this instance.
     *
     * `service' is an identifier of the IPC service specified by the client.
     *
     * Calling/Exit State:
     *     If PFC_IPCRESP_FATAL (-1) is returned by this method, the IPC
     *     framework considers as fatal error. In this case, any data added
     *     to the IPC service session is discarded.
     *
     *     A value other than PFC_IPCRESP_FATAL is treated as response of the
     *    IPC service, and it is sent to the client.
     */
    virtual pfc_ipcresp_t	ipcService(ipc::ServerSession &sess,
                                           pfc_ipcid_t service);

private:
    /* Register module-specific event handler. */
    int addEventHandler(pfc_evhandler_t &id, event_handler_t &handler,
                        const pfc_evmask_t *maskp, uint32_t priority,
                        const char *hname);

    /* Module descriptor. */
    const pfc_modattr_t	*_attr;

    /* Module name */
    std::string _name;

    /* Module identifier */
    pfc_module_t _module;
};

/*
 * Prototypes.
 */
extern pfc_module_t	module_register(const pfc_modattr_t *mattr,
                                        pfc_modfac_t factory,
                                        const char *tname);

/*
 * PFC_MODULE_BUILD is defined by the build system only for PFC module build.
 */
#ifdef	PFC_MODULE_BUILD

/*
 * Declare C++ module.
 * `nipcs' is the number of IPC services provided by this module.
 * Zero means no IPC service is provided.
 */
#define	PFC_MODULE_IPC_DECL(module_class, nipcs)                        \
    static void *                                                       \
    __module_factory(const pfc_modattr_t *mattr)                        \
    {                                                                   \
        module_class	*obj(new module_class(mattr));                  \
        void	*ptr(dynamic_cast<pfc::core::Module *>(obj));           \
                                                                        \
        if (PFC_EXPECT_FALSE(ptr == NULL)) {                            \
            delete obj;                                                 \
        }                                                               \
                                                                        \
        return ptr;                                                     \
    }                                                                   \
                                                                        \
    static void  PFC_FATTR_INIT                                         \
    __module_init(void)                                                 \
    {                                                                   \
        __PFC_MODULE_ATTR_DECL(mattr, PFC_MODTYPE_CXX, NULL, nipcs);    \
                                                                        \
        ::__pfc_this_module_name = mattr.pma_name;                      \
                                                                        \
        const char *tname(typeid(module_class *).name());               \
        ::__pfc_this_module_id =                                        \
              pfc::core::module_register(&mattr, __module_factory, tname); \
    }                                                                   \
                                                                        \
    pfc_module_t  __pfc_this_module_id PFC_ATTR_HIDDEN = PFC_MODULE_INVALID; \
    const char  *__pfc_this_module_name PFC_ATTR_HIDDEN

/*
 * Declare C++ module without IPC service.
 */
#define	PFC_MODULE_DECL(module_class)           \
	PFC_MODULE_IPC_DECL(module_class, 0)

/*
 * The ModuleConfBlock instance represents a bunch of parameters defined in
 * the module configuration file.
 * This class is only available in module source code.
 */
class ModuleConfBlock
    : public ConfBlock
{
public:
    /*
     * Create a block handle instance associated with a parameter block
     * in the module configuration file, specified by the block name.
     */
    ModuleConfBlock(const char *bname)
        : ConfBlock(pfc_module_conf_getblock(bname)) {}

    ModuleConfBlock(const std::string &bname)
        : ConfBlock(pfc_module_conf_getblock(bname.c_str())) {}

    /*
     * Create a block handle instance associated with a parameter map block
     * in the module configuration file, specified by the map name and its key.
     */
    ModuleConfBlock(const char *mname, const char *key)
        : ConfBlock(pfc_module_conf_getmap(mname, key)) {}

    ModuleConfBlock(const std::string &mname, const std::string &key)
        : ConfBlock(pfc_module_conf_getmap(mname.c_str(), key.c_str())) {}
};

#endif	/* PFC_MODULE_BUILD */

}	// core
}	// pfc

#endif	/* !_PFCXX_MODULE_HH */
