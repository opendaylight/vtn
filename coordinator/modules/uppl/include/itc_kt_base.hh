/*
 * Copyright (c) 2012-2013 NEC Corporation
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
#include "unc/uppl_common.h"
#include "ipc_connection_manager.hh"
#include "odbcm_utils.hh"

using std::vector;
using std::map;
using pfc::core::ipc::ServerSession;
using unc::uppl::ODBCMOperator;
using unc::uppl::DBTableSchema;

class Kt_Class_Attr_Syntax {
 public:
  pfc_ipctype_t data_type;
  uint32_t min_value;
  uint32_t max_value;
  uint32_t min_length;
  uint32_t max_length;
  bool mandatory_attrib;
  string default_value;
};

class Kt_Base {
 public:
  Kt_Base();

  virtual ~Kt_Base() {
  };
  unc_key_type_t get_key_type() {
    return UNC_KT_ROOT;
  };

  virtual UpplReturnCode Create(uint32_t session_id, uint32_t configuration_id,
                                void* key_struct, void* val_struct,
                                uint32_t data_type, ServerSession &sess) {
    return UPPL_RC_SUCCESS;
  };

  virtual UpplReturnCode CreateKeyInstance(void* key_struct,
                                           void* val_struct,
                                           uint32_t data_type,
                                           uint32_t key_type) {
    return UPPL_RC_SUCCESS;
  };

  virtual UpplReturnCode UpdateKeyInstance(void* key_struct, void* val_struct,
                                           uint32_t data_type,
                                           uint32_t key_type,
                                           void* &old_val_struct) {
    return UPPL_RC_SUCCESS;
  };

  virtual UpplReturnCode Update(uint32_t session_id, uint32_t configuration_id,
                                void* key_struct, void* val_struct,
                                uint32_t data_type, ServerSession &sess) {
    return UPPL_RC_SUCCESS;
  };

  virtual UpplReturnCode Delete(uint32_t session_id, uint32_t configuration_id,
                                void* key_struct, uint32_t data_type,
                                ServerSession &sess) {
    return UPPL_RC_SUCCESS;
  };

  virtual UpplReturnCode DeleteKeyInstance(void* key_struct,
                                           uint32_t data_type,
                                           uint32_t key_type) {
    return UPPL_RC_SUCCESS;
  };

  virtual UpplReturnCode ReadInternal(vector<void*> &key_struct,
                                      vector<void*> &val_struct,
                                      uint32_t data_type,
                                      uint32_t operation_type) {
    return UPPL_RC_SUCCESS;
  };

  virtual UpplReturnCode Read(uint32_t session_id, uint32_t configuration_id,
                              void* key_struct, void* val_struct,
                              uint32_t data_type, ServerSession &sess,
                              uint32_t option1, uint32_t option2);

  virtual UpplReturnCode ReadNext(void* key_struct,
                                  uint32_t data_type,
                                  uint32_t option1, uint32_t option2);

  virtual UpplReturnCode ReadBulk(void* key_struct,
                                  uint32_t data_type,
                                  uint32_t option1,
                                  uint32_t option2,
                                  uint32_t &max_rep_ct,
                                  int child_index,
                                  pfc_bool_t parent_call,
                                  pfc_bool_t is_read_next)=0;

  virtual UpplReturnCode ReadSiblingBegin(uint32_t session_id,
                                          uint32_t configuration_id,
                                          void* key_struct, void* val_struct,
                                          uint32_t data_type,
                                          ServerSession &sess,
                                          uint32_t option1, uint32_t option2,
                                          uint32_t &max_rep_ct);

  virtual UpplReturnCode ReadSibling(uint32_t session_id,
                                     uint32_t configuration_id,
                                     void* key_struct, void* val_struct,
                                     uint32_t data_type, ServerSession &sess,
                                     uint32_t option1, uint32_t option2,
                                     uint32_t &max_rep_ct);

  virtual UpplReturnCode ReadSiblingCount(uint32_t session_id,
                                          uint32_t configuration_id,
                                          void* key_struct, void* val_struct,
                                          uint32_t key_type,
                                          uint32_t data_type,
                                          ServerSession &sess,
                                          uint32_t option1, uint32_t option2);

  virtual UpplReturnCode PerformRead(uint32_t session_id,
                                     uint32_t configuration_id,
                                     void* key_struct, void* val_struct,
                                     uint32_t data_type,
                                     uint32_t operation_type,
                                     ServerSession &sess, uint32_t option1,
                                     uint32_t option2, uint32_t max_rep_ct)=0;

  virtual UpplReturnCode PerformSyntaxValidation(void* key_struct,
                                                 void* val_struct,
                                                 uint32_t operation,
                                                 uint32_t data_type)=0;

  virtual UpplReturnCode PerformSemanticValidation(void* key_struct,
                                                   void* val_struct,
                                                   uint32_t operation,
                                                   uint32_t data_type)=0;

  UpplReturnCode
  ValidateRequest(void* key_struct, void* val_struct, uint32_t operation,
                  uint32_t data_type, uint32_t key_type);

  virtual UpplReturnCode ValidateCtrlrValueCapability(string version,
                                                      uint32_t key_type) {
    return UPPL_RC_SUCCESS;
  };

  virtual UpplReturnCode ValidateCtrlrScalability(std::string version,
                                                  uint32_t key_type,
                                                  uint32_t data_type) {
    return UPPL_RC_SUCCESS;
  };

  // Caller can check whether a controller_id is existing (or)
  // A switch within a controller is existing
  // In this case list will contain two values -
  // a controller id and a switch id
  virtual UpplReturnCode IsKeyExists(unc_keytype_datatype_t data_type, vector<
                                     string> key_values) {
    return UPPL_RC_SUCCESS;
  };

  UpplReturnCode ConfigurationChangeNotification(uint32_t data_type,
                                                 uint32_t key_type,
                                                 uint32_t oper_type,
                                                 void *key_struct,
                                                 void* old_val_struct,
                                                 void* new_val_struct);

  virtual UpplReturnCode HandleDriverAlarms(uint32_t data_type,
                                            uint32_t alarm_type,
                                            uint32_t oper_type,
                                            void* key_struct,
                                            void* val_struct) {
    return UPPL_RC_SUCCESS;
  };

  virtual UpplReturnCode GetModifiedRows(vector<void *> &key_struct, vector<
                                         void *> &val_struct,
                                         CsRowStatus row_status) {
    return UPPL_RC_SUCCESS;
  };

  map<string, Kt_Class_Attr_Syntax> attr_syntax_map;
  virtual void Fill_Attr_Syntax_Map() {
  };

  virtual UpplReturnCode HandleOperStatus(uint32_t data_type,
                                          void *key_struct,
                                          void *value_struct) {
    return UPPL_RC_SUCCESS;
  };

  virtual UpplReturnCode NotifyOperStatus(uint32_t data_type,
                                          void *key_struct,
                                          void *value_struct) {
    return UPPL_RC_SUCCESS;
  };

  virtual pfc_bool_t CompareKeyStruct(void *key_struct1, void *key_struct2) {
    return UPPL_RC_SUCCESS;
  };

  virtual pfc_bool_t CompareValueStruct(void *value_struct1,
                                        void *value_struct2) {
    return UPPL_RC_SUCCESS;
  };
  virtual void
  PopulateDBSchemaForKtTable(DBTableSchema &kt_dbtableschema,
                             void* key_struct, void* val_struct,
                             uint8_t operation_type, uint32_t option1,
                             uint32_t option2,
                             vector<ODBCMOperator> &vect_key_operations,
                             void* &old_value_struct,
                             CsRowStatus row_status = NOTAPPLIED,
                             pfc_bool_t is_filtering = false,
                             pfc_bool_t is_state = PFC_FALSE)=0;
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
                           void *old_value_struct);

 private:
  UpplReturnCode ValidateKtRoot(uint32_t operation,
                                uint32_t data_type);
  UpplReturnCode ValidateKtCtrlBdry(uint32_t operation,
                                    uint32_t data_type);
  UpplReturnCode ValidateKtCtrDomain(uint32_t operation,
                                     uint32_t data_type);
  UpplReturnCode ValidateKtState(uint32_t operation,
                                 uint32_t data_type);
};

#define VALIDATE_STRING_FIELD(attr_name, field, ret_code) \
    { \
  map<string, Kt_Class_Attr_Syntax>::iterator itVal = \
  attr_syntax_map.find(attr_name); \
  if (itVal != attr_syntax_map.end()) { \
    Kt_Class_Attr_Syntax objAttr = itVal->second; \
    if (objAttr.data_type == PFC_IPCTYPE_STRING) { \
      if (field.length() < objAttr.min_length || \
          field.length() > objAttr.max_length) { \
        ret_code = UPPL_RC_ERR_CFG_SYNTAX; \
      } \
    } \
  } else { \
    ret_code = UPPL_RC_ERR_ATTRIBUTE_NOT_SUPPORTED; \
  } \
}

#define VALIDATE_IPV4_FIELD(attr_name, field, ret_code) \
    { \
    map<string, Kt_Class_Attr_Syntax>::iterator itVal = \
    attr_syntax_map.find(attr_name); \
    if (itVal != attr_syntax_map.end()) { \
      Kt_Class_Attr_Syntax objAttr = itVal->second; \
      if (objAttr.data_type == PFC_IPCTYPE_IPV4) { \
        if (field.s_addr < objAttr.min_value || \
            field.s_addr  > objAttr.max_value) { \
          ret_code = UPPL_RC_ERR_CFG_SYNTAX; \
        } \
      } \
    } else { \
      ret_code = UPPL_RC_ERR_ATTRIBUTE_NOT_SUPPORTED; \
    } \
    }

#define VALIDATE_IPV6_FIELD(attr_name, field, ret_code) \
    { \
  map<string, Kt_Class_Attr_Syntax>::iterator itVal = \
  attr_syntax_map.find(attr_name); \
  if (itVal != attr_syntax_map.end()) { \
    Kt_Class_Attr_Syntax objAttr = itVal->second; \
    if (objAttr.data_type == PFC_IPCTYPE_IPV6) { \
      if (*(field.s6_addr) < objAttr.min_value || \
          *(field.s6_addr)  > objAttr.max_value) { \
        ret_code = UPPL_RC_ERR_CFG_SYNTAX; \
      } \
    } \
  } else { \
    ret_code = UPPL_RC_ERR_ATTRIBUTE_NOT_SUPPORTED; \
  } \
}
#define VALIDATE_INT_FIELD(attr_name, field, ret_code) \
    { \
    map<string, Kt_Class_Attr_Syntax>::iterator itVal = \
    attr_syntax_map.find(attr_name); \
    if (itVal != attr_syntax_map.end()) { \
      Kt_Class_Attr_Syntax objAttr = itVal->second; \
      if (objAttr.data_type == PFC_IPCTYPE_UINT32 || \
          objAttr.data_type == PFC_IPCTYPE_UINT16 || \
          objAttr.data_type == PFC_IPCTYPE_UINT8) { \
        if (field < objAttr.min_value || \
            field > objAttr.max_value) { \
          ret_code = UPPL_RC_ERR_CFG_SYNTAX; \
        } \
      } \
    } else { \
      ret_code = UPPL_RC_ERR_ATTRIBUTE_NOT_SUPPORTED; \
    } \
}

#define VALIDATE_MANDATORY_FIELD(attr_name, mandatory) \
    { \
      map<string, Kt_Class_Attr_Syntax>::iterator itVal = \
      attr_syntax_map.find(attr_name); \
      if (itVal != attr_syntax_map.end()) { \
        Kt_Class_Attr_Syntax objAttr = itVal->second; \
        if (objAttr.mandatory_attrib == false) { \
          mandatory = PFC_FALSE; \
        } else { \
          mandatory = PFC_TRUE; \
        } \
      } else { \
        mandatory = PFC_FALSE; \
      } \
}

#define IS_VALID_STRING_KEY(attr_name, value, \
    operation, ret_code, mandatory) \
    { \
        VALIDATE_MANDATORY_FIELD(attr_name, mandatory); \
        if ((operation == UNC_OP_CREATE || operation == UNC_OP_READ || \
            operation == UNC_OP_UPDATE || operation == UNC_OP_DELETE) \
            && mandatory == PFC_TRUE && value.empty()) { \
          pfc_log_error( \
                         "Key value %s not found for %d operation", \
                         attr_name, operation); \
                         ret_code = UPPL_RC_ERR_CFG_SYNTAX; \
        } \
        VALIDATE_STRING_FIELD(attr_name, value, ret_code); \
        if (ret_code != UPPL_RC_SUCCESS) { \
          pfc_log_error("Length check failed for attribute %s", \
                        attr_name); \
        } \
}

#define IS_VALID_STRING_VALUE(attr_name, value, \
    operation, valid_value, ret_code, mandatory) \
    VALIDATE_MANDATORY_FIELD(attr_name, mandatory); \
    if (operation == UNC_OP_CREATE && (valid_value != UNC_VF_VALID && \
        mandatory == PFC_TRUE)) { \
      pfc_log_error(\
                    "Mandatory attribute %s in not available in " \
                    "%d operation", \
                    attr_name, operation); \
                    ret_code = UPPL_RC_ERR_CFG_SYNTAX; \
    } else if (valid_value == UNC_VF_VALID) { \
      if (mandatory == PFC_TRUE && value.empty()) { \
        pfc_log_error(\
                      "Mandatory attribute value %s in not " \
                      "available in %d operation", \
                      attr_name, operation); \
                      ret_code = UPPL_RC_ERR_CFG_SYNTAX; \
                      ret_code = UPPL_RC_ERR_CFG_SYNTAX; \
      } \
      VALIDATE_STRING_FIELD(attr_name, value, ret_code); \
      if (ret_code != UPPL_RC_SUCCESS) { \
        pfc_log_error("Length check failed for attribute %s", \
                      attr_name); \
      } \
}

#define IS_VALID_INT_VALUE(attr_name, value, \
    operation, valid_value, ret_code, mandatory) \
    VALIDATE_MANDATORY_FIELD(attr_name, mandatory); \
    if (operation == UNC_OP_CREATE && (valid_value != UNC_VF_VALID && \
        mandatory == PFC_TRUE)) { \
      pfc_log_error(\
                    "Mandatory attribute %s in not available in " \
                    "%d operation", \
                    attr_name, operation); \
                    ret_code = UPPL_RC_ERR_CFG_SYNTAX; \
    } else if (valid_value == UNC_VF_VALID) { \
      VALIDATE_INT_FIELD(attr_name, value, ret_code); \
      if (ret_code != UPPL_RC_SUCCESS) { \
        pfc_log_error("Syntax validation failed for attribute %s", \
                      attr_name); \
      } \
}

#define IS_VALID_IPV4_VALUE(attr_name, value, \
    operation, valid_value, ret_code, mandatory) \
    VALIDATE_MANDATORY_FIELD(attr_name, mandatory); \
    if (operation == UNC_OP_CREATE && (valid_value != UNC_VF_VALID && \
        mandatory == PFC_TRUE)) { \
      pfc_log_error(\
                    "Mandatory attribute %s in not available in " \
                    "%d operation", \
                    attr_name, operation); \
                    ret_code = UPPL_RC_ERR_CFG_SYNTAX; \
    } else if (valid_value == UNC_VF_VALID) { \
      VALIDATE_IPV4_FIELD(attr_name, value, ret_code); \
      if (ret_code != UPPL_RC_SUCCESS) { \
        pfc_log_error("Syntax validation failed for attribute %s", \
                      attr_name); \
      } \
}

#define IS_VALID_IPV6_VALUE(attr_name, value, \
    operation, valid_value, ret_code, mandatory) \
    VALIDATE_MANDATORY_FIELD(attr_name, mandatory); \
    if (operation == UNC_OP_CREATE && (valid_value != UNC_VF_VALID && \
        mandatory == PFC_TRUE)) { \
      pfc_log_error(\
                    "Mandatory attribute %s in not available in " \
                    "%d operation", \
                    attr_name, operation); \
                    ret_code = UPPL_RC_ERR_CFG_SYNTAX; \
    } else if (valid_value == UNC_VF_VALID) { \
      VALIDATE_IPV6_FIELD(attr_name, value, ret_code); \
      if (ret_code != UPPL_RC_SUCCESS) { \
        pfc_log_error("Syntax validation failed for attribute %s", \
                      attr_name); \
      } \
}
#endif
