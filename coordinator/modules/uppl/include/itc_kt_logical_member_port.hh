/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   KT Logical Member Port implementation
 * @file    itc_kt_logical_member_port.hh
 *
 */

#ifndef KT_LOGICAL_MEMBER_PORT_HH
#define KT_LOGICAL_MEMBER_PORT_HH

#include <vector>
#include <string>
#include "physicallayer.hh"
#include "itc_kt_state_base.hh"

using unc::uppl::ODBCMOperator;

/* @  Logical Member Port Class definition */
class Kt_LogicalMemberPort : public Kt_State_Base {
 public:
  Kt_LogicalMemberPort();

  ~Kt_LogicalMemberPort();

  UncRespCode DeleteKeyInstance(OdbcmConnectionHandler *db_conn,
      void* key_struct,
      uint32_t data_type,
      uint32_t key_type);

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

  UncRespCode IsKeyExists(OdbcmConnectionHandler *db_conn,
                             unc_keytype_datatype_t data_type,
                             const vector<string>& key_values);

  void Fill_Attr_Syntax_Map();
  UncRespCode ReadInternal(OdbcmConnectionHandler *db_conn,
                              vector<void *> &key_val,
                              vector<void *> &val_struct,
                              uint32_t data_type,
                              uint32_t operation_type);
  pfc_bool_t CompareKeyStruct(void *key_struct1,
                              void *key_struct2) {
    key_logical_member_port_t member_port_key1 =
        *(reinterpret_cast<key_logical_member_port_t*>(key_struct1));
    key_logical_member_port_t member_port_key2 =
        *(reinterpret_cast<key_logical_member_port_t*>(key_struct2));
    if (memcmp(
        member_port_key1.logical_port_key.domain_key.
        ctr_key.controller_name,
        member_port_key2.logical_port_key.domain_key.
        ctr_key.controller_name,
        sizeof(member_port_key1.logical_port_key.domain_key.
               ctr_key.controller_name)) == 0 &&
               memcmp(
                   member_port_key1.logical_port_key.domain_key.domain_name,
                   member_port_key2.logical_port_key.domain_key.domain_name,
                   sizeof(
                       member_port_key1.logical_port_key.
                       domain_key.domain_name)) == 0 &&
                       memcmp(
                           member_port_key1.logical_port_key.port_id,
                           member_port_key2.logical_port_key.port_id,
                           sizeof(
                               member_port_key1.
                               logical_port_key.port_id)) == 0 &&
                               memcmp(
                                   member_port_key1.physical_port_id,
                                   member_port_key2.physical_port_id,
                                   sizeof(
                                       member_port_key1.
                                       physical_port_id)) == 0 &&
                                       memcmp(
                                           member_port_key1.switch_id,
                                           member_port_key2.switch_id,
                                           sizeof(
                                               member_port_key1.
                                               switch_id)) == 0) {
      return PFC_TRUE;
    }
    return PFC_FALSE;
  }

  UncRespCode UpdateDomainNameForTP(OdbcmConnectionHandler *db_conn,
                                    void* key_struct,
                                    void* val_struct,
                                    uint32_t data_type,
                                    uint32_t key_type);

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
      pfc_bool_t is_state);


  void FillLogicalMemberPortValueStructure(OdbcmConnectionHandler *db_conn,
      DBTableSchema &kt_logical_member_port_dbtableschema,
      uint32_t &max_rep_ct,
      uint32_t operation_type,
      vector<key_logical_member_port_t> &logical_mem_port);

  UncRespCode PerformRead(OdbcmConnectionHandler *db_conn,
                             uint32_t session_id,
                             uint32_t configuration_id,
                             void* key_struct,
                             void* value_struct,
                             uint32_t data_type,
                             uint32_t operation_type,
                             ServerSession &sess,
                             uint32_t option1,
                             uint32_t option2,
                             uint32_t max_rep_ct);

  UncRespCode ReadLogicalMemberPortValFromDB(OdbcmConnectionHandler *db_conn,
      void* key_struct,
      uint32_t data_type,
      uint32_t operation_type,
      uint32_t &max_rep_ct,
      vector<key_logical_member_port_t> &logical_mem_port);

  UncRespCode ReadBulkInternal(OdbcmConnectionHandler *db_conn,
      void* key_struct,
      uint32_t data_type,
      uint32_t max_rep_ct,
      vector<key_logical_member_port_t> &vect_logical_mem_port);
};
#endif

