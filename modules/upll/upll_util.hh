/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UPLL_UPLL_UTIL_HH_
#define UPLL_UPLL_UTIL_HH_

#include <cstring>

namespace unc {
namespace upll {
namespace upll_util {

inline static char *upll_strncpy(char *dest, const char *src, size_t n) {
  strncpy(dest, src, n);
  if (n > 0)
    dest[n - 1] = '\0';
  return dest;
}

inline static char *upll_strncpy(uint8_t *dest, const uint8_t *src, size_t n) {
  return strncpy(reinterpret_cast<char*>(dest),
                 reinterpret_cast<const char*>(src),
                 n);
}

inline static char *upll_strncpy(char *dest, const uint8_t *src, size_t n) {
  return strncpy(dest, reinterpret_cast<const char*>(src), n);
}

inline static char *upll_strncpy(uint8_t *dest, const char *src, size_t n) {
  return strncpy(reinterpret_cast<char*>(dest), src, n);
}


}  // namespace upll_util
}  // namespace upll
}  // namespace unc

#endif  // UPLL_UPLL_UTIL_HH_
