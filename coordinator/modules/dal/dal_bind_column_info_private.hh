/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


/**
 * dal_bind_column_info_private.h
 *   Contains internal definitions related to class DalBindColumnInfo
 */ 

#ifndef __DAL_COLUMN_INFO_PRIVATE_HH__
#define __DAL_COLUMN_INFO_PRIVATE_HH__

namespace unc {
namespace upll {
namespace dal {

/**
 * Enumerations for IO code
 */
enum DalIoCode {
  kDalIoCodeInvalid = 0,  // Invalid IO Code
  kDalIoCodeInput,        // Input
  kDalIoCodeOutput,       // Output
  kDalIoCodeMatch         // Match
};

/**
 * Enumerations for IO type
 */
enum DalIoType {
  kDalIoInvalid = 0,     // Invalid Io Type combination
  kDalIoNone,            // Default Io Type
  kDalIoInputOnly,       // Input Only - for CREATE/UPDATE
  kDalIoOutputOnly,      // Output Only - for SELECT
  kDalIoMatchOnly,       // Match Only - WHERE clause
  kDalIoInputAndMatch,   // Input and Match
  kDalIoOutputAndMatch   // Output and Match
};

/**
 * Enumerations for Datatype code
 */
enum DalDataTypeCode {
  kDalDtCodeInvalid = 0,  // Invalid Code
  kDalDtCodeApp,          // Dal user Datatype
  kDalDtCodeDal           // Dal Datatype
};
}  // namespace dal
}  // namespace upll
}  // namespace unc
#endif  // __DAL_COLUMN_INFO_PRIVATE_HH__
