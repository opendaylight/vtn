/*
* Copyright (c) 2012-2013 NEC Corporation
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

#include <pfc/module.h>
#include <pfc/log.h>
#include <cxx/pfcxx/conf.hh>
#include <cxx/pfcxx/event.hh>
#include "cxx/pfcxx/ipc_client.hh"
#include<string>


namespace pfc {
namespace core {

class Module {
public:
    Module(const pfc_modattr_t *attr) {

    }
    virtual ~Module(void) {

    }

    static Module *getInstance(const char *name)
    {
    	return getModule(name);
    }

    static Module *getInstanceByType(const char *tname) {

    	return capaModule;
    }

    static inline pfc_bool_t
    isModuleEvent(Event *eobj)
    {

        return true;
    }


    static inline const char *
    getEventSender(Event *eobj)
    {
        return "Event";
    }

    virtual pfc_bool_t init(void)=0;

    virtual pfc_bool_t fini(void)=0;

    template <class T>
    inline void getInstance(T *&ptr)
    {
        Module	*m(Module::getInstanceByType(typeid(ptr).name()));

        ptr = dynamic_cast<T *>(m);
    }

    inline const char *name(void) const
    {
        return _attr->pma_name;
    }

    inline std::string getName(void) const
    {
        return _name;
    }

    inline pfc_module_t getModuleId(void) const
    {
        return _module;

    }

    inline uint8_t getVersion(void) const
    {
        return _attr->pma_version;
    }


    inline uint8_t getSystemVersion(void) const
    {
        return _attr->pma_sysversion;
    }


    inline const pfc_modattr_t *getAttribute(void) const
    {
        return _attr;
    }


    inline int
    reloadConf(void)
    {
        return 0;
    }

    inline int
    addEventHandler(pfc_evhandler_t &id, event_handler_t &handler,
                    uint32_t priority ,
                    const char *hname = PFC_LOG_IDENT)
    {
        return 0;
    }

    inline int
    addEventHandler(pfc_evhandler_t &id, event_handler_t &handler,
                    EventMask &mask,
                    uint32_t priority,
                    const char *hname = PFC_LOG_IDENT)
    {
        return 0;
    }

    inline int
    removeEventHandler(pfc_evhandler_t id,
                      const pfc_timespec_t *timeout = NULL)
   {
        return 0;
    }

    inline int	postEvent(const char* /*target*/, pfc_evtype_t /*type*/)
    {
    	return 0;
    }

    inline int
    postEvent(std::string &target, pfc_evtype_t type)
    {
        return 0;
    }

    inline int addIpcEventHandler(const char* /*channel*/, ipc::IpcEventHandler* /**handler*/,
                           const ipc::IpcEventAttr *attr = NULL) {
    	return 0;
    }

    int removeIpcEventHandler(pfc_ipcevhdlr_t /*id*/) {
    	return 0;
    }

    inline int
    removeIpcEventHandler(ipc::IpcEventHandler* /**handler*/)
    {

        return 0;
    }

    virtual pfc_ipcresp_t	ipcService(ipc::ServerSession& /*sess*/,
                                           pfc_ipcid_t /*service*/) {
    	return pfc_ipcresp_t();
    }

    //stub Methods
    static Module* getModule(const char* moduleName);

private:
    const pfc_modattr_t	*_attr;
    std::string _name;
    pfc_module_t _module;
protected:
    static Module* capaModule;
    static Module* tcLib;

};

/*
 * Prototypes.
 */
extern pfc_module_t	module_register(const pfc_modattr_t *mattr,
                                        pfc_modfac_t factory,
                                        const char *tname);

}
}
#endif

