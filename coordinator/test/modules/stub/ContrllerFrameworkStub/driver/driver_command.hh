/*
 * Copyright (c) 2013-2014 NEC Corporation
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
#include <driver/vtn_read_value_util.hh>
#include <vector>


namespace unc {
namespace driver {
typedef struct {
} val_root_t;

class driver_command {
 public:
  virtual ~driver_command() {
  }
  virtual UncRespCode revoke(unc::driver::controller* ctr_ptr) {
    return UNC_RC_SUCCESS;
  }
  virtual UncRespCode fetch_config(unc::driver::controller* ctr,
                                   void* parent_key,
                                   std::vector<unc::vtndrvcache::ConfigNode *>&) = 0;
};

template<class key_temp, class val_temp>
class vtn_driver_command: public driver_command {
 public:
  UncRespCode create_cmd(key_temp& key_st, val_temp& val_st,
                         unc::driver::controller*) {
    return UNC_RC_SUCCESS;
  }
  UncRespCode update_cmd(key_temp& key_st, val_temp& old_val_st,
                         val_temp& val_st,
                         unc::driver::controller*) {
    return UNC_RC_SUCCESS;
  }
  UncRespCode delete_cmd(key_temp& key_st, val_temp& val_st,
                         unc::driver::controller*) {
    return UNC_RC_SUCCESS;
  }

  UncRespCode fetch_config(
      unc::driver::controller* ctr,
      void* parent_key,
      std::vector<unc::vtndrvcache::ConfigNode *>&) {
    return UNC_RC_SUCCESS;
  }
};

class vtn_driver_read_command : public driver_command {
  public:
  virtual UncRespCode read_cmd(unc::driver::controller*,
                               unc::vtnreadutil::driver_read_util*) {
    return UNC_RC_SUCCESS;
  }

  virtual UncRespCode fetch_config(
             unc::driver::controller* ctr,
             void* parent_key,
             std::vector<unc::vtndrvcache::ConfigNode *>&) {
    return UNC_RC_SUCCESS;
  }
};

class root_driver_command : public driver_command {
 public:
  UncRespCode
      create_cmd(key_root_t& key,
                 val_root_t & val,
                 unc::driver::controller *conn) {
        return UNC_RC_SUCCESS;
      }

  UncRespCode
      update_cmd(key_root_t& key,
                 val_root_t & val,
                 unc::driver::controller *conn) {
        return UNC_RC_SUCCESS;
      }

  UncRespCode
      delete_cmd(key_root_t& key,
                 val_root_t & val,
                 unc::driver::controller *conn) {
        return UNC_RC_SUCCESS;
      }

  UncRespCode
      validate_op(key_root_t& key,
                  unc::driver::val_root_t & val,
                  unc::driver::controller *conn,
                  uint32_t op) {
        return UNC_RC_SUCCESS;
      }

  UncRespCode
      read_root_child(std::vector<unc::vtndrvcache::ConfigNode*>&,
                      unc::driver::controller*) {
        if (set_root_child == 1) {
          return UNC_RC_NO_SUCH_INSTANCE;
        } else if (set_root_child == 2) {
          return UNC_DRV_RC_ERR_GENERIC;
        } else {
          return UNC_RC_SUCCESS;
        }
      }

  UncRespCode
      read_all_child(unc::vtndrvcache::ConfigNode*,
                     std::vector<unc::vtndrvcache::ConfigNode*>&,
                     unc::driver::controller*) {
        return UNC_RC_SUCCESS;
      }

  UncRespCode fetch_config(
      unc::driver::controller* ctr,
      void* parent_key,
      std::vector<unc::vtndrvcache::ConfigNode *>&) {
    return UNC_RC_SUCCESS;
  }

  static uint32_t set_root_child;
};
}  // namespace driver
}  // namespace unc
#endif
