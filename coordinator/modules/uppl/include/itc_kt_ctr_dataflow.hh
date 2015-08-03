/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   KT Ctr_Dataflow implementation
 * @file    itc_kt_ctr_dataflow.hh
 *
 */

#ifndef KT_CTR_DATAFLOW_H
#define KT_CTR_DATAFLOW_H

#include <uncxx/dataflow.hh>
#include <vector>
#include <string>
#include "itc_kt_state_base.hh"
#include "physicallayer.hh"

using unc::uppl::ODBCMOperator;
using unc::dataflow::DataflowUtil;
using unc::dataflow::DataflowCmn;
using unc::dataflow::DataflowDetail;
using unc::dataflow::kidx_val_df_data_flow_cmn;


/* @  Ctr_Dataflow Class definition */
class Kt_Ctr_Dataflow : public Kt_State_Base {
 public:
  Kt_Ctr_Dataflow();

  ~Kt_Ctr_Dataflow();

  UncRespCode ReadInternal(OdbcmConnectionHandler *db_conn,
                              vector<void *> &key_struct,
                              vector<void *> &val_struct,
                              uint32_t data_type,
                              uint32_t operation_type);

  UncRespCode ReadBulk(OdbcmConnectionHandler *db_conn,
                          void* key_struct,
                          uint32_t data_type,
                          uint32_t &max_rep_ct,
                          int child_index,
                          pfc_bool_t parent_call,
                          pfc_bool_t is_read_next,
                          ReadRequest *read_req);

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
      pfc_bool_t is_state) {
  }

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

