/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __CONTROLLER_INTERFACE_HH__
#define __CONTROLLER_INTERFACE_HH__

#include <unc/keytype.h>
#include <keytree.hh>
#include <string>

namespace unc {
namespace driver {

typedef enum {
  CONNECTION_UP = 0,
  CONNECTION_DOWN
}ConnectionStatus;

class controller {
 public:
  controller():controller_cache(NULL),
  physical_port_cache(NULL) {}
  static controller* create_controll();
  static void set_controller_status(uint32_t ctr_st);
  virtual ~controller() {
    if (controller_ptr != NULL) {
      delete controller_ptr;
      controller_ptr = NULL;
    }

    if (controller_cache != NULL) {
      delete controller_cache;
      controller_cache = NULL;
    }
  }

  virtual std::string get_host_address() {
    return "0.0.0.0";
  }

  /**
   * @brief  - Method to get the audit status
   * @retval - PFC_TRUE/PFC_FALSE
   */
  pfc_bool_t get_audit_status() {
    return PFC_TRUE;
  }

   pfc_bool_t get_audit_result () {
    return audit_result_;
  }

   void set_audit_result (pfc_bool_t result){
    audit_result_=result;
   }

  /**
   * @brief  - Method to get the  user name
   * @retval - string - username configured
   */
  virtual  std::string  get_user_name() {
    return "admin";
  }

  /**
   * @brief  - Method to get the password
   * @retval - string - password configured
   */
  virtual std::string get_pass_word() {
    return "admin";
  }

  virtual pfc_bool_t update_ctr(const key_ctr_t& key_ctr,
                                const val_ctr_t& val_ctr) {
    return PFC_TRUE;
  }



  unc_keytype_ctrtype_t get_controller_type() {
    return UNC_CT_ODC;
  }
  bool is_ping_needed() {
    return true;
  }
  uint32_t get_ping_interval() {
    return 0;
  }
  uint32_t get_ping_fail_retry_count() {
    return 0;
  }
  ConnectionStatus get_connection_status() {
    if (set_status == 0) {
      return CONNECTION_UP;
    }
    return CONNECTION_DOWN;
  }
  void set_connection_status(uint32_t status) {
    set_status = status;
  }
  virtual std::string get_controller_id() {
    return "";
  }
  bool  ping_controller() {
    return true;
  }
  pfc_bool_t get_physical_port_details(unc::driver::controller*) {
    return true;
  }

  bool reset_connect() {
    return 0;
  }
  unc::vtndrvcache::KeyTree *controller_cache;
  unc::vtndrvcache::KeyTree *physical_port_cache;
  static controller * controller_ptr;
  static uint32_t set_status;
  pfc_bool_t audit_result_;
};
}  // namespace driver
}  // namespace unc
#endif
