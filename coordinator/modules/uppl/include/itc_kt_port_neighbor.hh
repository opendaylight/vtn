/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief    KT Port Neighbor implementation
 * @file     itc_kt_port_neighbor.hh
 *
 */

#ifndef KT_PORT_NEIGHBOR_H
#define KT_PORT_NEIGHBOR_H

#include <vector>
#include <string>
#include "physicallayer.hh"
#include "itc_kt_state_base.hh"
#include "ipc_client_configuration_handler.hh"
using unc::uppl::ODBCMOperator;
/* @ Port Neighbor Class definition */
class Kt_Port_Neighbor: public Kt_State_Base {
 public:
  Kt_Port_Neighbor();

  ~Kt_Port_Neighbor();

  UncRespCode ReadBulk(OdbcmConnectionHandler *db_conn,
                          void* key_struct,
                          uint32_t data_type,
                          uint32_t &max_rep_ct,
                          int child_index,
                          pfc_bool_t parent_call,
                          pfc_bool_t is_read_next,
                          ReadRequest *read_req) {return UNC_RC_SUCCESS;}

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

  UncRespCode UpdateKeyInstance(OdbcmConnectionHandler *db_conn,
                                   void* key_struct,
                                   void* val_struct,
                                   uint32_t data_type,
                                   uint32_t key_type,
                                   void* &old_val_struct);

  UncRespCode DeleteKeyInstance(OdbcmConnectionHandler *db_conn,
                                   void* key_struct,
                                   uint32_t data_type,
                                   uint32_t key_type);
   pfc_bool_t CompareValueStruct(void *value_struct1,
                                void *value_struct2) {
    val_port_st_neighbor_t port_val1 =
        *(reinterpret_cast<val_port_st_neighbor_t*>(value_struct1));
    val_port_st_neighbor_t port_val2 =
        *(reinterpret_cast<val_port_st_neighbor_t*>(value_struct2));
    if (memcmp(
            port_val1.connected_controller_id,
            port_val2.connected_controller_id,
            sizeof(port_val1.connected_controller_id)) == 0 &&
        memcmp(
            port_val1.connected_switch_id,
            port_val2.connected_switch_id,
            sizeof(port_val1.connected_switch_id)) == 0 &&
        memcmp(
            port_val1.connected_port_id,
            port_val2.connected_port_id,
            sizeof(port_val1.connected_port_id)) == 0) {
      return PFC_TRUE;
    }
    return PFC_FALSE;
  }
  pfc_bool_t CompareKeyStruct(void *key_struct1,
                              void *key_struct2) {
    key_port_t port_key1 =
        *(reinterpret_cast<key_port_t *>(key_struct1));
    key_port_t port_key2 =
        *(reinterpret_cast<key_port_t *>(key_struct2));
    if (memcmp(
        port_key1.sw_key.ctr_key.controller_name,
        port_key2.sw_key.ctr_key.controller_name,
        sizeof(port_key1.sw_key.ctr_key.controller_name)) == 0 &&
        memcmp(port_key1.sw_key.switch_id,
               port_key2.sw_key.switch_id,
               sizeof(port_key1.sw_key.switch_id)) == 0 &&
               memcmp(port_key1.port_id,
                      port_key2.port_id,
                      sizeof(port_key1.port_id)) == 0 ) {
      return PFC_TRUE;
    }
    return PFC_FALSE;
  }


  UncRespCode ReadInternal(OdbcmConnectionHandler *db_conn,
                              vector<void *> &key_val,
                              vector<void *> &val_struct,
                              uint32_t data_type,
                              uint32_t operation_type);
  UncRespCode ReadPortNeighbor(OdbcmConnectionHandler *db_conn,
                              uint32_t data_type,
                              void* key_struct,
                              uint32_t operation_type,
                              uint32_t &max_rep_ct,
                              vector<val_port_st_neighbor> &vect_val_port_nbr,
                              vector<key_port_t> &vect_key_port);


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
  void FrameCValidValue(string attr_value,
                            val_port_st_neighbor_t &obj_port_nbr);
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
                             uint32_t max_rep_ct) {
    return UNC_RC_SUCCESS;
  }
  void PopulatePortStateNeighborValStructure(OdbcmConnectionHandler *db_conn,
        val_port_st_neighbor_t *obj_val_port,
        vector<TableAttrSchema> &vect_table_attr_schema,
        vector<string> &vect_prim_keys,
        uint8_t operation_type,
        val_port_st_neighbor_t *val_port_nbr_st,
        stringstream &valid);
};
#endif
