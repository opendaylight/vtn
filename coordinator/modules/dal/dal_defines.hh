/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * dal_defines.hh
 *   Contains common definitions for DAL module
 */

#ifndef __DAL_DEFINES_H__
#define __DAL_DEFINES_H__

#include "upll/keytype_upll_ext.h"
#include "uncxx/tclib/tclib_defs.hh"
#include "uncxx/tclib/tclib_interface.hh"

namespace unc {
namespace upll {
namespace dal {

// Type definition for Config Type
typedef upll_keytype_datatype_t UpllCfgType;
// Type definition for ConfigMode
typedef TcConfigMode CfgModeType;

/**
 * Enumeration for C datatype used by dal user
 */
enum DalCDataType {
  kDalAppTypeInvalid = 0,  // Invalid C Datatype
  kDalChar,      // Character type
  kDalUint8,     // 8 bit datatype
  kDalUint16,    // 16 bit integer type
  kDalUint32,    // 32 bit integer type
  kDalUint64     // 64 bit integer type
};

/**
 * Enumeration for Error code to dal user
 */
enum DalResultCode {
  kDalRcSuccess = 0,
  kDalRcConnNotEstablished,   // For DCI interface ConnectToDB
  kDalRcConnNotAvailable,     // For all DCI and DMI APIs
  kDalRcNotDisconnected,      // For DCI interface DisconnectFromDB
  kDalRcTxnError,             // For DCI interaces Commit/RollbackTransaction
  // This should be internal to DAL. DAL user does not have
  // knowledge about Conn Handle
  kDalRcInvalidConnHandle,    // For DCI and DMI interfaces
  kDalRcInvalidCursor,        // For DMI interface GetNextRecord/CloseCursor
  kDalRcDataError,            // For all DMI Interfaces
  kDalRcRecordAlreadyExists,  // For DMI interface CreateRecord API
  kDalRcParentNotFound,       // For DMI interface CreateRecord API
  kDalRcRecordNotFound,       // For DMI interfaces Get/Update APIs
  kDalRcRecordNoMore,         // For DMI interface GetNextRecord
  // kDalRcAccessViolation not taken care
  kDalRcAccessViolation,      // For DMI Edit Interface with RO conn
  kDalRcConnTimeOut,          // For DCI and DMI interfaces
  kDalRcQueryTimeOut,         // For all DMI interfaces
  kDalRcMemoryError,          // For DCI and DMI interfaces
  kDalRcInternalError,        // For DCI and DMI interfaces
  kDalRcGeneralError          // For DCI and DMI interfaces
};  // enum DalResultCode

}  // namespace dal
}  // namespace upll
}  // namespace unc
#endif  // __DAL_DEFINES_H__
