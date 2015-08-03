/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * dal_bind_info.cc
 *   Implementation of methods in DalBindInfo
 */


#include <sstream>
#include "uncxx/upll_log.hh"
#include "dal_bind_info.hh"

namespace unc {
namespace upll {
namespace dal {

// Public Methods of DalBindInfo Class
// Constructor
DalBindInfo::DalBindInfo(const DalTableIndex table_index) {
  // PFC_ASSERT_NUM()
  if (table_index > schema::table::kDalNumTables) {
    UPLL_LOG_DEBUG("Invalid Table Index(%d)", table_index);
  }
  table_index_ = table_index;
  input_bind_count_ = 0;
  output_bind_count_ = 0;
  match_bind_count_ = 0;
}  // DalBindInfo::DalBindInfo

// Destructor
DalBindInfo::~DalBindInfo(void) {
  DalBindList::iterator iter;
  DalBindColumnInfo *col_info = NULL;

  while (!bind_list_.empty()) {
    col_info = bind_list_.back();
    bind_list_.pop_back();
    delete col_info;
  }
  if (bind_list_.empty()) {
    UPLL_LOG_TRACE("Cleared Dal Bind list for table index(%d)",
                   table_index_);
  } else {
    UPLL_LOG_DEBUG("Dal Bind list not cleared for table index(%d)",
                   table_index_);
  }
}  // DalBindInfo::~DalBindInfo

// Binds input address from dal user
bool
DalBindInfo::BindInput(const DalColumnIndex column_index,
                       const DalCDataType app_data_type,
                       const size_t array_size,
                       const void *bind_addr) {
  if (ValidateInputParams(column_index, app_data_type,
                          array_size, bind_addr) != true) {
    UPLL_LOG_DEBUG("Input parameter not valid");
    return false;
  }

  if (BindAttribute(kDalIoCodeInput, column_index, app_data_type,
                    array_size, &bind_addr) != true) {
    UPLL_LOG_DEBUG("Failed Binding Input for Column(%s) in Table(%s)",
                   schema::ColumnName(table_index_, column_index),
                   schema::TableName(table_index_));
    return false;
  }
  input_bind_count_++;

  UPLL_LOG_VERBOSE("Successfully Bind Input for Column(%s) in Table(%s)",
                 schema::ColumnName(table_index_, column_index),
                 schema::TableName(table_index_));
  return true;
}  // DalBindInfo::BindInput

// Binds output address from dal user
bool
DalBindInfo::BindOutput(const DalColumnIndex column_index,
                        const DalCDataType app_data_type,
                        const size_t array_size,
                        const void *bind_addr) {
  if (ValidateInputParams(column_index, app_data_type,
                          array_size, bind_addr) != true) {
    UPLL_LOG_DEBUG("Input parameter not valid");
    return false;
  }

  if (BindAttribute(kDalIoCodeOutput, column_index, app_data_type,
                    array_size, &bind_addr) != true) {
    UPLL_LOG_DEBUG("Failed Binding Output for Column(%s) in Table(%s)",
                   schema::ColumnName(table_index_, column_index),
                   schema::TableName(table_index_));
    return false;
  }
  output_bind_count_++;
  UPLL_LOG_VERBOSE("Successfully Bind Output for Column(%s) in Table(%s)",
                 schema::ColumnName(table_index_, column_index),
                 schema::TableName(table_index_));
  return true;
}  // DalBindInfo::BindOutput

// Binds match address from dal user
bool
DalBindInfo::BindMatch(const DalColumnIndex column_index,
                       const DalCDataType app_data_type,
                       const size_t array_size,
                       const void *bind_addr) {
  if (ValidateInputParams(column_index, app_data_type,
                          array_size, bind_addr) != true) {
    UPLL_LOG_DEBUG("Input parameter not valid");
    return false;
  }

  if (BindAttribute(kDalIoCodeMatch, column_index, app_data_type,
                    array_size, &bind_addr) != true) {
    UPLL_LOG_DEBUG("Failed Binding Match for Column(%s) in Table(%s)",
                   schema::ColumnName(table_index_, column_index),
                   schema::TableName(table_index_));
    return false;
  }
  match_bind_count_++;
  UPLL_LOG_VERBOSE("Successfully Bind Match for Column(%s) in Table(%s)",
                 schema::ColumnName(table_index_, column_index),
                 schema::TableName(table_index_));
  return true;
}  // DalBindInfo::BindMatch

// Copy result from DB to Appln buffer
bool
DalBindInfo::CopyResultToApp(void) {
  DalBindList::iterator iter;
  DalBindColumnInfo *col_info = NULL;

  for (iter = bind_list_.begin();
       iter != bind_list_.end();
       ++iter) {
    col_info = reinterpret_cast<DalBindColumnInfo *>(*iter);
    // PFC_ASSERT(col_info);
    if (col_info == NULL) {
      UPLL_LOG_DEBUG("NULL Bind Column Info for Table(%s)",
                     schema::TableName(table_index_));
      return false;
    }

    if (col_info->get_io_type() == kDalIoOutputOnly ||
        col_info->get_io_type() == kDalIoOutputAndMatch) {
      if (col_info->CopyResultToAppAddr(table_index_) != true) {
        UPLL_LOG_DEBUG("Failed copying result for Column(%s) in Table(%s)",
                       schema::ColumnName(table_index_,
                         col_info->get_column_index()),
                       schema::TableName(table_index_));
        return false;
      }
    }
  }

  UPLL_LOG_VERBOSE("Succesfully copied result for all Columns bound for output "
                 "in Table(%s)", schema::TableName(table_index_));
  return true;
}  // DalBindInfo::CopyResultToApp

// App Output Buffer
// Copy result from DB to Appln buffer
bool
DalBindInfo::ResetDalOutBuffer(void) {
  DalBindList::iterator iter;
  DalBindColumnInfo *col_info = NULL;

  for (iter = bind_list_.begin();
       iter != bind_list_.end();
       ++iter) {
    col_info = reinterpret_cast<DalBindColumnInfo *>(*iter);
    // PFC_ASSERT(col_info);
    if (col_info == NULL) {
      UPLL_LOG_DEBUG("NULL Bind Column Info for Table(%s)",
                     schema::TableName(table_index_));
      return false;
    }

    if (col_info->get_io_type() == kDalIoOutputOnly ||
        col_info->get_io_type() == kDalIoOutputAndMatch) {
      if (col_info->ResetDalOutputBuffer(table_index_) != true) {
        UPLL_LOG_DEBUG("Failed Restting DAL output buffer for Column(%s) "
                       "in Table(%s)",
                       schema::ColumnName(table_index_,
                       col_info->get_column_index()),
                       schema::TableName(table_index_));
        return false;
      }
    }
  }
  UPLL_LOG_VERBOSE("Succesfully reset DAL output buffer for all Columns bound "
                 "for output in Table(%s)", schema::TableName(table_index_));
  return true;
}  // DalBindInfo::ResetDalOutBuffer

// Private Methods of DalBindInfo
void
DalBindInfo::InitBindList(void) {
  DalBindColumnInfo *col_info;
  // Initialize each column and add it to the list
  for (uint16_t column_index = 0;
       column_index <
       static_cast<uint16_t>(schema::TableNumCols(table_index_));
       column_index++) {
    col_info = new DalBindColumnInfo(column_index);
    // PFC_ASSERT(col_info);
    // vector throws exception if it fails allocating memory
    bind_list_.push_back(col_info);
  }
}  // DalBindInfo::InitBindList


// Wrapper for BindInput/Output/Match
bool
DalBindInfo::BindAttribute(const DalIoCode io_code,
                           const DalColumnIndex column_index,
                           const DalCDataType app_data_type,
                           const size_t array_size,
                           const void **bind_addr) {
  DalBindColumnInfo *col_info = NULL;
  DalBindList::iterator iter;
  bool avail = false;

  if (bind_addr == NULL) {
    UPLL_LOG_DEBUG("Null Bind Address reference for Column(%s) in Table(%s)",
                   schema::ColumnName(table_index_, column_index),
                   schema::TableName(table_index_));
    return false;
  }

  if (io_code == kDalIoCodeInvalid) {
    UPLL_LOG_DEBUG("Invalid IO Code for Column(%s) in Table(%s)",
                   schema::ColumnName(table_index_, column_index),
                   schema::TableName(table_index_));
  }

  if (ValidateInputParams(column_index, app_data_type,
                          array_size, *bind_addr) != true) {
    UPLL_LOG_DEBUG("Input parameter not valid");
    return false;
  }

  // Create new DalBindColumnInfo instance if
  //   1. there is no instance available
  //   2. there is an instance and binding is already done.
  // Update the exisiting instance if binding is not done before.
  for (iter = bind_list_.begin(); iter != bind_list_.end(); ++iter) {
    col_info = *iter;
    if (col_info == NULL) {
      return false;
    }
    if (col_info->get_column_index() == column_index) {
      if (col_info->GetBindAvailability(io_code, &avail) == false) {
        UPLL_LOG_DEBUG("%s Binding already done/invalid for"
                       " table(%s) column(%s)",
                       DalBindColumnInfo::DalIoCodeToStr(io_code).c_str(),
                       schema::TableName(table_index_),
                       schema::ColumnName(table_index_,
                                          col_info->get_column_index()));
        return false;
      }
      break;
    }
  }
  if (avail == false) {
    col_info = new DalBindColumnInfo(column_index);
    bind_list_.push_back(col_info);
  }

  if (col_info->UpdateColumnInfo(
                  table_index_,
                  app_data_type,
                  schema::ColumnDalDataTypeId(table_index_, column_index),
                  array_size,
                  io_code,
                  bind_addr) != true) {
    UPLL_LOG_DEBUG("Failed Update Column Info for Column(%s) in Table(%s)",
                   schema::ColumnName(table_index_, column_index),
                   schema::TableName(table_index_));
    return false;
  }
  UPLL_LOG_VERBOSE("Successfully Updated Column Info for Column(%s) in "
                  "Table(%s)", schema::ColumnName(table_index_, column_index),
                  schema::TableName(table_index_));
  return true;
}  // DalBindInfo::BindAttribute

// Methods for Debugging and Testing
std::string
DalBindInfo::BindListToStr(void) {
  DalBindList::iterator iter;
  DalBindColumnInfo *col_info = NULL;
  std::stringstream ss;

  if (input_bind_count_ == 0 &&
      output_bind_count_ == 0 &&
      match_bind_count_ == 0) {
    UPLL_LOG_DEBUG("No Columns in Table(%s) bound for Input/Ouput/Match",
                   schema::TableName(table_index_));
  }

  ss << "\n*************************************"
     << "\n* Bind Info for Table - "
     << schema::TableName(table_index_);

  for (iter = bind_list_.begin();
       iter != bind_list_.end();
       ++iter) {
    col_info = reinterpret_cast<DalBindColumnInfo *>(*iter);
    if (col_info->get_io_type() != kDalIoNone &&
        col_info->get_io_type() != kDalIoInvalid) {
      ss  << "\n------------------------------------";
      ss << col_info->ColInfoToStr(table_index_).c_str();
    }
  }
  ss << "\n*************************************\n";
  return (ss.str());
}  // DalBindInfo::BindListToStr

std::string
DalBindInfo::BindListInputToStr(void) {
  DalBindList::iterator iter;
  DalBindColumnInfo *col_info = NULL;
  std::stringstream ss;

  if (input_bind_count_ == 0 &&
      output_bind_count_ == 0 &&
      match_bind_count_ == 0) {
    UPLL_LOG_DEBUG("No Columns in Table(%s) bound for Input/Ouput/Match",
                   schema::TableName(table_index_));
  }

  ss << "\n*************************************"
     << "\n* Input data for Table - "
     << schema::TableName(table_index_);
  ss << "\n*************************************";

  for (iter = bind_list_.begin();
       iter != bind_list_.end();
       ++iter) {
    col_info = reinterpret_cast<DalBindColumnInfo *>(*iter);
    ss << "\n" << col_info->ColInfoInputToStr(table_index_).c_str();
  }
  ss << "\n*************************************";
  return (ss.str());
}  // DalBindInfo::BindListInputToStr

std::string
DalBindInfo::BindListResultToStr(void) {
  DalBindList::iterator iter;
  DalBindColumnInfo *col_info = NULL;
  std::stringstream ss;

  if (output_bind_count_ == 0) {
    UPLL_LOG_DEBUG("No Columns in Table(%s) bound for Ouput",
                   schema::TableName(table_index_));
  }

  ss << "\n*************************************"
     << "\n* Result Data for Table - "
     << schema::TableName(table_index_);
  ss << "\n*************************************";
  ss << "\n{ ";

  for (iter = bind_list_.begin();
       iter != bind_list_.end();
       ++iter) {
    col_info = reinterpret_cast<DalBindColumnInfo *>(*iter);
    ss << col_info->ColInfoResultToStr(table_index_).c_str();
  }
  ss << "}";
  ss << "\n*************************************";
  UPLL_LOG_VERBOSE("\n%s", ss.str().c_str());
  return (ss.str());
}  // DalBindInfo::BindListResultToStr

bool
DalBindInfo::ValidateInputParams(const DalColumnIndex column_index,
                                 const DalCDataType app_data_type,
                                 const size_t array_size,
                                 const void *bind_addr) {
  if (column_index >= schema::TableNumCols(table_index_) &&
      column_index != schema::DAL_COL_STD_INTEGER) {
    UPLL_LOG_DEBUG("Invalid Column Index(%d) for Table(%s)",
                   column_index, schema::TableName(table_index_));
    return false;
  }

  if (app_data_type == kDalAppTypeInvalid) {
    UPLL_LOG_DEBUG("Invalid App data type for Column(%s) in Table(%s)",
                   schema::ColumnName(table_index_, column_index),
                   schema::TableName(table_index_));
    return false;
  }

  if (array_size == 0) {
    UPLL_LOG_DEBUG("Zero Array Size for Column(%s) in Table(%s)",
                   schema::ColumnName(table_index_, column_index),
                   schema::TableName(table_index_));
    return false;
  }

  if (bind_addr == NULL) {
    UPLL_LOG_DEBUG("Null Bind Address for Column(%s) in Table(%s)",
                   schema::ColumnName(table_index_, column_index),
                   schema::TableName(table_index_));
    return false;
  }
  return true;
}  // DalBindInfo::ValidateInputParams

}  // namespace dal
}  // namespace upll
}  // namespace unc
