/*
 * Copyright (c) 2012-2013 NEC Corporation
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

#define ALARM_UPPL_ALARMS_DEFAULT_FLOW 0x01
#define ALARM_UPPL_ALARMS_PORT_DIRECTION 0x02
#define ALARM_UPPL_ALARMS_PORT_CONGES 0x04
using unc::uppl::ODBCMOperator;
/* @ Port Class definition */
class Kt_Port: public Kt_State_Base {
 private:
  Kt_Base *parent;

 public:
  Kt_Port();

  ~Kt_Port();

  UpplReturnCode DeleteKeyInstance(void* key_struct,
                                   uint32_t data_type,
                                   uint32_t key_type);

  UpplReturnCode ReadInternal(vector<void *> &key_val,
                              vector<void *> &val_struct,
                              uint32_t data_type,
                              uint32_t operation_type);

  UpplReturnCode ReadBulk(void* key_struct,
                          uint32_t data_type,
                          uint32_t option1,
                          uint32_t option2,
                          uint32_t &max_rep_ct,
                          int child_index,
                          pfc_bool_t parent_call,
                          pfc_bool_t is_read_next);

  UpplReturnCode PerformSyntaxValidation(void* key_struct,
                                         void* val_struct,
                                         uint32_t operation,
                                         uint32_t data_type);

  UpplReturnCode PerformSemanticValidation(void* key_struct,
                                           void* val_struct,
                                           uint32_t operation,
                                           uint32_t data_type);

  UpplReturnCode HandleDriverAlarms(uint32_t data_type,
                                    uint32_t alarm_type,
                                    uint32_t oper_type,
                                    void* key_struct,
                                    void* val_struct);
  UpplReturnCode HandleOperStatus(uint32_t data_type,
                                  void *key_struct,
                                  void *value_struct);

  UpplReturnCode NotifyOperStatus(uint32_t data_type,
                                  void *key_struct,
                                  void *value_struct);

  UpplReturnCode GetOperStatus(uint32_t data_type,
                               void* key_struct,
                               uint8_t &oper_status);

  UpplReturnCode GetAlarmStatus(uint32_t data_type,
                                void* key_struct,
                                uint64_t &alarms_status);

  UpplReturnCode IsKeyExists(unc_keytype_datatype_t data_type,
                             vector<string> key_values);

  void Fill_Attr_Syntax_Map();
  UpplReturnCode UpdatePortValidFlag(void *key_struct,
                                     void *val_struct,
                                     val_port_st_t &val_port_valid_st,
                                     unc_keytype_validflag_t valid_val);
  UpplReturnCode PopulateSchemaForValidFlag(void* key_struct,
                                            void* val_struct,
                                            string new_val);
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
                    port_val1.logical_port_id == port_val2.logical_port_id) {
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

 private:
  void PopulateDBSchemaForKtTable(
      DBTableSchema &kt_dbtableschema,
      void* key_struct,
      void* val_struct,
      uint8_t operation_type,
      uint32_t option1,
      uint32_t option2,
      vector<ODBCMOperator> &vect_key_operations,
      void* &old_value_struct,
      CsRowStatus row_status= NOTAPPLIED,
      pfc_bool_t is_filtering= false,
      pfc_bool_t is_state= PFC_FALSE);

  void FillPortValueStructure(DBTableSchema &kt_port_dbtableschema,
                              vector<val_port_st_t> &vect_obj_val_port,
                              uint32_t &max_rep_ct,
                              uint32_t operation_type,
                              vector<key_port_t> &port_id);

  UpplReturnCode PerformRead(uint32_t session_id,
                             uint32_t configuration_id,
                             void* key_struct,
                             void* val_struct,
                             uint32_t data_type,
                             uint32_t operation_type,
                             ServerSession &sess,
                             uint32_t option1,
                             uint32_t option2,
                             uint32_t max_rep_ct);

  UpplReturnCode ReadPortValFromDB(void* key_struct,
                                   void* val_struct,
                                   uint32_t data_type,
                                   uint32_t operation_type,
                                   uint32_t &max_rep_ct,
                                   vector<val_port_st_t> &vect_val_port_st,
                                   vector<key_port_t> &port_id);

  UpplReturnCode ReadBulkInternal(void* key_struct,
                                  void* value_struct,
                                  uint32_t data_type,
                                  uint32_t max_rep_ct,
                                  vector<val_port_st_t> &vect_val_port,
                                  vector<key_port_t> &vect_port_id);
  UpplReturnCode SetOperStatus(uint32_t data_type,
                               void* key_struct,
                               UpplPortOperStatus oper_status,
                               bool is_single_key);
  UpplReturnCode ReadNeighbor(
      void* key_struct,
      void* val_struct,
      uint32_t data_type,
      val_port_st_neighbor &obj_neighbor);
  void FrameValidValue(string attr_value, val_port_st &obj_val_port);
  void GetPortValStructure(
        val_port_st_t *obj_val_port,
        vector<TableAttrSchema> &vect_table_attr_schema,
        vector<string> &vect_prim_keys,
        uint8_t operation_type,
        val_port_st_t *val_port_valid_st,
        stringstream &valid);
    void GetPortStateValStructure(
        val_port_st_t *obj_val_port,
        vector<TableAttrSchema> &vect_table_attr_schema,
        vector<string> &vect_prim_keys,
        uint8_t operation_type,
        val_port_st_t *val_port_valid_st,
        stringstream &valid);
};
#endif
