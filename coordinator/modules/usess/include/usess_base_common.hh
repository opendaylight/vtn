/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _USESS_BASE_COMMON_HH_
#define _USESS_BASE_COMMON_HH_

#include "usess_def.hh"

namespace unc {
namespace usess {

class UsessBaseCommon
{
public:
  UsessBaseCommon(void);
  ~UsessBaseCommon(void);

  void Hash(const char* str, const std::string& hash_key,
            const hash_type_e hash_type, std::string& hash_str) const;
  void Hash(const char* str, const pfc_timespec_t& hash_key,
            const hash_type_e hash_type, std::string& hash_str) const;
  void Hash(const char* str, const std::string& salt,
            std::string& hash_str) const;

  bool CheckDigest(const char* str, const std::string& hash_str,
                   const std::string& hash_key) const;
  bool CheckRegular(const char* check_str,
                    const std::string& regular_str) const;

};

}  // namespace usess
}  // namespace unc
#endif      // _USESS_BASE_COMMON_HH_
