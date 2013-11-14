/*
 * Copyright (c) 2013 NEC Corporation
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
class controller {
   public:
     controller():controller_cache(NULL) {}
     static controller* create_controll();
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
     std::string get_controller_id() {
            return "";
     }
     bool  ping_controller() {
         return true;
     }
     bool  reset_connect() {
           return true;
     }
     unc::vtndrvcache::KeyTree *controller_cache;
     static controller * controller_ptr;
};
}  // namespace driver
}  // namespace unc
#endif
