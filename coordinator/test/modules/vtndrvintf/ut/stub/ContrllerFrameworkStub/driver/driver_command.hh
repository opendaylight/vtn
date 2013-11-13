/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __CDF_DRIVER_COMMANDS_HH__
#define __CDF_DRIVER_COMMANDS_HH__

#include <pfc/ipc_struct.h>
#include <unc/unc_base.h>
#include <driver/controller_interface.hh>
#include <vector>


namespace unc {
namespace driver {
typedef struct {
} val_root_t;

typedef struct {
} key_root_t;

class driver_command {
  public:
        virtual ~driver_command() {
        }
        virtual unc_key_type_t get_key_type()=0;
        virtual drv_resp_code_t revoke(unc::driver::controller* ctr_ptr) {
                        return DRVAPI_RESPONSE_SUCCESS;
        }
        virtual drv_resp_code_t fetch_config(unc::driver::controller* ctr,
                             void* parent_key,
                             std::vector<unc::vtndrvcache::ConfigNode *>&) = 0;
};


class vtn_driver_command: public driver_command {
  public:
        drv_resp_code_t create_cmd(key_vtn_t& keyvtn_, val_vtn_t& valvtn_,
                        controller*)  {
            return DRVAPI_RESPONSE_SUCCESS;
        }

        drv_resp_code_t update_cmd(key_vtn_t& keyvtn_, val_vtn_t& valvtn_,
                        controller*)  {
            return DRVAPI_RESPONSE_SUCCESS;
        }

        drv_resp_code_t delete_cmd(key_vtn_t& keyvtn_, val_vtn_t& valvtn_,
                        controller*) {
            return DRVAPI_RESPONSE_SUCCESS;
        }

        drv_resp_code_t validate_op(key_vtn_t& keyvtn_, val_vtn_t& valvtn_,
                        controller*, uint32_t op)  {
            return DRVAPI_RESPONSE_SUCCESS;
        }

        unc_key_type_t get_key_type() {
                return UNC_KT_VTN;
        }
        drv_resp_code_t fetch_config(
                     unc::driver::controller* ctr,
                     void* parent_key,
                     std::vector<unc::vtndrvcache::ConfigNode *>&) {
           return DRVAPI_RESPONSE_SUCCESS;
        }
};

class vbr_driver_command: public driver_command {
  public:
        drv_resp_code_t create_cmd(key_vbr_t& keyvbr_, val_vbr_t& valvbr_,
                        unc::driver::controller*)  {
            return DRVAPI_RESPONSE_SUCCESS;
        }


       drv_resp_code_t update_cmd(key_vbr_t& keyvbr_, val_vbr_t& valvbr_,
                        unc::driver::controller*) {
            return DRVAPI_RESPONSE_SUCCESS;
        }

        drv_resp_code_t delete_cmd(key_vbr_t& keyvbr_, val_vbr_t& valvbr_,
                        unc::driver::controller*)  {
            return DRVAPI_RESPONSE_SUCCESS;
        }

        drv_resp_code_t validate_op(key_vbr_t& keyvbr_, val_vbr_t& valvbr_,
                        unc::driver::controller*, uint32_t op)  {
            return DRVAPI_RESPONSE_SUCCESS;
        }

        unc_key_type_t get_key_type() {
                return UNC_KT_VBRIDGE;
        }
        drv_resp_code_t fetch_config(
                     unc::driver::controller* ctr,
                     void* parent_key,
                     std::vector<unc::vtndrvcache::ConfigNode *>&) {
           return DRVAPI_RESPONSE_SUCCESS;
        }
};

class vbrif_driver_command: public driver_command {
  public:
        drv_resp_code_t create_cmd(key_vbr_if_t& key,
               pfcdrv_val_vbr_if_t& val, unc::driver::controller *conn) {
                return DRVAPI_RESPONSE_SUCCESS;
        }

        drv_resp_code_t update_cmd(key_vbr_if_t& key,
                  pfcdrv_val_vbr_if_t& val, unc::driver::controller *conn) {
              return DRVAPI_RESPONSE_SUCCESS;
        }

        drv_resp_code_t delete_cmd(key_vbr_if_t& key,
                  pfcdrv_val_vbr_if_t& val, unc::driver::controller *conn) {
               return DRVAPI_RESPONSE_SUCCESS;
        }

        drv_resp_code_t validate_op(key_vbr_if_t& key,
                        pfcdrv_val_vbr_if_t& val, unc::driver::controller* ctr,
                        uint32_t op) {
            return DRVAPI_RESPONSE_SUCCESS;
        }

        unc_key_type_t get_key_type() {
                return UNC_KT_VBR_IF;
        }
        drv_resp_code_t fetch_config(
                     unc::driver::controller* ctr,
                     void* parent_key,
                     std::vector<unc::vtndrvcache::ConfigNode *>&) {
           return DRVAPI_RESPONSE_SUCCESS;
        }
};

class controller_command: public driver_command {
  public:
        drv_resp_code_t create_cmd(key_ctr_t& key,
                        val_ctr_t & val, unc::driver::controller *conn) {
               return DRVAPI_RESPONSE_SUCCESS;
        }

        drv_resp_code_t update_cmd(key_ctr_t & key,
                    val_ctr_t& val, unc::driver::controller *conn) {
               return DRVAPI_RESPONSE_SUCCESS;
        }

        drv_resp_code_t delete_cmd(key_ctr_t & key,
                        val_ctr_t & val, unc::driver::controller *conn) {
              return DRVAPI_RESPONSE_SUCCESS;
        }

        drv_resp_code_t validate_op(key_ctr_t & key,
                        val_ctr_t& val, unc::driver::controller* ctr,
                        uint32_t op) {
               return DRVAPI_RESPONSE_SUCCESS;
        }

        unc_key_type_t get_key_type() {
                return UNC_KT_CONTROLLER;
        }
        drv_resp_code_t fetch_config(
                     unc::driver::controller* ctr,
                     void* parent_key,
                     std::vector<unc::vtndrvcache::ConfigNode *>&) {
           return DRVAPI_RESPONSE_SUCCESS;
        }
};

class root_driver_command : public driver_command {
   public:
        drv_resp_code_t
        create_cmd(unc::driver::key_root_t& key,
                   unc::driver::val_root_t & val,
                   unc::driver::controller *conn) {
           return DRVAPI_RESPONSE_SUCCESS;
        }

        drv_resp_code_t
        update_cmd(unc::driver::key_root_t& key,
                   unc::driver::val_root_t & val,
                   unc::driver::controller *conn) {
           return DRVAPI_RESPONSE_SUCCESS;
        }

        drv_resp_code_t
        delete_cmd(unc::driver::key_root_t& key,
                   unc::driver::val_root_t & val,
                   unc::driver::controller *conn) {
           return DRVAPI_RESPONSE_SUCCESS;
        }

        drv_resp_code_t
        validate_op(unc::driver::key_root_t& key,
                   unc::driver::val_root_t & val,
                   unc::driver::controller *conn,
                   uint32_t op) {
          return DRVAPI_RESPONSE_SUCCESS;
       }

        drv_resp_code_t
        read_root_child(std::vector<unc::vtndrvcache::ConfigNode*>&,
                        unc::driver::controller*) {
          if (set_root_child == 1) {
           return DRVAPI_RESPONSE_NO_SUCH_INSTANCE;
          } else if (set_root_child == 2) {
           return DRVAPI_RESPONSE_FAILURE;
          } else {
            return DRVAPI_RESPONSE_SUCCESS;
          }
        }

        drv_resp_code_t
        read_all_child(unc::vtndrvcache::ConfigNode*,
                       std::vector<unc::vtndrvcache::ConfigNode*>&,
                       unc::driver::controller*) {
           return DRVAPI_RESPONSE_SUCCESS;
       }

        unc_key_type_t get_key_type() {
                return UNC_KT_ROOT;
        }
        drv_resp_code_t fetch_config(
                     unc::driver::controller* ctr,
                     void* parent_key,
                     std::vector<unc::vtndrvcache::ConfigNode *>&) {
           return DRVAPI_RESPONSE_SUCCESS;
        }

        static uint32_t set_root_child;
};
}  // namespace driver
}  // namespace unc
#endif
