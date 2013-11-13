/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#include <pfc/ipc.h>
#include <netinet/in.h>
#include <string>
#include <map>
#include <list>
#include <pfc/ipc_client.h>
#include <pfcxx/ipc_server.hh>

#ifndef _PFCXX_IPC_CLIENT_HH
#define _PFCXX_IPC_CLIENT_HH

namespace pfc {
namespace core {
namespace ipc {

class ClientSession
{
    friend class  ::pfc::core::ipc::ServerSession;

public:
    /*
     * Constructors.
     */
    ClientSession(const char *name, pfc_ipcid_t service, int &err);
    ClientSession(const char *name, pfc_ipcid_t service, int &err,
                  uint32_t flags);
    ClientSession(const std::string &name, pfc_ipcid_t service, int &err);
    ClientSession(const std::string &name, pfc_ipcid_t service, int &err,
                  uint32_t flags);
    ClientSession(pfc_ipcconn_t conn, const char *name, pfc_ipcid_t service,
                  int &err);
    ClientSession(pfc_ipcconn_t conn, const char *name, pfc_ipcid_t service,
                  int &err, uint32_t flags);
    ClientSession(pfc_ipcconn_t conn, const std::string &name,
                  pfc_ipcid_t service, int &err);
    ClientSession(pfc_ipcconn_t conn, const std::string &name,
                  pfc_ipcid_t service, int &err, uint32_t flags);
   ClientSession(pfc_ipcsess_t *sess);
    explicit ClientSession();

    /*
     * Destructor.
     */
    ~ClientSession();

    /*
     * Instance methods.
     */
    int   reset(const char *name, pfc_ipcid_t service);
    int   reset(const std::string &name, pfc_ipcid_t service);

    int   setTimeout(const pfc_timespec_t *timeout=NULL);
    int   invoke(pfc_ipcresp_t &response);
    int   cancel(pfc_bool_t discard=PFC_FALSE);
    int   forward(ClientSession &sess, uint32_t begin = 0,
                  uint32_t end = UINT32_MAX);
    int   forward(ServerSession &sess, uint32_t begin = 0,
                  uint32_t end = UINT32_MAX);
    int   forwardTo(ServerSession &sess, uint32_t begin = 0,
                    uint32_t end = UINT32_MAX);

    int   addOutput(int8_t data);
    int   addOutputInt8(int8_t data);
    int   addOutput(uint8_t data);
    int   addOutputUint8(uint8_t data);
    int   addOutput(int16_t data);
    int   addOutputInt16(int16_t data);
    int   addOutput(uint16_t data);
    int   addOutputUint16(uint16_t data);
    int   addOutput(int32_t data);
    int   addOutputInt32(int32_t data);
    int   addOutput(uint32_t data);
    int   addOutputUint32(uint32_t data);
    int   addOutput(int64_t data);
    int   addOutputInt64(int64_t data);
    int   addOutput(uint64_t data);
    int   addOutputUint64(uint64_t data);
    int   addOutput(float data);
    int   addOutputFloat(float data);
    int   addOutput(double data);
    int   addOutputDouble(double data);
    int   addOutput(struct in_addr &data);
    int   addOutput(struct in6_addr &data);
    int   addOutput(const char *data);
    int   addOutput(const std::string &data);
    int   addOutput(const uint8_t *data, uint32_t length);
    int   addOutput(const pfc_ipcstdef_t &def, pfc_cptr_t data);
    int   addOutput(void);
    int   addOutput(key_ctr& data);
    int   addOutput(val_ctr& data);
    int   addOutput(key_boundary_t& data);
    int addOutput(key_logical_port data);
    int addOutput(val_logical_port& data);
    int addOutput(val_logical_port_st& data);

    int   getResponse(uint32_t index, int8_t &data);
    int   getResponse(uint32_t index, uint8_t &data);
    int   getResponse(uint32_t index, int16_t &data);
    int   getResponse(uint32_t index, uint16_t &data);
    int   getResponse(uint32_t index, int32_t &data);
    int   getResponse(uint32_t index, uint32_t &data);
    int   getResponse(uint32_t index, int64_t &data);
    int   getResponse(uint32_t index, uint64_t &data);
    int   getResponse(uint32_t index, float &data);
    int   getResponse(uint32_t index, double &data);
    int   getResponse(uint32_t index, struct in_addr &data);
    int   getResponse(uint32_t index, struct in6_addr &data);
    int   getResponse(uint32_t index, const char *&data);
    int   getResponse(uint32_t index, const uint8_t *&data, uint32_t &length);
    int   getResponse(uint32_t index, const pfc_ipcstdef_t &def,
                      pfc_ptr_t datap);
    int   getResponse(uint32_t index, key_ctr& data);
    int   getResponse(uint32_t index, val_phys_path_fault_alarm_t& data);
    int   getResponse(uint32_t index, key_ctr_domain_t& data);
    int   getResponse(uint32_t index, key_switch_t& data);
    int   getResponse(uint32_t index, key_port_t& data);
    int   getResponse(uint32_t index, val_port_st& data);
    int   getResponse(uint32_t index, val_switch_st& data);
    int   getResponse(uint32_t index, key_link_t& data);
    int   getResponse(uint32_t index, val_link_st& data);
    int   getResponse(uint32_t index, val_ctr_st& data);
    int   getResponse(uint32_t index, val_ctr_domain_st& data);
    int   getResponse(uint32_t index, key_logical_port_t& data);
    int   getResponse(uint32_t index, val_logical_port_st& data);
    int   getResponse(uint32_t index, key_logical_member_port_t& data);

    uint32_t getResponseCount(void);
    int getResponseType(uint32_t index, pfc_ipctype_t& type);

    int   getResponseStructName(uint32_t index, const char *&name);
    int   getResponseStructName(uint32_t index, std::string &name);

    //stubs
    static void stub_setResponseStructName(uint32_t index, std::string &name);
    static void clearStubData(void);
    static void stub_setResponse(int result);
    static void stub_setResponsetype(uint32_t index,pfc_ipctype_t ipctype );
    static void stub_setClientSessionErrorCode(int result);
    static void stub_setAddOutput(int result);
    static void stub_setAddOutput(uint32_t result);
    static void stub_setAddOutput(const char* data);
    static void stub_setResponseCount(int argCount);
    static void stub_setResponse( uint32_t index,uint32_t value );
    static void stub_setinvoke(pfc_ipcresp_t ipcresp,int err);

private:
// TODO add comments
// TODO keep only modifed files; rest take it from system directory
// TODO keep all stubs inside stub directory
// TODO all changes prefix/suffic with stub_/_stub.
    //stub data
    static std::map<uint32_t,pfc_ipctype_t > arg_parameters;
    static std::map<uint32_t,std::string> structNameMap;//<index,structname>
    static std::map<uint32_t,uint32_t> arg_map;
    static std::list<uint32_t> add_output_list;//list of addoutput values
    static int addOutPut_;
    static int responseResult_;
    static int ClientsesErrorCode_;
    static int argCount_;
    static pfc_ipcresp_t ipcresp_;
    static int err_;
    static std::list<const char*> add_output_str;
};

class IpcEventMask
{

public:
    /* Create an event mask which contains all IPC event types. */
    IpcEventMask(): _mask(PFC_IPC_EVENT_MASK_FILL) {}

    /* Create an event mask which contains the specified IPC event type. */
    explicit IpcEventMask(pfc_ipcevtype_t type)
        : _mask(PFC_IPC_EVENT_MASK_BIT(type)) {}

    /* Copy constructor. */
    explicit IpcEventMask(const IpcEventMask &mask) : _mask(mask._mask) {}


    inline void
    empty(void)
    {

    }


    inline void
    fill(void)
    {

    }

    inline int
    add(pfc_ipcevtype_t type)
    {
        return 0;
    }

    inline int
    remove(pfc_ipcevtype_t type)
    {
        return 0;
    }

    inline pfc_bool_t
    test(pfc_ipcevtype_t type) const
    {
        return true;
    }
    inline const pfc_ipcevmask_t *
    getMask(void) const
    {
        return &_mask;
    }

private:
    /* Event mask value. */
    pfc_ipcevmask_t    _mask;
};

class IpcEventAttr
{
public:
   IpcEventAttr()
   {

   }


   ~IpcEventAttr()
   {

   }


   inline void
   getHostSet(const char *&name) const
   {


   }


   inline int
   setHostSet(const char *name)
   {
       return 0;
   }

   inline void
   getTarget(const char *service, IpcEventMask &mask) const
   {


   }

   inline int
   addTarget(const char *service)
   {
       return 0;
   }

   inline int
   addTarget(const char *service, const IpcEventMask &mask)
   {
       return 0;
   }


   inline void
   resetTarget(void)
   {

   }

   inline void
   getPriority(uint32_t &priority) const
   {

   }

   inline void
   setPriority(uint32_t priority)
   {

   }

   inline void
   getLog(pfc_bool_t &log) const
   {

   }


   inline void
   setLog(pfc_bool_t log)
   {

   }

private:
   pfc_ipcevattr_t   _attr;
};

class IpcEvent
{
public:
    inline pfc_ipcevid_t
    getSerial(void) const
    {
        return pfc_ipcevid_t();
    }

    inline pfc_ipcevtype_t
    getType(void) const
    {
        return pfc_ipcevtype_t();
    }

    inline void
    getTime(pfc_timespec_t &ts) const
    {
        ts=pfc_timespec_t();
    }

    inline const char *
    getChannelName(void) const
    {
        return "channelName";
    }


    inline const pfc_hostaddr_t *
    getHostAddress(void) const
    {
      pfc_hostaddr_t* host((pfc_hostaddr_t*)malloc(sizeof( pfc_hostaddr_t)));
      return host;
    }

    inline const char *
    getServiceName(void) const
    {
        return "serviceName";
    }

    inline pfc_ipcsess_t *
    getSession(void) const
    {
         pfc_ipcsess_t* dummy;
        return dummy;
    }

    inline pfc_bool_t
    isStateChangeEvent(void) const
    {
        return true;
    }

private:
    explicit IpcEvent(pfc_ipcevent_t *event) : _event(event) {}
    pfc_ipcevent_t    *_event;
};

class IpcEventHandler
{
public:
    IpcEventHandler() : _id() {}
    virtual ~IpcEventHandler();
    inline pfc_ipcevhdlr_t
    getId(void) const
    {
        return _id;
    }

    virtual void  eventHandler(const IpcEvent &event) = 0;
    virtual const char  *getName(void);

private:
    pfc_ipcevhdlr_t    _id;
};

extern int  add_event_handler(const char *channel, IpcEventHandler *handler,
                              const IpcEventAttr *attr = NULL);
extern int  remove_event_handler(pfc_ipcevhdlr_t id);
static inline int
remove_event_handler(IpcEventHandler *handler)
{
    return 0;
}

}
}
}
#endif  /* !_PFCXX_IPC_CLIENT_HH */
