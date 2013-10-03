#ifndef _PHYSICAL_LAYER_STUB_HH_
#define _PHYSICAL_LAYER_STUB_HH_

#include <pfc/debug.h>
#include <pfc/log.h>
#include <pfc/hostaddr.h>
#include <pfcxx/module.hh>
#include <pfcxx/ipc_server.hh>
#include <pfcxx/ipc_client.hh>
#include <pfcxx/synch.hh>
#include "physicallayer.hh"
class PhysicalLayerStub : public pfc::core::Module {

public:

static void loadphysicallayer();
static void unloadphysicallayer();
IPCConnectionManager* get_ipc_connection_manager();
};
#endif
