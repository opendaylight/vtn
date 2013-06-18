/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * dal_error_handler.hh
 *   Contains Error codes and related methods
 *
 * Format - singleton
 */

#ifndef __DAL_ERROR_HANDLER_HH__
#define __DAL_ERROR_HANDLER_HH__

#include <sql.h>
#include <string>
#include <map>
#include "dal_defines.hh"

namespace unc {
namespace upll {
namespace dal {

struct DalErrMap {
  const std::string sql_state;
  const DalResultCode dal_rc;
};

/**
 * DalErrorHandler
 *   contains static methods for error handling
 *
 * Static Class. No need of object instantiation
 */
// As per this design, DalErrorHandler::FillErrorMap should be
// called by upll/dal module init for one time loading of error values.
// And DalErrorHandler::ClearErrorMap during upll/dal module fini.
class DalErrorHandler {
  public:
    /**
     * DalErrorHandler - Constructor
     *
     * @return void             - None
     */
    DalErrorHandler() {}

    /**
     * ~DalErrorHandler - Destructor
     *
     * @return void             - None
     */
    ~DalErrorHandler() {}

    /**
     * ProcessOdbcErrors
     *   Processes ODBC error codes to find current database state
     *   Finds the relevant DalResultCode from the database state value
     *
     * @param[in] handle_type     - Type of the connection handle
     * @param[in] handle          - Valid handle to get the error detail
     * @param[in] sql_rc          - Return code from ODBC
     * @param[in/out] dal_rc_code - Reference to the DalResultCode
     *                              On successful execution, this contains the
     *                              relevant DalResultCode
     *                              On Failure, stores kDalRcGeneralError
     *
     * @return                    - None.
     */
    static void ProcessOdbcErrors(const SQLSMALLINT handle_type,
                                  const SQLHANDLE handle,
                                  const SQLRETURN sql_rc,
                                  DalResultCode *dal_rc_code);

    /**
     * FillErrorMap
     *   Store all the possible error code mappings in a map single time.
     *
     * @return                    - true, if map filled
     *                              false, otherwise
     */
    // One time filling of map with SQLState and DalResultCode
    static bool FillErrorMap();

    /**
     * ClearErrorMap
     *   Clear the stored error code mappings
     *
     * @return                    - none.
     */
    static void ClearErrorMap();

  private:
    /**
     * FindDalResultCode
     *   Finds the relevant DalResultCode from the filled map data
     *
     * @return DalResultCode      - relevant result code
     */
    static inline DalResultCode FindDalResultCode(const SQLCHAR *sql_state);

    static std::map<std::string, DalResultCode> err_map_;
                                    // Map of sqlstate from ODBC to
                                    // the relevant DalResultCode
    static bool err_map_filled_;     // Status of err_map (true, if filled)
                                    // (false, if not filled)
};  // class DalOdbcMgr

}  // namespace dal
}  // namespace upll
}  // namespace unc
#endif  // __DAL_ERROR_HANDLER_HH__
