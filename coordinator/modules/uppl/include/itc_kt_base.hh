/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   KT Base implementation
 * @file    itc_kt_base.hh
 *
 */

#ifndef KT_BASE_HH
#define KT_BASE_HH

#include <string>
#include <vector>
#include <map>

#include "physical_common_def.hh"
#include "phy_util.hh"
#include "unc/uppl_common.h"
#include "ipc_connection_manager.hh"
#include "odbcm_utils.hh"
#include "odbcm_common.hh"
#include "odbcm_connection.hh"

using std::vector;
using std::map;
using pfc::core::ipc::ServerSession;
using unc::uppl::ODBCMOperator;
using unc::uppl::DBTableSchema;
using unc::uppl::ODBCMTableColumns;
using unc::uppl::OdbcmConnectionHandler;
using unc::uppl::ScopedDBConnection;

class ReadRequest;

class Kt_Class_Attr_Syntax {
 public:
  pfc_ipctype_t data_type;
  uint64_t min_value;
  uint64_t max_value;
  uint32_t min_length;
  uint32_t max_length;
  bool mandatory_attrib;
  string default_value;
};

class OperStatusHolder {
 private:
  unc_key_type_t key_type_;
  void* key_struct_;
  uint8_t oper_status_;
 public:
  OperStatusHolder(unc_key_type_t key_type,
                   void* key_struct,
                   uint8_t oper_status):key_type_(key_type),
                   key_struct_(key_struct),
                   oper_status_(oper_status) {
  };
  unc_key_type_t get_key_type() {return key_type_;}
  void* get_key_struct() {return key_struct_;}
  uint8_t get_oper_status() {return oper_status_;}
};

class Kt_Base {
 public:
  virtual ~Kt_Base() {
  };
  unc_key_type_t get_key_type() {
    return UNC_KT_ROOT;
  };

  virtual UncRespCode Create(OdbcmConnectionHandler *db_conn,
                                uint32_t session_id, uint32_t configuration_id,
                                void* key_struct, void* val_struct,
                                uint32_t data_type, ServerSession &sess) {
    return UNC_RC_SUCCESS;
  };

  virtual UncRespCode CreateKeyInstance(OdbcmConnectionHandler *db_conn,
                                           void* key_struct,
                                           void* val_struct,
                                           uint32_t data_type,
                                           uint32_t key_type) {
    return UNC_RC_SUCCESS;
  };

  virtual UncRespCode UpdateKeyInstance(OdbcmConnectionHandler *db_conn,
                                           void* key_struct, void* val_struct,
                                           uint32_t data_type,
                                           uint32_t key_type,
                                           void* &old_val_struct) {
    return UNC_RC_SUCCESS;
  };

  virtual UncRespCode Update(OdbcmConnectionHandler *db_conn,
                                uint32_t session_id, uint32_t configuration_id,
                                void* key_struct, void* val_struct,
                                uint32_t data_type, ServerSession &sess) {
    return UNC_RC_SUCCESS;
  };

  virtual UncRespCode Delete(OdbcmConnectionHandler *db_conn,
                                uint32_t session_id, uint32_t configuration_id,
                                void* key_struct, uint32_t data_type,
                                ServerSession &sess) {
    return UNC_RC_SUCCESS;
  };

  virtual UncRespCode DeleteKeyInstance(OdbcmConnectionHandler *db_conn,
                                           void* key_struct,
                                           uint32_t data_type,
                                           uint32_t key_type) {
    return UNC_RC_SUCCESS;
  };

  virtual UncRespCode ReadInternal(OdbcmConnectionHandler *db_conn,
                                      vector<void*> &key_struct,
                                      vector<void*> &val_struct,
                                      uint32_t data_type,
                                      uint32_t operation_type) {
    return UNC_RC_SUCCESS;
  };

  virtual UncRespCode Read(OdbcmConnectionHandler *db_conn,
                              uint32_t session_id, uint32_t configuration_id,
                              void* key_struct, void* val_struct,
                              uint32_t data_type, ServerSession &sess,
                              uint32_t option1, uint32_t option2);

  virtual UncRespCode ReadNext(OdbcmConnectionHandler *db_conn,
                                  uint32_t session_id, void* key_struct,
                                  uint32_t data_type,
                                  ReadRequest *read_req);

  virtual UncRespCode ReadBulk(OdbcmConnectionHandler *db_conn,
                                  void* key_struct,
                                  uint32_t data_type,
                                  uint32_t &max_rep_ct,
                                  int child_index,
                                  pfc_bool_t parent_call,
                                  pfc_bool_t is_read_next,
                                  ReadRequest *read_req)=0;

  virtual UncRespCode ReadSiblingBegin(OdbcmConnectionHandler *db_conn,
                                          uint32_t session_id,
                                          uint32_t configuration_id,
                                          void* key_struct, void* val_struct,
                                          uint32_t data_type,
                                          ServerSession &sess,
                                          uint32_t option1, uint32_t option2,
                                          uint32_t &max_rep_ct);

  virtual UncRespCode ReadSibling(OdbcmConnectionHandler *db_conn,
                                     uint32_t session_id,
                                     uint32_t configuration_id,
                                     void* key_struct, void* val_struct,
                                     uint32_t data_type, ServerSession &sess,
                                     uint32_t option1, uint32_t option2,
                                     uint32_t &max_rep_ct);

  virtual UncRespCode ReadSiblingCount(OdbcmConnectionHandler *db_conn,
                                          uint32_t session_id,
                                          uint32_t configuration_id,
                                          void* key_struct, void* val_struct,
                                          uint32_t key_type,
                                          uint32_t data_type,
                                          ServerSession &sess,
                                          uint32_t option1, uint32_t option2);

  virtual UncRespCode PerformRead(OdbcmConnectionHandler *db_conn,
                                     uint32_t session_id,
                                     uint32_t configuration_id,
                                     void* key_struct, void* val_struct,
                                     uint32_t data_type,
                                     uint32_t operation_type,
                                     ServerSession &sess, uint32_t option1,
                                     uint32_t option2, uint32_t max_rep_ct)=0;

  virtual UncRespCode PerformSyntaxValidation(
      OdbcmConnectionHandler *db_conn,
      void* key_struct,
      void* val_struct,
      uint32_t operation,
      uint32_t data_type)=0;

  virtual UncRespCode PerformSemanticValidation(
      OdbcmConnectionHandler *db_conn,
      void* key_struct,
      void* val_struct,
      uint32_t operation,
      uint32_t data_type)=0;

  UncRespCode
  ValidateRequest(OdbcmConnectionHandler *db_conn,
                  void* key_struct, void* val_struct, uint32_t operation,
                  uint32_t data_type, uint32_t key_type);

  virtual UncRespCode ValidateCtrlrValueCapability(string version,
                                                      uint32_t key_type) {
    return UNC_RC_SUCCESS;
  };

  virtual UncRespCode ValidateCtrlrScalability(std::string version,
                                                  uint32_t key_type,
                                                  uint32_t data_type) {
    return UNC_RC_SUCCESS;
  };

  // Caller can check whether a controller_id is existing (or)
  // A switch within a controller is existing
  // In this case list will contain two values -
  // a controller id and a switch id
  virtual UncRespCode IsKeyExists(OdbcmConnectionHandler *db_conn,
                                     unc_keytype_datatype_t data_type,
                                     const vector<string>& key_values) {
    return UNC_RC_SUCCESS;
  };

  UncRespCode ConfigurationChangeNotification(uint32_t data_type,
                                                 uint32_t key_type,
                                                 uint32_t oper_type,
                                                 void *key_struct,
                                                 void* old_val_struct,
                                                 void* new_val_struct);

  virtual UncRespCode HandleDriverAlarms(OdbcmConnectionHandler *db_conn,
                                            uint32_t data_type,
                                            uint32_t alarm_type,
                                            uint32_t oper_type,
                                            void* key_struct,
                                            void* val_struct) {
    return UNC_RC_SUCCESS;
  };

  virtual UncRespCode GetModifiedRows(OdbcmConnectionHandler *db_conn,
                                         vector<void *> &key_struct,
                                         vector<void *> &val_struct,
                                         CsRowStatus row_status) {
    return UNC_RC_SUCCESS;
  };

  virtual void Fill_Attr_Syntax_Map() {
  };

  virtual UncRespCode ValidateControllerCount(OdbcmConnectionHandler *db_conn,
                               void *key_struct,
                               void *val_struct,
                               uint32_t data_type) {
    return UNC_RC_SUCCESS;
  };

  virtual UncRespCode HandleOperStatus(OdbcmConnectionHandler *db_conn,
                                          uint32_t data_type,
                                          void *key_struct,
                                          void *value_struct) {
    return UNC_RC_SUCCESS;
  };

  virtual UncRespCode NotifyOperStatus(OdbcmConnectionHandler *db_conn,
                                          uint32_t data_type,
                                          void *key_struct,
                                          void *value_struct) {
    return UNC_RC_SUCCESS;
  };

  virtual pfc_bool_t CompareKeyStruct(void *key_struct1, void *key_struct2) {
    return UNC_RC_SUCCESS;
  };

  virtual pfc_bool_t CompareValueStruct(void *value_struct1,
                                        void *value_struct2) {
    return UNC_RC_SUCCESS;
  };
  virtual void
  PopulateDBSchemaForKtTable(OdbcmConnectionHandler *db_conn,
                             DBTableSchema &kt_dbtableschema,
                             void* key_struct, void* val_struct,
                             uint8_t operation_type, uint32_t data_type,
                             uint32_t option1, uint32_t option2,
                             vector<ODBCMOperator> &vect_key_operations,
                             void* &old_value_struct,
                             CsRowStatus row_status,
                             pfc_bool_t is_filtering,
                             pfc_bool_t is_state)=0;

  pfc_ipcevtype_t GetEventType(uint32_t key_type);
  int AddKeyStructuretoSession(uint32_t key_type,
                               ServerEvent &ser_event,
                               void *key_struct);
  int AddKeyStructuretoSession(uint32_t key_type,
                               ServerSession *session,
                               void *key_struct);
  int AddValueStructuretoSession(uint32_t key_type,
                                 uint32_t oper_type,
                                 uint32_t data_type,
                                 ServerEvent &ser_event,
                                 void *new_value_struct,
                                 void *old_value_struct);
  void ClearValueStructure(uint32_t key_type,
                           void *&old_value_struct);
  static map<unc_key_type_t, map<string, Kt_Class_Attr_Syntax> >
  attr_syntax_map_all;
  UncRespCode get_oper_status(vector<OperStatusHolder> &ref_oper_status,
                                   unc_key_type_t key_type,
                                   void* key_struct,
                                   uint8_t &oper_status);
  void ClearOperStatusHolder(vector<OperStatusHolder> &ref_oper_status);

 private:
  UncRespCode ValidateKtRoot(uint32_t operation,
                                uint32_t data_type);
  UncRespCode ValidateKtDataflow(uint32_t operation,
                                uint32_t data_type);
  UncRespCode ValidateKtCtrlBdry(uint32_t operation,
                                    uint32_t data_type);
  UncRespCode ValidateKtCtrDataflow(uint32_t operation,
                                uint32_t data_type);
  UncRespCode ValidateKtCtrDomain(uint32_t operation,
                                     uint32_t data_type);
  UncRespCode ValidateKtState(uint32_t operation,
                                 uint32_t data_type);
};

#define VALIDATE_STRING_FIELD(attr_name, field, ret_code) \
    { \
  map<string, Kt_Class_Attr_Syntax>::iterator itVal = \
  attr_syntax_map.find((attr_name)); \
  if (itVal != attr_syntax_map.end()) { \
    Kt_Class_Attr_Syntax objAttr = itVal->second; \
    if (objAttr.data_type == PFC_IPCTYPE_STRING) { \
      if ((field).length() < objAttr.min_length || \
          (field).length() > objAttr.max_length) { \
        (ret_code) = UNC_UPPL_RC_ERR_CFG_SYNTAX; \
      } \
    } \
  } else { \
    (ret_code) = UNC_UPPL_RC_ERR_ATTRIBUTE_NOT_SUPPORTED; \
  } \
}

#define VALIDATE_IPV4_FIELD(attr_name, field, ret_code) \
    { \
    map<string, Kt_Class_Attr_Syntax>::iterator itVal = \
    attr_syntax_map.find((attr_name)); \
    if (itVal != attr_syntax_map.end()) { \
      Kt_Class_Attr_Syntax objAttr = itVal->second; \
      if (objAttr.data_type == PFC_IPCTYPE_IPV4) { \
        if ((field).s_addr < objAttr.min_value || \
            (field).s_addr  > objAttr.max_value) { \
          (ret_code) = UNC_UPPL_RC_ERR_CFG_SYNTAX; \
        } \
      } \
    } else { \
      (ret_code) = UNC_UPPL_RC_ERR_ATTRIBUTE_NOT_SUPPORTED; \
    } \
    }

#define VALIDATE_IPV6_FIELD(attr_name, field, ret_code) \
    { \
  map<string, Kt_Class_Attr_Syntax>::iterator itVal = \
  attr_syntax_map.find((attr_name)); \
  if (itVal != attr_syntax_map.end()) { \
    Kt_Class_Attr_Syntax objAttr = itVal->second; \
    if (objAttr.data_type == PFC_IPCTYPE_IPV6) { \
      if (*((field).s6_addr) < objAttr.min_value || \
          *((field).s6_addr)  > objAttr.max_value) { \
        (ret_code) = UNC_UPPL_RC_ERR_CFG_SYNTAX; \
      } \
    } \
  } else { \
    (ret_code) = UNC_UPPL_RC_ERR_ATTRIBUTE_NOT_SUPPORTED; \
  } \
}
#define VALIDATE_INT_FIELD(attr_name, field, ret_code) \
    { \
    map<string, Kt_Class_Attr_Syntax>::iterator itVal = \
    attr_syntax_map.find((attr_name)); \
    if (itVal != attr_syntax_map.end()) { \
      Kt_Class_Attr_Syntax objAttr = itVal->second; \
      if (objAttr.data_type == PFC_IPCTYPE_UINT64 || \
          objAttr.data_type == PFC_IPCTYPE_UINT32 || \
          objAttr.data_type == PFC_IPCTYPE_UINT16 || \
          objAttr.data_type == PFC_IPCTYPE_UINT8) { \
        if ((field) < objAttr.min_value || \
            (field) > objAttr.max_value) { \
          (ret_code) = UNC_UPPL_RC_ERR_CFG_SYNTAX; \
        } \
      } \
    } else { \
      (ret_code) = UNC_UPPL_RC_ERR_ATTRIBUTE_NOT_SUPPORTED; \
    } \
}

#define VALIDATE_MANDATORY_FIELD(attr_name, mandatory) \
    { \
      map<string, Kt_Class_Attr_Syntax>::iterator itVal = \
      attr_syntax_map.find((attr_name)); \
      if (itVal != attr_syntax_map.end()) { \
        Kt_Class_Attr_Syntax objAttr = itVal->second; \
        if (objAttr.mandatory_attrib == false) { \
          (mandatory) = PFC_FALSE; \
        } else { \
          (mandatory) = PFC_TRUE; \
        } \
      } else { \
        (mandatory) = PFC_FALSE; \
      } \
}

#define IS_VALID_STRING_KEY(attr_name, value, \
    operation, ret_code, mandatory) \
    { \
        VALIDATE_MANDATORY_FIELD((attr_name), (mandatory)); \
        if (((operation) == UNC_OP_CREATE || (operation) == UNC_OP_READ || \
            (operation) == UNC_OP_UPDATE || (operation) == UNC_OP_DELETE) \
            && (mandatory) == PFC_TRUE && (value).empty()) { \
          pfc_log_error( \
                         "Key value %s not found for %d operation", \
                         attr_name, operation); \
                         (ret_code) = UNC_UPPL_RC_ERR_CFG_SYNTAX; \
        } \
        VALIDATE_STRING_FIELD((attr_name), (value), (ret_code)); \
        if ((ret_code) != UNC_RC_SUCCESS) { \
          pfc_log_error("Length check failed for attribute %s", \
                        attr_name); \
        } \
}

#define IS_VALID_VLAN_ID(attr_name, value, \
    operation, ret_code, mandatory) \
    { \
       VALIDATE_MANDATORY_FIELD((attr_name), (mandatory)); \
        if (((operation) == UNC_OP_READ) \
            && mandatory == PFC_TRUE \
            && ((PhyUtil::uint16tostr(value)).empty())) { \
          pfc_log_error( \
                         "Key value %s not found for %d operation", \
                         attr_name, operation); \
                         (ret_code) = UNC_UPPL_RC_ERR_CFG_SYNTAX; \
        }  \
    map<string, Kt_Class_Attr_Syntax>::iterator itVal = \
    attr_syntax_map.find(attr_name); \
    if (itVal != attr_syntax_map.end()) { \
      Kt_Class_Attr_Syntax objAttr = itVal->second; \
      if (objAttr.data_type == PFC_IPCTYPE_UINT16) { \
        if ((value) < objAttr.min_value || \
            (value) > objAttr.max_value) { \
          if ((value) == 0xffff) { \
            (ret_code) = UNC_RC_SUCCESS; \
          } else { \
            (ret_code) = UNC_UPPL_RC_ERR_CFG_SYNTAX; \
            pfc_log_error("Value check failed for attribute %s", \
                        attr_name); \
          } \
      } \
     } \
    } else { \
      (ret_code) = UNC_UPPL_RC_ERR_ATTRIBUTE_NOT_SUPPORTED; \
    } \
}


#define IS_VALID_STRING_VALUE(attr_name, value, \
    operation, valid_value, ret_code, mandatory) \
    VALIDATE_MANDATORY_FIELD((attr_name), (mandatory)); \
    if ((operation) == UNC_OP_CREATE && ((valid_value) != UNC_VF_VALID && \
        (mandatory) == PFC_TRUE)) { \
      pfc_log_error(\
                    "Mandatory attribute %s in not available in " \
                    "%d operation", \
                    attr_name, operation); \
                    (ret_code) = UNC_UPPL_RC_ERR_CFG_SYNTAX; \
    } else if ((valid_value) == UNC_VF_VALID) { \
      if ((mandatory) == PFC_TRUE && (value).empty()) { \
        pfc_log_error(\
                      "Mandatory attribute value %s in not " \
                      "available in %d operation", \
                      attr_name, operation); \
                      (ret_code) = UNC_UPPL_RC_ERR_CFG_SYNTAX; \
                      (ret_code) = UNC_UPPL_RC_ERR_CFG_SYNTAX; \
      } \
      VALIDATE_STRING_FIELD((attr_name), (value), (ret_code)); \
      if ((ret_code) != UNC_RC_SUCCESS) { \
        pfc_log_error("Length check failed for attribute %s", \
                      attr_name); \
      } \
}

#define IS_VALID_INT_VALUE(attr_name, value, \
    operation, valid_value, ret_code, mandatory) \
    VALIDATE_MANDATORY_FIELD((attr_name), (mandatory)); \
    if ((operation) == UNC_OP_CREATE && ((valid_value) != UNC_VF_VALID && \
        (mandatory) == PFC_TRUE)) { \
      pfc_log_error(\
                    "Mandatory attribute %s in not available in " \
                    "%d operation", \
                    attr_name, operation); \
                    (ret_code) = UNC_UPPL_RC_ERR_CFG_SYNTAX; \
    } else if ((valid_value) == UNC_VF_VALID) { \
      VALIDATE_INT_FIELD((attr_name), (value), (ret_code)); \
      if ((ret_code) != UNC_RC_SUCCESS) { \
        pfc_log_error("Syntax validation failed for attribute %s", \
                      attr_name); \
      } \
}

#define IS_VALID_IPV4_VALUE(attr_name, value, \
    operation, valid_value, ret_code, mandatory) \
    VALIDATE_MANDATORY_FIELD((attr_name), (mandatory)); \
    if ((operation) == UNC_OP_CREATE && ((valid_value) != UNC_VF_VALID && \
        (mandatory) == PFC_TRUE)) { \
      pfc_log_error(\
                    "Mandatory attribute %s in not available in " \
                    "%d operation", \
                    attr_name, operation); \
                    (ret_code) = UNC_UPPL_RC_ERR_CFG_SYNTAX; \
    } else if ((valid_value) == UNC_VF_VALID) { \
      VALIDATE_IPV4_FIELD((attr_name), (value), (ret_code)); \
      if ((ret_code) != UNC_RC_SUCCESS) { \
        pfc_log_error("Syntax validation failed for attribute %s", \
                      attr_name); \
      } \
}

#define IS_VALID_IPV6_VALUE(attr_name, value, \
    operation, valid_value, ret_code, mandatory) \
    VALIDATE_MANDATORY_FIELD((attr_name), (mandatory)); \
    if ((operation) == UNC_OP_CREATE && ((valid_value) != UNC_VF_VALID && \
        (mandatory) == PFC_TRUE)) { \
      pfc_log_error(\
                    "Mandatory attribute %s in not available in " \
                    "%d operation", \
                    attr_name, operation); \
                    (ret_code) = UNC_UPPL_RC_ERR_CFG_SYNTAX; \
    } else if ((valid_value) == UNC_VF_VALID) { \
      VALIDATE_IPV6_FIELD((attr_name), (value), (ret_code)); \
      if ((ret_code) != UNC_RC_SUCCESS) { \
        pfc_log_error("Syntax validation failed for attribute %s", \
                      attr_name); \
      } \
}

#define ADD_CTRL_OPER_STATUS(controller_name, oper_status, ref_oper_status) \
    key_ctr_t *ctlr_key = new key_ctr_t; \
    memset((ctlr_key->controller_name), 0, 32); \
    memcpy((ctlr_key->controller_name), ((controller_name).c_str()), \
           ((controller_name).length())+1); \
    OperStatusHolder obj_oper_status_ctr(UNC_KT_CONTROLLER, \
                                     reinterpret_cast<void *>(ctlr_key), \
                                     (oper_status)); \
    (ref_oper_status).push_back(obj_oper_status_ctr); \

#define GET_ADD_CTRL_OPER_STATUS(ctr_name, vect_operstatus, data_type, db_conn)\
    key_ctr_t *ctr_key = new key_ctr_t; \
    memset(ctr_key->controller_name, 0, 32); \
    memcpy((ctr_key->controller_name), ((ctr_name).c_str()), \
           ((ctr_name).length())+1); \
    void* key_type_struct = reinterpret_cast<void*>(ctr_key); \
    Kt_Controller controller; \
    uint8_t ctr_oper_status = \
          (UpplControllerOperStatus) UPPL_CONTROLLER_OPER_UP; \
    uint32_t ctrl_data_type = (data_type); \
    if ((data_type) == UNC_DT_IMPORT) { \
       ctrl_data_type = UNC_DT_RUNNING; \
    } \
    UncRespCode read_status = controller.GetOperStatus( \
        (db_conn), ctrl_data_type, key_type_struct, ctr_oper_status); \
    pfc_log_debug("Controller's read_status %d, oper_status %d", \
              read_status, ctr_oper_status); \
    OperStatusHolder obj_oper_status_ctr(UNC_KT_CONTROLLER, \
                                     reinterpret_cast<void *>(ctr_key), \
                                     ctr_oper_status); \
    (vect_operstatus).push_back(obj_oper_status_ctr);

#define ADD_SWITCH_OPER_STATUS(sw_key, oper_status, ref_oper_status) \
    key_switch_t *p_switch = new key_switch_t(sw_key); \
    OperStatusHolder obj_oper_status_sw(UNC_KT_SWITCH, \
                                     reinterpret_cast<void*>(p_switch), \
                                     (oper_status)); \
    (ref_oper_status).push_back(obj_oper_status_sw);

#define ADD_PORT_OPER_STATUS(port_key, oper_status, ref_oper_status) \
    key_port_t *p_port = new key_port_t(port_key); \
    OperStatusHolder obj_oper_status_port(UNC_KT_PORT, \
                                     reinterpret_cast<void*>(p_port), \
                                     (oper_status)); \
    (ref_oper_status).push_back(obj_oper_status_port);

#define ADD_LP_PORT_OPER_STATUS(lp_key, oper_status, ref_oper_status) \
    key_logical_port_t *p_lport = new key_logical_port_t(lp_key); \
    OperStatusHolder obj_oper_status_lp(UNC_KT_LOGICAL_PORT, \
                                     reinterpret_cast<void*>(p_lport), \
                                     (oper_status)); \
    (ref_oper_status).push_back(obj_oper_status_lp); \

#endif
