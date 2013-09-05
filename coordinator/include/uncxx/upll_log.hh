/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UPLL_LOG_HH_
#define UPLL_LOG_HH_

#include "pfc/log.h"

#define UPLL_LOG(level, fmt, ...) do {                                   \
  pfc_log_##level("%s:%d:%s: " fmt, __FILE__, __LINE__,                   \
                  __FUNCTION__, ##__VA_ARGS__);                           \
} while (0);

#define UPLL_LOG_ERROR(fmt, ...) do { UPLL_LOG(error, fmt , ##__VA_ARGS__); } while (0);    // NOLINT
#define UPLL_LOG_WARN(fmt, ...) do { UPLL_LOG(warn, fmt , ##__VA_ARGS__); } while (0);      // NOLINT
#define UPLL_LOG_DEBUG(fmt, ...) do { UPLL_LOG(debug, fmt , ##__VA_ARGS__); } while (0);    // NOLINT
#define UPLL_LOG_TRACE(fmt, ...) do { UPLL_LOG(trace, fmt , ##__VA_ARGS__); } while (0);    // NOLINT
#define UPLL_LOG_INFO(fmt, ...) do { UPLL_LOG(info, fmt , ##__VA_ARGS__); } while (0);      // NOLINT
#define UPLL_LOG_NOTICE(fmt, ...) do { UPLL_LOG(notice, fmt , ##__VA_ARGS__); } while (0);  // NOLINT
#define UPLL_LOG_VERBOSE(fmt, ...) do { UPLL_LOG(verbose, fmt , ##__VA_ARGS__); } while (0);// NOLINT

#define UPLL_FUNC_IN() do {                                             \
  pfc_log_trace("%s:%d: Entering function %s", __FILE__, __LINE__,      \
                __FUNCTION__);                                          \
} while (0);

#define UPLL_FUNC_OUT() do {                                             \
  pfc_log_trace("%s:%d: Leaving function %s", __FILE__, __LINE__,        \
                __FUNCTION__);                                           \
} while (0);

#ifdef PFC_VERBOSE_DEBUG
class LogFuncTrace {
 public:
  inline LogFuncTrace(const char *file_name, int line_no, const char *fn_name) {
    file_ = file_name;
    line_ = line_no;
    func_ = fn_name;
#ifdef UPLL_PERF_TIME
    pfc_clock_gettime(&start);
#endif
    pfc_log_trace("%s:%d: Entering function %s", file_, line_, func_);
  }
  inline ~LogFuncTrace() {
#ifdef UPLL_PERF_TIME
    pfc_timespec_t end;
    pfc_clock_gettime(&end);
    pfc_timespec_sub(&end, &start);
    uint64_t elapsed(pfc_clock_time2msec(&end));
    pfc_log_trace("%s:%d: Leaving function %s: CONSUMED :%" PFC_PFMT_u64 ": ms"
                , file_, line_, func_, elapsed);
#else
    pfc_log_trace("%s:%d: Leaving function %s", file_, line_, func_);
#endif
  }
#ifdef UPLL_PERF_TIME
  pfc_timespec_t start;
#endif
  const char *file_, *func_;
  int line_;
};
#define UPLL_FUNC_TRACE                                                 \
    LogFuncTrace func_trace___(__FILE__, __LINE__,   __FUNCTION__);
#else
#define UPLL_FUNC_TRACE
#endif  // PFC_VERBOSE_DEBUG

class LogFuncTracePerf {
 public:
  inline LogFuncTracePerf(const char *file_name, int line_no, const char *fn_name) {
    file_ = file_name;
    line_ = line_no;
    func_ = fn_name;
    pfc_clock_gettime(&start);
    pfc_log_info("%s:%d: Entering function %s", file_, line_, func_);
  }
  inline ~LogFuncTracePerf() {
    pfc_timespec_t end;
    pfc_clock_gettime(&end);
    pfc_timespec_sub(&end, &start);
    uint64_t elapsed(pfc_clock_time2msec(&end));
    pfc_log_info("%s:%d: Leaving function %s: CONSUMED :%" PFC_PFMT_u64 ": ms"
                , file_, line_, func_, elapsed);
  }
  pfc_timespec_t start;
  const char *file_, *func_;
  int line_;
};
#if 0
#define UPLL_FUNC_TRACE_PERF                                                 \
    LogFuncTracePerf func_trace_perf___(__FILE__, __LINE__,   __FUNCTION__);
#else
#define UPLL_FUNC_TRACE_PERF
#endif

#endif  // UPLL_LOG_HH_
