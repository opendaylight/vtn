/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


/*
 *@brief   Physical ITC Configuration header
 *@file    itc_configuration_request.hh
 *Desc:This header file contains the declaration of ConfigurationRequest class
*/


#ifndef _ITC_CONFIGURATION_REQUEST_HH_
#define _ITC_CONFIGURATION_REQUEST_HH_

#include <string>
#include "physical_common_def.hh"
#include "unc/uppl_common.h"
#include "physical_itc_req.hh"
#include "itc_transaction_request.hh"
#include "itc_audit_request.hh"
#include "unc/keytype.h"
#include "itc_state_change.hh"
#include "pfc/log.h"
#include "pfc/debug.h"

using unc::uppl::ITCReq;
using unc::uppl::OdbcmConnectionHandler;

/*
 * This Class which will be inherited from ITCReq
 * class to process config req.
 * For further info,see the comments in .cc file
 */

class ConfigurationRequest: public ITCReq {
 public:
  ConfigurationRequest();
  ~ConfigurationRequest();
  UncRespCode ProcessReq(ServerSession &session,
                            physical_request_header &obj_req_hdr);
  UncRespCode ValidateReq(OdbcmConnectionHandler *db_conn,
                             ServerSession &session,
      physical_request_header obj_req_hdr,
      void* &key_struct,
      void* &val_struct,
      Kt_Base* &KtObj);

 private:
  key_root_t key_root_obj;
  key_ctr_t key_ctr_obj;
  val_ctr_t val_ctr_obj;
  key_ctr_domain_t key_domain_obj;
  val_ctr_domain_t val_domain_obj;
  key_boundary_t key_boundary_obj;
  val_boundary_t val_boundary_obj;
  UncRespCode ValidateController(OdbcmConnectionHandler *db_conn,
                                    ServerSession &session,
                                    uint32_t data_type,
                                    uint32_t operation,
                                    void* &key_struct,
                                    void* &val_struct);
  UncRespCode ValidateDomain(OdbcmConnectionHandler *db_conn,
                                ServerSession &session,
                                uint32_t data_type,
                                uint32_t operation,
                                void* &key_struct,
                                void* &val_struct);
  UncRespCode ValidateBoundary(OdbcmConnectionHandler *db_conn,
                                  ServerSession &session,
                                  uint32_t data_type,
                                  uint32_t operation,
                                  void* &key_struct,
                                  void* &val_struct);
};

#endif
