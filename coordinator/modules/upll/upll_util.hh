/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UPLL_UPLL_UTIL_HH_
#define UPLL_UPLL_UTIL_HH_

#include <sys/time.h>
#include <sstream>
#include <cstring>
#include <string>
#include "cxx/pfcxx/synch.hh"
#include "uncxx/upll_log.hh"

extern bool fatal_done;
extern pfc::core::Mutex  fatal_mutex_lock;

#define UPLL_LOG_FATAL(fmt, ...) do {                                   \
  fatal_mutex_lock.lock();                                              \
  if (!fatal_done) {                                                    \
    UPLL_LOG(fatal, fmt , ##__VA_ARGS__);                               \
    fatal_done = true;                                                  \
  } else {                                                              \
    UPLL_LOG(info, fmt , ##__VA_ARGS__);                                \
  }                                                                     \
  fatal_mutex_lock.unlock();                                            \
} while (0);


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
inline static int upll_strncmp(uint8_t *str1, const uint8_t *str2, size_t n) {
  return strncmp(reinterpret_cast<const char*>(str1),
                 reinterpret_cast<const char*>(str2),
                 n);
}
inline std::string getTime() {
  struct timeval _timeval;
  gettimeofday(&_timeval, NULL);
  std::stringstream ss;
  ss <<_timeval.tv_sec <<_timeval.tv_usec;
  return ss.str();
}

}  // namespace upll_util
}  // namespace upll
}  // namespace unc

#endif  // UPLL_UPLL_UTIL_HH_
