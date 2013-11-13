/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef ODC_LOG_HH_
#define ODC_LOG_HH_
#include <string.h>

#include "pfc/log.h"

#define FILESTR (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define ODC_LOG(level, fmt, ...) do {                                   \
  pfc_log_##level("%s:%d:%s: " fmt, FILESTR, __LINE__,                   \
                  __FUNCTION__, ##__VA_ARGS__);                           \
} while (0);

#ifdef PFC_VERBOSE_DEBUG
class LogFuncTrace {
 public:
  inline LogFuncTrace(const char *file_name, int line_no, const char *fn_name) {
    file_ = file_name;
    line_ = line_no;
    func_ = fn_name;
    pfc_log_trace("%s:%d: Entering function %s", file_, line_, func_);
  }
  inline ~LogFuncTrace() {
    pfc_log_trace("%s:%d: Leaving function %s", file_, line_, func_);
  }
  const char *file_, *func_;
  int line_;
};

#define ODC_FUNC_TRACE                                                 \
    LogFuncTrace func_trace___(FILESTR, __LINE__,   __FUNCTION__);
#else
#define ODC_FUNC_TRACE
#endif  // ODC_VERBOSE_DEBUG

#endif  // ODC_LOG_HH_
