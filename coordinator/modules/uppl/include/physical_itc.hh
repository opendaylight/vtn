/*
 * Copyright (c) 2012-2014 NEC Corporation
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
#include "physical_common_def.hh"
#include "unc/uppl_common.h"
#include "itc_transaction_request.hh"
#include "itc_db_config.hh"
#include "itc_audit_request.hh"
#include "unc/keytype.h"
#include "unc/usess_ipc.h"
#include "itc_state_change.hh"
#include "pfc/log.h"

using unc::uppl::TransactionRequest;
using unc::uppl::AuditRequest;
using unc::uppl::DBConfigurationRequest;
using unc::uppl::SystemStateChangeRequest;

class ConfigurationRequest;
class ReadRequest;

/**************************************************************************
 * It is a singleton class which will invoke respective configuration classes.
 * For further info,see the comments in .cc file
 * ***************************************************************************/

class InternalTransactionCoordinator {
  public:
    ~InternalTransactionCoordinator();
    static InternalTransactionCoordinator* get_internaltransactioncoordinator();
    TransactionRequest *transaction_req();
    AuditRequest *audit_req();
    DBConfigurationRequest *db_config_req();
    SystemStateChangeRequest *system_state_change_req();

    UncRespCode PerformConfigIdValidation(ServerSession &session,
                                             uint32_t sessionId,
                                             uint32_t configId);

    UncRespCode ProcessReq(ServerSession &session, pfc_ipcid_t);
    UncRespCode ProcessReq(ServerSession &session);
    UncRespCode ProcessEvent(const IpcEvent &event);

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
    pfc_bool_t IsControllerInImport(string controller_name) {
      if (controller_name == controller_in_import_) {
        return PFC_TRUE;
      }
      return PFC_FALSE;
    }

  private:
    uint16_t config_request_status_;
    uint16_t trans_state_;
    string controller_in_import_;
    InternalTransactionCoordinator();
    static InternalTransactionCoordinator* internal_transaction_coordinator_;
    TransactionRequest *transaction_req_;
    DBConfigurationRequest *db_config_req_;
    SystemStateChangeRequest *system_state_change_req_;
    AuditRequest *audit_req_;

    UncRespCode ProcessConfigRequest(ServerSession &session,
                                        physical_request_header &obj_req_hdr,
                                        physical_response_header &rsh);
    UncRespCode ProcessReadRequest(ServerSession &session,
                                      physical_request_header &obj_req_hdr,
                                      physical_response_header &rsh);
    UncRespCode ProcessImportRequest(ServerSession &session,
                                        uint32_t operation);
    UncRespCode ProcessIsCandidateDirty(ServerSession &session,
                                           uint32_t operation);
};
#endif
