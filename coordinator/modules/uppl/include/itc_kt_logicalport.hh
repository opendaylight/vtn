/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   KT Logical Port implementation
 * @file    itc_kt_logicalport.hh
 *
 */

#ifndef KT_LOGICALPORT_HH
#define KT_LOGICALPORT_HH

#include <vector>
#include <string>
#include "physicallayer.hh"
#include "itc_kt_state_base.hh"

typedef enum {
  KIdxLogicalMemberPort = 0,
}KtLogicalPortChildClass;

#define UNC_KT_LOGICAL_PORT_CHILD_COUNT 1
using unc::uppl::ODBCMOperator;

/* @ Loical Port Class definition */
class Kt_LogicalPort: public Kt_State_Base {
 private:
  Kt_Base *child[UNC_KT_LOGICAL_PORT_CHILD_COUNT];

 public:
  Kt_LogicalPort();

  ~Kt_LogicalPort();

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
                                  void *value_struct,
                                  vector<OperStatusHolder> &ref_oper_status,
                                  unc_key_type_t caller_kt);

  UncRespCode HandleOperDownCriteriaFromPortStatus(
      OdbcmConnectionHandler *db_conn,
      uint32_t data_type,
      void *key_struct,
      void *value_struct,
      vector<uint32_t> &vectOperStatus,
      pfc_bool_t is_delete_call);

  UncRespCode GetOperDownCriteria(OdbcmConnectionHandler *db_conn,
                                     uint32_t data_type,
                                     void* key_struct,
                                     uint32_t &oper_down_criteria);

  UncRespCode InvokeBoundaryNotifyOperStatus(OdbcmConnectionHandler *db_conn,
                                                uint32_t data_type,
                                                void *key_struct);

  UncRespCode NotifyOperStatus(OdbcmConnectionHandler *db_conn,
                                  uint32_t data_type,
                                  void *key_struct,
                                  void *value_struct,
                                  vector<OperStatusHolder> &ref_oper_status);

  UncRespCode GetOperStatus(OdbcmConnectionHandler *db_conn,
                               uint32_t data_type,
                               void* key_struct, uint8_t &oper_status);

  UncRespCode IsKeyExists(OdbcmConnectionHandler *db_conn,
                             unc_keytype_datatype_t data_type,
                             const vector<string>& key_values);
  void Fill_Attr_Syntax_Map();

  // Used by KT_CONTROLLER for PATH_FAULT Alarm
  void GetAllPortId(OdbcmConnectionHandler *db_conn,
                    uint32_t data_type,
                    string controller_name,
                    string switch_id,
                    string &domain_name,
                    vector <string> &logical_port_id,
                    pfc_bool_t is_single_logical_port);

  pfc_bool_t CompareValueStruct(void *value_struct1,
                                void *value_struct2) {
    val_logical_port_st_t logport_val1 =
        *(reinterpret_cast<val_logical_port_st_t *> (value_struct1));
    val_logical_port_st_t logport_val2 =
        *(reinterpret_cast<val_logical_port_st_t *> (value_struct2));
    if (memcmp(
        logport_val1.logical_port.description,
        logport_val2.logical_port.description,
        sizeof(logport_val1.logical_port.description)) == 0 &&
        logport_val1.logical_port.port_type ==
            logport_val2.logical_port.port_type &&
            memcmp(
                logport_val1.logical_port.switch_id,
                logport_val2.logical_port.switch_id,
                sizeof(logport_val1.logical_port.switch_id)) == 0 &&
                memcmp(
                    logport_val1.logical_port.physical_port_id,
                    logport_val2.logical_port.physical_port_id,
                    sizeof(
                        logport_val1.logical_port.physical_port_id)) == 0 &&
                        logport_val1.logical_port.oper_down_criteria ==
                            logport_val2.logical_port.oper_down_criteria &&
                            logport_val1.oper_status ==
                                logport_val2.oper_status) {
      return PFC_TRUE;
    }
    return PFC_FALSE;
  }
  pfc_bool_t CompareKeyStruct(void *key_struct1,
                              void *key_struct2) {
    key_logical_port_t logport_key1 =
        *(reinterpret_cast<key_logical_port_t *> (key_struct1));
    key_logical_port_t logport_key2 =
        *(reinterpret_cast<key_logical_port_t *> (key_struct2));
    if (memcmp(
        logport_key1.domain_key.ctr_key.controller_name,
        logport_key2.domain_key.ctr_key.controller_name,
        sizeof(logport_key1.domain_key.ctr_key.controller_name)) == 0 &&
        memcmp(
            logport_key1.domain_key.domain_name,
            logport_key2.domain_key.domain_name,
            sizeof(logport_key1.domain_key.domain_name)) == 0 &&
            memcmp(
                logport_key1.port_id,
                logport_key2.port_id,
                sizeof(logport_key1.port_id)) == 0) {
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

  void FillLogicalPortValueStructure(OdbcmConnectionHandler *db_conn,
      DBTableSchema &kt_logicalport_dbtableschema,
      vector<val_logical_port_st_t> &vect_obj_val_logical_port,
      uint32_t &max_rep_ct,
      uint32_t operation_type,
      vector<key_logical_port_t> &controller_id);

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

  UncRespCode ReadLogicalPortValFromDB(OdbcmConnectionHandler *db_conn,
      void* key_struct,
      void* val_struct,
      uint32_t data_type,
      uint32_t operation_type,
      uint32_t &max_rep_ct,
      vector<val_logical_port_st_t> &vect_val_logical_port_st,
      vector<key_logical_port_t> &logical_port_id);

  UncRespCode ReadBulkInternal(OdbcmConnectionHandler *db_conn,
      void* key_struct,
      void* value_struct,
      uint32_t data_type,
      uint32_t max_rep_ct,
      vector<val_logical_port_st_t> &vect_val_logical_port,
      vector<key_logical_port_t> &vect_logical_port_id);

  void* getChildKeyStruct(int child_class,
                          string logicalport_id,
                          string controller_name,
                          string domain_name);

  Kt_Base* GetChildClassPointer(KtLogicalPortChildClass KIndex);

  UncRespCode SetOperStatus(OdbcmConnectionHandler *db_conn,
                               uint32_t data_type,
                               void* key_struct,
                               void* val_struct,
                               UpplLogicalPortOperStatus oper_status);
  UncRespCode GetOperStatusFromOperDownCriteria(
      OdbcmConnectionHandler *db_conn,
      uint32_t data_type,
      void *key_struct,
      void *value_struct,
      UpplLogicalPortOperStatus &logical_port_oper_status);
  void FreeChildKeyStruct(int child_class,
                          void *key_struct);
  UncRespCode GetLogicalPortValidFlag(OdbcmConnectionHandler *db_conn,
      void *key_struct,
      val_logical_port_st_t &val_logical_port_valid_st,
      uint32_t data_type);
  void FrameValidValue(string attr_value,
      val_logical_port_st &obj_val_logical_port_st,
      val_logical_port_t &obj_val_logical_port);
  UncRespCode GetValidFlag(OdbcmConnectionHandler *db_conn,
      uint32_t data_type,
      void* key_struct,
      string *valid_flag);
  UncRespCode GetPortOperStatus(OdbcmConnectionHandler *db_conn,
      key_port_t &port_key,
      uint8_t *port_oper_status,
      uint32_t data_type);
  void GetAllLogicalPort(OdbcmConnectionHandler *db_conn,
      string controller_name,
      string domain_name,
      string switch_id,
      string phy_port_id,
      vector<key_logical_port_t> &vectLogicalPortKey,
      uint32_t data_type);
  UncRespCode ValidatePortType(uint8_t port_type);
  UncRespCode FillBoundaryCandidateDetails(
           OdbcmConnectionHandler *db_conn, 
           val_port_st_neighbor &obj_neighbor,
           val_logical_port_boundary &val_lp_boundary, 
           uint32_t data_type);
};
#endif
