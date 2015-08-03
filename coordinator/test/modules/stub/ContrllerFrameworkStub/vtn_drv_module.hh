/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VTNDRVMOD_HH__
#define __VTNDRVMOD_HH__

#include <unc/keytype.h>
#include <pfcxx/ipc_server.hh>
#include <pfc/log.h>
#include <pfcxx/synch.hh>
#include <kt_handler.hh>
#include <controller_fw.hh>
#include <driver/driver_command.hh>
#include <driver/driver_interface.hh>
#include <vtndrvintf_defs.h>
#include <map>

namespace unc {
namespace driver {

class VtnDrvIntf :public pfc::core::Module {
 public:
  typedef std::map<unc_key_type_t, pfc_ipcstdef_t*> kt_map;
  static kt_map key_map;
  static kt_map val_map;
  explicit VtnDrvIntf(const pfc_modattr_t* attr);

  ~VtnDrvIntf() {
  }

  pfc_bool_t init(void) {
    initialize_map();
    return PFC_TRUE;
  }

  void  initialize_map() {
    ODC_FUNC_TRACE;
    uint32_t loop = 0;
    unc_key_type_t KT[] = {UNC_KT_VTN, UNC_KT_VBRIDGE, UNC_KT_VBR_IF,
      UNC_KT_VBR_VLANMAP, UNC_KT_FLOWLIST,
      UNC_KT_FLOWLIST_ENTRY, UNC_KT_VTN_FLOWFILTER,
      UNC_KT_VTN_FLOWFILTER_ENTRY, UNC_KT_VBR_FLOWFILTER,
      UNC_KT_VBR_FLOWFILTER_ENTRY, UNC_KT_VBRIF_FLOWFILTER,
      UNC_KT_VRTIF_FLOWFILTER_ENTRY, UNC_KT_VTERMINAL,
      UNC_KT_VTERM_IF, UNC_KT_VTERMIF_FLOWFILTER,
      UNC_KT_VTERMIF_FLOWFILTER_ENTRY};
    uint32_t kt_size = sizeof KT/sizeof(unc_key_type_t);
    for (loop = 0; loop < kt_size; loop++) {
      switch (KT[loop]) {
        case UNC_KT_VTN:
          {
            pfc_ipcstdef_t *stdef_k = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_k, key_vtn);
            pfc_ipcstdef_t *stdef_v = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_v, val_vtn);
            key_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_k));
            val_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_v));
            break;
          }
        case UNC_KT_VBRIDGE:
          {
            pfc_ipcstdef_t *stdef_kvbr = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_kvbr, key_vbr);
            pfc_ipcstdef_t *stdef_vbr = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_vbr, val_vbr);
            key_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_kvbr));
            val_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_vbr));
            break;
          }
        case UNC_KT_VBR_IF:
          {
            pfc_ipcstdef_t *stdef_kvbrif = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_kvbrif, key_vbr_if);
            pfc_ipcstdef_t *stdef_vbrif = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_vbrif, val_vbr_if);
            key_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_kvbrif));
            val_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_vbrif));
            break;
          }
        case UNC_KT_VBR_VLANMAP:
          {
            pfc_ipcstdef_t *stdef_kvbrvlanmap = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_kvbrvlanmap, key_vlan_map);
            pfc_ipcstdef_t *stdef_vbrvlanmap = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_vbrvlanmap, val_vlan_map);
            key_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_kvbrvlanmap));
            val_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_vbrvlanmap));
            break;
          }
        case UNC_KT_FLOWLIST:
          {
            pfc_ipcstdef_t *stdef_kflowlist = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_kflowlist, key_flowlist);
            pfc_ipcstdef_t *stdef_val_flowlist = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_val_flowlist, val_flowlist);
            key_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_kflowlist));
            val_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_val_flowlist));
            break;
          }
        case UNC_KT_FLOWLIST_ENTRY:
          {
            pfc_ipcstdef_t *stdef_kflowlist_entry = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_kflowlist_entry, key_flowlist_entry);
            pfc_ipcstdef_t *stdef_val_flowlist_entry = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_val_flowlist_entry, val_flowlist_entry);
            key_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_kflowlist_entry));
            val_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_val_flowlist_entry));
            break;
          }
        case UNC_KT_VTN_FLOWFILTER:
          {
            pfc_ipcstdef_t *stdef_kvtn_flowfilter = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_kvtn_flowfilter, key_vtn_flowfilter);
            pfc_ipcstdef_t *stdef_val_vtn_flowfilter = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_val_vtn_flowfilter, val_flowfilter);
            key_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_kvtn_flowfilter));
            val_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_val_vtn_flowfilter));
            break;
          }
        case UNC_KT_VTN_FLOWFILTER_ENTRY:
          {
            pfc_ipcstdef_t *stdef_kvtn_flowfilter_entry = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_kvtn_flowfilter_entry,
                               key_vtn_flowfilter_entry);
            pfc_ipcstdef_t *stdef_val_vtn_flowfilter_entry = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_val_vtn_flowfilter_entry,
                               val_vtn_flowfilter_entry);
            key_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_kvtn_flowfilter_entry));
            val_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_val_vtn_flowfilter_entry));
            break;
          }
        case UNC_KT_VBR_FLOWFILTER:
          {
            pfc_ipcstdef_t *stdef_kvbr_flowfilter = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_kvbr_flowfilter, key_vbr_flowfilter);
            pfc_ipcstdef_t *stdef_val_vbr_flowfilter = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_val_vbr_flowfilter, val_flowfilter);
            key_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_kvbr_flowfilter));
            val_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_val_vbr_flowfilter));
            break;
          }
        case UNC_KT_VBR_FLOWFILTER_ENTRY:
          {
            pfc_ipcstdef_t *stdef_kvbr_flowfilter_entry = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_kvbr_flowfilter_entry,
                               key_vbr_flowfilter_entry);
            pfc_ipcstdef_t *stdef_val_vbr_flowfilter_entry = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_val_vbr_flowfilter_entry,
                               val_flowfilter_entry);
            key_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_kvbr_flowfilter_entry));
            val_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_val_vbr_flowfilter_entry));
            break;
          }
        case UNC_KT_VBRIF_FLOWFILTER:
          {
            pfc_ipcstdef_t *stdef_kvbrif_flowfilter = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_kvbrif_flowfilter, key_vbr_if_flowfilter);
            pfc_ipcstdef_t *stdef_val_vbrif_flowfilter = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_val_vbrif_flowfilter,
                               pfcdrv_val_vbrif_vextif);
            key_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_kvbrif_flowfilter));
            val_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_val_vbrif_flowfilter));
            break;
          }
        case UNC_KT_VRTIF_FLOWFILTER_ENTRY:
          {
            pfc_ipcstdef_t *stdef_kvbrif_flowfilter_entry = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_kvbrif_flowfilter_entry,
                               key_vbr_if_flowfilter_entry);
            pfc_ipcstdef_t *stdef_val_vbrif_flowfilter_entry = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_val_vbrif_flowfilter_entry,
                               pfcdrv_val_flowfilter_entry);
            key_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_kvbrif_flowfilter_entry));
            val_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_val_vbrif_flowfilter_entry));
            break;
          }
        case UNC_KT_VTERMINAL:
          {
            pfc_ipcstdef_t *stdef_key_vterm = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_key_vterm, key_vterm);
            pfc_ipcstdef_t *stdef_val_vterm = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_val_vterm, val_vterm);
            key_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_key_vterm));
            val_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_val_vterm));
            break;
          }
        case UNC_KT_VTERM_IF:
          {
            pfc_ipcstdef_t *stdef_key_vterm_if = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_key_vterm_if, key_vterm_if);
            pfc_ipcstdef_t *stdef_val_vterm_if = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_val_vterm_if, val_vterm_if);
            key_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_key_vterm_if));
            val_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_val_vterm_if));
            break;
          }
        case UNC_KT_VTERMIF_FLOWFILTER:
          {
            pfc_ipcstdef_t *stdef_kvtermif_flowfilter = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_kvtermif_flowfilter,
                               key_vterm_if_flowfilter);
            pfc_ipcstdef_t *stdef_val_vtermif_flowfilter = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_val_vtermif_flowfilter, val_flowfilter);
            key_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_kvtermif_flowfilter));
            val_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_val_vtermif_flowfilter));
            break;
          }
        case UNC_KT_VTERMIF_FLOWFILTER_ENTRY:
          {
            pfc_ipcstdef_t *stdef_kvtermif_flowfilter_entry = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_kvtermif_flowfilter_entry,
                               key_vterm_if_flowfilter_entry);
            pfc_ipcstdef_t *stdef_val_vtermif_flowfilter_entry = new pfc_ipcstdef_t;
            PFC_IPC_STDEF_INIT(stdef_val_vtermif_flowfilter_entry,
                               val_flowfilter_entry);
            key_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_kvtermif_flowfilter_entry));
            val_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(KT[loop],
                                                                      stdef_val_vtermif_flowfilter_entry));
            break;
          }
        default:
          break;
      }
    }
  }

  pfc_bool_t fini() {
    return PFC_TRUE;
  }
  static VtnDrvIntf theInstance;

  static void stub_loadVtnDrvModule(void);
  static void stub_unloadVtnDrvModule(void);

  pfc_ipcresp_t ipcService(pfc::core::ipc::ServerSession& sess,
                           pfc_ipcid_t service) {
    //  return PFC_IPCINT_EVSESS_OK;

    return pfc_ipcresp_t();
  }

  VtnDrvRetEnum get_request_header(pfc::core::ipc::ServerSession*sess,
                                   odl_drv_request_header_t &request_hdr) {
    return VTN_DRV_RET_SUCCESS;
  }

  KtHandler*  get_kt_handler(unc_key_type_t kt) {
    return NULL;
  }

  VtnDrvRetEnum register_driver(driver *drv_obj) {
    return VTN_DRV_RET_SUCCESS;
  }

  void logicalport_event(oper_type operation,
                         key_logical_port_t &key_struct,
                         val_logical_port_st &val_struct) {
  }

  void logicalport_event(oper_type operation,
                         key_logical_port_t &key_struct,
                         val_logical_port_st_t &new_val_struct,
                         val_logical_port_st_t &old_val_struct) {
  }

  void port_event(oper_type operation,
                  key_port_t &key_struct,
                  val_port_st_t &val_struct) {
  }

  void port_event(oper_type operation,
                  key_port_t &key_struct,
                  val_port_st_t &new_val_struct,
                  val_port_st_t &old_val_struct) {
  }

  void switch_event(oper_type operation,
                    key_switch_t &key_struct,
                    val_switch_st_t &val_struct) {
  }

  void switch_event(oper_type operation,
                    key_switch_t &key_struct,
                    val_switch_st_t &new_val_struct,
                    val_switch_st_t &old_val_struct) {
  }

  void link_event(oper_type operation,
                  key_link_t &key_struct,
                  val_link_st_t &val_struct) {
  }

  void link_event(oper_type operation,
                  key_link_t &key_struct,
                  val_link_st_t &new_val_struct,
                  val_link_st_t &old_val_struct) {

  }
  void event_start(std::string ctr_name) {
  }
};
}  // namespace driver
}  // namespace unc
#endif

