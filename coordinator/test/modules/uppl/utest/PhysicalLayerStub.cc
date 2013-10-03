#include "PhysicalLayerStub.hh"
using namespace unc::uppl;
void PhysicalLayerStub :: loadphysicallayer() {

   pfc_modattr_t* modattr;
   PhysicalLayer = new unc::uppl::PhysicalLayer(modattr);
}

IPCConnectionManager* PhysicalLayerStub::get_ipc_connection_manager() {
  return new IPCConnectionManager;
}

void PhysicalLayerStub :: unloadphysicallayer() {

   PhysicalLayer = NULL;
}


