/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   KT Dataflow implementation
 * @file    itc_kt_dataflow.hh
 *
 */

#ifndef KT_DATAFLOW_V2_HH
#define KT_DATAFLOW_V2_HH

#include <uncxx/dataflow.hh>
#include <vector>
#include <map>
#include <list>
#include <string>
#include "physicallayer.hh"
#include "itc_kt_base.hh"
#include "itc_kt_dataflow.hh"
#include "odbcm_utils.hh"

#define KT_DATAFLOW_V2_OPER_STATUS_REF 1

using std::vector;
using std::map;
using std::multimap;
using unc::uppl::ODBCMOperator;
using unc::dataflow::DataflowUtil;
using unc::dataflow::DataflowCmn;
using unc::dataflow::DataflowDetail;
using unc::dataflow::kidx_val_df_data_flow_cmn;

#define PFC_DEF_TIMEOUT 3600


/* @ Controller Class definition */
class Kt_DataflowV2: public Kt_Dataflow {
  public:
    Kt_DataflowV2();

    ~Kt_DataflowV2();


    UncRespCode ReadBulk(OdbcmConnectionHandler *db_conn,
                            void* key_struct,
                            uint32_t data_type,
                            uint32_t &max_rep_ct,
                            int child_index,
                            pfc_bool_t parent_call,
                            pfc_bool_t is_read_next,
                            ReadRequest *read_req) {
      pfc_log_debug("ReadBulk operation is not allowed in KT_DATAFLOWV2");
      return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    }


    UncRespCode PerformSyntaxValidation(OdbcmConnectionHandler *db_conn,
                                           void* key_struct,
                                           void* val_struct,
                                           uint32_t operation,
                                           uint32_t data_type);

    UncRespCode PerformSemanticValidation(OdbcmConnectionHandler *db_conn,
                                             void* key_struct,
                                             void* val_struct,
                                             uint32_t operation,
                                             uint32_t data_type);



    void Fill_Attr_Syntax_Map();


    UncRespCode RequestSingleFlow(OdbcmConnectionHandler *db_conn,
                                          uint32_t session_id,
                                          uint32_t configuration_id,
                                          void* key_struct,
                                          void* val_struct,
                                          ServerSession &sess,
                                          bool is_head_node,
                                          string &ingress_bdry_id,
                                          DataflowDetail*& df_segm);


  private:
    void PopulateDBSchemaForKtTable(OdbcmConnectionHandler *db_conn,
        DBTableSchema &kt_dbtableschema,
        void* key_struct,
        void* val_struct,
        uint8_t operation_type,
        uint32_t data_type,
        uint32_t option1,
        uint32_t option2,
        vector<ODBCMOperator> &vect_key_operations,
        void* &old_value_struct,
        CsRowStatus row_status,
        pfc_bool_t is_filtering,
        pfc_bool_t is_state) {}


    UncRespCode PerformRead(OdbcmConnectionHandler *db_conn,
                               uint32_t session_id,
                               uint32_t configuration_id,
                               void* key_struct,
                               void* val_struct,
                               uint32_t data_type,
                               uint32_t operation_type,
                               ServerSession &sess,
                               uint32_t option1,
                               uint32_t option2,
                               uint32_t max_rep_ct);
};
#endif
