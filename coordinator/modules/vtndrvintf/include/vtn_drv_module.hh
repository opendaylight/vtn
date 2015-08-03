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
#include <vtn_drv_transaction_handle.hh>
#include <driver/driver_interface.hh>
#include <unc/unc_events.h>
#include <string>
#include <map>
#include <utility>

namespace unc {
namespace driver {

/*
 *  The IPC event that the IPC server transmitted is received.
 */
enum ValDomainIndex {
  VAL_DOMAIN_STRUCT = 0
};

enum ValDomainEventIndex {
  VAL_DOMAIN_EVENT_ATTR1 = 0,
  VAL_DOMAIN_EVENT_ATTR2,
  VAL_DOMAIN_EVENT_ATTR3,
  VAL_DOMAIN_EVENT_ATTR4,
  VAL_DOMAIN_EVENT_ATTR5
};


class VtnDrvIntf :public pfc::core::Module {
 public:
  typedef std::map<unc_key_type_t, pfc_ipcstdef_t*> kt_map;
  static kt_map key_map;
  static kt_map val_map;

  /**
   * @brief  - Method to initialize pfc_ipcstdef_t pointer with keytype
   * @retval - None
   */
  void  initialize_map(void);
  /**
   * @brief     : Constructor
   * @param[in] : pfc_modattr_t*
   **/
  explicit VtnDrvIntf(const pfc_modattr_t* attr);


  /**
   * @brief : Destructor
   **/
  ~VtnDrvIntf();

  /**
   * @brief  : This Function is called to load the
   *           vtndrvintf module
   * @return : PFC_TRUE/PFC_FALSE
   **/
  pfc_bool_t init(void);

  /**
   * @brief  : This Function is called to unload the
   *           vtndrvintf module
   * @return : PFC_TRUE/PFC_FALSE
   **/
  pfc_bool_t fini(void);
  /**
   * @brief     - Read configuration file of vtndrvintf
   * @param[in] - None
   * @return    - None
   */
  void read_conf_file();

  //  structure to store configuration file parsed values
  typedef struct {
    uint32_t time_interval;
  }conf_info;

  /**
   * @brief     : Used register the driver handler with respect to the
   *              controller type
   * @param[in] : driver pointer
   * @return    : VTN_DRV_RET_FAILURE /VTN_DRV_RET_SUCCESS
   **/
  VtnDrvRetEnum register_driver(driver *drv_obj);

  /**
   * @brief     : This Function recevies the ipc request and process the same
   * @param[in] : sess, service
   * @retval    : PFC_IPCRESP_FATAL/PFC_IPCINT_EVSESS_OK
   */
  pfc_ipcresp_t ipcService(pfc::core::ipc::ServerSession &sess,
                            pfc_ipcid_t service);

  /**
   * @brief     : This function parse the session and fills
   *              odl_drv_request_header_t
   * @param[in] : sess, request_hdr
   * @retval    : VTN_DRV_RET_FAILURE /VTN_DRV_RET_SUCCESS
   */
  VtnDrvRetEnum get_request_header(pfc::core::ipc::ServerSession*sess,
                            odl_drv_request_header_t &request_hdr);

  /**
   * @brief     : This Function  returns the key type handler pointer for
   *              the appropriate key types
   * @param[in] : key type
   * @retval    : KtHandler pointer
   */
  KtHandler*  get_kt_handler(unc_key_type_t kt);

  /**
   * @brief     : This Function  returns the key type handler pointer for
   *              the appropriate key types
   * @param[in] : key type
   * @retval    : KtHandler pointer
   */
  KtHandler*  get_read_kt_handler(unc_key_type_t kt);

  /**
   * @brief     : This Function  intialize controller instance
   * @param[in] : ControllerFramework pointer
   * @retval    : None
   */
  void  set_controller_instance(ControllerFramework* ctrl_inst) {
    ctrl_inst_ =  ctrl_inst;
  }

  /**
   * @brief     : Method to post domain create events to UPPL
   * @param[in] : ctr_name, domain_name
   * @retval    : None
   */
  void domain_event(std::string ctr_name,
                                std::string doamin_name);

  /**
   * @brief     : Method to post logical port create/delete  events to UPPL
   * @param[in] : oper_type, key_logical_port_t, val_logical_port_st
   * @retval    : None
   */
  void logicalport_event(oper_type operation,
                         key_logical_port_t &key_struct,
                         val_logical_port_st_t &val_struct);
  /**
   * @brief     : Method to post port create/delete events to UPPL
   * @param[in] : oper_type, key_port_t, val_port_st
   * @retval    : None
   */
  void port_event(oper_type operation,
                  key_port_t &key_struct,
                  val_port_st_t &val_struct);
  /**
   * @brief     : Method to post port update events to UPPL
   * @param[in] : oper_type, key_port_t, val_port_st, val_port_st
   * @retval    : None
   */
  void port_event(oper_type operation,
                  key_port_t &key_struct,
                  val_port_st_t &new_val_struct,
                  val_port_st_t &old_val_struct);
  /**
   * @brief     : Method to post switch create/delete events to UPPL
   * @param[in] : oper_type, key_switch, val_switch_st
   * @retval    : None
   */
  void switch_event(oper_type operation,
                    key_switch_t &key_struct,
                    val_switch_st_t &val_struct);
  /**
   * @brief     : Method to post switch update events to UPPL
   * @param[in] : oper_type, key_switch, val_switch_st, val_switch_st
   * @retval    : None
   */
  void switch_event(oper_type operation,
                    key_switch_t &key_struct,
                    val_switch_st_t &new_val_struct,
                    val_switch_st_t &old_val_struct);

  /**
   * @brief     : Method to post link create/delete events to UPPL
   * @param[in] : oper_type, key_link_t, val_link_st
   * @retval    : None
   */
  void link_event(oper_type operation,
                  key_link_t &key_struct,
                  val_link_st_t &val_struct);
  /**
   * @brief     : Method to post link update events to UPPL
   * @param[in] : oper_type, key_link_t, val_link_st, val_link_st
   * @retval    : None
   */
  void link_event(oper_type operation, key_link_t &key_struct,
                  val_link_st_t &new_val_struct,
                  val_link_st_t &old_val_struct);

  /**
   * @brief     : Method to post EVENT START to UPPL
   * @param[in] : controller name
   * @retval    : None
   **/
  void event_start(std::string ctr_name);


  /**
   * @brief     : Method to create Kt handler
   * @param[in] : unc_key_type_t
   * @retval    : None
   */
  template <typename key, typename value>
  void create_handler(unc_key_type_t keytype);

  /**
   * @brief     : Method to create Kt handler for read
   * @param[in] : unc_key_type_t
   * @retval    : None
   */
  template <typename key, typename value>
  void create_handler_read(unc_key_type_t keytype);

#define POPULATE_STDEF(key, value, keytype, stdef_k, stdef_v)  \
  pfc_ipcstdef_t *stdef_k = new pfc_ipcstdef_t; \
  PFC_IPC_STDEF_INIT(stdef_k, key); \
  pfc_ipcstdef_t *stdef_v = new pfc_ipcstdef_t;\
  PFC_IPC_STDEF_INIT(stdef_v, value); \
  key_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(keytype,\
                                                            stdef_k));\
  val_map.insert(std::pair<unc_key_type_t, pfc_ipcstdef_t*>(keytype, \
                                                            stdef_v));

  // used for Controller ping
  pfc::core::TaskQueue* taskq_;

 private:
  // To store key type handler pointer for supported keytypes
  std::map <unc_key_type_t, unc::driver::KtHandler*> map_kt_;
  std::map <unc_key_type_t, unc::driver::KtHandler*> read_map_kt_;

  // To store ControllerFramework instance
  ControllerFramework* ctrl_inst_;

  conf_info conf_parser_;  //  conf file information
};
}  // namespace driver
}  // namespace unc
#endif
