/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   KT Link implementation
 * @file    itc_kt_link.hh
 *
 */

#ifndef KT_PHYSICALLAYER_LINK_H_
#define KT_PHYSICALLAYER_LINK_H_

#include <string>
#include <vector>
#include "itc_kt_state_base.hh"
#include "physicallayer.hh"

using unc::uppl::ODBCMOperator;

/* @ Link Class definition */
class Kt_Link : public Kt_State_Base {
 public:
  Kt_Link();

  ~Kt_Link();

  UncRespCode DeleteKeyInstance(OdbcmConnectionHandler *db_conn,
                                   void* key_struct,
                                   uint32_t data_type,
                                   uint32_t key_type);
  UncRespCode ReadInternal(OdbcmConnectionHandler *db_conn,
      vector<void *> &key_val,
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

  UncRespCode IsKeyExists(OdbcmConnectionHandler *db_conn,
                             unc_keytype_datatype_t data_type,
                             const vector<string>& key_values);

  void Fill_Attr_Syntax_Map();

  UncRespCode HandleOperStatus(OdbcmConnectionHandler *db_conn,
                                  uint32_t data_type,
                                  void *key_struct,
                                  void *value_struct);

  pfc_bool_t CompareValueStruct(void *value_struct1,
                              void *value_struct2) {
    val_link_st_t link_val1 =
        *(reinterpret_cast<val_link_st_t*>(value_struct1));
    val_link_st_t link_val2 =
        *(reinterpret_cast<val_link_st_t*>(value_struct2));
    if (memcmp(
        link_val1.link.description,
        link_val2.link.description,
        sizeof(link_val1.link.description)) == 0 &&
        link_val1.oper_status == link_val2.oper_status) {
      return PFC_TRUE;
    }
    return PFC_FALSE;
  }
  pfc_bool_t CompareKeyStruct(void *key_struct1,
                            void *key_struct2) {
    key_link_t link_key1 =
        *(reinterpret_cast<key_link_t*>(key_struct1));
    key_link_t link_key2 =
        *(reinterpret_cast<key_link_t*>(key_struct2));
    if (memcmp(
        link_key1.ctr_key.controller_name,
        link_key2.ctr_key.controller_name,
        sizeof(link_key1.ctr_key.controller_name)) == 0 &&
        memcmp(
            link_key1.switch_id1,
            link_key2.switch_id1,
            sizeof(link_key1.switch_id1)) == 0 &&
            memcmp(
                link_key1.port_id1,
                link_key2.port_id1,
                sizeof(link_key1.port_id1)) ==0 &&
                memcmp(
                    link_key1.switch_id2,
                    link_key2.switch_id2,
                    sizeof(link_key1.switch_id2)) == 0 &&
                    memcmp(
                        link_key1.port_id2,
                        link_key2.port_id2,
                        sizeof(link_key1.port_id2)) == 0)  {
      return PFC_TRUE;
    }
    return PFC_FALSE;
  }

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

  void FillLinkValueStructure(OdbcmConnectionHandler *db_conn,
                              DBTableSchema &kt_link_dbtableschema,
                              vector<val_link_st_t> &vect_obj_val_link,
                              uint32_t &max_rep_ct,
                              uint32_t operation_type,
                              vector<key_link_t> &link_id);

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

  UncRespCode ReadLinkValFromDB(OdbcmConnectionHandler *db_conn,
                                   void* key_struct,
                                   void* val_struct,
                                   uint32_t data_type,
                                   uint32_t operation_type,
                                   uint32_t &max_rep_ct,
                                   vector<val_link_st_t> &vect_val_link_st,
                                   vector<key_link_t> &link_id,
                                   uint32_t option1,
                                   uint32_t option2);

  UncRespCode ReadBulkInternal(OdbcmConnectionHandler *db_conn,
                                  void* key_struct,
                                  uint32_t data_type,
                                  uint32_t max_rep_ct,
                                  vector<val_link_st_t> &vect_val_link_st,
                                  vector<key_link_t> &vect_link_id);

  UncRespCode SetOperStatus(OdbcmConnectionHandler *db_conn,
                               uint32_t data_type,
                               void* key_struct,
                               UpplLinkOperStatus oper_status);
  UncRespCode GetLinkValidFlag(OdbcmConnectionHandler *db_conn,
      void *key_struct,
      val_link_st_t &val_link_valid_st,
      uint32_t data_type);
  UncRespCode GetOperStatus(OdbcmConnectionHandler *db_conn,
                               uint32_t data_type,
                               void* key_struct, uint8_t &oper_status);
  void FrameValidValue(string attr_value, val_link_st &obj_val_link);
  void PopulatePrimaryKeys(uint32_t operation_type,
                           uint32_t option1,
                           uint32_t option2,
                           string switch_id1,
                           string switch_id2,
                           string port_id1,
                           string port_id2,
                           vector<string> &vect_prim_keys,
                           vector<ODBCMOperator> &vect_prim_keys_operation);
};
#endif
