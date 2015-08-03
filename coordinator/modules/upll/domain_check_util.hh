/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UPLL_DOMAIN_UTIL_HH_
#define UPLL_DOMAIN_UTIL_HH_

#include "unc/uppl_common.h"
#include "unc/keytype.h"
#include "unc/upll_errno.h"
#include "dal/dal_dml_intf.hh"
#include "dal_schema.hh"

namespace unc {
namespace upll {
namespace domain_util {

using unc::upll::dal::DalDmlIntf;
using unc::upll::dal::schema::table::kDalTableIndex;

class DomainUtil {
  public:
    static upll_rc_t ValidateSpineDomain(const char* controller_name,
                                         const char* domain_name,
                                         DalDmlIntf *dmi,
                                         upll_keytype_datatype_t dt_type);
    static upll_rc_t IsDomainLeaf(const char* controller_name,
                                  const char* domain_name,
                                  DalDmlIntf *dmi,
                                  upll_keytype_datatype_t dt_type);
    static upll_rc_t GetDomainTypeFromPhysical(const char* ctrlr_name,
                                               const char* domain_name,
                                               UpplDomainType *dom_type);
  private:
    static upll_rc_t IsDomainExistInTable(const char* ctrlr_name,
                                          const char* domain_name,
                                          DalDmlIntf *dmi,
                                          upll_keytype_datatype_t dt_type,
                                          kDalTableIndex tbl_idx);
};  // DomainUtil
}  // domain_util
}  // upll
}  // unc

#endif  // UPLL_DOMAIN_UTIL_HH_
