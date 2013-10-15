/*
* Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * This program and the accompanying materials are made  available under the
 * terms of the Eclipse Public License v1.0 which  accompanies
 * this  distribution, * and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __CDF_DRIVER_COMMANDS_HH__
#define __CDF_DRIVER_COMMANDS_HH__

#include <driver/controller_interface.hh>
#include <pfc/ipc_struct.h>
#include <unc/unc_base.h>
#include <confignode.hh>
#include <vector>

namespace unc {
namespace driver {

typedef struct val_root {
  uint8_t     root_key;
} val_root_t;

/*
 * @desc:Base Class For Driver Commands
 */
class driver_command {
  public:
    virtual ~driver_command() {}
    virtual unc_key_type_t get_key_type()=0;
};

/*
 * Class to be extended for VTN Commands
 */
class vtn_driver_command: public driver_command {
  public:
  /* Create VTN  in the controller*/
  virtual drv_resp_code_t create_cmd(key_vtn_t& keyvtn_, val_vtn_t& valvtn_,
                                     unc::driver::controller*)=0;
  /* Update VTN  in the controller*/
  virtual drv_resp_code_t update_cmd(key_vtn_t& keyvtn_, val_vtn_t& valvtn_,
                                     unc::driver::controller*)=0;
  /* Delete VTN  in the controller*/
  virtual drv_resp_code_t delete_cmd(key_vtn_t& keyvtn_, val_vtn_t& valvtn_,
                                     unc::driver::controller*)=0;
  /* Validate Operation during Vote */
  virtual drv_resp_code_t validate_op(key_vtn_t& keyvtn_, val_vtn_t& valvtn_,
                                      unc::driver::controller*, uint32_t op)=0;
  unc_key_type_t get_key_type() {
    return UNC_KT_VTN;
  }
};

class vbr_driver_command: public driver_command {
  public:
  virtual drv_resp_code_t create_cmd(key_vbr_t& keyvbr_, val_vbr_t& valvbr_,
                                     unc::driver::controller*) = 0;
  virtual drv_resp_code_t update_cmd(key_vbr_t& keyvbr_, val_vbr_t& valvbr_,
                                     unc::driver::controller*) = 0;
  virtual drv_resp_code_t delete_cmd(key_vbr_t& keyvbr_, val_vbr_t& valvbr_,
                                     unc::driver::controller*)=0;
  virtual drv_resp_code_t validate_op(key_vbr_t& keyvbr_, val_vbr_t& valvbr_,
                                      unc::driver::controller*, uint32_t op)=0;
  unc_key_type_t get_key_type() {
    return UNC_KT_VBRIDGE;
  }
};

class vbrif_driver_command: public driver_command {
  public:
  virtual drv_resp_code_t create_cmd(key_vbr_if_t& key,
          pfcdrv_val_vbr_if_t& val, unc::driver::controller *conn)=0;
  virtual drv_resp_code_t update_cmd(key_vbr_if_t& key,
          pfcdrv_val_vbr_if_t& val, unc::driver::controller *conn) = 0;
  virtual drv_resp_code_t delete_cmd(key_vbr_if_t& key,
          pfcdrv_val_vbr_if_t& val, unc::driver::controller *conn) = 0;
  virtual drv_resp_code_t validate_op(key_vbr_if_t& key,
          pfcdrv_val_vbr_if_t& val, unc::driver::controller* ctr,
          uint32_t op)=0;
  unc_key_type_t get_key_type() {
    return UNC_KT_VBR_IF;
  }
};

class controller_command: public driver_command {
  public:
  drv_resp_code_t create_cmd(key_ctr_t& key,
                             val_ctr_t & val, unc::driver::controller *conn) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  drv_resp_code_t update_cmd(key_ctr_t & key,
                             val_ctr_t& val, unc::driver::controller *conn) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  drv_resp_code_t delete_cmd(key_ctr_t & key,
                             val_ctr_t & val, unc::driver::controller *conn) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  drv_resp_code_t validate_op(key_ctr_t & key,
                              val_ctr_t& val, unc::driver::controller* ctr,
                              uint32_t op) {
    return DRVAPI_RESPONSE_FAILURE;
  }

  unc_key_type_t get_key_type() {
    return UNC_KT_VBR_IF;
  }
};

class root_driver_command : public driver_command {
  public:
        virtual drv_resp_code_t
        create_cmd(key_root_t& key,
                   val_root_t & val,
                   unc::driver::controller *conn) = 0;

        virtual drv_resp_code_t
        update_cmd(key_root_t& key,
                   val_root_t & val,
                   unc::driver::controller *conn) = 0;

        virtual drv_resp_code_t
        delete_cmd(key_root_t& key,
                   val_root_t & val,
                   unc::driver::controller *conn)=0;

        virtual drv_resp_code_t
        validate_op(key_root_t& key,
                   val_root_t & val,
                   unc::driver::controller *conn,
                   uint32_t op) = 0;

        virtual drv_resp_code_t
        read_root_child(std::vector<unc::vtndrvcache::ConfigNode*>&,
                        unc::driver::controller*) = 0;

        virtual drv_resp_code_t
        read_all_child(unc::vtndrvcache::ConfigNode*,
                       std::vector<unc::vtndrvcache::ConfigNode*>&,
                       unc::driver::controller*)=0;

        unc_key_type_t get_key_type() {
                return UNC_KT_ROOT;
        }
};
}  // namespace driver
}  // namespace unc
#endif
