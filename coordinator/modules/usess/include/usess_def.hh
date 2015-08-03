/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _USESS_DEF_HH_
#define _USESS_DEF_HH_

#include "unc/base.h"
#include "pfcxx/module.hh"
#include "unc/usess_ipc.h"

namespace unc {
namespace usess {

// ---------------------------------------------------------------------
// Definition of Macro.
// ---------------------------------------------------------------------

// Cast uint8_t to char*.
#define CAST_IPC_STRING(str) \
({ \
  const void* str_tmp = str; \
  static_cast<const char*>(str_tmp); \
})

// log output
// Note: "CLASS_NAME" the declaration at the top of the class for each source.
#define L_FATAL(format, ...) \
  pfc_log_fatal("%s.%s:" format, CLASS_NAME, __FUNCTION__, __VA_ARGS__)
#define L_ERROR(format, ...) \
  pfc_log_error("%s.%s:" format, CLASS_NAME, __FUNCTION__, __VA_ARGS__)

#define L_ERROR2(format, ...) \
{ \
  pfc_log_error("%s.%s:" format, CLASS_NAME, __FUNCTION__, __VA_ARGS__); \
}

#define L_WARN(format, ...) \
  pfc_log_warn("%s.%s:" format, CLASS_NAME, __FUNCTION__, __VA_ARGS__)
#define L_INFO(format, ...) \
  pfc_log_info("%s.%s:" format, CLASS_NAME, __FUNCTION__, __VA_ARGS__)
#define L_DEBUG(format, ...) \
  pfc_log_debug("%s.%s:" format, CLASS_NAME, __FUNCTION__, __VA_ARGS__)
#define L_TRACE(format, ...) \
  pfc_log_trace("%s.%s:" format, CLASS_NAME, __FUNCTION__, __VA_ARGS__)
#define L_VERBOSE(format, ...) \
  pfc_log_verbose("%s.%s:" format, CLASS_NAME, __FUNCTION__, __VA_ARGS__)

// function start/completed log.
#define L_FUNCTION_START() \
  pfc_log_verbose("%s.%s:start", CLASS_NAME, __FUNCTION__)
#define L_FUNCTION_COMPLETE() \
  pfc_log_verbose("%s.%s:completed", CLASS_NAME, __FUNCTION__)

// ///////////////////////////////////////////////////
// error check & error message output.
// ///////////////////////////////////////////////////
#define WARN_IF(CONDITIONS, FMT, ...) \
  if (CONDITIONS) { \
    L_WARN(FMT, __VA_ARGS__); \
  }

#define GOTO_IF(CONDITIONS, LABEL, FMT, ...) \
  if (CONDITIONS) { \
    L_ERROR(FMT, __VA_ARGS__); \
    goto LABEL; \
  }

#define GOTO_IF2(CONDITIONS, LABEL, FMT, ...) \
  if (CONDITIONS) { \
    L_ERROR2(FMT, __VA_ARGS__); \
    goto LABEL; \
  }

#define RETURN_IF(CONDITIONS, RET, FMT, ...) \
  if (CONDITIONS) { \
    L_ERROR(FMT, __VA_ARGS__); \
    return RET; \
  }

#define RETURN_IF2(CONDITIONS, RET, FMT, ...) \
  if (CONDITIONS) { \
    L_ERROR2(FMT, __VA_ARGS__); \
    return RET; \
  }

#define WARN_CODESET_IF(CONDITIONS, CODE, ERRVAL, FMT, ...) \
  if (CONDITIONS) { \
    L_WARN(FMT, __VA_ARGS__); \
    CODE = ERRVAL; \
  }

#define GOTO_CODESET_IF(CONDITIONS, LABEL, CODE, ERRVAL, FMT, ...) \
  if (CONDITIONS) { \
    L_ERROR(FMT, __VA_ARGS__); \
    CODE = ERRVAL; \
    goto LABEL; \
  }

#define GOTO_CODESET_IF2(CONDITIONS, LABEL, CODE, ERRVAL, FMT, ...) \
  if (CONDITIONS) { \
    L_ERROR2(FMT, __VA_ARGS__); \
    CODE = ERRVAL; \
    goto LABEL; \
  }

#define GOTO_CODESET_DETAIL_IF(CONDITIONS, LABEL, CODE, ERRVAL, RTN, FMT, ...) \
  GOTO_CODESET_IF(CONDITIONS, LABEL, CODE, ERRVAL, FMT " err=%d (%s)", \
              __VA_ARGS__, RTN, strerror(RTN))

#define GOTO_CODESET_DETAIL_IF2(CONDITIONS, LABEL, CODE, ERRVAL, RTN, FMT, ...) \
  GOTO_CODESET_IF2(CONDITIONS, LABEL, CODE, ERRVAL, FMT " err=%d (%s)", \
              __VA_ARGS__, RTN, strerror(RTN))


#define WARN_CODESET_DETAIL_IF(CONDITIONS, CODE, ERRVAL, RTN, FMT, ...) \
  WARN_CODESET_IF(CONDITIONS, CODE, ERRVAL, FMT " err=%d (%s)", \
              __VA_ARGS__, RTN, strerror(RTN))

}  // namespace usess
}  // namespace unc

#endif // _USESS_DEF_HH_
