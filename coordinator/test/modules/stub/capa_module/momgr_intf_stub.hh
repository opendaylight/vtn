/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _TEST_UPLL_MOMGR_INTF_STUB_HH
#define _TEST_UPLL_MOMGR_INTF_STUB_HH

#include "vbr_momgr.hh"

namespace unc {
namespace upll {
namespace kt_momgr {


class VbrMoMgrStub : public VbrMoMgr {
 public:
  VbrMoMgrStub() {}
  ~VbrMoMgrStub() {}
  bool GetCreateCapability(const char *ctrlr_name,
                           unc_key_type_t keytype,
                           uint32_t *instance_count,
                           uint32_t *num_attrs,
                           const uint8_t **attrs) {
    *instance_count = *max_inst_;
    *num_attrs = *num_attrs_;
    if (*num_attrs == 0) return false;
    *attrs = attrs_;
    return true;
  }

  bool GetUpdateCapability(const char *ctrlr_name,
                           unc_key_type_t keytype,
                           uint32_t *num_attrs,
                           const uint8_t **attrs) {
    num_attrs = num_attrs_;
    if (*num_attrs == 0) return false;
    *attrs = attrs_;
    return true;
  }

  bool GetReadCapability(const char *ctrlr_name,
                         unc_key_type_t keytype,
                         uint32_t *num_attrs,
                         const uint8_t **attrs) {
    num_attrs = num_attrs_;
    if (*num_attrs == 0) return false;
    *attrs = attrs_;
    return true;
  }


  void set_max_inst(uint32_t *max_inst) {
    max_inst_ = max_inst;
  }
  void set_num_attrs(uint32_t *num_attrs) {
    num_attrs_ = num_attrs;
  }
  void set_attrs(uint8_t *attrs) {
    attrs_ = attrs;
  }

 private:
  uint32_t *max_inst_;
  uint32_t *num_attrs_;
  uint8_t *attrs_;
};

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc

#endif  /* !_TEST_UPLL_MOMGR_INTF_STUB_HH */
