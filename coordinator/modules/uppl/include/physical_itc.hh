/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


/*
 *@brief   Physical Internal Transaction Coordinator header
 *@file    physical_itc.hh
 *
 *Desc:This header file contains the declaration of
 *InternalTransactionCoordinator class
 */

#ifndef _PFC_PHYSICALINTERNALTRANSACTIONCOORDINATOR_H_
#define _PFC_PHYSICALINTERNALTRANSACTIONCOORDINATOR_H_

#include <string>
#include <vector>
#include "physical_common_def.hh"
#include "unc/uppl_common.h"
#include "itc_transaction_request.hh"
#include "itc_db_config.hh"
#include "itc_audit_request.hh"
#include "unc/keytype.h"
#include "itc_state_change.hh"
#include "pfc/log.h"

using unc::uppl::TransactionRequest;
using unc::uppl::AuditRequest;
using unc::uppl::DBConfigurationRequest;
using unc::uppl::SystemStateChangeRequest;

class ConfigurationRequest;
class ReadRequest;

typedef enum {
  IS_KEY = 0,
      IS_VALUE,
      IS_STATE_VALUE,
      IS_SEPARATOR
}ValueType;

struct BulkReadBuffer {
  unc_key_type_t key_type;
  ValueType value_type;
  void* value;
};

/**************************************************************************
 * It is a singleton class which will invoke respective configuration classes.
 * For further info,see the comments in .cc file
 * ***************************************************************************/

class InternalTransactionCoordinator {
  public:
    InternalTransactionCoordinator();
    ~InternalTransactionCoordinator();

    TransactionRequest *transaction_req();
    AuditRequest *audit_req();
    DBConfigurationRequest *db_config_req();
    SystemStateChangeRequest *system_state_change_req();

    UpplReturnCode PerformConfigIdValidation(ServerSession &session,
                                             uint32_t sessionId,
                                             uint32_t configId);

    UpplReturnCode ProcessReq(ServerSession &session, pfc_ipcid_t);
    UpplReturnCode ProcessReq(ServerSession &session);
    UpplReturnCode ProcessEvent(const IpcEvent &event);

    /*  set_config_request_status
     *
     *  @Description    : This function sets the config request state
     *
     *  @param[in]: status enum value
     *
     *  @return   : void
     *
     * */

    inline void set_config_request_status(uint16_t status) {
      config_request_status_= status;
    }

    /*  config_request_status
     *
     *  @Description    : This function gets the config request state
     *
     *  @param[in]: NA
     *
     *  @return   : config status
     *
     *  */

    inline uint16_t config_request_status() {
      return config_request_status_;
    }

    /*  set_audit_state
     *
     *  @Description    : This function sets the audit state
     *
     *  @param[in]: audit status enum value
     *
     *  @return   : void
     *
     *  */
    inline void set_audit_state(uint16_t audit_state) {
      audit_state_= audit_state;
    }

    /*  audit_state
     *
     *  @Description    : This function gets the audit state
     *
     *  @param[in]: none
     *
     *  @return   : audit status value
     *
     *  */
    inline uint16_t audit_state() {
      return audit_state_;
    }

    /* set_import_state
     *
     *  @Description    : This function sets the import status
     *
     *  @param[in]: import status value
     *
     *  @return   : void
     *
     *  */
    inline void set_import_state(uint16_t import_state) {
      import_state_ = import_state;
    }

    /* import_state
     *
     *  @Description    : This function gets the import state
     *
     *  @param[in]: none
     *
     *  @return   : import status value
     *
     *  * */
    inline uint16_t import_state() {
      return import_state_;
    }

    /* set_trans_state
     *
     *  @Description    : This function sets the transaction status
     *
     *  @param[in]: transaction status value
     *
     *  @return   : void
     *
     *  */
    inline void set_trans_state(uint16_t trans_state) {
      trans_state_= trans_state;
    }

    /* import_state
     *
     *  @Description    : This function gets the import state
     *
     *  @param[in]: none
     *
     *  @return   : import status value
     *
     *  */
    inline uint16_t trans_state() {
      return trans_state_;
    }

    void AddToBuffer(BulkReadBuffer objBuffer) {
      vect_bulk_read_buffer.push_back(objBuffer);
    }

    void FlushBulkReadBuffer() {
      vect_bulk_read_buffer.clear();
    }

    vector<BulkReadBuffer> get_readbulk_buffer() {
      return vect_bulk_read_buffer;
    }

  private:
    uint16_t config_request_status_;
    uint16_t trans_state_;
    uint16_t import_state_;
    uint16_t audit_state_;

    TransactionRequest *transaction_req_;
    DBConfigurationRequest *db_config_req_;
    SystemStateChangeRequest *system_state_change_req_;
    AuditRequest *audit_req_;

    vector<BulkReadBuffer> vect_bulk_read_buffer;

    UpplReturnCode ProcessConfigRequest(ServerSession &session,
                                        physical_request_header obj_req_hdr,
                                        physical_response_header &rsh);
    UpplReturnCode ProcessReadRequest(ServerSession &session,
                                      physical_request_header obj_req_hdr,
                                      physical_response_header &rsh);
    UpplReturnCode ProcessImportRequest(ServerSession &session,
                                        uint32_t operation);
};
#endif
