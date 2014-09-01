/*      Copyright (c) 2013 NEC Corporation                        */
/*      NEC CONFIDENTIAL AND PROPRIETARY                          */
/*      All rights reserved by NEC Corporation.                   */
/*      This program must be used solely for the purpose for      */
/*      which it was furnished by NEC Corporation.   No part      */
/*      of this program may be reproduced  or  disclosed  to      */
/*      others,  in  any form,  without  the  prior  written      */
/*      permission of NEC Corporation.    Use  of  copyright      */
/*      notice does not evidence publication of the program.      */

/*
 * dal_conn_intf.h 
 *   Contians class definition for Database connection and transaction
 *   interfaces
 *
 *   Implemented by DalOdbcMgr
 */ 
#ifndef __DAL_CONN_INTF_HH__
#define __DAL_CONN_INTF_HH__

#include "dal_defines.hh"

namespace unc {
namespace upll {
namespace dal {

enum DalConnType {
  kDalConnReadOnly = 0,  // Read Only Connection
  kDalConnReadWrite      // Read Write Connection
};
enum DalConnState {
        kDalDbDisconnected = 0,  // DB disconnected
              kDalDbConnected          // DB connected
                      };


/**
 *  DalConnIntf 
 *    Connection and Transaction APIs for database
 *
 *  Inherited by DalOdbcMgr
 */
class DalConnIntf {
  public:
    /**
     * DalConnIntf - Constructor
     *
     * @return void             - None
     */
    DalConnIntf() {
    }

    /**
     * ~DalConnIntf - Destructor
     *
     * @return void             - None
     */
    virtual ~DalConnIntf() {
    }

    /**
     * ConnectToDb
     *   Connects to the DB with the connection parameters from conf file.
     *
     * @param[in] conn_type       - Type of the connection (Read-Only or Read/Write)
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     */
    virtual DalResultCode ConnectToDb(const DalConnType conn_type)= 0;

    /**
     * DisconnectFromDb
     * Disconnects from the database for the corresponding connection
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     */
    virtual DalResultCode DisconnectFromDb() = 0;

    /**
     * CommitTransaction
     * Commits all the pending changes in the database for the correpsonding
     * connection.
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     */
    virtual DalResultCode CommitTransaction() = 0;

    /**
     * RollbackTransaction
     * Discards all the pending changes in the database for the correpsonding
     * connection.
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     */
    virtual DalResultCode RollbackTransaction() = 0;
    
    virtual DalConnType get_conn_type() = 0;


};

}  // namespace dal
}  // namespace upll
}  // namespace unc
#endif  // __DAL_CONN_INTF_HH__
