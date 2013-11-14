 /*
  * Copyright (c) 2013 NEC Corporation
  * All rights reserved.
  *
  * This program and the accompanying materials are made available under the
  * terms of the Eclipse Public License v1.0 which accompanies this
  * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
  */

#ifndef __DRIVER_STUB_HH__
#define __DRIVER_STUB_HH__


#include <keytree.hh>
#include <driver_command.hh>
#include <controller_interface.hh>
#include <unc/keytype.h>
#include <unc/unc_base.h>
#include <pfc/ipc_struct.h>
#include <vtndrvintf_defs.h>
#include <string>

namespace unc {
namespace driver {

class driver {
 public:
    driver() {}
    virtual ~driver() {
      if (driver_ptr != NULL) {
           delete driver_ptr;
           driver_ptr = NULL;
      }
    }
     bool is_2ph_commit_support_needed() {
        return true;
     }
     bool is_audit_collection_needed() {
        return true;
     }

     controller* add_controller(key_ctr_t& key_ctr,
                                val_ctr_t& val_ctr) {
            controller* ctrl_inst = NULL;
            if (set_ctrl)
               ctrl_inst = controller::create_controll();
            return ctrl_inst;
     }

     controller* update_controller(key_ctr_t& key_ctr,
                                   val_ctr_t& val_ctr,
                                   controller* ctrl_inst) {
            ctrl_inst = NULL;
            if (set_ctrl)
               ctrl_inst = controller::create_controll();
            return ctrl_inst;
     }

     unc_keytype_ctrtype_t get_controller_type() {
           return UNC_CT_ODC;
     }

     bool delete_controller(controller* delete_inst) {
         return true;
     }
     virtual driver_command* create_driver_command(unc_key_type_t key_type) {
             driver_command *ptr = NULL;
             switch (key_type) {
                 case UNC_KT_CONTROLLER:
                       ptr = static_cast<driver_command*>
                            (new controller_command);
                       return ptr;
                       break;
                 case UNC_KT_ROOT:
                       ptr = static_cast<driver_command*>
                            (new root_driver_command);
                       return ptr;
                       break;
                  case UNC_KT_VTN:
                       ptr = static_cast<driver_command*>
                            (new vtn_driver_command);
                       return ptr;
                       break;
                  case UNC_KT_VBRIDGE:
                       ptr = static_cast<driver_command*>
                            (new vbr_driver_command);
                       return ptr;
                       break;
                 case UNC_KT_VBR_IF:
                       ptr = static_cast<driver_command*>
                            (new vbrif_driver_command);
                       return ptr;
                       break;
                  default:
                       return ptr;
                       break;
              }
     }

     static driver* create_driver();
     static driver* driver_ptr;
     static void set_ctrl_instance(uint32_t ctrl_inst);
     static uint32_t set_ctrl;
};
}  // namespace driver
}  // namespace unc
#endif
