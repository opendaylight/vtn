/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * dal_bind_column_info.cc
 *   Implementation of methods in DalBindColumnInfo
 */
#include <stdint.h>
#include <netinet/in.h>
#include <cstdlib>
#include <sstream>
#include "pfc/base.h"
#include "unc/base.h"
#include "uncxx/upll_log.hh"
#include "dal_bind_column_info.hh"

namespace unc {
namespace upll {
namespace dal {
// Public Methods of DalBindColumnInfo Class
// Constructor
DalBindColumnInfo::DalBindColumnInfo(const DalColumnIndex column_index) {
  // Initializing the attributes of DalBindColumnInfo
  column_index_ = column_index;
  app_data_type_ = kDalAppTypeInvalid;
  app_array_size_ = 0;
  io_type_ = kDalIoNone;
  app_out_addr_ = NULL;
  db_in_out_addr_ = NULL;
  db_match_addr_ = NULL;
  buff_len_ptr_ = NULL;
}  // DalBindColumnInfo::DalBindColumnInfo

// Destructor
DalBindColumnInfo::~DalBindColumnInfo(void) {
  if (db_in_out_addr_ != NULL) {
    free(db_in_out_addr_);
    db_in_out_addr_ = NULL;
  }
  if (db_match_addr_ != NULL) {
    free(db_match_addr_);
    db_match_addr_ = NULL;
  }
  if (buff_len_ptr_ != NULL) {
    free(buff_len_ptr_);
    buff_len_ptr_ = NULL;
  }
}  // DalBindColumnInfo::~DalBindColumnInfo

// Updates the Column Info with the user supplied data
bool
DalBindColumnInfo::UpdateColumnInfo(const DalTableIndex table_index,
                                    const DalCDataType app_data_type,
                                    const SQLSMALLINT dal_data_type,
                                    const size_t array_size,
                                    const DalIoCode io_code,
                                    const void **bind_addr) {
  DalIoType io_type = kDalIoInvalid;
  size_t binary_array_size = 0;

  // Validating table_index
  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid table(%s) for Column(%s)",
                   schema::TableName(table_index),
                   schema::ColumnName(table_index, column_index_));
    return false;
  }

  if (column_index_ >= schema::TableNumCols(table_index) &&
      column_index_ != schema::DAL_COL_STD_INTEGER) {
    UPLL_LOG_DEBUG("Invalid Column(%s) for Table(%s)(columns - %zd)",
                   schema::ColumnName(table_index, column_index_),
                   schema::TableName(table_index),
                   schema::TableNumCols(table_index));
    return false;
  }

  if (app_data_type == kDalAppTypeInvalid) {
    UPLL_LOG_DEBUG("Invalid App datatype(%s) for Table(%s) Column(%s)",
                   AppDataTypeToStr(app_data_type).c_str(),
                   schema::TableName(table_index),
                   schema::ColumnName(table_index, column_index_));
    return false;
  }

  if (dal_data_type == SQL_UNKNOWN_TYPE) {
    UPLL_LOG_DEBUG("Invalid DAL datatype(%s) for Table(%s) Column(%s)",
                   DalDataTypeToStr(dal_data_type).c_str(),
                   schema::TableName(table_index),
                   schema::ColumnName(table_index, column_index_));
    return false;
  }

  // Validating dal user C data type
  if (IsDataTypeCompatible(app_data_type, dal_data_type) != true) {
    UPLL_LOG_DEBUG("Datatype mismatch for Table(%s) Column(%s): "
                   "\nApp Data Type - %s, DB Data Type - %s",
                   schema::TableName(table_index),
                   schema::ColumnName(table_index, column_index_),
                   AppDataTypeToStr(app_data_type).c_str(),
                   DalDataTypeToStr(dal_data_type).c_str());
    return false;
  }

  // Validating array_size used by dal user
  if (array_size == 0) {
    UPLL_LOG_DEBUG("Zero array size for Table(%s) Column(%s)",
                   schema::TableName(table_index),
                   schema::ColumnName(table_index, column_index_));
    return false;
  }
  // If DB datatype is BINARY, array size should be number of bytes.
  if (dal_data_type == SQL_BINARY) {
    /* In case of SQL_BINARY, array size should be always 1 
     * When CheckInstance is called from CreateRecord: During BindMatch
     * in CheckInstance, the size of binary (nwm-host-addr) is 4 instead of 1
     * since it was updated as 4 when BindAttribute was called for CreateRecord */
    if (array_size == 1)
      binary_array_size = CalculateAppBufferSize(app_data_type, array_size);
    else
      binary_array_size = array_size;
    if (binary_array_size !=
        schema::ColumnDbArraySize(table_index, column_index_)) {
      UPLL_LOG_DEBUG("Array size mismatch for Table(%s) Column(%s): "
                    "\nApp Array size - %zd, DB Array Size - %zd",
                    schema::TableName(table_index),
                    schema::ColumnName(table_index, column_index_),
                    binary_array_size,
                    schema::ColumnDbArraySize(table_index, column_index_));
      return false;
    }
  } else {
    if (array_size !=
        schema::ColumnDbArraySize(table_index, column_index_)) {
      UPLL_LOG_DEBUG("Array size mismatch for Table(%s) Column(%s): "
                    "\nApp Array size - %zd, DB Array Size - %zd",
                    schema::TableName(table_index),
                    schema::ColumnName(table_index, column_index_),
                    array_size,
                    schema::ColumnDbArraySize(table_index, column_index_));
      return false;
    }
  }

  // Validating io_code input
  if (io_code == kDalIoCodeInvalid) {
    UPLL_LOG_DEBUG("Invalid IO Code(%s) for Table(%s) Column(%s)",
                   DalIoCodeToStr(io_code).c_str(),
                   schema::TableName(table_index),
                   schema::ColumnName(table_index, column_index_));
    return false;
  }

  // Validating Bind Address
  if (bind_addr == NULL) {
    UPLL_LOG_DEBUG("Invalid Bind Address reference for Table(%s) Column(%s)",
                   schema::TableName(table_index),
                   schema::ColumnName(table_index, column_index_));
    return false;
  }

  if (*bind_addr == NULL) {
    UPLL_LOG_DEBUG("Invalid User Bind Address for Table(%s) Column(%s)",
                   schema::TableName(table_index),
                   schema::ColumnName(table_index, column_index_));
    return false;
  }

  // Compute the IO type and update it
  io_type = ComputeIoType(io_type_, io_code);
  if (io_type == kDalIoInvalid) {
    UPLL_LOG_DEBUG("Invalid Binding for Table(%s) Column(%s): "
                  "\nRequested Bind Code - %s, Existing Bind Type - %s, "
                  "Computed Bind Type - %s",
                   schema::TableName(table_index),
                   schema::ColumnName(table_index, column_index_),
                   DalIoCodeToStr(io_code).c_str(),
                   DalIoTypeToStr(io_type_).c_str(),
                   DalIoTypeToStr(io_type).c_str());
    return false;
  }

  io_type_ = io_type;

  // One time updation of properties
  if (io_type == kDalIoInputOnly ||
      io_type == kDalIoOutputOnly ||
      io_type == kDalIoMatchOnly) {
    app_data_type_ = app_data_type;
    // If DB datatype is BINARY, array size should be number of bytes.
    if (dal_data_type == SQL_BINARY) {
      app_array_size_ = binary_array_size;
    } else {
      app_array_size_ = array_size;
    }

    // assign values for buffer length
    buff_len_ptr_ = reinterpret_cast<SQLLEN*>(malloc(sizeof(SQLLEN)));
    // PFC_ASSERT(buff_len_ptr_);
    if (buff_len_ptr_ == NULL) {
      pfc_log_fatal("%s:%d: Error allocating Memory for Buffer length ptr "
                    "for Table(%s) Column(%s)", __func__, __LINE__,
                   schema::TableName(table_index),
                   schema::ColumnName(table_index, column_index_));
      return false;
    }
    memset(buff_len_ptr_, 0, sizeof(SQLLEN));
    *buff_len_ptr_ = app_array_size_;
    UPLL_LOG_VERBOSE("Done with first time updation of common member variables "
                   "for Table(%s) Column(%s) with the following data: \n"
                   "App data type - %s\nApp array size - %zd\n"
                   "Buff Len Ptr - %p\nBuff Len - %ld\n",
                   schema::TableName(table_index),
                   schema::ColumnName(table_index, column_index_),
                   AppDataTypeToStr(app_data_type_).c_str(),
                   app_array_size_, buff_len_ptr_, *buff_len_ptr_);
  }

  // Updating the bind address based on the IO code
  if (io_code == kDalIoCodeInput) {
    // Allocates memory for the size of dal_data_type for db_in_out_addr
    // Copies the min(dal_data_type, app_data_type) bytes of data from
    // bind_addr to db_in_out_addr
    if (AllocateAndCopy(&(db_in_out_addr_),
                        const_cast<const void**>(bind_addr),
                        app_data_type_, dal_data_type,
                        app_array_size_) != true) {
      UPLL_LOG_DEBUG("Failure Updating Input Bind Address for "
                     "Table(%s) Column(%s)",
                     schema::TableName(table_index),
                     schema::ColumnName(table_index, column_index_));
      return false;
    }
    UPLL_LOG_VERBOSE("Updated Input Binding for Table(%s) Column(%s):"
                   "\n User Address - %p, Value - %s"
                   "\n DAL Address - %p, Value - %s",
                   schema::TableName(table_index),
                   schema::ColumnName(table_index, column_index_),
                   *bind_addr,
                   ValueInBindAddrToStr(table_index,
                     const_cast<const void**>(bind_addr)).c_str(),
                   db_in_out_addr_,
                   ValueInBindAddrToStr(table_index,
                     const_cast<const void**>(&db_in_out_addr_)).c_str());
  } else if (io_code == kDalIoCodeOutput) {
    // Stores the memory for copy output to dal user
    app_out_addr_ = const_cast<void*>(*bind_addr);
    // Allocates memory for the size of dal_data_type for db_in_out_addr
    if (AllocateColBuffer(&(db_in_out_addr_),
                          dal_data_type, app_array_size_) != true) {
      UPLL_LOG_DEBUG("Failure Allocating DAL Output Buffer for "
                     "Table(%s) Column(%s)",
                     schema::TableName(table_index),
                     schema::ColumnName(table_index, column_index_));
      return false;
    }
    UPLL_LOG_VERBOSE("Updated Output Binding for Table(%s) Column(%s):"
                   "\n User Address - %p, Value - %s"
                   "\n DAL Address - %p, Value - %s",
                   schema::TableName(table_index),
                   schema::ColumnName(table_index, column_index_),
                   *bind_addr,
                   ValueInBindAddrToStr(table_index,
                    const_cast<const void**>(bind_addr)).c_str(),
                   db_in_out_addr_,
                   ValueInBindAddrToStr(table_index,
                     const_cast<const void **>(&db_in_out_addr_)).c_str());
  } else if (io_code == kDalIoCodeMatch) {
    // Allocates memory for the size of dal_data_type for db_in_out_addr
    // Copies the min(dal_data_type, app_data_type) bytes of data from
    // bind_addr to db_match_addr
    if (AllocateAndCopy(&(db_match_addr_),
                        const_cast<const void**>(bind_addr),
                        app_data_type_, dal_data_type,
                        app_array_size_) != true) {
      UPLL_LOG_DEBUG("Failure Updating Match Bind Address for "
                     "Table(%s) Column(%s)",
                     schema::TableName(table_index),
                     schema::ColumnName(table_index, column_index_));
      return false;
    }
    UPLL_LOG_VERBOSE("Updated Match Binding for Table(%s) Column(%s):"
                   "\n User Address - %p, Value - %s"
                   "\n DAL Address - %p, Value - %s",
                   schema::TableName(table_index),
                   schema::ColumnName(table_index, column_index_),
                   *bind_addr,
                   ValueInBindAddrToStr(table_index,
                     const_cast<const void**>(bind_addr)).c_str(),
                   db_match_addr_,
                   ValueInBindAddrToStr(table_index,
                     const_cast<const void**>(&db_match_addr_)).c_str());
  }

  return true;
}  // DalBindColumnInfo::UpdateColumnInfo

// Copy result stored in DAL output buffer(db_in_out_addr_) to
// Appln output buffer(app_out_addr_)
bool
DalBindColumnInfo::CopyResultToAppAddr(const DalTableIndex table_index) {
  DalDataTypeCode dt_code;

  // Validating table_index
  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid table(%s) for Column(%s)",
                   schema::TableName(table_index),
                   schema::ColumnName(table_index, column_index_));
    return false;
  }

  if (column_index_ >= schema::TableNumCols(table_index) &&
      column_index_ != schema::DAL_COL_STD_INTEGER) {
    UPLL_LOG_DEBUG("Invalid Column(%s) for Table(%s)(columns - %zd)",
                   schema::ColumnName(table_index, column_index_),
                   schema::TableName(table_index),
                   schema::TableNumCols(table_index));
    return false;
  }

  // Finding smaller Datatype to perform copy operation
  // DB does not store unsigned data. To handle this
  // DB use larger datatype compared to application.
  // Copying smaller data in general to larger memory is safe
  GetCopyDataType(app_data_type_,
                  schema::ColumnDalDataTypeId(table_index, column_index_),
                  &dt_code);

  // PFC_ASSERT(!db_in_out_addr_);
  if (db_in_out_addr_ == NULL) {
    UPLL_LOG_DEBUG("Null DAL Output buffer");
    return false;
  }

  // PFC_ASSERT(!app_out_addr_);
  if (app_out_addr_ == NULL) {
    UPLL_LOG_DEBUG("Null User Output buffer");
    return false;
  }

  // Copies the contents available in Dal Output Address to User Output Address
  if (dt_code == kDalDtCodeApp) {
    if (CopyUsingAppDataType(&app_out_addr_,
                             const_cast<const void **>(&db_in_out_addr_),
                             app_data_type_,
                             app_array_size_) != true) {
      UPLL_LOG_DEBUG("Copy Result Failed for Table(%s) Column(%s):"
                     "\nSrc - DAL Output Address - %p, Value - %s"
                     "\nDest - User Output Address - %p, Value - %s",
                     schema::TableName(table_index),
                     schema::ColumnName(table_index, column_index_),
                     db_in_out_addr_,
                     ValueInBindAddrToStr(table_index,
                       const_cast<const void**>(&db_in_out_addr_)).c_str(),
                     app_out_addr_,
                     ValueInBindAddrToStr(table_index,
                       const_cast<const void**>(&app_out_addr_)).c_str());
      return false;
    }
  } else if (dt_code == kDalDtCodeDal) {
    if (CopyUsingDalDataType(&app_out_addr_,
          const_cast<const void **>(&db_in_out_addr_),
          schema::ColumnDalDataTypeId(table_index, column_index_),
          app_data_type_, app_array_size_, false) != true) {
      UPLL_LOG_DEBUG("Copy Result Failed for Column(%s) in Table(%s):"
                     "\nSrc - DAL Output Address - %p, Value - %s"
                     "\nDest - User Output Address - %p, Value - %s",
                     schema::ColumnName(table_index, column_index_),
                     schema::TableName(table_index),
                     db_in_out_addr_,
                     ValueInBindAddrToStr(table_index,
                       const_cast<const void**>(&db_in_out_addr_)).c_str(),
                     app_out_addr_,
                     ValueInBindAddrToStr(table_index,
                       const_cast<const void**>(&app_out_addr_)).c_str());
      return false;
    }
  } else {
    UPLL_LOG_DEBUG("Data Type mismatch");
    return false;
  }
  UPLL_LOG_VERBOSE("Copied result for Column(%s) in Table(%s): "
                 "Src(%p, %s) Dest (%p, %s)",
                 schema::ColumnName(table_index, column_index_),
                 schema::TableName(table_index),
                 db_in_out_addr_,
                 ValueInBindAddrToStr(table_index,
                   const_cast<const void**>(&db_in_out_addr_)).c_str(),
                 app_out_addr_,
                 ValueInBindAddrToStr(table_index,
                   const_cast<const void**>(&app_out_addr_)).c_str());
  return true;
}  // DalBindColumnInfo::CopyResultToAppAddr

// Private Methods of DalBindInfo class
// Allocates memory for the size of dal_data_type for db_in_out_addr
// Copies the min(dal_data_type, app_data_type) bytes of data from src to dest
bool
DalBindColumnInfo::AllocateAndCopy(void **dest,
                                   const void **src,
                                   const DalCDataType app_data_type,
                                   const SQLSMALLINT dal_data_type,
                                   const size_t array_size) {
  DalDataTypeCode dt_code = kDalDtCodeInvalid;

  // Validating inputs
  if (src == NULL) {
    UPLL_LOG_DEBUG("Null Source Address Reference");
    return false;
  }

  if (*src == NULL) {
    UPLL_LOG_DEBUG("Null Source Address");
    return false;
  }

  if (dest == NULL) {
    UPLL_LOG_DEBUG("Null Destination Address Reference");
    return false;
  }

  if (app_data_type == kDalAppTypeInvalid) {
    UPLL_LOG_DEBUG("Invalid App datatype(%d)",
                  app_data_type);
    return false;
  }

  if (dal_data_type == SQL_UNKNOWN_TYPE) {
    UPLL_LOG_DEBUG("Invalid DAL datatype(%d)",
                  dal_data_type);
    return false;
  }

  if (array_size == 0) {
    UPLL_LOG_DEBUG("Zero Array Size");
    return false;
  }

  // Allocate Memory based on the DAL C Datatype
  if (AllocateColBuffer(dest, dal_data_type, array_size) != true) {
    UPLL_LOG_DEBUG("Failure allocating memory for destination");
    return false;
  }

  // Validating the allocated memory
  if (*dest == NULL) {
    UPLL_LOG_DEBUG("Null Destination Address");
    return false;
  }

  // Finding smaller Datatype to perform copy operation
  // DB does not store unsigned data. To handle this
  // DB use larger datatype compared to application.
  // Copying smaller data in general to larger memory is safe
  GetCopyDataType(app_data_type, dal_data_type, &dt_code);

  // Copy contents of source address to destination address
  if (dt_code == kDalDtCodeApp) {
    if (CopyUsingAppDataType(dest, src,
                             app_data_type,
                             array_size) != true) {
      UPLL_LOG_DEBUG("Error while Copying data from source(%p) to "
                    "destination(%p)", *src, *dest);
      return false;
    }
  } else if (dt_code == kDalDtCodeDal) {
    if (CopyUsingDalDataType(dest, src,
                             dal_data_type,
                             app_data_type,
                             array_size, true) != true) {
      UPLL_LOG_DEBUG("Error while Copying data from source(%p) to "
                    "destination(%p)", *src, *dest);
      return false;
    }
  } else {
    UPLL_LOG_DEBUG("Cannot Copy Data - Datatype mismatch Error");
    return false;
  }
  UPLL_LOG_VERBOSE("Copied Data from source(%p) to destination(%p)",
               *src, *dest);

  return true;
}  // DalBindColumnInfo::AllocateAndCopy

// Allocates memory for the size of dal_data_type for db_in_out_addr
bool
DalBindColumnInfo::AllocateColBuffer(void **col_buff,
                                     const SQLSMALLINT dal_data_type,
                                     const size_t array_size) {
  size_t alloc_size = 0;

  // Validaing Inputs
  if (col_buff == NULL) {
    UPLL_LOG_DEBUG("Null Address Reference");
    return false;
  }

  if (dal_data_type == SQL_UNKNOWN_TYPE) {
    UPLL_LOG_DEBUG("Invalid DAL datatype(%d)", dal_data_type);
    return false;
  }

  if (array_size == 0) {
    UPLL_LOG_DEBUG("Zero Array Size");
    return false;
  }

  alloc_size = CalculateDalBufferSize(dal_data_type, array_size);
  if (alloc_size == 0) {
    // PFC_ASSERT_INT(alloc_size);
    UPLL_LOG_DEBUG("Zero Buffer Size");
    return false;
  }

  *col_buff = malloc(alloc_size);

  if (*col_buff == NULL) {
    // PFC_ASSERT(*col_buff);
    pfc_log_fatal("%s:%d: Error allocating Memory", __func__, __LINE__);
    return false;
  }
  memset(*col_buff, 0, alloc_size);

  UPLL_LOG_VERBOSE("Allocated Memory for %zd bytes", alloc_size);
  return true;
}  // DalBindColumnInfo::AllocateColBuffer

// Resets the DAL output buffer to store further results
bool
DalBindColumnInfo::ResetDalOutputBuffer(const DalTableIndex table_index) {
  // Validating table_index
  if (table_index >= schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid table(%s) for Column(%s)",
                   schema::TableName(table_index),
                   schema::ColumnName(table_index, column_index_));
    return false;
  }

  if (column_index_ >= schema::TableNumCols(table_index) &&
      column_index_ != schema::DAL_COL_STD_INTEGER) {
    UPLL_LOG_DEBUG("Invalid Column(%s) for Table(%s)(columns - %zd)",
                   schema::ColumnName(table_index, column_index_),
                   schema::TableName(table_index),
                   schema::TableNumCols(table_index));
    return false;
  }

  return (ResetColBuffer(&(db_in_out_addr_),
            schema::ColumnDalDataTypeId(table_index, column_index_),
            app_array_size_));
}  // DalBindColumnInfo::ResetDalOutputBuffer

// Wrapper to caluculate DAL Buffer size to allocate memory
size_t
DalBindColumnInfo::CalculateDalBufferSize(const SQLSMALLINT dal_data_type,
                                          const size_t array_size) {
  size_t buffer_size = 0;

  // PFC_ASSERT_NUM(array_size == 0);
  // Validating Inputs
  if (dal_data_type == SQL_UNKNOWN_TYPE) {
    UPLL_LOG_DEBUG("Invalid DAL datatype(%d)", dal_data_type);
    return false;
  }

  if (array_size == 0) {
    UPLL_LOG_DEBUG("Zero Array Size");
    return buffer_size;
  }

  switch (dal_data_type) {
    case SQL_C_CHAR:
      buffer_size = sizeof(SQLCHAR) * array_size;
      break;

    case SQL_C_TINYINT:
    case SQL_C_STINYINT:
      buffer_size = sizeof(SCHAR) * array_size;
      break;

    case SQL_C_UTINYINT:
      buffer_size = sizeof(UCHAR) * array_size;
      break;

    case SQL_C_SHORT:
    case SQL_C_SSHORT:
      buffer_size = sizeof(SQLSMALLINT) * array_size;
      break;

    case SQL_C_USHORT:
      buffer_size = sizeof(SQLUSMALLINT) * array_size;
      break;

    case SQL_C_LONG:
    case SQL_C_SLONG:
      buffer_size = sizeof(SQLINTEGER) * array_size;
      break;

    case SQL_C_ULONG:
      buffer_size = sizeof(SQLUINTEGER) * array_size;
      break;

    case SQL_C_SBIGINT:
      buffer_size = sizeof(SQLBIGINT) * array_size;
      break;

    case SQL_C_UBIGINT:
      buffer_size = sizeof(SQLUBIGINT) * array_size;
      break;

    case SQL_C_BINARY:
      buffer_size = sizeof(BYTE) * array_size;
      break;

    case SQL_C_NUMERIC:
      // Not supported for this release
      buffer_size = 0;
      break;

    case SQL_C_TIMESTAMP:
      // Not supported for this release
      buffer_size = 0;
      break;

    case SQL_C_GUID:
      // Not supported for this release
      buffer_size = 0;
      break;

    default:
      return 0;
  }
  UPLL_LOG_VERBOSE("Calculated DAL Buffer size - %zd"
                 " for DAL datatype(%d) and array size(%zd)",
                 buffer_size, dal_data_type, array_size);
  return buffer_size;
}  // DalBindColumnInfo::CalculateDalBufferSize

// Wrapper to calculate App Buffer size
size_t
DalBindColumnInfo::CalculateAppBufferSize(const DalCDataType app_data_type,
                                          const size_t array_size) {
  size_t buffer_size = 0;

  // PFC_ASSERT_NUM(array_size == 0);
  // Validating Inputs
  if (app_data_type == kDalAppTypeInvalid) {
    UPLL_LOG_DEBUG("Invalid App datatype(%d)", app_data_type);
    return buffer_size;
  }

  if (array_size == 0) {
    UPLL_LOG_DEBUG("Zero Array Size");
    return buffer_size;
  }

  switch (app_data_type) {
    case kDalChar:
      buffer_size = array_size * sizeof(char);  // NOLINT
      break;

    case kDalUint8:
      buffer_size = array_size * sizeof(uint8_t);
      break;

    case kDalUint16:
      buffer_size = array_size * sizeof(uint16_t);
      break;

    case kDalUint32:
      buffer_size = array_size * sizeof(uint32_t);
      break;

    case kDalUint64:
      buffer_size = array_size * sizeof(uint64_t);
      break;

    default:
      UPLL_LOG_DEBUG("Invalid App data type");
      return 0;
  }  // switch(app_data_type)

  UPLL_LOG_VERBOSE("Calculated App Buffer size - %zd"
                " for App datatype(%d) and array size(%zd)",
                buffer_size, app_data_type, array_size);
  return buffer_size;
}  // DalBindColumnInfo::CalculateDalBufferSize

// Finding smaller Datatype to perform copy operation
// DB does not store unsigned data. To handle this
// DB use larger datatype compared to application.
// Copying smaller data in general to larger memory is safe
void
DalBindColumnInfo::GetCopyDataType(const DalCDataType app_data_type,
                                   const SQLSMALLINT dal_data_type,
                                   DalDataTypeCode *data_type_code) {
  *data_type_code = kDalDtCodeInvalid;
  switch (dal_data_type) {
    case SQL_C_CHAR:
      if (app_data_type == kDalChar || app_data_type == kDalUint8) {
        *data_type_code = kDalDtCodeDal;
      }
      break;

    case SQL_C_TINYINT:
    case SQL_C_STINYINT:
    case SQL_C_UTINYINT:
      if (app_data_type == kDalChar ||
          app_data_type == kDalUint8 ||
          app_data_type == kDalUint16 ||
          app_data_type == kDalUint32 ||
          app_data_type == kDalUint64) {
        *data_type_code = kDalDtCodeDal;
      }
      break;

    case SQL_C_SHORT:
    case SQL_C_SSHORT:
    case SQL_C_USHORT:
      if (app_data_type == kDalChar ||
          app_data_type == kDalUint8) {
        *data_type_code = kDalDtCodeApp;
      } else if (app_data_type == kDalUint16 ||
          app_data_type == kDalUint32 ||
          app_data_type == kDalUint64) {
        *data_type_code = kDalDtCodeDal;
      }
      break;

    case SQL_C_LONG:
    case SQL_C_SLONG:
    case SQL_C_ULONG:
      if (app_data_type == kDalChar ||
          app_data_type == kDalUint8 ||
          app_data_type == kDalUint16) {
        *data_type_code = kDalDtCodeApp;
      } else if (app_data_type == kDalUint32 ||
          app_data_type == kDalUint64) {
        *data_type_code = kDalDtCodeDal;
      }
      break;

    case SQL_C_SBIGINT:
    case SQL_C_UBIGINT:
      if (app_data_type == kDalChar ||
          app_data_type == kDalUint8 ||
          app_data_type == kDalUint16 ||
          app_data_type == kDalUint32) {
        *data_type_code = kDalDtCodeApp;
      } else if (app_data_type == kDalUint64) {
        *data_type_code = kDalDtCodeDal;
      }
      break;

    case SQL_C_BINARY:
      *data_type_code = kDalDtCodeDal;
      break;

    case SQL_C_NUMERIC:
      // Not supported for this release
      *data_type_code = kDalDtCodeInvalid;
      break;

    case SQL_C_TIMESTAMP:
      // Not supported for this release
      *data_type_code = kDalDtCodeInvalid;
      break;

    case SQL_C_GUID:
      // Not supported for this release
      *data_type_code = kDalDtCodeInvalid;
      break;

    default:
      *data_type_code = kDalDtCodeInvalid;
      break;
  }
  UPLL_LOG_VERBOSE("Copy Data type code - %d", *data_type_code);
  return;
}  // DalBindColumnInfo::GetCopyDatatype

// Copy Using Appln Data Type
bool
DalBindColumnInfo::CopyUsingAppDataType(void **dest, const void **src,
                                        const DalCDataType app_data_type,
                                        const size_t array_size) {
  size_t i = 0;

  // Validating inputs
  // PFC_ASSERT(src);
  // PFC_ASSERT(*src);
  // PFC_ASSERT(dest);
  // PFC_ASSERT(*dest);
  // PFC_ASSERT_NUM(array_size == 0);
  if (src == NULL) {
    UPLL_LOG_DEBUG("Null Source Address Reference");
    return false;
  }

  if (*src == NULL) {
    UPLL_LOG_DEBUG("Null Source Address");
    return false;
  }

  if (dest == NULL) {
    UPLL_LOG_DEBUG("Null Destination Address Reference");
    return false;
  }

  if (*dest == NULL) {
    UPLL_LOG_DEBUG("Null Destination Address");
    return false;
  }

  if (app_data_type == kDalAppTypeInvalid) {
    UPLL_LOG_DEBUG("Invalid App datatype(%d)",
                  app_data_type);
    return false;
  }

  if (array_size == 0) {
    UPLL_LOG_DEBUG("Zero Array Size");
    return false;
  }

  // Copying Data from App according to the App data type
  switch (app_data_type) {
    case kDalChar:
      for (i = 0; i < array_size; i++) {
        *((reinterpret_cast<char *>(*dest))+i) =
          *((reinterpret_cast<const char *>(*src))+i);
        if (*((reinterpret_cast<const char *>(*src))+i) == '\0') {
          break;
        }
      }
      break;

    case kDalUint8:
      for (i = 0; i < array_size; i++) {
        *((reinterpret_cast<uint8_t *>(*dest))+i) =
          *((reinterpret_cast<const uint8_t *>(*src))+i);
      }
      break;

    case kDalUint16:
      for (i = 0; i < array_size; i++) {
        *((reinterpret_cast<uint16_t *>(*dest))+i) =
          *((reinterpret_cast<const uint16_t *>(*src))+i);
      }
      break;

    case kDalUint32:
      for (i = 0; i < array_size; i++) {
        *((reinterpret_cast<uint32_t *>(*dest))+i) =
          *((reinterpret_cast<const uint32_t *>(*src))+i);
      }
      break;

    case kDalUint64:
      for (i = 0; i < array_size; i++) {
        *((reinterpret_cast<uint64_t *>(*dest))+i) =
          *((reinterpret_cast<const uint64_t *>(*src))+i);
      }
      break;

    default:
      UPLL_LOG_DEBUG("Invalid App data type");
      return false;
  }  // switch(app_data_type)

  UPLL_LOG_VERBOSE("Copied contents of source address(%p) to "
               "destination(%p) address", *src, *dest);
  return true;
}  // DalBindColumnInfo::CopyUsingAppDataType

// Copy using Dal Data Type
bool
DalBindColumnInfo::CopyUsingDalDataType(void **dest, const void **src,
                                        const SQLSMALLINT dal_data_type,
                                        const DalCDataType app_data_type,
                                        const size_t array_size,
                                        const bool input) {
  size_t i = 0;
  // Validating inputs
  // PFC_ASSERT(src);
  // PFC_ASSERT(*src);
  // PFC_ASSERT(dest);
  // PFC_ASSERT(*dest);
  // PFC_ASSERT_NUM(array_size == 0);
  if (src == NULL) {
    UPLL_LOG_DEBUG("Null Source Address Reference");
    return false;
  }

  if (*src == NULL) {
    UPLL_LOG_DEBUG("Null Source Address");
    return false;
  }

  if (dest == NULL) {
    UPLL_LOG_DEBUG("Null Destination Address Reference");
    return false;
  }

  if (*dest == NULL) {
    UPLL_LOG_DEBUG("Null Destination Address");
    return false;
  }

  if (dal_data_type == SQL_UNKNOWN_TYPE) {
    UPLL_LOG_DEBUG("Invalid DAL datatype(%d)",
                  dal_data_type);
    return false;
  }

  if (array_size == 0) {
    UPLL_LOG_DEBUG("Zero Array Size");
    return false;
  }

  // Copying Data from App according to the App data type
  switch (dal_data_type) {
    case SQL_C_CHAR:
     for (i = 0; i < array_size; i++) {
        *((reinterpret_cast<SQLCHAR *>(*dest))+i) =
          *((reinterpret_cast<const SQLCHAR *>(*src))+i);
        if (*((reinterpret_cast<const SQLCHAR *>(*src))+i) == '\0') {
          break;
        }
      }
      break;

    case SQL_C_TINYINT:
    case SQL_C_STINYINT:
     for (i = 0; i < array_size; i++) {
        *((reinterpret_cast<SCHAR *>(*dest))+i) =
          *((reinterpret_cast<const SCHAR *>(*src))+i);
      }
      break;

    case SQL_C_UTINYINT:
     for (i = 0; i < array_size; i++) {
        *((reinterpret_cast<UCHAR *>(*dest))+i) =
          *((reinterpret_cast<const UCHAR *>(*src))+i);
      }
      break;

    case SQL_C_SHORT:
    case SQL_C_SSHORT:
     for (i = 0; i < array_size; i++) {
        *((reinterpret_cast<SQLSMALLINT *>(*dest))+i) =
          *((reinterpret_cast<const SQLSMALLINT *>(*src))+i);
      }
      break;

    case SQL_C_USHORT:
     for (i = 0; i < array_size; i++) {
        *((reinterpret_cast<SQLUSMALLINT *>(*dest))+i) =
          *((reinterpret_cast<const SQLUSMALLINT *>(*src))+i);
      }
      break;

    case SQL_C_LONG:
    case SQL_C_SLONG:
     for (i = 0; i < array_size; i++) {
        *((reinterpret_cast<SQLINTEGER *>(*dest))+i) =
          *((reinterpret_cast<const SQLINTEGER *>(*src))+i);
      }
      break;

    case SQL_C_ULONG:
     for (i = 0; i < array_size; i++) {
        *((reinterpret_cast<SQLUINTEGER *>(*dest))+i) =
          *((reinterpret_cast<const SQLUINTEGER *>(*src))+i);
      }
      break;

    case SQL_C_SBIGINT:
     for (i = 0; i < array_size; i++) {
        *((reinterpret_cast<SQLBIGINT *>(*dest))+i) =
          *((reinterpret_cast<const SQLBIGINT *>(*src))+i);
      }
      break;

    case SQL_C_UBIGINT:
     for (i = 0; i < array_size; i++) {
        *((reinterpret_cast<SQLUBIGINT *>(*dest))+i) =
          *((reinterpret_cast<const SQLUBIGINT *>(*src))+i);
      }
      break;

    case SQL_C_BINARY:
      if (app_data_type == kDalChar ||
          app_data_type == kDalUint8) {
        for (i = 0; i < array_size; i++) {
          *((reinterpret_cast<SQLCHAR *>(*dest))+i) =
            *((reinterpret_cast<const SQLCHAR *>(*src))+i);
        }
      } else if (app_data_type == kDalUint16) {
        if (array_size != 2) {
          UPLL_LOG_DEBUG("Wrong Array Size(%"PFC_PFMT_u64") for"
                         " App Data type(%d) and DAL datatype(%d)",
                        (uint64_t)array_size, app_data_type, dal_data_type);
          return false;
        }
        if (input == true) {
          *(reinterpret_cast<uint16_t *>(*dest)) =
            htons(*(reinterpret_cast<const uint16_t *>(*src)));
        } else {
          *(reinterpret_cast<uint16_t *>(*dest)) =
            ntohs(*(reinterpret_cast<const uint16_t *>(*src)));
        }
      } else if (app_data_type == kDalUint32) {
        if (array_size != 4) {
          UPLL_LOG_DEBUG("Wrong Array Size(%"PFC_PFMT_u64") for"
                         " App Data type(%d) and DAL datatype(%d)",
                        (uint64_t)array_size, app_data_type, dal_data_type);
          return false;
        }
          *(reinterpret_cast<uint32_t *>(*dest)) =
            (*(reinterpret_cast<const uint32_t *>(*src)));
#if 0
        if (input == true) {
          *(reinterpret_cast<uint32_t *>(*dest)) =
            htonl(*(reinterpret_cast<const uint32_t *>(*src)));
        } else {
          *(reinterpret_cast<uint32_t *>(*dest)) =
            ntohl(*(reinterpret_cast<const uint32_t *>(*src)));
        }
#endif
      } else if (app_data_type == kDalUint64) {
        if (array_size != 8) {
          UPLL_LOG_DEBUG("Wrong Array Size(%"PFC_PFMT_u64") for"
                         " App Data type(%d) and DAL datatype(%d)",
                        (uint64_t)array_size, app_data_type, dal_data_type);
          return false;
        }
        if (input == true) {
          *(reinterpret_cast<uint64_t *>(*dest)) =
            htonll(*(reinterpret_cast<const uint64_t *>(*src)));
        } else {
          *(reinterpret_cast<uint64_t *>(*dest)) =
            ntohll(*(reinterpret_cast<const uint64_t *>(*src)));
        }
      } else {
        return false;
      }
      break;

    case SQL_C_TIMESTAMP:
      UPLL_LOG_DEBUG("Type not supported yet");
      return false;

    case SQL_C_NUMERIC:
      UPLL_LOG_DEBUG("Type not supported yet");
      return false;

    case SQL_C_GUID:
      UPLL_LOG_DEBUG("Type not supported yet");
      return false;

    default:
      UPLL_LOG_DEBUG("Invalid DAL data type");
      return false;
  }

  UPLL_LOG_VERBOSE("Copied contents of source address(%p) to "
               "destination(%p) address", *src, *dest);
  return true;
}  // DalBindColumnInfo::CopyUsingDalDataType

// Resets Memory based on the type and array_size
bool
DalBindColumnInfo::ResetColBuffer(void **col_buff,
                                 const SQLSMALLINT dal_data_type,
                                 const size_t array_size) {
  size_t alloc_size = 0;

  if (col_buff == NULL) {
    UPLL_LOG_DEBUG("Null Column Buffer Reference");
    return false;
  }

  if (*col_buff == NULL) {
    UPLL_LOG_DEBUG("Null Column Buffer");
    return false;
  }

  if (dal_data_type == SQL_UNKNOWN_TYPE) {
    UPLL_LOG_DEBUG("Invalid DAL datatype(%d)",
                  dal_data_type);
    return false;
  }

  if (array_size == 0) {
    UPLL_LOG_DEBUG("Zero Array Size");
    return false;
  }

  alloc_size = CalculateDalBufferSize(dal_data_type, array_size);
  if (alloc_size == 0) {
    // PFC_ASSERT_INT(alloc_size);
    UPLL_LOG_DEBUG("Zero Buffer Size");
    return false;
  }

  memset(*col_buff, 0, alloc_size);

  UPLL_LOG_VERBOSE("Contents of address(%p) reset to 0",
               *col_buff);
  return true;
}  // DalBindColumnInfo::ResetColBuffer

// Finding relevant IoType
// When multiple match binding required, change it here
DalIoType
DalBindColumnInfo::ComputeIoType(const DalIoType curr_io_type,
                                 const DalIoCode io_code) {
  switch (curr_io_type) {
    case kDalIoNone:
      if (io_code == kDalIoCodeInput) {
        return kDalIoInputOnly;
      } else if (io_code == kDalIoCodeOutput) {
        return kDalIoOutputOnly;
      } else if (io_code == kDalIoCodeMatch) {
        return kDalIoMatchOnly;
      }
      break;

    case kDalIoInputOnly:
      if (io_code == kDalIoCodeMatch) {
        return kDalIoInputAndMatch;
      }
      break;

    case kDalIoOutputOnly:
      if (io_code == kDalIoCodeOutput) {
        return kDalIoOutputOnly;
      } else if (io_code == kDalIoCodeMatch) {
        return kDalIoOutputAndMatch;
      }
      break;

    case kDalIoMatchOnly:
      if (io_code == kDalIoCodeInput) {
        return kDalIoInputAndMatch;
      } else if (io_code == kDalIoCodeOutput) {
        return kDalIoOutputAndMatch;
      } else if (io_code == kDalIoCodeMatch) {
        return kDalIoMatchOnly;
      }
      break;

    case kDalIoInputAndMatch:
      if (io_code == kDalIoCodeMatch) {
        return kDalIoInputAndMatch;
      }
      break;

    case kDalIoOutputAndMatch:
      if (io_code == kDalIoCodeOutput) {
        return kDalIoOutputAndMatch;
      } else if (io_code == kDalIoCodeMatch) {
        return kDalIoOutputAndMatch;
      }
      break;

    default:
      break;
  }  // switch(curr_io_type)
  return kDalIoInvalid;
}  // DalBindColumnInfo::ComputeIoType

// Is Binding possible for the given io_code on this instance
bool
DalBindColumnInfo::GetBindAvailability(const DalIoCode io_code,
                                       bool *avail) {
  switch (io_code) {
    case kDalIoCodeInput:
      if (io_type_ == kDalIoNone ||
          io_type_ == kDalIoMatchOnly) {
        *avail = true;
      } else {
        return false;
      }
      break;
    case kDalIoCodeOutput:
      if (io_type_ == kDalIoNone ||
          io_type_ == kDalIoMatchOnly) {
        *avail = true;
      } else if (io_type_ == kDalIoOutputOnly ||
          io_type_ == kDalIoOutputAndMatch) {
        *avail = false;
      } else {
        return false;
      }
      break;
    case kDalIoCodeMatch:
      if (io_type_ == kDalIoNone ||
          io_type_ == kDalIoInputOnly ||
          io_type_ == kDalIoOutputOnly) {
        *avail = true;
      } else if (io_type_ == kDalIoMatchOnly ||
          io_type_ == kDalIoInputAndMatch ||
          io_type_ == kDalIoOutputAndMatch) {
        *avail = false;
      } else {
        return false;
      }
      break;

    default:
      return false;
  }  // switch(io_code)
  return true;
}  // DalBindColumnInfo::GetBindAvailability

// Compare Datatype of Appln and Schema
bool
DalBindColumnInfo::IsDataTypeCompatible(const DalCDataType app_data_type,
                                        const SQLSMALLINT dal_data_type) {
  switch (dal_data_type) {
    case SQL_C_CHAR:
      if (app_data_type == kDalChar) {
        return true;
      }
      break;

    case SQL_C_TINYINT:
    case SQL_C_STINYINT:
      if (app_data_type == kDalChar) {
        return true;
      }
      break;

    case SQL_C_UTINYINT:
      if (app_data_type == kDalChar || app_data_type == kDalUint8) {
        return true;
      }
      break;

    case SQL_C_SHORT:
    case SQL_C_SSHORT:
      if (app_data_type == kDalChar || app_data_type == kDalUint8) {
        return true;
      }
      break;

    case SQL_C_USHORT:
      if (app_data_type == kDalChar ||
          app_data_type == kDalUint8 ||
          app_data_type == kDalUint16) {
        return true;
      }
      break;

    case SQL_C_LONG:
    case SQL_C_SLONG:
      if (app_data_type == kDalChar ||
          app_data_type == kDalUint8 ||
          app_data_type == kDalUint16) {
        return true;
      }
      break;

    case SQL_C_ULONG:
      if (app_data_type == kDalChar ||
          app_data_type == kDalUint8 ||
          app_data_type == kDalUint16 ||
          app_data_type == kDalUint32) {
        return true;
      }
      break;

    case SQL_C_SBIGINT:
      if (app_data_type == kDalChar ||
          app_data_type == kDalUint8 ||
          app_data_type == kDalUint16 ||
          app_data_type == kDalUint32) {
        return true;
      }
      break;

    case SQL_C_UBIGINT:
      if (app_data_type == kDalChar ||
          app_data_type == kDalUint8 ||
          app_data_type == kDalUint16 ||
          app_data_type == kDalUint32 ||
          app_data_type == kDalUint64) {
        return true;
      }
      break;

    case SQL_C_BINARY:
      if (app_data_type == kDalChar ||
          app_data_type == kDalUint8 ||
          app_data_type == kDalUint16 ||
          app_data_type == kDalUint32 ||
          app_data_type == kDalUint64) {
        return true;
      }
      break;

    case SQL_C_TIMESTAMP:
      // Not supported for this release
      break;

    case SQL_C_NUMERIC:
      // Not supported for this release
      break;

    case SQL_C_GUID:
      // Not supported for this release
      break;

    default:
      break;
  }
  return false;
}  // DalBindColumnInfo::IsDataTypeCompatible

std::string
DalBindColumnInfo::ColInfoToStr(const DalTableIndex table_index) const {
  std::stringstream ss;
  ss << "\n  Column Name : " << schema::ColumnName(table_index, column_index_)
     << "\n  App Data Type : " << AppDataTypeToStr(app_data_type_)
     << "\n  App Array Size : " << app_array_size_
     << "\n  Bind Type : " << DalIoTypeToStr(io_type_);
  if (app_out_addr_ != NULL) {
    ss  << "\n  App Output Address : " << app_out_addr_
        << "\n  App Output Value : "
        << ValueInBindAddrToStr(table_index,
                            const_cast<const void**>(&app_out_addr_));
  }
  if (db_in_out_addr_ != NULL) {
    ss << "\n  DAL In/Out Address : " << db_in_out_addr_
       << "\n  DAL In/Out Value : "
       << ValueInBindAddrToStr(table_index,
                            const_cast<const void**>(&db_in_out_addr_));
  }
  if (db_match_addr_ != NULL) {
    ss << "\n  DAL Match Address : " << db_match_addr_
       << "\n  DAL Match Value : "
       << ValueInBindAddrToStr(table_index,
                            const_cast<const void**>(&db_match_addr_));
  }
  if (buff_len_ptr_ != NULL) {
    ss << "\n  DAL BuffLen : " << *buff_len_ptr_;
  }
  return ss.str();
}  // DalBindColumnInfo::ColInfoToStr

std::string
DalBindColumnInfo::ColInfoInputToStr(const DalTableIndex table_index) const {
  std::stringstream ss;
  ss << "{" << schema::ColumnName(table_index, column_index_) << ", "
     << AppDataTypeToStr(app_data_type_) << ", "
     << app_array_size_ << ", " << DalIoTypeToStr(io_type_);
  if (io_type_ == kDalIoOutputOnly || io_type_ == kDalIoOutputAndMatch) {
    if (app_out_addr_ != NULL) {
      ss  << ", OUTPUT[" << app_out_addr_ << "]";
    } else {
      ss << ", OUTPUT[NULL]";
    }
  }
  if (io_type_ == kDalIoInputOnly || io_type_ == kDalIoInputAndMatch) {
    if (db_in_out_addr_ != NULL) {
      ss << ", INPUT[" << db_in_out_addr_ << ", "
         << ValueInBindAddrToStr(table_index,
                            const_cast<const void**>(&db_in_out_addr_)) << "]";
    } else {
      ss << ", INPUT[NULL]";
    }
  }
  if (io_type_ == kDalIoMatchOnly || io_type_ == kDalIoInputAndMatch ||
      io_type_ == kDalIoOutputAndMatch) {
    if (db_match_addr_ != NULL) {
      ss << ", MATCH[" << db_match_addr_ << ", "
       << ValueInBindAddrToStr(table_index,
                            const_cast<const void**>(&db_match_addr_)) << "]";
    } else {
      ss << ", MATCH[NULL]";
    }
  }
  ss << "}";
  return ss.str();
}  // DalBindColumnInfo::ColInfoInputToStr

std::string
DalBindColumnInfo::ColInfoResultToStr(const DalTableIndex table_index) const {
  std::stringstream ss;
  if (io_type_ == kDalIoOutputOnly || io_type_ == kDalIoOutputAndMatch) {
    ss << "[";
    if (app_out_addr_ != NULL) {
       ss << schema::ColumnName(table_index, column_index_) << ", "
          << ValueInBindAddrToStr(table_index,
                            const_cast<const void**>(&app_out_addr_));
    } else {
      ss << "NULL";
    }
    ss << "] ";
  }
  return ss.str();
}  // DalBindColumnInfo::ColInfoResultToStr

std::string
DalBindColumnInfo::AppDataTypeToStr(DalCDataType app_data_type) const {
  switch (app_data_type) {
    case kDalChar:
      return std::string("Char");
    case kDalUint8:
      return std::string("Uint8");
    case kDalUint16:
      return std::string("Uint16");
    case kDalUint32:
      return std::string("Uint32");
    case kDalUint64:
      return std::string("Uint64");
    default:
      return std::string("Invalid App Datatype");
  }
}  // DalBindColumnInfo::AppDataTypeToStr

std::string
DalBindColumnInfo::DalDataTypeToStr(SQLSMALLINT dal_data_type) const {
  switch (dal_data_type) {
    case SQL_C_CHAR:
      return "SQL_C_CHAR";
    case SQL_C_TINYINT:
      return "SQL_C_TINYNT";
    case SQL_C_STINYINT:
      return "SQL_C_STINYNT";
    case SQL_C_UTINYINT:
      return "SQL_C_UTINYNT";
    case SQL_C_SHORT:
      return "SQL_C_SHORT";
    case SQL_C_SSHORT:
      return "SQL_C_SSHORT";
    case SQL_C_USHORT:
      return "SQL_C_USHORT";
    case SQL_C_LONG:
      return "SQL_C_LONG";
    case SQL_C_SLONG:
      return "SQL_C_SLONG";
    case SQL_C_ULONG:
      return "SQL_C_ULONG";
    case SQL_C_SBIGINT:
      return "SQL_C_SBIGINT";
    case SQL_C_UBIGINT:
      return "SQL_C_UBIGINT";
    case SQL_C_BINARY:
      return "SQL_C_BINARY";
    case SQL_C_NUMERIC:
      return "Unsupported Datatype ";
    case SQL_C_TIMESTAMP:
      return "Unsupported Datatype ";
    case SQL_C_GUID:
      return "Unsupported Datatype ";
    default:
      return "Invalid Datatype ";
  }  // switch(dal_data_type)
}  // DalBindColumnInfo::DalDataTypeToStr

std::string
DalBindColumnInfo::DalIoCodeToStr(DalIoCode io_code) {
  switch (io_code) {
    case kDalIoCodeInput:
      return std::string("Input");
    case kDalIoCodeOutput:
      return std::string("Output");
    case kDalIoCodeMatch:
      return std::string("Match");
    default:
      return std::string("Invalid IoCode");
  }
}

std::string
DalBindColumnInfo::DalIoTypeToStr(DalIoType io_type) const {
  switch (io_type) {
    case kDalIoNone:
      return std::string("None");
    case kDalIoInputOnly:
      return std::string("InputOnly");
    case kDalIoOutputOnly:
      return std::string("OutputOnly");
    case kDalIoMatchOnly:
      return std::string("MatchOnly");
    case kDalIoInputAndMatch:
      return std::string("InputAndMatch");
    case kDalIoOutputAndMatch:
      return std::string("OutputAndMatch");
    default:
      return std::string("Invalid IoType");
  }
}

std::string
DalBindColumnInfo::ValueInBindAddrToStr(const DalTableIndex table_index,
                                        const void **addr) const {
  DalDataTypeCode dt_code = kDalDtCodeInvalid;

  if (addr == NULL || *addr == NULL) {
    return std::string("(null)");
  }
  GetCopyDataType(app_data_type_,
                  schema::ColumnDalDataTypeId(table_index, column_index_),
                  &dt_code);
  if (dt_code == kDalDtCodeApp) {
    return AppValueToStr(app_data_type_, addr);
  } else if (dt_code == kDalDtCodeDal) {
    return DalValueToStr(
                   schema::ColumnDalDataTypeId(table_index, column_index_),
                   addr);
  }
  return std::string("Invalid Datatype");
}  // DalBindColumnInfo::PrintValueInBindAddr


// Printing values based on Appln datatype
std::string
DalBindColumnInfo::AppValueToStr(const DalCDataType app_data_type,
                                 const void **addr) const {
  std::stringstream ss;

  if (addr == NULL || *addr == NULL) {
    ss << "(null)";
    return ss.str();
  }

  switch (app_data_type) {
    case kDalChar:
      for (size_t i = 0; i < app_array_size_; i++) {
        if (*((reinterpret_cast<const char*>(*addr))+i) == 0) {
          break;
        }
        ss << *((reinterpret_cast<const char *>(*addr))+i);
      }
      break;

    case kDalUint8:
      {
        char xtemp[3];
        ss << "0x";
        for (size_t i = 0; i < app_array_size_; i++) {
          memset(xtemp, 0, 3);
          snprintf(xtemp, sizeof(xtemp),
                   "%02X", *((reinterpret_cast<const uint8_t *>(*addr))+i));
          ss << xtemp;
        }
      }
      break;

    case kDalUint16:
      for (size_t i = 0; i < app_array_size_; i++) {
        ss << *((reinterpret_cast<const uint16_t  *>(*addr))+i) << " ";
      }
      break;

    case kDalUint32:
      for (size_t i = 0; i < app_array_size_; i++) {
        ss << *((reinterpret_cast<const uint32_t  *>(*addr))+i) << " ";
      }
      break;

    case kDalUint64:
      for (size_t i = 0; i < app_array_size_; i++) {
        ss << *((reinterpret_cast<const uint64_t  *>(*addr))+i) << " ";
      }
      break;

    default:
      ss << "Invalid Datatype ";
  }  // switch(app_data_type)

  return ss.str();
}  // DalBindColumnInfo::AppValueToStr

// Printing values based on DAL datatype
std::string
DalBindColumnInfo::DalValueToStr(const SQLSMALLINT dal_data_type,
                                 const void **addr) const {
  std::stringstream ss;
  SCHAR schar_temp = 0;
  UCHAR uchar_temp = 0;

  if (addr == NULL || *addr == NULL) {
    ss << "(null)";
    return ss.str();
  }

  switch (dal_data_type) {
    case SQL_C_CHAR:
      for (size_t i = 0; i < app_array_size_; i++) {
        if (*((reinterpret_cast<const SQLCHAR *>(*addr))+i) == 0) {
          break;
        }
        ss << *((reinterpret_cast<const SQLCHAR *>(*addr))+i);
      }
      break;

    case SQL_C_TINYINT:
    case SQL_C_STINYINT:
      for (size_t i = 0; i < app_array_size_; i++) {
        schar_temp = *((reinterpret_cast<const SCHAR *>(*addr))+i);
        ss << (SQLSMALLINT)(schar_temp) << " ";
      }
      break;

    case SQL_C_UTINYINT:
      for (size_t i = 0; i < app_array_size_; i++) {
        uchar_temp = *((reinterpret_cast<const UCHAR *>(*addr))+i);
        ss << (SQLSMALLINT)(uchar_temp) << " ";
      }
      break;

    case SQL_C_SHORT:
    case SQL_C_SSHORT:
      for (size_t i = 0; i < app_array_size_; i++) {
        ss << *((reinterpret_cast<const SQLSMALLINT *>(*addr))+i);
      }
      break;

    case SQL_C_USHORT:
      for (size_t i = 0; i < app_array_size_; i++) {
        ss << *((reinterpret_cast<const SQLUSMALLINT *>(*addr))+i);
      }
      break;

    case SQL_C_LONG:
    case SQL_C_SLONG:
      for (size_t i = 0; i < app_array_size_; i++) {
        ss << *((reinterpret_cast<const SQLINTEGER *>(*addr))+i);
      }
      break;

    case SQL_C_ULONG:
      for (size_t i = 0; i < app_array_size_; i++) {
        ss << *((reinterpret_cast<const SQLUINTEGER *>(*addr))+i);
      }
      break;

    case SQL_C_SBIGINT:
      for (size_t i = 0; i < app_array_size_; i++) {
        ss << *((reinterpret_cast<const SQLBIGINT *>(*addr))+i);
      }
      break;

    case SQL_C_UBIGINT:
      for (size_t i = 0; i < app_array_size_; i++) {
        ss << *((reinterpret_cast<const SQLUBIGINT *>(*addr))+i);
      }
      break;

    case SQL_C_BINARY:
      {
        char xtemp[3];
        ss << "0x";
        for (size_t i = 0; i < app_array_size_; i++) {
          memset(xtemp, 0, 3);
          snprintf(xtemp, sizeof(xtemp),
                   "%02X", *((reinterpret_cast<const SCHAR *>(*addr))+i));
          ss << xtemp;
        }
      }
      break;

    case SQL_C_NUMERIC:
      // Not supported for this release
      ss << "Unsupported Datatype ";
      break;

    case SQL_C_TIMESTAMP:
      // Not supported for this release
      ss << "Unsupported Datatype ";
      break;

    case SQL_C_GUID:
      // Not supported for this release
      ss << "Unsupported Datatype ";
      break;

    default:
      ss << "Invalid Datatype ";
  }  // switch(dal_data_type)

  return ss.str();
}  // DalBindColumnInfo::DalValToStr
}  // namespace dal
}  // namespace upll
}  // namespace unc
