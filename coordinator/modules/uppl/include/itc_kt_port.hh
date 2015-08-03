/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief    KT Port implementation
 * @file     itc_kt_port.hh
 *
 */

#ifndef KT_PORT_H
#define KT_PORT_H

#include <vector>
#include <string>
#include "physicallayer.hh"
#include "itc_kt_state_base.hh"
#include "ipc_client_configuration_handler.hh"
#define ALARM_UPPL_ALARMS_DEFAULT_FLOW 0x01
#define ALARM_UPPL_ALARMS_PORT_DIRECTION 0x02
#define ALARM_UPPL_ALARMS_PORT_CONGES 0x04
using unc::uppl::ODBCMOperator;
/* @ Port Class definition */
class Kt_Port: public Kt_State_Base {
 public:
  Kt_Port();

  ~Kt_Port();

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

  UncRespCode HandleDriverAlarms(OdbcmConnectionHandler *db_conn,
                                    uint32_t data_type,
                                    uint32_t alarm_type,
                                    uint32_t oper_type,
                                    void* key_struct,
                                    void* val_struct);
  UncRespCode HandleOperStatus(OdbcmConnectionHandler *db_conn,
                                  uint32_t data_type,
                                  void *key_struct,
                                  void *value_struct);

  UncRespCode NotifyOperStatus(OdbcmConnectionHandler *db_conn,
                                  uint32_t data_type,
                                  void *key_struct,
                                  void *value_struct,
                                  vector<OperStatusHolder> &ref_oper_status);

  UncRespCode GetOperStatus(OdbcmConnectionHandler *db_conn,
                               uint32_t data_type,
                               void* key_struct,
                               uint8_t &oper_status);

  UncRespCode GetAlarmStatus(OdbcmConnectionHandler *db_conn,
                                uint32_t data_type,
                                void* key_struct,
                                uint64_t &alarms_status);

  UncRespCode IsKeyExists(OdbcmConnectionHandler *db_conn,
                             unc_keytype_datatype_t data_type,
                             const vector<string>& key_values);

  void Fill_Attr_Syntax_Map();
  UncRespCode UpdatePortValidFlag(OdbcmConnectionHandler *db_conn,
                                     void *key_struct,
                                     void *val_struct,
                                     val_port_st_t &val_port_valid_st,
                                     unc_keytype_validflag_t valid_val,
                                     uint32_t data_type);
  UncRespCode PopulateSchemaForValidFlag(OdbcmConnectionHandler *db_conn,
                                            void* key_struct,
                                            void* val_struct,
                                            string new_val,
                                            uint32_t data_type);
  pfc_bool_t CompareValueStruct(void *value_struct1,
                                void *value_struct2) {
    val_port_st_t port_val1 =
        *(reinterpret_cast<val_port_st_t*>(value_struct1));
    val_port_st_t port_val2 =
        *(reinterpret_cast<val_port_st_t*>(value_struct2));
    if (port_val1.port.port_number == port_val2.port.port_number &&
        memcmp(
            port_val1.port.description,
            port_val2.port.description,
            sizeof(port_val1.port.description)) == 0 &&
            port_val1.port.admin_status == port_val2.port.admin_status &&
            port_val1.port.trunk_allowed_vlan ==
                port_val2.port.trunk_allowed_vlan &&
                port_val1.oper_status == port_val2.oper_status &&
                memcmp(
                    port_val1.mac_address,
                    port_val2.mac_address,
                    sizeof(port_val1.mac_address)) == 0 &&
                    port_val1.direction == port_val2.direction &&
                    port_val1.duplex == port_val2.duplex &&
                    port_val1.speed == port_val2.speed &&
                    port_val1.alarms_status == port_val2.alarms_status &&
                    memcmp(
                        port_val1.logical_port_id,
                        port_val2.logical_port_id,
                        sizeof(port_val1.logical_port_id)) == 0) {
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

  UncRespCode ReadNeighbor(OdbcmConnectionHandler *db_conn,
      void* key_struct,
      void* val_struct,
      uint32_t data_type,
      val_port_st_neighbor &obj_neighbor);
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

  void FillPortValueStructure(OdbcmConnectionHandler *db_conn,
                              DBTableSchema &kt_port_dbtableschema,
                              vector<val_port_st_t> &vect_obj_val_port,
                              uint32_t &max_rep_ct,
                              uint32_t operation_type,
                              vector<key_port_t> &port_id);

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

  UncRespCode ReadPortValFromDB(OdbcmConnectionHandler *db_conn,
                                   void* key_struct,
                                   void* val_struct,
                                   uint32_t data_type,
                                   uint32_t operation_type,
                                   uint32_t &max_rep_ct,
                                   vector<val_port_st_t> &vect_val_port_st,
                                   vector<key_port_t> &port_id);

  UncRespCode ReadBulkInternal(OdbcmConnectionHandler *db_conn,
                                  void* key_struct,
                                  void* value_struct,
                                  uint32_t data_type,
                                  uint32_t max_rep_ct,
                                  vector<val_port_st_t> &vect_val_port,
                                  vector<key_port_t> &vect_port_id);
  UncRespCode SetOperStatus(OdbcmConnectionHandler *db_conn,
                               uint32_t data_type,
                               void* key_struct,
                               UpplPortOperStatus oper_status);
  void FrameValidValue(string attr_value, val_port_st &obj_val_port);
  void GetPortValStructure(OdbcmConnectionHandler *db_conn,
        val_port_st_t *obj_val_port,
        vector<TableAttrSchema> &vect_table_attr_schema,
        vector<string> &vect_prim_keys,
        uint8_t operation_type,
        val_port_st_t *val_port_valid_st,
        stringstream &valid);
    void GetPortStateValStructure(OdbcmConnectionHandler *db_conn,
        val_port_st_t *obj_val_port,
        vector<TableAttrSchema> &vect_table_attr_schema,
        vector<string> &vect_prim_keys,
        uint8_t operation_type,
        val_port_st_t *val_port_valid_st,
        stringstream &valid);
    UncRespCode SubDomainOperStatusHandling(
        OdbcmConnectionHandler *db_conn,
        uint32_t data_type,
        string controller_name,
        string switch_name,
        string physical_port_id);
};
#endif
