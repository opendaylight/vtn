/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * dal_cusor.cc
 *   Implementation of methods in DalCursor
 */ 

#include "uncxx/upll_log.hh"
#include "dal_error_handler.hh"
#include "dal_cursor.hh"

namespace unc {
namespace upll {
namespace dal {

/* Get Next Record */
// If has_two_handles is true, do SQLFetch from both the handles
DalResultCode
DalCursor::GetNextRecord() const {
  DalResultCode dal_rc;

  if (stmt_handle_1_ == SQL_NULL_HANDLE) {
    UPLL_LOG_DEBUG("NULL Statement Handle 1");
    return kDalRcGeneralError;
  }

  if (bind_info_1_ == NULL) {
    UPLL_LOG_DEBUG("NULL Bind Info 1");
    return kDalRcGeneralError;
  }

  dal_rc = GetNextRecordFromStmt(stmt_handle_1_, bind_info_1_);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Error Fetching record from Stmt Handle 1(%p)",
                   dal_rc, stmt_handle_1_);
    return dal_rc;
  }
  if (has_two_handles_ == true) {
    if (stmt_handle_2_ == SQL_NULL_HANDLE) {
      UPLL_LOG_DEBUG("NULL Statement Handle 2");
      return kDalRcGeneralError;
    }

    if (bind_info_2_ == NULL) {
      UPLL_LOG_DEBUG("NULL Bind Info 2");
      return kDalRcGeneralError;
    }

    dal_rc = GetNextRecordFromStmt(stmt_handle_2_, bind_info_2_);
    if (dal_rc != kDalRcSuccess) {
      UPLL_LOG_DEBUG("Err - %d. Error Fetching record from Stmt Handle 2(%p)",
                     dal_rc, stmt_handle_2_);
      return dal_rc;
    }
  }
  UPLL_LOG_TRACE("Success Fetching record from Cursor");
  return kDalRcSuccess;
}  // DalCursor::GetNextRecord

// Close Cursor
DalResultCode
DalCursor::CloseCursor(bool delete_bind) {
  DalResultCode dal_rc_1 = kDalRcSuccess;
  DalResultCode dal_rc_2 = kDalRcSuccess;

  if (has_two_handles_ == true) {
    if (delete_bind == true)
      delete bind_info_2_;
    if (stmt_handle_2_ != SQL_NULL_HANDLE) {
      dal_rc_2 = CloseStmtHandle(const_cast<SQLHANDLE>(stmt_handle_2_));
      stmt_handle_2_ = SQL_NULL_HANDLE;
      if (dal_rc_2 != kDalRcSuccess) {
        UPLL_LOG_DEBUG("Stmt Handle 2 not released");
        if (dal_rc_2 == kDalRcConnNotAvailable) {
          return dal_rc_2;
        }
      }
    } else {
      UPLL_LOG_DEBUG("NULL Statement Handle 2");
      dal_rc_2 = kDalRcGeneralError;
    }
  }
  if (delete_bind == true)
    delete bind_info_1_;
  if (stmt_handle_1_ == SQL_NULL_HANDLE) {
    UPLL_LOG_DEBUG("NULL Statement Handle 1");
    return kDalRcGeneralError;
  }

  dal_rc_1 = CloseStmtHandle(const_cast<SQLHANDLE>(stmt_handle_1_));
  stmt_handle_1_ = SQL_NULL_HANDLE;
  if (dal_rc_1 != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Stmt Handle 1 not released");
    return dal_rc_1;
  }

  if (dal_rc_2 != kDalRcSuccess) {
    return dal_rc_2;
  }

  UPLL_LOG_TRACE("Success closing Cursor");
  return kDalRcSuccess;
}  // DalCursor::CloseCursor

// Private Methods of DalCursor
// GetNextRecord from the specific Stmt Handle
DalResultCode
DalCursor::GetNextRecordFromStmt(const SQLHANDLE stmt_handle,
                                 const DalBindInfo *bind_info) {
  SQLRETURN     sql_rc;
  DalResultCode dal_rc;

  if (stmt_handle == SQL_NULL_HANDLE) {
    UPLL_LOG_DEBUG("NULL Statement Handle");
    return kDalRcGeneralError;
  }

  if (bind_info == NULL) {
    UPLL_LOG_DEBUG("NULL Bind Info");
    return kDalRcGeneralError;
  }

  // Resetting Dal Out Buffer Space to avoid overwirting of data
  if (const_cast<DalBindInfo*>(bind_info)->ResetDalOutBuffer() != true) {
    UPLL_LOG_DEBUG("Error resetting Dal Out Buffer");
    return kDalRcGeneralError;
  }

  // Fetching results from the resultset
  sql_rc = SQLFetch(stmt_handle);
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_STMT,
                                     stmt_handle,
                                     sql_rc, &dal_rc);
  if (dal_rc != kDalRcSuccess) {
    if (dal_rc == kDalRcRecordNotFound) {
      dal_rc = kDalRcRecordNoMore;
      UPLL_LOG_TRACE("Err - %d. No more result", dal_rc);
    } else {
      UPLL_LOG_INFO("Err - %d. Failed to Fetch result", dal_rc);
    }
    return dal_rc;
  }

  if (const_cast<DalBindInfo*>(bind_info)->CopyResultToApp() != true) {
    UPLL_LOG_DEBUG("Failed to copy result");
    return kDalRcGeneralError;
  }
  UPLL_LOG_TRACE("%s",
      ((const_cast<DalBindInfo *>(bind_info))->BindListResultToStr()).c_str());
  UPLL_LOG_TRACE("Success Fetching record from Stmt Handle");
  return kDalRcSuccess;
}  // DalCursor::GetNextRecordFromStmt

// Close specific Statement Handle
DalResultCode
DalCursor::CloseStmtHandle(SQLHANDLE stmt_handle) {
  SQLRETURN     sql_rc;
  DalResultCode dal_rc;

  if (stmt_handle == NULL) {
    UPLL_LOG_DEBUG("NULL Statement Handle");
    return kDalRcGeneralError;
  }

  sql_rc = SQLCloseCursor(stmt_handle);
  DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_STMT,
                                     stmt_handle,
                                     sql_rc, &dal_rc);
  if (dal_rc != kDalRcSuccess) {
    UPLL_LOG_DEBUG("Err - %d. Failed to close statement handle", dal_rc);
  }

  sql_rc = SQLFreeHandle(SQL_HANDLE_STMT, stmt_handle);
  if (sql_rc != SQL_SUCCESS && sql_rc != SQL_SUCCESS_WITH_INFO) {
    DalResultCode free_dal_rc;
    DalErrorHandler::ProcessOdbcErrors(SQL_HANDLE_STMT, stmt_handle,
                                       sql_rc, &free_dal_rc);
    if (free_dal_rc != kDalRcSuccess) {
      UPLL_LOG_TRACE("Err - %d. Failed to Free Handle %p",
                     free_dal_rc, stmt_handle);
      return ((dal_rc == kDalRcSuccess) ? free_dal_rc : dal_rc);
    }
  }
  stmt_handle = NULL;
  UPLL_LOG_TRACE("Success closing Stmt Handle");
  return kDalRcSuccess;
}  // DalCursor::CloseStmtHandle

}  // namespace dal
}  // namespace upll
}  // namespace unc
