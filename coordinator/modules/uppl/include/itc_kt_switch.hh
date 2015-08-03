/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   KT Switch implementation
 * @file    itc_kt_switch.hh
 *
 */

#ifndef KT_SWITCH_HH
#define KT_SWITCH_HH

#include <vector>
#include <string>
#include "itc_kt_state_base.hh"
#include "physicallayer.hh"

#define KT_SWITCH_CHILD_COUNT 1

#define ALARM_UPPL_ALARMS_FLOW_ENT_FULL 0x01  // Bit 0 UPPL_ALARMS_FLOW_ENT_FULL
#define ALARM_UPPL_ALARMS_OFS_LACK_FEATURES 0x02  // Bit 1
                                               // UPPL_ALARMS_OFS_LACK_FEATURES
#define ALARM_UPPL_ALARMS_OFS_DISABLED 0x04  // Bit 2 UPPL_ALARMS_OFS_DISABLED
using unc::uppl::ODBCMOperator;

typedef enum {
  KIdxPort
}KtSwitchChildClass;


/* @  Switch Class definition */
class Kt_Switch : public Kt_State_Base {
 private:
  Kt_Base *child[KT_SWITCH_CHILD_COUNT];

 public:
  Kt_Switch();

  ~Kt_Switch();

  UncRespCode DeleteKeyInstance(OdbcmConnectionHandler *db_conn,
                                   void* key_struct,
                                   uint32_t data_type,
                                   uint32_t key_type);

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

  UncRespCode HandleDriverAlarms(OdbcmConnectionHandler *db_conn,
                                    uint32_t data_type,
                                    uint32_t alarm_type,
                                    uint32_t oper_type,
                                    void* key_struct,
                                    void* val_struct);

  UncRespCode IsKeyExists(OdbcmConnectionHandler *db_conn,
                             unc_keytype_datatype_t data_type,
                             const vector<string>& key_values);

  UncRespCode NotifyOperStatus(OdbcmConnectionHandler *db_conn,
                                  uint32_t data_type,
                                  void* key_struct,
                                  void* value_struct,
                                  vector<OperStatusHolder> &ref_oper_status);

  UncRespCode HandleOperStatus(OdbcmConnectionHandler *db_conn,
                                  uint32_t data_type,
                                  void *key_struct,
                                  void *value_struct);

  UncRespCode GetOperStatus(OdbcmConnectionHandler *db_conn,
                               uint32_t data_type,
                               void* key_struct,
                               uint8_t &oper_status);

  UncRespCode GetAlarmStatus(OdbcmConnectionHandler *db_conn,
                                uint32_t data_type,
                                void* key_struct,
                                uint64_t &alarms_status);
  void Fill_Attr_Syntax_Map();

  UncRespCode UpdateSwitchValidFlag(OdbcmConnectionHandler *db_conn,
                                       void *key_struct,
                                       void *val_struct,
                                       val_switch_st_t &val_switch_val_st,
                                       unc_keytype_validflag_t valid_val,
                                       uint32_t data_type);

  UncRespCode PopulateSchemaForValidFlag(OdbcmConnectionHandler *db_conn,
                                            void* key_struct,
                                            void* val_struct,
                                            string valid_new,
                                            uint32_t data_type);

  pfc_bool_t CompareValueStruct(void *val_struct1,
                                void *val_struct2) {
    val_switch_st_t switch_val1 =
        *(reinterpret_cast<val_switch_st_t*>(val_struct1));
    val_switch_st_t switch_val2 =
        *(reinterpret_cast<val_switch_st_t*>(val_struct2));
    char *ip_value1 = new char[16];
    inet_ntop(AF_INET, &switch_val1.switch_val.ip_address,
              ip_value1, INET_ADDRSTRLEN);
    char *ip_value2 = new char[16];
    inet_ntop(AF_INET, &switch_val2.switch_val.ip_address,
              ip_value2, INET_ADDRSTRLEN);
    if (memcmp(
        switch_val1.switch_val.description,
        switch_val2.switch_val.description,
        sizeof(switch_val1.switch_val.description)) == 0 &&
        memcmp(switch_val1.switch_val.model, switch_val2.switch_val.model,
               sizeof(switch_val1.switch_val.model)) == 0 &&
               strncmp(ip_value1, ip_value2, 16) == 0 &&
               switch_val1.switch_val.admin_status ==
                   switch_val2.switch_val.admin_status &&
                   memcmp(
                       switch_val1.switch_val.domain_name,
                       switch_val2.switch_val.domain_name,
                       sizeof(switch_val1.switch_val.domain_name)) == 0 &&
                       memcmp(
                           switch_val1.switch_val.valid,
                           switch_val2.switch_val.valid,
                           sizeof(switch_val1.switch_val.valid)) == 0 &&
                           switch_val1.oper_status == switch_val2.oper_status
                           &&
                           memcmp(switch_val1.manufacturer,
                                  switch_val2.manufacturer,
                                  sizeof(switch_val1.manufacturer)) == 0&&
                                  memcmp(switch_val1.hardware,
                                         switch_val2.hardware,
                                         sizeof(switch_val1.hardware)) == 0 &&
                                         memcmp(
                                             switch_val1.software,
                                             switch_val2.software,
                                sizeof(switch_val1.software)) == 0 &&
                                switch_val1.alarms_status ==
                                switch_val2.alarms_status ) {
      delete []ip_value1;
      delete []ip_value2;
      return PFC_TRUE;
    }
    delete []ip_value1;
    delete []ip_value2;
    return PFC_FALSE;
  };
  pfc_bool_t CompareKeyStruct(void* key_struct1,
                              void* key_struct2) {
    key_switch_t switch_key1 =
        *(reinterpret_cast<key_switch_t*>(key_struct1));
    key_switch_t switch_key2 =
        *(reinterpret_cast<key_switch_t*>(key_struct2));
    if (memcmp(switch_key1.ctr_key.controller_name,
               switch_key2.ctr_key.controller_name,
               sizeof(switch_key1.ctr_key.controller_name)) == 0 &&
               memcmp(switch_key1.switch_id,
                      switch_key2.switch_id,
                      sizeof(switch_key1.switch_id)) == 0) {
      return PFC_TRUE;
    }
    return PFC_FALSE;
  };

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

  void FillSwitchValueStructure(OdbcmConnectionHandler *db_conn,
      DBTableSchema &kt_switch_dbtableschema,
      vector<val_switch_st_t> &vect_obj_val_switch,
      uint32_t &max_rep_ct,
      uint32_t operation_type,
      vector<key_switch_t> &vect_switch_id);

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

  UncRespCode ReadSwitchValFromDB(OdbcmConnectionHandler *db_conn,
      void* key_struct,
      void* val_struct,
      uint32_t data_type,
      uint32_t operation_type,
      uint32_t &max_rep_ct,
      vector<val_switch_st_t> &vect_val_switch_st,
      vector<key_switch_t> &vect_switch_id);

  UncRespCode ReadBulkInternal(OdbcmConnectionHandler *db_conn,
                                  void* key_struct,
                                  void* val_struct,
                                  uint32_t data_type,
                                  uint32_t max_rep_ct,
                                  vector<val_switch_st_t> &vect_val_switch,
                                  vector<key_switch_t> &vect_switch_id);

  void* getChildKeyStruct(int child_class,
                          string switch_id,
                          string controller_name);
  Kt_Base* GetChildClassPointer(KtSwitchChildClass KIndex);

  UncRespCode SetOperStatus(OdbcmConnectionHandler *db_conn,
                               uint32_t data_type,
                               void* key_struct,
                               UpplSwitchOperStatus oper_status);
  void FreeChildKeyStruct(int child_class,
                          void *key_struct);
  void FrameValidValue(string attr_value,
                       val_switch_st &obj_val_st);
  void GetSwitchValStructure(OdbcmConnectionHandler *db_conn,
      val_switch_st_t *obj_val_switch,
      vector<TableAttrSchema> &vect_table_attr_schema,
      vector<string> &vect_prim_keys,
      uint8_t operation_type,
      val_switch_st_t *val_switch_valid_st,
      stringstream &valid);
  void GetSwitchStateValStructure(OdbcmConnectionHandler *db_conn,
      val_switch_st_t *obj_val_switch,
      vector<TableAttrSchema> &vect_table_attr_schema,
      vector<string> &vect_prim_keys,
      uint8_t operation_type,
      val_switch_st_t *val_switch_valid_st,
      stringstream &valid);
};
#endif

