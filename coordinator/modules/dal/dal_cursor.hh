/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * dal_cusor.hh
 *   Contains definitions of DalCursor
 */ 

#ifndef __DAL_CURSOR_HH__
#define __DAL_CURSOR_HH__

#include <sql.h>
#include "dal_defines.hh"
#include "dal_bind_info.hh"

namespace unc {
namespace upll {
namespace dal {

/**
 * DalCursor
 *   Contains cursor information and related methods
 */
class DalCursor {
  public :

    /* Constructor */
    /**
     * DalCursor - Constructor
     *   Initilizes the DalCursor properties for single statement handle
     *
     * @param[in] stmt_handle   - Valid statement handle contains multiple
     *                            records in the resultset of Database
     * @param[in] bind_addr     - Valid bind information bound to the
     *                            corresponding handle
     * @return void             - None
     */
    explicit DalCursor(const SQLHANDLE stmt_handle,
                       const DalBindInfo *bind_info) {
      stmt_handle_1_ = const_cast<SQLHANDLE>(stmt_handle);
      bind_info_1_ = const_cast<DalBindInfo *>(bind_info);
      stmt_handle_2_ = SQL_NULL_HANDLE;
      bind_info_2_ = NULL;
      has_two_handles_ = false;
    }

    /**
     * DalCursor - Constructor
     *   Initilizes the DalCursor properties for two statement handles
     *
     * @param[in] handle_1      - Valid statement handle contains multiple
     *                            records in the resultset of Database
     * @param[in] bind_ptr_1    - Valid bind information bound to the
     *                            corresponding handle
     * @param[in] handle_2      - Valid statement handle contains multiple
     *                            records in the resultset of Database
     * @param[in] bind_ptr_2    - Valid bind information bound to the
     *                            corresponding handle
     * @return void             - None
     */
    explicit DalCursor(const SQLHANDLE handle_1,
                       const DalBindInfo *bind_ptr_1,
                       const SQLHANDLE handle_2,
                       const DalBindInfo *bind_ptr_2) {
      stmt_handle_1_ = const_cast<SQLHANDLE>(handle_1);
      bind_info_1_ = const_cast<DalBindInfo *>(bind_ptr_1);
      stmt_handle_2_ = const_cast<SQLHANDLE>(handle_2);
      bind_info_2_ = const_cast<DalBindInfo *>(bind_ptr_2);
      has_two_handles_ = true;
    }

    /**
     * ~DalCursor - Constructor
     *   Destroys the DalCursor instance
     *
     * @return void             - None
     */
    ~DalCursor() {
      if (stmt_handle_1_ != NULL) {
        SQLFreeHandle(SQL_HANDLE_STMT, stmt_handle_1_);
      }
      if (stmt_handle_2_ != NULL) {
        SQLFreeHandle(SQL_HANDLE_STMT, stmt_handle_2_);
      }
    }

    /**
     * GetNextRecord
     *   Fetches the current record pointed by the statement handle in
     *   the result set
     *   If two statement handles are there, fetches records from both the
     *   handles and store the results in the corresponding bind_info buffers
     *
     * @return DalResultCode    - kDalRcSuccess in case of success
     *                          - Valid errorcode otherwise
     *                            On successful execution, the output buffers
     *                            of dal user in the bind_info are 
     *                            populated with the result from the 
     *                            corresponding statement handle
     * @Note:
     * If there are 1 handle, fetch record from 1st handle
     *   If failed for 1st handle, return error from 1st handle
     *   If success for 1st handle, return kDalRcSuccess, store results for
     *   1st bind_info
     * If there are 2 handles, fetch record from 1st handle
     *   If failed for 1st handle, return error from 1st handle
     *   If success for 1st handle, perform fetch record from 2nd handle
     *   If failed for 2nd handle, return error from 2nd handle
     *   If success for 1st handle, store the results in the corresponding 
     *   bind info.
     */
    DalResultCode GetNextRecord() const;

    /**
     * CloseCursor
     *   Destroys the statement handles
     *
     * @param[in] delete_bind   - If true, deletes the associated bind_info
     *                          - If false, does not delete bind_info
     * @return DalResultCode    - kDalRcSuccess in case of success
     *                          - Valid errorcode otherwise
     *                            On successful execution, the handles
     *                            are destroyed and not available for
     *                            further processing
     * @Note:
     * If there are 1 handle, Close 1st handle
     *   If Close failed for 1st handle, return error from 1st handle
     *   If Close success for 1st handle, return kDalRcSuccess
     * If there are 2 handles, Close 2nd handle
     *   If Close failed for 2nd handle, Close 1st handle
     *      If Close failed for 1st handle, return error from 1st handle
     *      If Close success for 1st handle, return error from 2nd handle
     *   If Close Success for 2nd handle, Close 1st handle
     *      If Close failed for 1st handle, return error from 1st handle
     *      If Close success for 1st handle, return kDalRcSuccess
     */
    DalResultCode CloseCursor(bool delete_bind);

  protected :
  private :
    /**
     * GetNextRecordFromStmt
     *   Wrapper for GetNextRecord for the specific statement handle and the
     *   correpsonding bind_info
     *
     * @param[in] stmt_handle   - Valid statement handle contains multiple
     *                            records in the resultset of Database
     * @param[in] bind_addr     - Valid bind information bound to the
     *                            corresponding handle
     *
     * @return DalResultCode    - kDalRcSuccess in case of success
     *                          - Valid errorcode otherwise
     */
    static DalResultCode GetNextRecordFromStmt(const SQLHANDLE stmt_handle,
                                               const DalBindInfo *bind_info);
    /**
     * CloseStmtHandle
     *   Wrapper for CloseCursor for the specific statement handle
     *
     * @return DalResultCode    - kDalRcSuccess in case of success
     *                          - Valid errorcode otherwise
     */
    static DalResultCode CloseStmtHandle(SQLHANDLE stmt_handle);

    SQLHANDLE stmt_handle_1_;
    SQLHANDLE stmt_handle_2_;
    DalBindInfo *bind_info_1_;
    DalBindInfo *bind_info_2_;

    bool has_two_handles_;
};  // class DalCursor

}  // namespace dal
}  // namespace upll
}  // namespace unc
#endif  // __DAL_CURSOR_HH__
