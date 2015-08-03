/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


/*
 *@brief   Physical ITC ReadRequest header
 *@file    itc_read_request.hh
 * Desc:This header file contains the declaration of ReadRequest class
 *
 */

#ifndef _ITC_READ_REQUEST_HH_
#define _ITC_READ_REQUEST_HH_

#include <string>
#include <vector>
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

/*
 * This class which will be inherited from ITCReq class 
 * to process read requests
 * For further info,see the comments in .cc file
 **/

class ReadRequest: public ITCReq {
  public:
    ReadRequest();
    ~ReadRequest();
    UncRespCode ProcessReq(ServerSession &session,
                              physical_request_header &obj_req_hdr);
    void AddToBuffer(BulkReadBuffer objBuffer) {
      vect_bulk_read_buffer.push_back(objBuffer);
    }

    vector<BulkReadBuffer>& get_readbulk_buffer() {
      return vect_bulk_read_buffer;
    }
    void FlushBulkReadBuffer();

  private:
    key_root_t key_root_obj;
    key_ctr_t key_ctr_obj;
    val_ctr_t val_ctr_obj;
    key_dataflow_t key_dataflow_obj;
    key_dataflow_v2_t key_dataflow_v2_obj;
    val_dataflow_v2_t val_dataflow_v2_obj;
    key_ctr_dataflow_t key_ctr_dataflow_obj;
    key_ctr_domain_t key_domain_obj;
    val_ctr_domain_t val_domain_obj;
    key_logical_port_t key_logical_port_obj;
    val_logical_port_st_t val_logical_port_obj;
    key_logical_member_port_t key_logical_member_port_obj;
    key_switch_t key_switch_obj;
    val_switch_st_t val_switch_obj;
    key_port_t key_port_obj;
    val_port_st_t val_port_obj;
    key_link_t key_link_obj;
    val_link_st_t val_link_obj;
    key_boundary_t key_boundary_obj;
    val_boundary_t val_boundary_obj;
    vector<BulkReadBuffer> vect_bulk_read_buffer;
    UncRespCode ProcessReadOperation(OdbcmConnectionHandler *db_conn,
                                        ServerSession &session,
                                        Kt_Base *KtObj,
                                        physical_request_header &obj_req_hdr,
                                        void* key_struct,
                                        void* val_struct,
                                        uint32_t operation_type);
    UncRespCode FrameReadBulkResponse(ServerSession &session,
                                         uint32_t session_id,
                                         uint32_t config_id,
                                         uint32_t operation,
                                         uint32_t data_type,
                                         uint32_t option1,
                                         uint32_t option2);
    void GetControllerStructure(ServerSession &session,
                                void * &key_struct,
                                void * &val_struct,
                                physical_response_header &rsh);
    void GetDataflowStructure(ServerSession &session,
                                void * &key_struct,
                                physical_response_header &rsh);
    void GetDataflowV2Structure(ServerSession &session,
                                void * &key_struct,
                                void * &val_struct,
                                physical_response_header &rsh);
    void GetCtrDataflowStructure(ServerSession &session,
                                void * &key_struct,
                                physical_response_header &rsh);
    void GetDomainStructure(ServerSession &session,
                            void * &key_struct,
                            void * &val_struct,
                            physical_response_header &rsh);
    void GetLogicalPortStructure(ServerSession &session,
                                 void * &key_struct,
                                 void * &val_struct,
                                 physical_response_header &rsh);
    void GetLogicalMemberPortStructure(ServerSession &session,
                                       void * &key_struct,
                                       void * &val_struct,
                                       physical_response_header &rsh);
    void GetSwitchStructure(ServerSession &session,
                            void * &key_struct,
                            void * &val_struct,
                            physical_response_header &rsh);
    void GetPortStructure(ServerSession &session,
                          void * &key_struct,
                          void * &val_struct,
                          physical_response_header &rsh);
    void GetLinkStructure(ServerSession &session,
                          void * &key_struct,
                          void * &val_struct,
                          physical_response_header &rsh);
    void GetBoundaryStructure(ServerSession &session,
                              void * &key_struct,
                              void * &val_struct,
                              physical_response_header &rsh);
    void AddControllerStructure(ServerSession &session,
                                BulkReadBuffer obj_buffer,
                                int &err);
    void AddDomainStructure(ServerSession &session,
                            BulkReadBuffer obj_buffer,
                            int &err);
    void AddLogicalPortStructure(ServerSession &session,
                                 BulkReadBuffer obj_buffer,
                                 int &err);
    void AddLogicalMemberPortStructure(ServerSession &session,
                                       BulkReadBuffer obj_buffer,
                                       int &err);
    void AddSwitchStructure(ServerSession &session,
                            BulkReadBuffer obj_buffer,
                            int &err);
    void AddPortStructure(ServerSession &session,
                          BulkReadBuffer obj_buffer,
                          int &err);
    void AddLinkStructure(ServerSession &session,
                          BulkReadBuffer obj_buffer,
                          int &err);
    void AddBoundaryStructure(ServerSession &session,
                              BulkReadBuffer obj_buffer,
                              int &err);
    void ClearControllerStructure(BulkReadBuffer obj_buffer);
    void ClearDomainStructure(BulkReadBuffer obj_buffer);
    void ClearLogicalPortStructure(BulkReadBuffer obj_buffer);
    void ClearLogicalMemberPortStructure(BulkReadBuffer obj_buffer);
    void ClearSwitchStructure(BulkReadBuffer obj_buffer);
    void ClearPortStructure(BulkReadBuffer obj_buffer);
    void ClearLinkStructure(BulkReadBuffer obj_buffer);
    void ClearBoundaryStructure(BulkReadBuffer obj_buffer);
};

#define ADD_KEY_TO_SESSION(err, session, key_type, key_value, ipc_keytype_t) { \
    err |= session.addOutput((uint32_t)key_type); \
    ipc_keytype_t *key = \
    reinterpret_cast<ipc_keytype_t*>(key_value); \
    if (key != NULL) { \
      pfc_log_debug("Key added: %s", IpctUtil::get_string(*key).c_str()); \
      err |= session.addOutput(*key); \
    } \
}

#define ADD_VALUE_TO_SESSION(err, session, value, ipc_keytype_t) { \
    ipc_keytype_t *val = \
    reinterpret_cast<ipc_keytype_t*>(value); \
    if (val != NULL) { \
      pfc_log_debug("Value added: %s", IpctUtil::get_string(*val).c_str()); \
      err |= session.addOutput(*val); \
    } \
}

#define CLEAR_KEY(key_value, ipc_keytype_t) { \
    ipc_keytype_t *key = \
    reinterpret_cast<ipc_keytype_t*>(key_value); \
    if (key != NULL) { \
      delete key; \
      key = NULL; \
    } \
}

#define CLEAR_VALUE(value, ipc_keytype_t) { \
    ipc_keytype_t *val = \
    reinterpret_cast<ipc_keytype_t*>(value); \
    if (val != NULL) { \
      delete val; \
      val = NULL; \
    } \
}
#endif
