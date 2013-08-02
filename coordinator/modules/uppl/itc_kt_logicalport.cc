/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   KT Logicalport implementation
 * @file    itc_kt_logicalport.cc
 *
 */

#include "itc_kt_logicalport.hh"
#include "itc_kt_controller.hh"
#include "itc_kt_ctr_domain.hh"
#include "itc_kt_port.hh"
#include "itc_kt_logical_member_port.hh"
#include "itc_kt_boundary.hh"
#include "itc_kt_switch.hh"
#include "odbcm_db_varbind.hh"
using unc::uppl::PhysicalLayer;
using unc::uppl::ODBCMOperator;

/** Constructor
 * * @Description : This function instantiates parent and child key types for
 * kt_logicalport
 * * * @param[in] : None
 * * * @return    : None
 * */
Kt_LogicalPort::Kt_LogicalPort() {
  parent = NULL;
  for (int i = 0; i < UNC_KT_LOGICAL_PORT_CHILD_COUNT; ++i) {
    child[i] = NULL;
  }
  // Populate structure to be used for syntax validation
  Fill_Attr_Syntax_Map();
}

/** Destructor
 * * @Description : This function clears the parent and child key types
 * instances for kt_logicalport
 * * * @param[in] : None
 * * * @return    : None
 * */
Kt_LogicalPort::~Kt_LogicalPort() {
  if (parent != NULL) {
    delete parent;
    parent = NULL;
  }
  // Delete all child objects
  for (int i = 0; i < UNC_KT_LOGICAL_PORT_CHILD_COUNT; ++i) {
    if (child[i] != NULL) {
      delete child[i];
      child[i] = NULL;
    }
  }
}

/** GetChildClassPointer
 *  * @Description : This function creates a new child class instance
 *   class of KtLogicalPort based on index passed
 *  * @param[in] : KtLogicalPortChildClass
 *  * @return    : Kt_Base*
 */
Kt_Base* Kt_LogicalPort::GetChildClassPointer(KtLogicalPortChildClass KIndex) {
  switch (KIndex) {
    case KIdxLogicalMemberPort: {
      if (child[KIndex] == NULL) {
        child[KIndex] = new Kt_LogicalMemberPort();
      }
      break;
    }
    default: {
      pfc_log_info("Invalid index %d passed to GetChildClassPointer()",
                   KIndex);
      PFC_ASSERT(PFC_FALSE);
      return NULL;
    }
  }
  return child[KIndex];
}

/**DeleteKeyInstance
 * * @Description : This function deletes given instance of UNC_KT_LOGICAL_PORT
 * * * @param[in] : key_struct and data type
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_LogicalPort::DeleteKeyInstance(void* key_struct,
                                                 uint32_t data_type,
                                                 uint32_t key_type) {
  UpplReturnCode delete_status = UPPL_RC_SUCCESS;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();

  // Check whether operation is allowed on the given DT type
  if (((unc_keytype_datatype_t)data_type != UNC_DT_STATE) &&
      ((unc_keytype_datatype_t)data_type != UNC_DT_IMPORT)) {
    pfc_log_error("Delete operation is provided on unsupported data type");
    delete_status = UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    return delete_status;
  } else {
    pfc_log_debug("Delete instance for logical port is supported");
  }

  key_logical_port_t *obj_key_logical_port=
      reinterpret_cast<key_logical_port_t*>(key_struct);
  string controller_name =
      (const char*)obj_key_logical_port->domain_key.ctr_key.controller_name;
  pfc_log_info("controller name: %s", controller_name.c_str());
  string domain_name= (const char*)obj_key_logical_port->domain_key.domain_name;
  string logical_port_id = (const char*)obj_key_logical_port->port_id;

  // Check memberport for corresponding logical port reference and delete it
  for (int child_class = 0;
      child_class < UNC_KT_LOGICAL_PORT_CHILD_COUNT;
      ++child_class) {
    // Filling key_struct corresponding to tht key type
    void *child_key_struct = getChildKeyStruct(child_class,
                                               logical_port_id,
                                               controller_name,
                                               domain_name);
    child[child_class] = GetChildClassPointer(
        (KtLogicalPortChildClass)child_class);
    if (child[child_class] != NULL) {
      UpplReturnCode ch_delete_status = child[child_class]->DeleteKeyInstance(
          child_key_struct,
          data_type,
          UNC_KT_LOGICAL_MEMBER_PORT);
      delete child[child_class];
      child[child_class] = NULL;
      FreeChildKeyStruct(child_class, child_key_struct);
      if (ch_delete_status == UPPL_RC_ERR_NO_SUCH_INSTANCE) {
        pfc_log_debug("No child exists");
      } else if (ch_delete_status != UPPL_RC_SUCCESS) {
        // child delete failed, so return error
        pfc_log_error("Delete failed for child %d with error %d",
                      child_class, ch_delete_status);
        delete_status = UPPL_RC_ERR_CFG_SEMANTIC;
        break;
      }
    } else {
      // Free child key struct
      FreeChildKeyStruct(child_class, child_key_struct);
    }
  }
  // Structure used to send request to ODBC
  DBTableSchema kt_logicalport_dbtableschema;

  // Construct Primary key list
  vector<string> vect_prim_keys;
  if (!controller_name.empty()) {
    vect_prim_keys.push_back(CTR_NAME);
  }
  if (!domain_name.empty()) {
    vect_prim_keys.push_back(DOMAIN_NAME);
  }
  if (!logical_port_id.empty()) {
    vect_prim_keys.push_back(LP_PORT_ID);
  }

  // Construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  TableAttrSchema kt_logicalport_table_attr_schema;
  list< vector<TableAttrSchema> > row_list;

  // controller_name
  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // domain_name
  pfc_log_info("domain name: %s", domain_name.c_str());
  PhyUtil::FillDbSchema(DOMAIN_NAME, domain_name,
                        domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // port_id
  PhyUtil::FillDbSchema(LP_PORT_ID, logical_port_id,
                        logical_port_id.length(), DATATYPE_UINT8_ARRAY_320,
                        vect_table_attr_schema);

  // Send request to ODBC for logicalport_table delete
  kt_logicalport_dbtableschema.set_table_name(UPPL_LOGICALPORT_TABLE);
  kt_logicalport_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_logicalport_dbtableschema.set_row_list(row_list);

  ODBCM_RC_STATUS delete_db_status = physical_layer->get_odbc_manager()-> \
      DeleteOneRow((unc_keytype_datatype_t)data_type,
                   kt_logicalport_dbtableschema);
  pfc_log_info("DeleteOneRow response from DB is %d", delete_db_status);
  if (delete_db_status != ODBCM_RC_SUCCESS) {
    if (delete_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      pfc_log_fatal("DB connection not available or cannot access DB");
      delete_status = UPPL_RC_ERR_DB_ACCESS;
    } else if (delete_db_status == ODBCM_RC_ROW_NOT_EXISTS) {
      pfc_log_error("given instance does not exist");
      delete_status = UPPL_RC_ERR_NO_SUCH_INSTANCE;
    } else {
      // log error to log daemon
      pfc_log_error("Delete operation has failed in logicalport table");
      delete_status = UPPL_RC_ERR_DB_DELETE;
    }
  } else {
    // deletion success send notification to boundary to
    // update operstatus to invalid
    int err = InvokeBoundaryNotifyOperStatus(data_type, key_struct);
    if (err != UPPL_RC_SUCCESS) {
      pfc_log_error(
          "Delete instance: notify request for boundary is failed %d", err);
    }
    // Notify port to update logicalportid as invalid
    void *value_struct = NULL;
    vector<uint32_t> vectOperStatus;
    // call below function to fill port and switch id and call port function
    // to update logicalport id as invalid PFC_TRUE is to mention call is
    // from deletekeyinstance
    HandleOperDownCriteriaFromPortStatus(data_type,
                                         key_struct,
                                         value_struct,
                                         vectOperStatus,
                                         PFC_TRUE);
  }
  return delete_status;
}

/** ReadInternal
 * * @Description :This function reads the given instance of UNC_KT_LOGICAL_PORT
 * * * @param[in] : key_struct, value_struct, data_type
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_LogicalPort::ReadInternal(
    vector<void *> &key_val,
    vector<void *> &val_struct,
    uint32_t data_type,
    uint32_t operation_type) {
  pfc_log_debug("Inside ReadInternal of UNC_KT_LOGICAL_PORT");
  vector<key_logical_port_t> vect_logicalport_id;
  vector<val_logical_port_t> vect_val_logical_port;
  vector<val_logical_port_st_t> vect_val_logical_port_st;
  uint32_t max_rep_ct = 1;
  if (operation_type != UNC_OP_READ) {
    max_rep_ct = UPPL_MAX_REP_CT;
  }
  void *key_struct = key_val[0];
  void *void_val_struct = NULL;
  if (!val_struct.empty()) {
    void_val_struct = val_struct[0];
  }
  // Get read response from database
  UpplReturnCode read_status = ReadLogicalPortValFromDB(
      key_struct,
      void_val_struct,
      data_type,
      operation_type,
      max_rep_ct,
      vect_val_logical_port,
      vect_val_logical_port_st,
      vect_logicalport_id);
  key_val.clear();
  val_struct.clear();
  if (read_status != UPPL_RC_SUCCESS) {
    pfc_log_error("Read operation has failed");
  } else {
    pfc_log_debug("Read operation is success");
    for (unsigned int iIndex = 0 ; iIndex < vect_val_logical_port_st.size();
        ++iIndex) {
      key_logical_port_t *key_log_port = new key_logical_port_t
          (vect_logicalport_id[iIndex]);
      key_val.push_back(reinterpret_cast<void *>(key_log_port));
      val_logical_port_st_t *val_log_port =
          new val_logical_port_st_t(vect_val_logical_port_st[iIndex]);
      val_struct.push_back(reinterpret_cast<void *>(val_log_port));
    }
  }
  return read_status;
}

/** ReadBulk
 * * @Description : This function reads the max_rep_ct number of instances of
 * the UNC_KT_LOGICAL_PORT
 * * * @param[in] : configuration id, key_struct, value_struct,
 *                  data_type, option1, option2 and max_rep_ct
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_LogicalPort::ReadBulk(void* key_struct,
                                        uint32_t data_type,
                                        uint32_t option1,
                                        uint32_t option2,
                                        uint32_t &max_rep_ct,
                                        int child_index,
                                        pfc_bool_t parent_call,
                                        pfc_bool_t is_read_next) {
  UpplReturnCode read_status = UPPL_RC_SUCCESS;
  key_logical_port_t obj_key_logical_port=
      *(reinterpret_cast<key_logical_port_t*>(key_struct));
  if ((unc_keytype_datatype_t)data_type != UNC_DT_STATE) {
    // Not supported
    pfc_log_debug("ReadBulk operation is not allowed in %d data type",
                  data_type);
    read_status = UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    return read_status;
  }
  string str_controller_name = reinterpret_cast<char *>(obj_key_logical_port.
      domain_key.ctr_key.controller_name);
  pfc_log_info("controller name: %s", str_controller_name.c_str());
  string str_domain_name = reinterpret_cast<char *>(obj_key_logical_port.
      domain_key.domain_name);
  string str_logicalport_id =
      reinterpret_cast<char *>(obj_key_logical_port.port_id);
  pfc_bool_t log_port_exists = PFC_FALSE;
  vector<val_logical_port_st_t> vect_val_logical_port_st;
  vector<key_logical_port_t> vect_logicalport_id;
  // Check for child call
  if (max_rep_ct == 0) {
    pfc_log_info("max_rep_ct is 0");
    return UPPL_RC_SUCCESS;
  }
  if (child_index == -1 &&
      !str_logicalport_id.empty()) {
    // Check for key existence
    vector<string> vect_key_value;
    vect_key_value.push_back(str_controller_name);
    vect_key_value.push_back(str_domain_name);
    vect_key_value.push_back(str_logicalport_id);
    UpplReturnCode key_exist_status = IsKeyExists(
        (unc_keytype_datatype_t)data_type,
        vect_key_value);
    if (key_exist_status == UPPL_RC_SUCCESS) {
      log_port_exists = PFC_TRUE;
    }
  }
  void *val_struct = NULL;
  // Read the logical port values based on given key structure
  read_status = ReadBulkInternal(key_struct, val_struct, data_type,
                                 max_rep_ct, vect_val_logical_port_st,
                                 vect_logicalport_id);

  pfc_log_debug("Read status from logical_port is %d with result size %d",
                read_status, static_cast<int>(vect_logicalport_id.size()));
  if (log_port_exists == PFC_TRUE) {
    vect_logicalport_id.insert(vect_logicalport_id.begin(),
                               obj_key_logical_port);
    val_logical_port_st_t dummy_val_log_port;
    vect_val_logical_port_st.insert(vect_val_logical_port_st.begin(),
                                    dummy_val_log_port);
  }
  if (read_status == UPPL_RC_SUCCESS ||
      log_port_exists == PFC_TRUE) {
    vector<val_logical_port_st_t>::iterator vect_iter =
        vect_val_logical_port_st.begin();
    vector<key_logical_port_t> ::iterator logicalport_iter =
        vect_logicalport_id.begin();
    for (; logicalport_iter != vect_logicalport_id.end();
        ++logicalport_iter, ++vect_iter) {
      pfc_log_debug("Iterating value entries...");
      PhysicalCore *physical_core = PhysicalLayer::get_instance()->
          get_physical_core();
      InternalTransactionCoordinator *itc_trans  =
          physical_core->get_internal_transaction_coordinator();
      if (log_port_exists == PFC_FALSE) {
        pfc_log_debug(
            "Adding logical_port - '%s' to session",
            reinterpret_cast<char *>((*logicalport_iter).port_id));
        key_logical_port_t *key_buffer = new key_logical_port_t
            (*logicalport_iter);
        BulkReadBuffer obj_key_buffer = {
            UNC_KT_LOGICAL_PORT, IS_KEY,
            reinterpret_cast<void*>(key_buffer)
        };
        itc_trans->AddToBuffer(obj_key_buffer);
        val_logical_port_st_t *val_buffer = new val_logical_port_st_t
            (*vect_iter);
        BulkReadBuffer obj_value_buffer = {
            UNC_KT_LOGICAL_PORT, IS_STATE_VALUE,
            reinterpret_cast<void*>(val_buffer)
        };
        itc_trans->AddToBuffer(obj_value_buffer);
        BulkReadBuffer obj_sep_buffer = {
            UNC_KT_LOGICAL_PORT, IS_SEPARATOR, NULL
        };
        itc_trans->AddToBuffer(obj_sep_buffer);
        --max_rep_ct;
        if (max_rep_ct == 0) {
          pfc_log_debug("max_rep_ct reached zero, so returning");
          return UPPL_RC_SUCCESS;
        }
      }
      log_port_exists = PFC_FALSE;
      str_logicalport_id =
          reinterpret_cast<char *>((*logicalport_iter).port_id);
      int st_child_index =
          (child_index >= 0 && child_index <= KIdxLogicalMemberPort) \
          ? child_index+1 : KIdxLogicalMemberPort;
      for (int kIdx = st_child_index; kIdx <= KIdxLogicalMemberPort;
          ++kIdx) {
        pfc_log_debug("Iterating Logical port child %d", kIdx);
        void *child_key_struct = getChildKeyStruct(kIdx,
                                                   str_logicalport_id,
                                                   str_controller_name,
                                                   str_domain_name);
        child[kIdx] = GetChildClassPointer(
            (KtLogicalPortChildClass)kIdx);
        if (child[kIdx] == NULL) {
          // Free child key struct
          FreeChildKeyStruct(kIdx, child_key_struct);
          continue;
        }
        pfc_log_debug("Calling child %d read bulk", kIdx);
        UpplReturnCode ch_read_status = child[kIdx]->ReadBulk(
            child_key_struct,
            data_type,
            option1,
            option2,
            max_rep_ct,
            -1,
            true,
            is_read_next);
        pfc_log_debug("ReadBulk response from child %d is %d",
                      kIdx, ch_read_status);
        delete child[kIdx];
        child[kIdx] = NULL;
        FreeChildKeyStruct(kIdx, child_key_struct);
        if (max_rep_ct == 0) {
          pfc_log_debug("max_rep_ct reached zero, so returning");
          return UPPL_RC_SUCCESS;
        }
      }
      // reset child index
      child_index = -1;
    }
  }
  if (max_rep_ct > 0 && parent_call == false) {
    pfc_log_debug("max_rep_ct is %d and parent_call is %d, calling parent",
                  max_rep_ct, parent_call);
    // Filling key_struct corresponding to that key type
    Kt_Ctr_Domain nextKin;
    key_ctr_domain_t nextkin_key_struct;
    memcpy(reinterpret_cast<char*>(nextkin_key_struct.domain_name),
           str_domain_name.c_str(),
           str_domain_name.length()+1);
    memcpy(
        reinterpret_cast<char*>(nextkin_key_struct.ctr_key.controller_name),
        str_controller_name.c_str(),
        str_controller_name.length()+1);
    read_status = nextKin.ReadBulk(
        reinterpret_cast<void *>(&nextkin_key_struct),
        data_type,
        option1, option2, max_rep_ct,
        0,
        false,
        is_read_next);
    pfc_log_debug("read status from next kin Kt_Domain is %d",
                  read_status);
    return UPPL_RC_SUCCESS;
  }
  pfc_log_debug("logical port - reached end of table");
  pfc_log_debug("read_status=%d", read_status);
  if (read_status == UPPL_RC_ERR_NO_SUCH_INSTANCE) {
    read_status = UPPL_RC_SUCCESS;
  }
  return read_status;
}

/** ReadBulkInternal
 * * @Description : This function reads the max_rep_ct number of instances of
 * the UNC_KT_LOGICAL_PORT
 * * * @param[in] : key_struct, value_struct, data_type, max_rep_ct
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_LogicalPort::ReadBulkInternal(
    void* key_struct,
    void* val_struct,
    uint32_t data_type,
    uint32_t max_rep_ct,
    vector<val_logical_port_st_t> &vect_val_logical_port,
    vector<key_logical_port_t> &vect_logical_port_id) {
  if (max_rep_ct <= 0) {
    return UPPL_RC_SUCCESS;
  }
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode read_status = UPPL_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;
  DBTableSchema kt_logicalport_dbtableschema;
  // Populate DBSchema for logicalport_table
  void* old_value_struct;
  vector<ODBCMOperator> vect_key_operations;
  PopulateDBSchemaForKtTable(kt_logicalport_dbtableschema,
                             key_struct,
                             val_struct,
                             UNC_OP_READ_BULK, 0, 0,
                             vect_key_operations, old_value_struct);
  // Read rows from DB
  read_db_status = physical_layer->get_odbc_manager()-> \
      GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                  kt_logicalport_dbtableschema,
                  (unc_keytype_operation_t)UNC_OP_READ_BULK);
  if (read_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
    pfc_log_debug("No record found");
    read_status = UPPL_RC_ERR_NO_SUCH_INSTANCE;
    return read_status;
  } else if (read_db_status != ODBCM_RC_SUCCESS) {
    read_status = UPPL_RC_ERR_DB_GET;
    // log error to log daemon
    pfc_log_error("Read operation has failed with error %d", read_db_status);
    return read_status;
  }
  // From the values received from DB, populate val_logical_port structure
  FillLogicalPortValueStructure(kt_logicalport_dbtableschema,
                                vect_val_logical_port,
                                max_rep_ct,
                                UNC_OP_READ_BULK,
                                vect_logical_port_id);
  return read_status;
}

/** PerformSyntaxValidation
 * * @Description : This function performs syntax validation for
 *  UNC_KT_LOGICAL_PORT
 * * * @param[in] : key_struct, value_struct, operation and data_type
 * * * @return    : UPPL_RC_SUCCESS or UPPL_RC_ERR_*
 * */
UpplReturnCode Kt_LogicalPort::PerformSyntaxValidation(void* key_struct,
                                                       void* val_struct,
                                                       uint32_t operation,
                                                       uint32_t data_type) {
  pfc_log_info("Syntax Validation of UNC_KT_LOGICAL_PORT");
  UpplReturnCode ret_code = UPPL_RC_SUCCESS;
  pfc_ipcresp_t mandatory = PFC_TRUE;

  // Validate key structure
  key_logical_port *key = reinterpret_cast<key_logical_port_t*>(key_struct);
  string value =
      reinterpret_cast<char*>(key->domain_key.ctr_key.controller_name);
  IS_VALID_STRING_KEY(CTR_NAME, value, operation, ret_code, mandatory);
  if (ret_code != UPPL_RC_SUCCESS) {
    return UPPL_RC_ERR_CFG_SYNTAX;
  }

  value = reinterpret_cast<char*>(key->domain_key.domain_name);
  IS_VALID_STRING_KEY(DOMAIN_NAME, value, operation, ret_code, mandatory);
  if (ret_code != UPPL_RC_SUCCESS) {
    return UPPL_RC_ERR_CFG_SYNTAX;
  }

  value = reinterpret_cast<char*>(key->port_id);
  IS_VALID_STRING_KEY(LP_PORT_ID, value, operation, ret_code, mandatory);
  if (ret_code != UPPL_RC_SUCCESS) {
    return UPPL_RC_ERR_CFG_SYNTAX;
  }
  // Validate value structure
  if (val_struct != NULL) {
    unsigned int valid_val = 0;
    val_logical_port *val_lp =
        reinterpret_cast<val_logical_port_t*>(val_struct);

    // validate description
    valid_val = PhyUtil::uint8touint(val_lp->valid[kIdxLogicalPortDescription]);
    string value = reinterpret_cast<char*>(val_lp->description);
    IS_VALID_STRING_VALUE(LP_DESCRIPTION, value, operation,
                          valid_val, ret_code, mandatory);
    if (ret_code != UPPL_RC_SUCCESS) {
      return UPPL_RC_ERR_CFG_SYNTAX;
    }

    // Validate port_type
    valid_val = PhyUtil::uint8touint(val_lp->valid[kIdxLogicalPortType]);
    IS_VALID_INT_VALUE(LP_PORT_TYPE, val_lp->port_type, operation,
                       valid_val, ret_code, mandatory);
    if (ret_code != UPPL_RC_SUCCESS) {
      return UPPL_RC_ERR_CFG_SYNTAX;
    }

    // validate switch_id
    value = reinterpret_cast<char*>(val_lp->switch_id);
    valid_val = PhyUtil::uint8touint(val_lp->valid[kIdxLogicalPortSwitchId]);
    IS_VALID_STRING_VALUE(LP_SWITCH_ID, value, operation,
                          valid_val, ret_code, mandatory);
    if (ret_code != UPPL_RC_SUCCESS) {
      return UPPL_RC_ERR_CFG_SYNTAX;
    }

    // validate physical port id
    valid_val = PhyUtil::uint8touint(val_lp->
                                     valid[kIdxLogicalPortPhysicalPortId]);
    value = reinterpret_cast<char*>(val_lp->physical_port_id);
    IS_VALID_STRING_VALUE(LP_PHYSICAL_PORT_ID, value, operation,
                          valid_val, ret_code, mandatory);

    // Validate oper_down_criteria
    valid_val = PhyUtil::uint8touint(val_lp->
                                     valid[kIdxLogicalPortOperDownCriteria]);
    IS_VALID_INT_VALUE(LP_OPER_DOWN_CRITERIA, val_lp->oper_down_criteria,
                       operation, valid_val, ret_code, mandatory);
    if (ret_code != UPPL_RC_SUCCESS) {
      return UPPL_RC_ERR_CFG_SYNTAX;
    }
  }
  return ret_code;
}

/** PerformSemanticValidation
 * * @Description : This function performs semantic validation
 * for UNC_KT_LOGICAL_PORT
 * * * @param[in] : key_struct, value_struct, operation and data_type
 * * * @return    : UPPL_RC_SUCCESS or UPPL_RC_ERR_*
 * */
UpplReturnCode Kt_LogicalPort::PerformSemanticValidation(void* key_struct,
                                                         void* val_struct,
                                                         uint32_t operation,
                                                         uint32_t data_type) {
  pfc_log_info("Inside PerformSemanticValidation of KT_LOGICAL_PORT");
  UpplReturnCode status = UPPL_RC_SUCCESS;
  key_logical_port *obj_key =
      reinterpret_cast<key_logical_port_t*>(key_struct);

  string controller_name =
      (const char*)obj_key->domain_key.ctr_key.controller_name;
  string domain_name = (const char*)obj_key->domain_key.domain_name;
  string port_id = (const char*)obj_key->port_id;

  vector<string> sw_vect_key_value;
  sw_vect_key_value.push_back(controller_name);
  sw_vect_key_value.push_back(domain_name);
  sw_vect_key_value.push_back(port_id);

  UpplReturnCode KeyStatus = IsKeyExists(
      (unc_keytype_datatype_t)data_type,
      sw_vect_key_value);
  pfc_log_debug("IsKeyExists status %d", KeyStatus);

  if (operation == UNC_OP_CREATE) {
    if (KeyStatus == UPPL_RC_SUCCESS) {
      pfc_log_error("Key instance already exists");
      pfc_log_error("Hence create operation not allowed");
      status = UPPL_RC_ERR_INSTANCE_EXISTS;
    } else {
      pfc_log_info("key instance not exist create operation allowed");
    }
  } else if (operation == UNC_OP_UPDATE || operation == UNC_OP_DELETE ||
      operation == UNC_OP_READ) {
    // In case of update/delete/read operation, key should exist
    if (KeyStatus != UPPL_RC_SUCCESS) {
      pfc_log_error("Key instance does not exist");
      pfc_log_error("Hence update/delete/read operation not allowed");
      status = UPPL_RC_ERR_NO_SUCH_INSTANCE;
    } else {
      pfc_log_info(
          "key instance exist update/del/read operation allowed");
    }
  }
  if (operation == UNC_OP_CREATE && status == UPPL_RC_SUCCESS) {
    vector<string> parent_vect_key_value;
    parent_vect_key_value.push_back(controller_name);
    parent_vect_key_value.push_back(domain_name);
    Kt_Ctr_Domain KtObj;
    UpplReturnCode parent_key_status = KtObj.IsKeyExists(
        (unc_keytype_datatype_t)data_type, parent_vect_key_value);
    pfc_log_debug("Parent IsKeyExists status %d", parent_key_status);
    if (parent_key_status != UPPL_RC_SUCCESS) {
      status = UPPL_RC_ERR_PARENT_DOES_NOT_EXIST;
    }
  }
  pfc_log_debug("Return Code SemanticValidation: %d", status);
  return status;
}

/** HandleDriverAlarms
 * * @Description : This function processes the alarm notification
 * received from driver
 * * * @param[in] : alarm type, operation, key_struct and value_struct
 * * * @return    : Success or Failure
 * */
UpplReturnCode Kt_LogicalPort::HandleDriverAlarms(uint32_t data_type,
                                                  uint32_t alarm_type,
                                                  uint32_t oper_type,
                                                  void* key_struct,
                                                  void* val_struct) {
  UpplReturnCode status = UPPL_RC_SUCCESS;
  // Following alarms are sent for kt_logicalport
  if (alarm_type != UNC_SUBDOMAIN_SPLIT) {
    pfc_log_info("%d alarm received for logical port is ignored", alarm_type);
    return UPPL_RC_ERR_BAD_REQUEST;
  }
  pfc_log_info("UNC_SUBDOMAIN_SPLIT alarm sent by driver");
  pfc_log_info("operation type: %d", oper_type);
  uint8_t oper_status_db = 0;
  UpplReturnCode read_status = GetOperStatus(data_type,
                                             key_struct,
                                             oper_status_db);
  if (read_status != UPPL_RC_SUCCESS) {
    pfc_log_info("Unable to get current oper_status from db");
    return UPPL_RC_SUCCESS;
  }
  if (oper_status_db == UPPL_LOGICAL_PORT_OPER_UNKNOWN) {
    pfc_log_info("Logical Port oper is unknown, not processing alarm");
    return UPPL_RC_SUCCESS;
  }
  UpplLogicalPortOperStatus old_oper_status =
      (UpplLogicalPortOperStatus)oper_status_db;
  UpplLogicalPortOperStatus new_oper_status =
      UPPL_LOGICAL_PORT_OPER_UNKNOWN;
  if (oper_type == UNC_OP_CREATE) {
    new_oper_status = UPPL_LOGICAL_PORT_OPER_DOWN;
  } else {
    new_oper_status = UPPL_LOGICAL_PORT_OPER_UP;
  }
  pfc_log_info("Oper_status received from db: %d", old_oper_status);
  pfc_log_info("Oper_status to be set to  db: %d", new_oper_status);
  if (new_oper_status == old_oper_status) {
    pfc_log_debug("Old and new oper status are same, so do nothing");
    return UPPL_RC_SUCCESS;
  }
  if (new_oper_status == UPPL_LOGICAL_PORT_OPER_UP) {
    GetOperStatusFromOperDownCriteria(data_type,
                                      key_struct,
                                      val_struct,
                                      new_oper_status);
  }
  pfc_log_info("Oper_status to be set to  db: %d", new_oper_status);
  if (new_oper_status == old_oper_status) {
    pfc_log_debug("Old and new oper status are same, so do nothing");
    return UPPL_RC_SUCCESS;
  }
  // Set oper_status update in DB
  status = SetOperStatus(data_type, key_struct, NULL, new_oper_status);
  pfc_log_debug("Update oper_status return: %d", status);
  if (status == UPPL_RC_SUCCESS) {
    // Call NotifyOperStatus of referred key types
    status = NotifyOperStatus(data_type, key_struct, val_struct);
    pfc_log_debug("Notify oper_status return: %d", status);
  }
  return status;
}


/** HandleOperStatus
 * * @Description : This function performs the required actions when oper status
 * changes
 * * * @param[in] : Key and value struct
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_LogicalPort::HandleOperStatus(uint32_t data_type,
                                                void *key_struct,
                                                void *value_struct) {
  FN_START_TIME("HandleOperStatus", "LogicalPort");
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode return_code = UPPL_RC_SUCCESS;
  pfc_log_debug("LogicalPort Handle Oper Status Enter");

  if (key_struct == NULL) {
    pfc_log_debug("Key struct is not available in request");
    return_code = UPPL_RC_ERR_BAD_REQUEST;
    FN_END_TIME("HandleOperStatus", "LogicalPort");
    return return_code;
  }
  key_logical_port_t *obj_key_logical_port =
      reinterpret_cast<key_logical_port_t*>(key_struct);
  val_logical_port_st_t *obj_val_logical_port =
      reinterpret_cast<val_logical_port_st_t*>(value_struct);
  string controller_name =
      (const char*) obj_key_logical_port->domain_key.ctr_key.controller_name;
  string domain_name =
      (const char*) obj_key_logical_port->domain_key.domain_name;
  string port_id =
      (const char*) obj_key_logical_port->port_id;
  string switch_id;
  string phy_port_id;

  if (obj_val_logical_port != NULL) {
    int valid_val = PhyUtil::uint8touint(
        obj_val_logical_port->logical_port.valid[kIdxLogicalPortSwitchId]);
    pfc_log_debug("Valid of Switch Id: %d", valid_val);
    if (valid_val == UNC_VF_VALID) {
      switch_id =
          (const char*) obj_val_logical_port->logical_port.switch_id;
    }
    valid_val = PhyUtil::uint8touint(
        obj_val_logical_port->logical_port.
        valid[kIdxLogicalPortPhysicalPortId]);
    pfc_log_debug("Valid of Physical Port Id: %d", valid_val);
    if (valid_val == UNC_VF_VALID) {
      phy_port_id =
          (const char*) obj_val_logical_port->logical_port.physical_port_id;
    }
  }
  val_logical_port_st_t obj_val_logical_port_st;
  memset(obj_val_logical_port_st.logical_port.switch_id, '\0', 256);
  memset(obj_val_logical_port_st.logical_port.physical_port_id, '\0', 32);
  memset(obj_val_logical_port_st.logical_port.valid, UNC_VF_INVALID, 5);
  memset(obj_val_logical_port_st.valid, UNC_VF_INVALID, 2);
  stringstream ss;
  ss << "Handle operstatus ctr_name:" << controller_name
      << "\tdomain_name:" << domain_name
      << "\tlogical_port_id:" << port_id
      << "\tswitch_id:" << switch_id
      << "\tphy_port_id:" << phy_port_id;
  string strName = ss.str();
  pfc_log_debug("Handle operstatus ip vals:%s", strName.c_str());
  vector<key_logical_port_t> vectLogicalPortKey;
  // Get the controller's oper status and decide on the oper_status
  key_ctr_t ctr_key;
  memcpy(ctr_key.controller_name, controller_name.c_str(),
         (controller_name.length())+1);
  uint8_t ctrl_oper_status = 0;
  UpplLogicalPortOperStatus logical_port_oper_status =
      UPPL_LOGICAL_PORT_OPER_UNKNOWN;
  Kt_Controller controller;
  UpplReturnCode read_status = controller.GetOperStatus(
      data_type, reinterpret_cast<void*>(&ctr_key), ctrl_oper_status);
  if (read_status != UPPL_RC_SUCCESS) {
    pfc_log_info("Controller's oper_status read returned failure");
    FN_END_TIME("HandleOperStatus", "LogicalPort");
    return return_code;
  }
  if (ctrl_oper_status ==
      (UpplControllerOperStatus) UPPL_CONTROLLER_OPER_DOWN) {
    // set oper status to unknown
    logical_port_oper_status = UPPL_LOGICAL_PORT_OPER_UNKNOWN;
    return_code = SetOperStatus(
        data_type,
        key_struct,
        reinterpret_cast<void*>(&obj_val_logical_port_st),
        logical_port_oper_status);
    FN_END_TIME("HandleOperStatus", "LogicalPort");
    return return_code;
  }
  pfc_log_info("Controller's oper_status %d", ctrl_oper_status);
  if (ctrl_oper_status !=
      (UpplControllerOperStatus) UPPL_CONTROLLER_OPER_UP) {
    pfc_log_info("Controller oper status is not up, returning");
    FN_END_TIME("HandleOperStatus", "LogicalPort");
    return return_code;
  }
  pfc_log_info("Set logical port oper status as up");
  logical_port_oper_status = UPPL_LOGICAL_PORT_OPER_UP;
  // get switch oper status:
  key_switch_t switch_key;
  memcpy(switch_key.ctr_key.controller_name, controller_name.c_str(),
         (controller_name.length())+1);
  if (!switch_id.empty()) {
    pfc_log_debug("Sending switch key");
    memcpy(switch_key.switch_id, switch_id.c_str(),
           (switch_id.length())+1);
  } else {
    memset(switch_key.switch_id, 0, 256);
  }
  uint8_t switch_oper_status = 0;
  logical_port_oper_status =
      UPPL_LOGICAL_PORT_OPER_UNKNOWN;
  Kt_Switch switch_obj;
  read_status = switch_obj.GetOperStatus(
      data_type, reinterpret_cast<void*>(&switch_key), switch_oper_status);
  if (read_status == UPPL_RC_SUCCESS) {
    pfc_log_info("switch oper_status %d", switch_oper_status);
    if (switch_oper_status ==
        (UpplSwitchOperStatus) UPPL_SWITCH_OPER_UP) {
      pfc_log_debug("Set logical port oper status as up");
      logical_port_oper_status = UPPL_LOGICAL_PORT_OPER_UP;
      if (value_struct != NULL) {
        obj_val_logical_port_st =
            *(reinterpret_cast<val_logical_port_st_t*>(value_struct));
      }
    } else if (switch_oper_status ==
        (UpplSwitchOperStatus) UPPL_SWITCH_OPER_DOWN) {
      logical_port_oper_status = UPPL_LOGICAL_PORT_OPER_DOWN;
      if (value_struct != NULL) {
        obj_val_logical_port_st =
            *(reinterpret_cast<val_logical_port_st_t*>(value_struct));
      }
    }
  } else {
    pfc_log_error("switch oper_status read returned failure");
  }

  // check oper down criteria
  pfc_log_debug("iterate all logical port in handle oper status");
  port_id.clear();
  // Get all logical ports associated with controller and operdowncriteria
  while (true) {
    DBTableSchema kt_ctr_domain_dbtableschema;
    vector<string> vect_prim_keys;
    vect_prim_keys.push_back(CTR_NAME);
    vect_prim_keys.push_back(DOMAIN_NAME);

    vector<TableAttrSchema> vect_table_attr_schema;
    list< vector<TableAttrSchema> > row_list;
    PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                          controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
    PhyUtil::FillDbSchema(DOMAIN_NAME, domain_name,
                          domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
    if (!switch_id.empty()) {
      vect_prim_keys.push_back(LP_SWITCH_ID);
      PhyUtil::FillDbSchema(LP_SWITCH_ID, switch_id,
                            switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                            vect_table_attr_schema);
    }

    if (!phy_port_id.empty()) {
      vect_prim_keys.push_back(LP_PHYSICAL_PORT_ID);
      PhyUtil::FillDbSchema(LP_PHYSICAL_PORT_ID, phy_port_id,
                            phy_port_id.length(), DATATYPE_UINT8_ARRAY_32,
                            vect_table_attr_schema);
    }

    vect_prim_keys.push_back(LP_PORT_ID);
    PhyUtil::FillDbSchema(LP_PORT_ID, port_id,
                          port_id.length(), DATATYPE_UINT8_ARRAY_320,
                          vect_table_attr_schema);

    kt_ctr_domain_dbtableschema.set_table_name(UPPL_LOGICALPORT_TABLE);
    kt_ctr_domain_dbtableschema.set_primary_keys(vect_prim_keys);
    row_list.push_back(vect_table_attr_schema);
    kt_ctr_domain_dbtableschema.set_row_list(row_list);
    ODBCM_RC_STATUS db_status = physical_layer->get_odbc_manager()->
        GetBulkRows((unc_keytype_datatype_t)data_type, UPPL_MAX_REP_CT,
                    kt_ctr_domain_dbtableschema,
                    (unc_keytype_operation_t)UNC_OP_READ_SIBLING_BEGIN);
    if (db_status != ODBCM_RC_SUCCESS) {
      // No other logical port available
      pfc_log_debug("No other logical port available - HandleOperStatus End");
      break;
    }
    list<vector<TableAttrSchema> > ::iterator iter_list;
    for (iter_list = kt_ctr_domain_dbtableschema.row_list_.begin();
        iter_list != kt_ctr_domain_dbtableschema.row_list_.end();
        ++iter_list) {
      vector<TableAttrSchema> attributes_vector = *iter_list;
      vector<TableAttrSchema> :: iterator iter_vector;
      key_logical_port_t logical_port;
      for (iter_vector = attributes_vector.begin();
          iter_vector != attributes_vector.end();
          ++iter_vector) {
        /* Get attribute name of a row */
        TableAttrSchema tab_att_schema = (*iter_vector);

        if (tab_att_schema.table_attribute_name == CTR_NAME) {
          PhyUtil::GetValueFromDbSchema(tab_att_schema,
                                        controller_name,
                                        DATATYPE_UINT8_ARRAY_32);
          memcpy(logical_port.domain_key.ctr_key.controller_name,
                 controller_name.c_str(),
                 controller_name.length()+1);
        }
        if (tab_att_schema.table_attribute_name ==
            DOMAIN_NAME) {
          PhyUtil::GetValueFromDbSchema(tab_att_schema,
                                        domain_name,
                                        DATATYPE_UINT8_ARRAY_32);
          memcpy(logical_port.domain_key.domain_name,
                 domain_name.c_str(),
                 domain_name.length()+1);
        }
        if (tab_att_schema.table_attribute_name == LP_PORT_ID) {
          PhyUtil::GetValueFromDbSchema(tab_att_schema, port_id,
                                        DATATYPE_UINT8_ARRAY_320);
          memcpy(logical_port.port_id, port_id.c_str(),
                 port_id.length()+1);
        }
      }
      vectLogicalPortKey.push_back(logical_port);
    }
  }
  vector<key_logical_port_t>::iterator keyItr =
      vectLogicalPortKey.begin();
  for (; keyItr != vectLogicalPortKey.end(); ++keyItr) {
    // key_logical_port_t objKeyLogicalPort;
    // handle operstatus criteria
    string port_id = (const char*)((*keyItr).port_id);
    pfc_log_debug("Port_Id: %s", port_id.c_str());
    unsigned trunk_port = port_id.find("TP-");
    unsigned sd_port = port_id.find("SD-");
    if (trunk_port == 0 || sd_port == 0) {
      pfc_log_debug("Call oper down criteria function");
      return_code = GetOperStatusFromOperDownCriteria(
          data_type,
          reinterpret_cast<void *> (&(*keyItr)),
          value_struct,
          logical_port_oper_status);
      if (return_code != UPPL_RC_SUCCESS) {
        continue;
      }
      pfc_log_debug("Handle logical port oper status val: %d",
                    logical_port_oper_status);
    }
    pfc_log_debug("Set logical port oper status: %d",
                  logical_port_oper_status);
    return_code = SetOperStatus(data_type,
                                reinterpret_cast<void *> (&(*keyItr)), NULL,
                                logical_port_oper_status);
    pfc_log_debug("Set logical port oper retrun code: %d",
                  return_code);
  }
  // notify operstatus using logical port siblings as key
  if (!vectLogicalPortKey.empty()) {
    vector<key_logical_port_t>::iterator keyItr =
        vectLogicalPortKey.begin();
    for (; keyItr != vectLogicalPortKey.end(); ++keyItr) {
      pfc_log_debug("Set logical port oper stat ret code: %d", return_code);
      if (return_code == UPPL_RC_SUCCESS) {
        // Call referred classes' notify oper_status functions
        return_code = NotifyOperStatus(
            data_type, reinterpret_cast<void *> (&(*keyItr)), NULL);
        pfc_log_debug("logicalport Notify Operstatus ret: %d", return_code);
      } else {
        pfc_log_error("operstatus update failure");
      }
    }
  } else {
    pfc_log_info("Logicalport sibling count is 0");
  }
  pfc_log_debug("LogicalPort Handle Oper Status End");
  FN_END_TIME("HandleOperStatus", "LogicalPort");
  return return_code;
}

/** GetOperStatusFromOperDownCriteria
 * * @Description : This function retrieves the oper status of port based on
 * operdown criteria
 * * * @param[in] : Key and value struct
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_LogicalPort::GetOperStatusFromOperDownCriteria(
    uint32_t data_type,
    void *key_struct,
    void *value_struct,
    UpplLogicalPortOperStatus &logical_port_oper_status  ) {
  uint32_t oper_down_criteria;
  vector<uint32_t> vectOperStatus;
  // get port oper status,PFC_FALSE as call is not from deletekeyinstance
  pfc_log_debug("Enter GetOperStatusFromOperDownCriteria");
  HandleOperDownCriteriaFromPortStatus(data_type, key_struct, value_struct,
                                       vectOperStatus, PFC_FALSE);
  // get oper down from DB
  UpplReturnCode return_code = UPPL_RC_SUCCESS;
  return_code = GetOperDownCriteria(data_type, key_struct, oper_down_criteria);
  if (return_code != UPPL_RC_SUCCESS) {
    pfc_log_info("Logicalport OperStatusFromOperDownCriteria read error");
    return return_code;
  }
  pfc_log_debug("Logical port OperStatusFromOperDownCriteria criteria: %d",
                oper_down_criteria);
  // oper_down_criteria is ALL
  if (oper_down_criteria == (UpplLogicalPortOperDownCriteria)
      UPPL_OPER_DOWN_CRITERIA_ALL) {
    vector<uint32_t>::iterator operstatus_itr = vectOperStatus.begin();
    // iterate on the port oper status values
    for (; operstatus_itr != vectOperStatus.end(); ++operstatus_itr) {
      // if any instance oper status of port is down or unknown
      // then set status to down else oper status to up
      if ((*operstatus_itr) == 0 || (*operstatus_itr) == 2) {
        logical_port_oper_status = UPPL_LOGICAL_PORT_OPER_DOWN;
        break;
      } else if ((*operstatus_itr) == 1) {
        logical_port_oper_status = UPPL_LOGICAL_PORT_OPER_UP;
      }
    }
    pfc_log_debug("Logical port OperStatusFromOperDownCriteria status: %d",
                  logical_port_oper_status);
  } else if (oper_down_criteria == (UpplLogicalPortOperDownCriteria)
      UPPL_OPER_DOWN_CRITERIA_ANY) {
    // oper_down_criteria is ANY
    vector<uint32_t>::iterator operstatus_itr = vectOperStatus.begin();
    // iterate on the port oper status values
    for (; operstatus_itr != vectOperStatus.end(); ++operstatus_itr) {
      // if any instance oper status of port is up
      // then set status to up else oper status to down
      if ((*operstatus_itr) == 1) {
        logical_port_oper_status = UPPL_LOGICAL_PORT_OPER_UP;
        break;
      } else if ((*operstatus_itr) == 0 || (*operstatus_itr) == 2) {
        logical_port_oper_status = UPPL_LOGICAL_PORT_OPER_DOWN;
      }
    }
    pfc_log_debug("Logicalport OperStatusFromOperDownCriteria Any status:%d",
                  logical_port_oper_status);
  } else {
    return_code = UPPL_RC_ERR_BAD_REQUEST;
  }
  pfc_log_debug("Exit logical port OperStatusFromOperDownCriteria");
  return return_code;
}

/** HandleOperDownCriteriaFromPortStatus
 * * @Description : This function retrieves the oper status of port based on
 * logical member port key values
 * * * @param[in] : Key and value struct
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_LogicalPort::HandleOperDownCriteriaFromPortStatus(
    uint32_t data_type,
    void *key_struct,
    void *value_struct,
    vector<uint32_t> &vectOperStatus,
    pfc_bool_t is_delete_call) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode return_code = UPPL_RC_SUCCESS;

  pfc_log_debug("Entering Logical port Handle OperDownCriteriaFromPortStatus");
  if (key_struct == NULL) {
    return_code = UPPL_RC_ERR_BAD_REQUEST;
    return return_code;
  }
  key_logical_port_t *obj_key_logical_port =
      reinterpret_cast<key_logical_port_t*>(key_struct);
  string controller_name =
      (const char*) obj_key_logical_port->domain_key.ctr_key.controller_name;
  string domain_name =
      (const char*) obj_key_logical_port->domain_key.domain_name;
  string port_id =
      (const char*) obj_key_logical_port->port_id;
  stringstream ss;
  ss << "Handle operstatus ctr_name:" << controller_name
      << "\tdomain_name:" << domain_name
      << "\tlogical_port_id:" << port_id;
  string strName = ss.str();
  pfc_log_debug("Handle operdowncriteria ip vals: %s", strName.c_str());
  // Call referred classes' notify oper_status functions
  // Get all domains associated with controller
  port_id.clear();
  while (true) {
    DBTableSchema kt_logicalport_dbtableschema;
    vector<string> vect_prim_keys;
    vector<TableAttrSchema> vect_table_attr_schema;
    list< vector<TableAttrSchema> > row_list;
    vect_prim_keys.push_back(CTR_NAME);
    vect_prim_keys.push_back(DOMAIN_NAME);
    PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                          controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
    PhyUtil::FillDbSchema(DOMAIN_NAME, domain_name,
                          domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
    // if (!port_id.empty()) {
    vect_prim_keys.push_back(LP_PORT_ID);
    // }
    PhyUtil::FillDbSchema(LP_PORT_ID, port_id,
                          port_id.length(), DATATYPE_UINT8_ARRAY_320,
                          vect_table_attr_schema);
    vect_prim_keys.push_back(LP_SWITCH_ID);
    string switch_id;
    PhyUtil::FillDbSchema(LP_SWITCH_ID, switch_id,
                          switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                          vect_table_attr_schema);
    string physical_port_id;
    PhyUtil::FillDbSchema(LP_PHYSICAL_PORT_ID, physical_port_id,
                          physical_port_id.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);

    kt_logicalport_dbtableschema.set_table_name("logical_member_port_table");
    kt_logicalport_dbtableschema.set_primary_keys(vect_prim_keys);

    row_list.push_back(vect_table_attr_schema);
    kt_logicalport_dbtableschema.set_row_list(row_list);

    pfc_log_debug("Iterate to get Switch and Port details");

    // Get number of instances based on member_ports_count
    ODBCM_RC_STATUS db_status = physical_layer->get_odbc_manager()->
        GetBulkRows((unc_keytype_datatype_t)data_type,
                    UPPL_MAX_REP_CT,
                    kt_logicalport_dbtableschema,
                    (unc_keytype_operation_t)UNC_OP_READ_SIBLING_BEGIN);
    if (db_status != ODBCM_RC_SUCCESS) {
      pfc_log_info("ReadBulk failure in HandleOperDownCriteria");
      break;
    }
    // To traverse the list
    list<vector<TableAttrSchema> > ::iterator iter_list;
    for (iter_list = kt_logicalport_dbtableschema.row_list_.begin();
        iter_list != kt_logicalport_dbtableschema.row_list_.end();
        ++iter_list) {
      key_logical_member_port_t logical_memeber_port;

      vector<TableAttrSchema> attributes_vector = *iter_list;
      vector<TableAttrSchema> :: iterator iter_vector;
      for (iter_vector = attributes_vector.begin();
          iter_vector != attributes_vector.end();
          ++iter_vector) {
        /* Get attribute name of a row */
        TableAttrSchema tab_att_schema = (*iter_vector);
        if (tab_att_schema.table_attribute_name == CTR_NAME) {
          PhyUtil::GetValueFromDbSchema(tab_att_schema, controller_name,
                                        DATATYPE_UINT8_ARRAY_32);
          memcpy(logical_memeber_port.logical_port_key.domain_key.ctr_key.
                 controller_name, controller_name.c_str(),
                 controller_name.length()+1);
        } else if (tab_att_schema.table_attribute_name == LP_SWITCH_ID) {
          string switch_id;
          PhyUtil::GetValueFromDbSchema(tab_att_schema, switch_id,
                                        DATATYPE_UINT8_ARRAY_256);
          // key_logical_port_t logical_port;
          memcpy(logical_memeber_port.switch_id,
                 switch_id.c_str(),
                 switch_id.length()+1);
        } else if (tab_att_schema.table_attribute_name ==
            LP_PHYSICAL_PORT_ID) {
          string physical_port_id;
          PhyUtil::GetValueFromDbSchema(tab_att_schema, physical_port_id,
                                        DATATYPE_UINT8_ARRAY_32);
          memcpy(logical_memeber_port.physical_port_id,
                 physical_port_id.c_str(),
                 physical_port_id.length()+1);
        } else if (tab_att_schema.table_attribute_name == LP_PORT_ID) {
          PhyUtil::GetValueFromDbSchema(tab_att_schema, port_id,
                                        DATATYPE_UINT8_ARRAY_320);
        }
      }

      UpplReturnCode read_status = UPPL_RC_SUCCESS;
      Kt_Port port;
      key_port_t obj_key_port;
      string port_controller_name = (const char*) logical_memeber_port.
          logical_port_key.domain_key.ctr_key.controller_name;
      memcpy(obj_key_port.sw_key.ctr_key.controller_name,
             port_controller_name.c_str(),
             (port_controller_name.length())+1);

      string port_switch_id = (const char*) logical_memeber_port.switch_id;
      memcpy(obj_key_port.sw_key.switch_id,
             port_switch_id.c_str(),
             (port_switch_id.length())+1);

      string phy_port_id =
          (const char*) logical_memeber_port.physical_port_id;
      memcpy(obj_key_port.port_id,
             phy_port_id.c_str(),
             (phy_port_id.length())+1);

      // call port function to set logicalport id valid status as invalid
      if (is_delete_call == PFC_TRUE) {
        val_port_st_t obj_val_port;

        // set obj_val_port.port valid as invalid
        memset(obj_val_port.port.valid, UNC_VF_INVALID, 4);
        // set obj_val_port valid as invalid
        memset(obj_val_port.valid, UNC_VF_INVALID, 7);

        string logical_port_id = (const char*)obj_key_logical_port->port_id;
        memcpy(obj_val_port.logical_port_id,
               logical_port_id.c_str(),
               (logical_port_id.length())+1);
        obj_val_port.valid[kIdxPortLogicalPortId] = UNC_VF_VALID;

        read_status = port.UpdatePortValidFlag(
            reinterpret_cast<void*>(&obj_key_port),
            reinterpret_cast<void*>(&obj_val_port),
            obj_val_port,
            UNC_VF_INVALID);
      } else {
        // get the port oper_status
        uint8_t port_oper_status = 0;

        pfc_log_debug("Logicalport get CriteriaFromPortStatus");
        read_status = port.GetOperStatus(
            data_type, reinterpret_cast<void*>(&obj_key_port),
            port_oper_status);
        if (read_status == UPPL_RC_SUCCESS) {
          pfc_log_debug("port oper_status in logport: %d", port_oper_status);
          vectOperStatus.push_back(port_oper_status);
        } else {
          pfc_log_info("port oper_status read returned failure");
        }
        return_code = read_status;
      }
    }
  }
  pfc_log_debug("Exit Logicalport Handle OperDownCriteriaFromPortStatus");
  return return_code;
}

/** GetOperDownCriteria
 * * @Description : This function retrieves the oper down criteria from db
 * * * @param[in] : Key and oper_status
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_LogicalPort::GetOperDownCriteria(
    uint32_t data_type,
    void* key_struct,
    uint32_t &oper_down_criteria) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  key_logical_port_t *obj_key_logical_port =
      reinterpret_cast<key_logical_port_t*>(key_struct);
  TableAttrSchema kt_logical_port_table_attr_schema;
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  vector<string> vect_prim_keys;

  string controller_name = (const char*)obj_key_logical_port->domain_key.
      ctr_key.controller_name;

  if (!controller_name.empty()) {
    vect_prim_keys.push_back(CTR_NAME);
  }

  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  string domain_name = (const char*)obj_key_logical_port->
      domain_key.domain_name;

  if (!domain_name.empty()) {
    vect_prim_keys.push_back(DOMAIN_NAME);
  }

  PhyUtil::FillDbSchema(DOMAIN_NAME, domain_name,
                        domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  string port_id = (const char*)obj_key_logical_port->port_id;

  if (!port_id.empty()) {
    vect_prim_keys.push_back(LP_PORT_ID);
  }

  PhyUtil::FillDbSchema(LP_PORT_ID, port_id,
                        port_id.length(), DATATYPE_UINT8_ARRAY_320,
                        vect_table_attr_schema);

  string oper_down_criteria_value;
  PhyUtil::FillDbSchema(LP_OPER_DOWN_CRITERIA, oper_down_criteria_value,
                        oper_down_criteria_value.length(),
                        DATATYPE_UINT16,
                        vect_table_attr_schema);

  DBTableSchema kt_logicalport_dbtableschema;
  kt_logicalport_dbtableschema.set_table_name(UPPL_LOGICALPORT_TABLE);
  kt_logicalport_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_logicalport_dbtableschema.set_row_list(row_list);

  // Call ODBCManager and read
  ODBCM_RC_STATUS read_db_status =
      physical_layer->get_odbc_manager()->GetOneRow(
          (unc_keytype_datatype_t)data_type,
          kt_logicalport_dbtableschema);
  if (read_db_status != ODBCM_RC_SUCCESS) {
    // log error
    pfc_log_error("oper down criteria read operation failed");
    return UPPL_RC_ERR_DB_GET;
  }

  // read the oper_down_criteria value
  list < vector<TableAttrSchema> > res_logicalport_row_list =
      kt_logicalport_dbtableschema.get_row_list();
  list < vector<TableAttrSchema> >::iterator res_logicalport_iter =
      res_logicalport_row_list.begin();
  // populate IPC value structure based on the response recevied from DB
  for (; res_logicalport_iter!= res_logicalport_row_list.end();
      ++res_logicalport_iter) {
    vector<TableAttrSchema> res_logicalport_table_attr_schema =
        (*res_logicalport_iter);
    vector<TableAttrSchema>:: iterator vect_logicalport_iter =
        res_logicalport_table_attr_schema.begin();
    for (; vect_logicalport_iter != res_logicalport_table_attr_schema.end();
        ++vect_logicalport_iter) {
      // populate values from logicalport_table
      TableAttrSchema tab_schema = (*vect_logicalport_iter);
      string attr_name = tab_schema.table_attribute_name;
      string attr_value;
      if (attr_name == LP_OPER_DOWN_CRITERIA) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT16);
        oper_down_criteria = atoi(attr_value.c_str());
        pfc_log_debug("oper_down_criteria: %d", oper_down_criteria);
        break;
      }
    }
  }
  return UPPL_RC_SUCCESS;
}

/** IsLogicalPortReferred
 *  * @Description : This function returns true if given controller_name,
 *  domain and port_id are referred in logical port table
 *  * @param[in] : controller_name, domain_name, port_id
 *  * @return    : PFC_TRUE/PFC_FALSE
 */
pfc_bool_t Kt_LogicalPort::IsLogicalPortReferred(
    string controller_name,
    string domain_name) {
  pfc_bool_t is_ref = PFC_FALSE;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  DBTableSchema kt_dbtableschema;
  vector<TableAttrSchema> vect_table_attr_schema;
  TableAttrSchema kt_ctr_domain_table_attr_schema;
  list< vector<TableAttrSchema> > row_list;
  vector<string> vect_prim_keys;

  // Check whether controller_name1 is reffered
  vect_prim_keys.push_back(CTR_NAME);
  vect_prim_keys.push_back(DOMAIN_NAME);

  //  controller_name
  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(),
                        DATATYPE_UINT8_ARRAY_32, vect_table_attr_schema);
  PhyUtil::FillDbSchema(DOMAIN_NAME, domain_name,
                        domain_name.length(),
                        DATATYPE_UINT8_ARRAY_32, vect_table_attr_schema);
  kt_dbtableschema.set_table_name("logical_port_table");
  kt_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_dbtableschema.set_row_list(row_list);

  //  Send request to ODBC for logical_member_port_table
  ODBCM_RC_STATUS read_db_status = physical_layer->
      get_odbc_manager()->IsRowExists(UNC_DT_STATE,
                                      kt_dbtableschema);
  if (read_db_status == ODBCM_RC_ROW_EXISTS) {
    //  read count
    is_ref = PFC_TRUE;
  }
  return is_ref;
}

/** InvokeBoundaryNotifyOperStatus
 * * @Description : This function invokes the notifyoperstatus of boundary
 * * * @param[in] : Key and value struct
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_LogicalPort::InvokeBoundaryNotifyOperStatus(
    uint32_t data_type,
    void *key_struct) {
  UpplReturnCode return_code = UPPL_RC_SUCCESS;
  key_logical_port_t *obj_key_logical_port =
      reinterpret_cast<key_logical_port_t*>(key_struct);

  string controller_name =
      (const char*)obj_key_logical_port->domain_key.ctr_key.controller_name;
  string domain_name =
      (const char*)obj_key_logical_port->domain_key.domain_name;
  string port_id =
      (const char*)obj_key_logical_port->port_id;
  // Notify UNC_KT_BOUNDARY
  Kt_Boundary boundary;
  val_boundary_t obj_val_boundary1;
  memset(&obj_val_boundary1, 0, sizeof(val_boundary_t));
  memset(obj_val_boundary1.valid, 0, 7);
  memcpy(obj_val_boundary1.controller_name1,
         controller_name.c_str(), controller_name.length()+1);
  obj_val_boundary1.valid[kIdxBoundaryControllerName1] = UNC_VF_VALID;
  memcpy(obj_val_boundary1.domain_name1,
         domain_name.c_str(), domain_name.length()+1);
  obj_val_boundary1.valid[kIdxBoundaryDomainName1] = UNC_VF_VALID;
  memcpy(obj_val_boundary1.logical_port_id1,
         port_id.c_str(), port_id.length()+1);
  obj_val_boundary1.valid[kIdxBoundaryLogicalPortId1] = UNC_VF_VALID;
  memset(obj_val_boundary1.controller_name2, 0, 32);
  memset(obj_val_boundary1.domain_name2, 0, 32);
  memset(obj_val_boundary1.logical_port_id2, 0, 320);
  return_code = boundary.HandleOperStatus(
      data_type, NULL, reinterpret_cast<void *> (&obj_val_boundary1));
  pfc_log_debug("Invoke Notify for boundary class %d", return_code);

  // Boundary takes care of filling name2 values
  return return_code;
}

/** NotifyOperStatus
 * * @Description : This function performs the notifies other associated
 * key types when oper status changes
 * * * @param[in] : Key and value struct
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_LogicalPort::NotifyOperStatus(uint32_t data_type,
                                                void *key_struct,
                                                void *value_struct) {
  UpplReturnCode return_code = UPPL_RC_SUCCESS;
  key_logical_port_t *obj_key_logical_port =
      reinterpret_cast<key_logical_port_t*>(key_struct);

  string controller_name =
      (const char*)obj_key_logical_port->domain_key.ctr_key.controller_name;
  string domain_name =
      (const char*)obj_key_logical_port->domain_key.domain_name;
  string port_id =
      (const char*)obj_key_logical_port->port_id;

  // Notify UNC_KT_BOUNDARY
  Kt_Boundary boundary;
  val_boundary_t obj_val_boundary1;
  memset(&obj_val_boundary1, 0, sizeof(val_boundary_t));
  memcpy(obj_val_boundary1.controller_name1,
         controller_name.c_str(),
         (controller_name.length())+1);
  memcpy(obj_val_boundary1.domain_name1,
         domain_name.c_str(),
         (domain_name.length())+1);
  memcpy(obj_val_boundary1.logical_port_id1,
         port_id.c_str(), port_id.length()+1);
  memset(obj_val_boundary1.controller_name2, '\0', 32);
  memset(obj_val_boundary1.domain_name2, '\0', 32);
  memset(obj_val_boundary1.logical_port_id2, '\0', 320);
  obj_val_boundary1.valid[kIdxBoundaryLogicalPortId1] = UNC_VF_VALID;
  obj_val_boundary1.valid[kIdxBoundaryControllerName1] = UNC_VF_VALID;
  obj_val_boundary1.valid[kIdxBoundaryDomainName1] = UNC_VF_VALID;
  return_code = boundary.HandleOperStatus(
      data_type, NULL, reinterpret_cast<void *> (&obj_val_boundary1));
  pfc_log_info("HandleOperStatus C1D1L1 from boundary returned %d",
               return_code);

  val_boundary_t obj_val_boundary2;
  memset(obj_val_boundary2.controller_name1, '\0', 32);
  memset(obj_val_boundary2.domain_name1, '\0', 32);
  memset(obj_val_boundary2.logical_port_id1, '\0', 320);
  memcpy(obj_val_boundary2.controller_name2,
         controller_name.c_str(),
         (controller_name.length())+1);
  memcpy(obj_val_boundary2.domain_name2,
         domain_name.c_str(),
         (domain_name.length())+1);
  memcpy(obj_val_boundary2.logical_port_id2,
         port_id.c_str(), port_id.length()+1);
  obj_val_boundary2.valid[kIdxBoundaryLogicalPortId2] = UNC_VF_VALID;
  obj_val_boundary2.valid[kIdxBoundaryControllerName2] = UNC_VF_VALID;
  obj_val_boundary2.valid[kIdxBoundaryDomainName2] = UNC_VF_VALID;
  return_code = boundary.HandleOperStatus(
      data_type, NULL, reinterpret_cast<void *> (&obj_val_boundary2));
  pfc_log_info("HandleOperStatus C2D2L2 from boundary returned %d",
               return_code);

  return return_code;
}

/** GetOperStatus
 * * @Description : This function updates the oper status in db
 * * * @param[in] : Key and oper_status
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_LogicalPort::GetOperStatus(uint32_t data_type,
                                             void* key_struct,
                                             uint8_t &oper_status) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  key_logical_port *obj_key_logical_port =
      reinterpret_cast<key_logical_port_t*>(key_struct);
  TableAttrSchema kt_logical_port_table_attr_schema;
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  vector<string> vect_prim_keys;
  string controller_name = (const char*)obj_key_logical_port->domain_key.
      ctr_key.controller_name;
  string domain_name = (const char*)obj_key_logical_port->
      domain_key.domain_name;
  string port_id = (const char*)obj_key_logical_port->port_id;
  if (!controller_name.empty()) {
    vect_prim_keys.push_back(CTR_NAME);
  }
  if (!domain_name.empty()) {
    vect_prim_keys.push_back(DOMAIN_NAME);
  }
  if (!port_id.empty()) {
    vect_prim_keys.push_back(LP_PORT_ID);
  }

  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  PhyUtil::FillDbSchema(DOMAIN_NAME, domain_name,
                        domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);


  PhyUtil::FillDbSchema(LP_PORT_ID, port_id,
                        port_id.length(), DATATYPE_UINT8_ARRAY_320,
                        vect_table_attr_schema);

  string oper_value;
  PhyUtil::FillDbSchema(LP_OPER_STATUS, oper_value,
                        oper_value.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);

  DBTableSchema kt_logicalport_dbtableschema;
  kt_logicalport_dbtableschema.set_table_name(UPPL_LOGICALPORT_TABLE);
  kt_logicalport_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_logicalport_dbtableschema.set_row_list(row_list);

  // Call ODBCManager and update
  ODBCM_RC_STATUS read_db_status =
      physical_layer->get_odbc_manager()->GetOneRow(
          (unc_keytype_datatype_t)data_type,
          kt_logicalport_dbtableschema);
  if (read_db_status != ODBCM_RC_SUCCESS) {
    // log error
    pfc_log_error("oper_status read operation failed");
    return UPPL_RC_ERR_DB_GET;
  }

  // read the oper_status value
  list < vector<TableAttrSchema> > res_logicalport_row_list =
      kt_logicalport_dbtableschema.get_row_list();
  list < vector<TableAttrSchema> >::iterator res_logicalport_iter =
      res_logicalport_row_list.begin();
  // populate IPC value structure based on the response received from DB
  for (; res_logicalport_iter!= res_logicalport_row_list.end();
      ++res_logicalport_iter) {
    vector<TableAttrSchema> res_logicalport_table_attr_schema =
        (*res_logicalport_iter);
    vector<TableAttrSchema>:: iterator vect_logicalport_iter =
        res_logicalport_table_attr_schema.begin();
    for (; vect_logicalport_iter != res_logicalport_table_attr_schema.end();
        ++vect_logicalport_iter) {
      // populate values from logicalport_table
      TableAttrSchema tab_schema = (*vect_logicalport_iter);
      string attr_name = tab_schema.table_attribute_name;
      string attr_value;
      if (attr_name == LP_OPER_STATUS) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT16);
        oper_status = atoi(attr_value.c_str());
        pfc_log_debug("oper_status: %d", oper_status);
        break;
      }
    }
  }
  return UPPL_RC_SUCCESS;
}

/** SetOperStatus
 * * @Description : This function updates the oper status in db
 * * * @param[in] : Key and oper_status
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_LogicalPort::SetOperStatus(
    uint32_t data_type,
    void* key_struct,
    void* val_struct,
    UpplLogicalPortOperStatus oper_status) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  key_logical_port_t *obj_key_logical_port=
      reinterpret_cast<key_logical_port_t*>(key_struct);
  val_logical_port_st_t *obj_val_logical_port=
      reinterpret_cast<val_logical_port_st_t*>(val_struct);
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  vector<string> vect_prim_keys;
  string controller_name = (const char*)obj_key_logical_port->domain_key.
      ctr_key.controller_name;
  pfc_log_debug("logical port set oper_status");
  if (!controller_name.empty()) {
    vect_prim_keys.push_back(CTR_NAME);
  }
  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  string domain_name = (const char*)obj_key_logical_port->
      domain_key.domain_name;

  string port_id = (const char*)obj_key_logical_port->port_id;
  string switch_id, phy_port_id;
  if (obj_val_logical_port != NULL) {
    switch_id = (const char*) obj_val_logical_port->logical_port.switch_id;
    phy_port_id =
        (const char*) obj_val_logical_port->logical_port.physical_port_id;
  }


  if (!domain_name.empty()) {
    vect_prim_keys.push_back(DOMAIN_NAME);
    PhyUtil::FillDbSchema(DOMAIN_NAME, domain_name,
                          domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
  }
  if (!port_id.empty()) {
    vect_prim_keys.push_back(LP_PORT_ID);
    PhyUtil::FillDbSchema(LP_PORT_ID, port_id,
                          port_id.length(), DATATYPE_UINT8_ARRAY_320,
                          vect_table_attr_schema);
  }

  if (!switch_id.empty()) {
    vect_prim_keys.push_back(LP_SWITCH_ID);
    PhyUtil::FillDbSchema(LP_SWITCH_ID, switch_id,
                          switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                          vect_table_attr_schema);
  }
  if (!phy_port_id.empty()) {
    vect_prim_keys.push_back(LP_PHYSICAL_PORT_ID);
    PhyUtil::FillDbSchema(LP_PHYSICAL_PORT_ID, phy_port_id,
                          phy_port_id.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
  }

  string oper_value = PhyUtil::uint8tostr(oper_status);
  PhyUtil::FillDbSchema(LP_OPER_STATUS, oper_value,
                        oper_value.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);
  string valid_flag;
  // Update valid flag as well
  UpplReturnCode get_ret = GetValidFlag(data_type, key_struct, &valid_flag);
  if (get_ret == UPPL_RC_SUCCESS) {
    string new_valid = valid_flag.substr(0, 5);
    new_valid += "1";
    pfc_log_debug("New Valid to be set: %s", new_valid.c_str());
    PhyUtil::FillDbSchema(LP_CTR_VALID, new_valid,
                          new_valid.length(), DATATYPE_UINT8_ARRAY_6,
                          vect_table_attr_schema);
  }

  DBTableSchema kt_logicalport_dbtableschema;
  kt_logicalport_dbtableschema.set_table_name(UPPL_LOGICALPORT_TABLE);
  kt_logicalport_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_logicalport_dbtableschema.set_row_list(row_list);

  // Call ODBCManager and update
  ODBCM_RC_STATUS update_db_status =
      physical_layer->get_odbc_manager()->UpdateOneRow(
          (unc_keytype_datatype_t)data_type,
          kt_logicalport_dbtableschema);
  if (update_db_status != ODBCM_RC_SUCCESS) {
    // log error
    pfc_log_error("oper_status update operation failed");
    return UPPL_RC_ERR_DB_UPDATE;
  } else {
    // Notify operstatus change to northbound
    val_logical_port_st old_val_logical_port, new_val_logical_port;
    uint8_t old_oper_status = 0;
    UpplReturnCode read_status = GetOperStatus(data_type,
                                               key_struct,
                                               old_oper_status);
    if (read_status == UPPL_RC_SUCCESS && data_type != UNC_DT_IMPORT) {
      memset(old_val_logical_port.valid, 0, 2);
      old_val_logical_port.oper_status = old_oper_status;
      old_val_logical_port.valid[kIdxLogicalPortStOperStatus] = UNC_VF_VALID;
      memset(new_val_logical_port.valid, 0, 2);
      new_val_logical_port.oper_status = oper_status;
      new_val_logical_port.valid[kIdxLogicalPortStOperStatus] = UNC_VF_VALID;
      int err = 0;
      // Send notification to Northbound
      ServerEvent ser_evt(
          (pfc_ipcevtype_t)UPPL_EVENTS_KT_LOGICAL_PORT, err);
      ser_evt.addOutput((uint32_t)UNC_OP_UPDATE);
      ser_evt.addOutput(data_type);
      ser_evt.addOutput((uint32_t)UPPL_EVENTS_KT_LOGICAL_PORT);
      ser_evt.addOutput(*obj_key_logical_port);
      ser_evt.addOutput(new_val_logical_port);
      ser_evt.addOutput(old_val_logical_port);
      PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
      // Notify operstatus modifications
      UpplReturnCode status = (UpplReturnCode) physical_layer
          ->get_ipc_connection_manager()->SendEvent(&ser_evt);
      pfc_log_debug("Event notification status %d", status);
    }
  }
  return UPPL_RC_SUCCESS;
}

/** IsKeyExists
 * * @Description : This function checks whether the logicalport_id exists in DB
 * * * @param[in] : data type and key value
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_LogicalPort::IsKeyExists(unc_keytype_datatype_t data_type,
                                           vector<string> key_values) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UpplReturnCode check_status = UPPL_RC_SUCCESS;
  if (key_values.empty()) {
    // No key given, return failure
    pfc_log_error("No key given. Returning error");
    return UPPL_RC_ERR_BAD_REQUEST;
  }
  string controller_name = key_values[0];
  string domain_name = key_values[1];
  string logicalport_id = key_values[2];
  // Structure used to send request to ODBC
  DBTableSchema kt_logicalport_dbtableschema;

  // Construct Primary key list
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_NAME);
  if (!domain_name.empty()) {
    vect_prim_keys.push_back(DOMAIN_NAME);
  }
  if (!logicalport_id.empty()) {
    vect_prim_keys.push_back(LP_PORT_ID);
  }

  // Construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  TableAttrSchema kt_logicalport_table_attr_schema;
  list< vector<TableAttrSchema> > row_list;

  // controller_name
  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // domain_name
  pfc_log_info("domain name: %s", domain_name.c_str());
  PhyUtil::FillDbSchema(DOMAIN_NAME, domain_name,
                        domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // port_id
  PhyUtil::FillDbSchema(LP_PORT_ID, logicalport_id,
                        logicalport_id.length(), DATATYPE_UINT8_ARRAY_320,
                        vect_table_attr_schema);

  kt_logicalport_dbtableschema.set_table_name(UPPL_LOGICALPORT_TABLE);
  kt_logicalport_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_logicalport_dbtableschema.set_row_list(row_list);

  // Send request to ODBC for controlle_table
  ODBCM_RC_STATUS check_db_status = physical_layer->get_odbc_manager()->\
      IsRowExists(data_type, kt_logicalport_dbtableschema);
  if (check_db_status == ODBCM_RC_ROW_EXISTS) {
    pfc_log_debug("DB returned success for Row exists");
  } else {
    pfc_log_error("DB Returned failure for IsRowExists");
    check_status = UPPL_RC_ERR_NO_SUCH_INSTANCE;
  }
  return check_status;
}

/** PopulateDBSchemaForKtTable
 * * @Description : This function populates the DBAttrSchema to be used to send
 *                  request to ODBC
 * * * @param[in] : DBTableSchema, key_struct, val_struct, operation_type
 * * * @return    : Success or associated error code
 * */
void Kt_LogicalPort::PopulateDBSchemaForKtTable(
    DBTableSchema &kt_logicalport_dbtableschema,
    void* key_struct,
    void* val_struct,
    uint8_t operation_type,
    uint32_t option1,
    uint32_t option2,
    vector<ODBCMOperator> &vect_key_operations,
    void* &old_value_struct,
    CsRowStatus row_status,
    pfc_bool_t is_filtering,
    pfc_bool_t is_state) {
  // Construct Primary key list
  vector<string> vect_prim_keys;

  // Construct TableAttrSchema structuree
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;

  key_logical_port_t *obj_key_logical_port =
      reinterpret_cast<key_logical_port_t*>(key_struct);
  val_logical_port_st_t *obj_val_logical_port_st =
      reinterpret_cast<val_logical_port_st_t*>(val_struct);

  stringstream valid;
  pfc_log_info("operation: %d", operation_type);
  // controller_name
  string controller_name =
      (const char*)obj_key_logical_port->domain_key.ctr_key.controller_name;
  pfc_log_info("controller name: %s", controller_name.c_str());
  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // domain_name
  string domain_name =
      (const char*)obj_key_logical_port->domain_key.domain_name;
  pfc_log_info("domain name: %s", domain_name.c_str());
  PhyUtil::FillDbSchema(DOMAIN_NAME, domain_name,
                        domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // port_id
  string port_id = (const char*)obj_key_logical_port->port_id;
  if (operation_type == UNC_OP_READ_SIBLING_BEGIN ||
      operation_type == UNC_OP_READ_SIBLING_COUNT) {
    // ignore port_id
    port_id = "";
  }
  pfc_log_info("port_id : %s", port_id.c_str());
  PhyUtil::FillDbSchema(LP_PORT_ID, port_id,
                        port_id.length(), DATATYPE_UINT8_ARRAY_320,
                        vect_table_attr_schema);

  if (!controller_name.empty()) {
    vect_prim_keys.push_back(CTR_NAME);
  }
  if (!domain_name.empty()) {
    vect_prim_keys.push_back(DOMAIN_NAME);
  }
  val_logical_port_st_t *val_logical_port_valid_st = NULL;
  if (operation_type == UNC_OP_UPDATE) {
    // get valid array for update req
    pfc_log_debug("Get Valid value from logicalport Update Valid Flag");
    val_logical_port_valid_st = new val_logical_port_st_t();
    GetLogicalPortValidFlag(key_struct, *val_logical_port_valid_st);
    old_value_struct = reinterpret_cast<void*>(val_logical_port_valid_st);
  }
  uint16_t valid_val = 0, prev_db_val = 0;
  unsigned int valid_value_struct = UNC_VF_VALID;
  if (obj_val_logical_port_st != NULL) {
    valid_value_struct = PhyUtil::uint8touint(
        obj_val_logical_port_st->valid[kIdxLogicalPortSt]);
  }
  pfc_log_debug("valid_value_struct: %d", valid_value_struct);
  string value;
  // Description
  if (obj_val_logical_port_st != NULL && valid_value_struct == UNC_VF_VALID) {
    valid_val = PhyUtil::uint8touint(obj_val_logical_port_st->logical_port.
                                     valid[kIdxLogicalPortDescription]);
    value = (const char*)obj_val_logical_port_st->logical_port.
        description;
    if (operation_type == UNC_OP_UPDATE) {
      prev_db_val =
          PhyUtil::uint8touint(val_logical_port_valid_st->logical_port.
                               valid[kIdxLogicalPortDescription]);
    }
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(LP_DESCRIPTION, value,
                        value.length(), DATATYPE_UINT8_ARRAY_128,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // port_type
  if (obj_val_logical_port_st != NULL && valid_value_struct == UNC_VF_VALID) {
    valid_val = PhyUtil::uint8touint(obj_val_logical_port_st->logical_port.
                                     valid[kIdxLogicalPortType]);
    value = PhyUtil::uint8tostr(
        obj_val_logical_port_st->logical_port.port_type);
    if (operation_type == UNC_OP_UPDATE) {
      prev_db_val =
          PhyUtil::uint8touint(val_logical_port_valid_st->logical_port.
                               valid[kIdxLogicalPortType]);
    }
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(LP_PORT_TYPE, value,
                        value.length(), DATATYPE_UINT16,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  // switch_id
  if (obj_val_logical_port_st != NULL && valid_value_struct == UNC_VF_VALID) {
    valid_val = PhyUtil::uint8touint(obj_val_logical_port_st->logical_port.
                                     valid[kIdxLogicalPortSwitchId]);
    value = (const char*)obj_val_logical_port_st->logical_port.
        switch_id;
    if (operation_type == UNC_OP_UPDATE) {
      prev_db_val =
          PhyUtil::uint8touint(val_logical_port_valid_st->logical_port.
                               valid[kIdxLogicalPortSwitchId]);
    }
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(LP_SWITCH_ID, value,
                        value.length(), DATATYPE_UINT8_ARRAY_256,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // physical_port_id
  if (obj_val_logical_port_st != NULL && valid_value_struct == UNC_VF_VALID) {
    valid_val = PhyUtil::uint8touint(obj_val_logical_port_st->logical_port.
                                     valid[kIdxLogicalPortPhysicalPortId]);
    value = (const char*)obj_val_logical_port_st->logical_port.
        physical_port_id;
    if (operation_type == UNC_OP_UPDATE) {
      prev_db_val =
          PhyUtil::uint8touint(val_logical_port_valid_st->logical_port.
                               valid[kIdxLogicalPortPhysicalPortId]);
    }
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(LP_PHYSICAL_PORT_ID, value,
                        value.length(), DATATYPE_UINT8_ARRAY_32,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // oper_down_criteria
  if (obj_val_logical_port_st != NULL && valid_value_struct == UNC_VF_VALID) {
    valid_val = PhyUtil::uint8touint(obj_val_logical_port_st->logical_port.
                                     valid[kIdxLogicalPortOperDownCriteria]);
    value = PhyUtil::uint8tostr(obj_val_logical_port_st->
                                logical_port.oper_down_criteria);
    if (operation_type == UNC_OP_UPDATE) {
      prev_db_val =
          PhyUtil::uint8touint(val_logical_port_valid_st->logical_port.
                               valid[kIdxLogicalPortOperDownCriteria]);
    }
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(LP_OPER_DOWN_CRITERIA, value,
                        value.length(), DATATYPE_UINT16,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  // oper status
  if (obj_val_logical_port_st != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_logical_port_st->
                                     valid[kIdxLogicalPortStOperStatus]);
    if (valid_val == UNC_VF_VALID) {
      value = PhyUtil::uint8tostr(obj_val_logical_port_st->oper_status);
    }
    if (operation_type == UNC_OP_UPDATE) {
      prev_db_val =
          PhyUtil::uint8touint(val_logical_port_valid_st->logical_port.
                               valid[kIdxLogicalPortStOperStatus]);
    }
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(LP_OPER_STATUS, value,
                        value.length(), DATATYPE_UINT16,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  // valid
  stringstream dummy_valid;
  valid_val = UPPL_NO_VAL_STRUCT;
  prev_db_val = 0;
  PhyUtil::FillDbSchema(LP_CTR_VALID, valid.str(),
                        ODBCM_SIZE_6, DATATYPE_UINT8_ARRAY_6,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, dummy_valid);
  // For read_bulk/ read_next port_id is required
  if ((operation_type == UNC_OP_READ && !port_id.empty()) ||
      operation_type != UNC_OP_READ) {
    vect_prim_keys.push_back(LP_PORT_ID);
  }
  PhyUtil::reorder_col_attrs(vect_prim_keys, vect_table_attr_schema);
  kt_logicalport_dbtableschema.set_table_name(UPPL_LOGICALPORT_TABLE);
  kt_logicalport_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_logicalport_dbtableschema.set_row_list(row_list);
  return;
}
/**
 * * @Description : This function populate val_logical_port_t by value retrieved
 * from database
 * * * @param[in] : logicalportcommon table dbtable schema, openflow logicalport
 * db schema, value structure and max_rep_ct, operation type
 * * * @return    : Filled val_logical_port and logicalport id
 * */
void Kt_LogicalPort::FillLogicalPortValueStructure(
    DBTableSchema &kt_logicalport_dbtableschema,
    vector<val_logical_port_st_t> &vect_obj_val_logical_port,
    uint32_t &max_rep_ct,
    uint32_t operation_type,
    vector<key_logical_port_t> &vect_logical_port_id) {
  // populate IPC value structure based on the response recevied from DB
  list < vector<TableAttrSchema> > res_logical_port_row_list =
      kt_logicalport_dbtableschema.get_row_list();

  list < vector<TableAttrSchema> > :: iterator res_logical_port_iter =
      res_logical_port_row_list.begin();

  max_rep_ct = res_logical_port_row_list.size();
  pfc_log_debug("res_logical_port_row_list.size: %d", max_rep_ct);
  key_logical_port_t obj_key_logical_port;

  for (; res_logical_port_iter != res_logical_port_row_list.end();
      ++res_logical_port_iter) {
    vector<TableAttrSchema> res_logical_port_table_attr_schema =
        (*res_logical_port_iter);
    vector<TableAttrSchema> :: iterator vect_logical_port_iter =
        res_logical_port_table_attr_schema.begin();

    memset(&obj_key_logical_port, '\0', sizeof(obj_key_logical_port));
    val_logical_port_t obj_val_logical_port;
    memset(&obj_val_logical_port, 0, sizeof(obj_val_logical_port));
    val_logical_port_st_t obj_val_logical_port_st;
    memset(&obj_val_logical_port_st, 0, sizeof(obj_val_logical_port_st));

    for (; vect_logical_port_iter != res_logical_port_table_attr_schema.end();
        ++vect_logical_port_iter) {
      // populate values from logicalport_table
      TableAttrSchema tab_schema = (*vect_logical_port_iter);
      string attr_name = tab_schema.table_attribute_name;
      string attr_value;
      if (attr_name == LP_PORT_ID) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_320);
        memcpy(obj_key_logical_port.port_id,
               attr_value.c_str(),
               attr_value.length()+1);
        pfc_log_debug("logical_port id: %s", reinterpret_cast<char *>
        (&obj_key_logical_port.port_id));
      }
      if (attr_name == DOMAIN_NAME) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_32);
        memcpy(obj_key_logical_port.domain_key.domain_name,
               attr_value.c_str(),
               attr_value.length()+1);
        pfc_log_debug("domain_name: %s", reinterpret_cast<char *>
        (&obj_key_logical_port.domain_key.domain_name));
      }
      if (attr_name == CTR_NAME) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_32);
        memcpy(obj_key_logical_port.domain_key.ctr_key.controller_name,
               attr_value.c_str(),
               attr_value.length()+1);
        pfc_log_debug("controller_name: %s", reinterpret_cast<char *>
        (&obj_key_logical_port.domain_key.ctr_key.controller_name));
      }
      if (attr_name == LP_DESCRIPTION) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_128);
        memcpy(obj_val_logical_port.description,
               attr_value.c_str(),
               attr_value.length()+1);
        pfc_log_debug("description: %s", attr_value.c_str());
      }
      if (attr_name == LP_PORT_TYPE) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT16);
        obj_val_logical_port.port_type = atoi(attr_value.c_str());
        pfc_log_debug("port_type: %s", attr_value.c_str());
      }
      if (attr_name == LP_SWITCH_ID) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_256);
        memcpy(obj_val_logical_port.switch_id,
               attr_value.c_str(),
               attr_value.length()+1);
        pfc_log_debug("switch_id: %s", attr_value.c_str());
      }
      if (attr_name == LP_PHYSICAL_PORT_ID) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_32);
        memcpy(obj_val_logical_port.physical_port_id,
               attr_value.c_str(),
               attr_value.length()+1);
        pfc_log_debug("physical_port_id: %s", attr_value.c_str());
      }
      if (attr_name == LP_OPER_DOWN_CRITERIA) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT16);
        obj_val_logical_port.oper_down_criteria =
            atoi(attr_value.c_str());
        pfc_log_debug("oper_down_criteria: %s", attr_value.c_str());
      }
      if (attr_name == LP_OPER_STATUS) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT16);
        obj_val_logical_port_st.oper_status =
            atoi(attr_value.c_str());
        pfc_log_debug("oper_status: %s", attr_value.c_str());
      }
      if (attr_name == LP_CTR_VALID) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_6);
        memset(obj_val_logical_port_st.valid, 0, 2);
        memset(obj_val_logical_port.valid, 0, 5);
        FrameValidValue(attr_value, obj_val_logical_port_st,
                        obj_val_logical_port);
        pfc_log_debug("valid: %s", attr_value.c_str());
      }
    }
    // copy val_t to val_st_t structure
    memcpy(&obj_val_logical_port_st.logical_port,
           &obj_val_logical_port,
           sizeof(val_logical_port_t));
    vect_obj_val_logical_port.push_back(obj_val_logical_port_st);
    // populate key structure
    vect_logical_port_id.push_back(obj_key_logical_port);
    pfc_log_debug("result - vect_obj_val_logical_port size: %d",
                  (unsigned int) vect_obj_val_logical_port.size());
    pfc_log_debug("result - vect_logical_port_id size: %d",
                  (unsigned int) vect_logical_port_id.size());
  }
  return;
}

/** PerformRead
 * * @Description : This function reads the instance of UNC_KT_LOGICAL_PORT
 *                  based on operation type - READ, READ_SIBLING_BEGIN,
 *                  READ_SIBLING
 * * * @param[in] : ipc session id, configuration id, key_struct, value_struct,
 *                  data_type, operation type, ServerSession, option1, option2,
 *                  max_rep_ct
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_LogicalPort::PerformRead(uint32_t session_id,
                                           uint32_t configuration_id,
                                           void* key_struct,
                                           void* val_struct,
                                           uint32_t data_type,
                                           uint32_t operation_type,
                                           ServerSession &sess,
                                           uint32_t option1,
                                           uint32_t option2,
                                           uint32_t max_rep_ct) {
  pfc_log_debug("Inside PerformRead");
  key_logical_port_t *obj_key_logical_port =
      reinterpret_cast<key_logical_port_t*>(key_struct);
  physical_response_header rsh = {session_id,
      configuration_id,
      operation_type,
      max_rep_ct,
      option1,
      option2,
      data_type,
      0};
  if (operation_type == UNC_OP_READ) {
    max_rep_ct = 1;
  }

  // Invalid operation
  if (option1 != UNC_OPT1_NORMAL) {
    pfc_log_error("PerformRead provided on unsupported option1");
    rsh.result_code = UPPL_RC_ERR_INVALID_OPTION1;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_LOGICAL_PORT);
    if (err != 0) {
      pfc_log_debug("addOutput failed for physical_response_header");
      return UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UPPL_RC_SUCCESS;
  }

  if (option2 != UNC_OPT2_NONE) {
    pfc_log_error("PerformRead provided on unsupported option2");
    rsh.result_code = UPPL_RC_ERR_INVALID_OPTION2;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_LOGICAL_PORT);
    if (err != 0) {
      pfc_log_debug("addOutput failed for physical_response_header");
      return UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UPPL_RC_SUCCESS;
  }

  UpplReturnCode read_status = UPPL_RC_SUCCESS;

  if ((unc_keytype_datatype_t)data_type != UNC_DT_STATE) {
    pfc_log_error("Read operation is provided on unsupported data type");
    rsh.result_code = UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_LOGICAL_PORT);
    err |= sess.addOutput(*obj_key_logical_port);
    if (err != 0) {
      pfc_log_debug("addOutput failed for physical_response_header");
      return UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UPPL_RC_SUCCESS;
  }
  // Read operations will return logical_port_st based on modified fd
  vector<key_logical_port_t> vect_logicalport_id;
  vector<val_logical_port_t> vect_val_logical_port;
  vector<val_logical_port_st_t> vect_val_logical_port_st;
  read_status = ReadLogicalPortValFromDB(key_struct,
                                         val_struct,
                                         data_type,
                                         operation_type,
                                         max_rep_ct,
                                         vect_val_logical_port,
                                         vect_val_logical_port_st,
                                         vect_logicalport_id);
  rsh.result_code = read_status;
  rsh.max_rep_count = max_rep_ct;
  int err = PhyUtil::sessOutRespHeader(sess, rsh);
  if (err != 0) {
    pfc_log_error("Failure in addOutput");
    return UPPL_RC_ERR_IPC_WRITE_ERROR;
  }

  pfc_log_debug("read status val in performread = %d", read_status);
  if (read_status == UPPL_RC_SUCCESS) {
    for (unsigned int index = 0; index < vect_logicalport_id.size();
        ++index) {
      sess.addOutput((uint32_t)UNC_KT_LOGICAL_PORT);
      sess.addOutput((key_logical_port_t)vect_logicalport_id[index]);
      sess.addOutput(vect_val_logical_port_st[index]);
      if (index < vect_logicalport_id.size() -1) {
        sess.addOutput();  //  Seperator
      }
    }
  } else {
    pfc_log_error("Read operation on dt_state failed with status: %d",
                  read_status);
    rsh.max_rep_count = 0;
    sess.addOutput((uint32_t) UNC_KT_LOGICAL_PORT);
    sess.addOutput(*obj_key_logical_port);
  }

  pfc_log_debug("Perform Read completed");
  return UPPL_RC_SUCCESS;
}
/** ReadLogicalPortValFromDB
 * * @Description : This function reads the instance of UNC_KT_LOGICAL_PORT
 *                  based on operation type - READ, READ_SIBLING_BEGIN,
 *                  READ_SIBLING from data base
 * * * @param[in] : key_struct, value_struct, ipc session id, configuration id,
 *                  data_type, operation type, max_rep_ct
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_LogicalPort::ReadLogicalPortValFromDB(
    void* key_struct,
    void* val_struct,
    uint32_t data_type,
    uint32_t operation_type,
    uint32_t &max_rep_ct,
    vector<val_logical_port_t> &vect_val_logical_port,
    vector<val_logical_port_st_t> &vect_val_logical_port_st,
    vector<key_logical_port_t> &logicalport_id) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();

  UpplReturnCode read_status = UPPL_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;
  if (operation_type < UNC_OP_READ) {
    // Unsupported operation type for this function
    return UPPL_RC_SUCCESS;
  }
  DBTableSchema kt_logicalport_dbtableschema;
  void* old_value_struct;
  vector<ODBCMOperator> vect_key_operations;
  PopulateDBSchemaForKtTable(kt_logicalport_dbtableschema,
                             key_struct,
                             val_struct,
                             operation_type, 0, 0, vect_key_operations,
                             old_value_struct);
  if (operation_type == UNC_OP_READ) {
    read_db_status = physical_layer->get_odbc_manager()->
        GetOneRow((unc_keytype_datatype_t)data_type,
                  kt_logicalport_dbtableschema);
  } else {
    read_db_status = physical_layer->get_odbc_manager()->
        GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                    kt_logicalport_dbtableschema,
                    (unc_keytype_operation_t)operation_type);
  }
  if (read_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
    pfc_log_debug("No record found");
    read_status = UPPL_RC_ERR_NO_SUCH_INSTANCE;
    return read_status;
  } else if (read_db_status != ODBCM_RC_SUCCESS) {
    read_status = UPPL_RC_ERR_DB_GET;
    // log error to log daemon
    pfc_log_error("Read operation has failed with error %d", read_db_status);
    return read_status;
  }
  FillLogicalPortValueStructure(kt_logicalport_dbtableschema,
                                vect_val_logical_port_st,
                                max_rep_ct,
                                operation_type,
                                logicalport_id);
  pfc_log_debug("vect_val_logical_port_st size: %d",
                (unsigned int)vect_val_logical_port_st.size());
  pfc_log_debug("logicalport_id size: %d", (unsigned int)logicalport_id.size());
  if (vect_val_logical_port_st.empty()) {
    // Read failed , return error
    read_status = UPPL_RC_ERR_DB_GET;
    // log error to log daemon
    pfc_log_error("Read operation has failed after reading response");
    return read_status;
  }
  pfc_log_debug("Read operation Completed with result: %d", read_status);
  return read_status;
}

/** getChildKeyStruct
 * * @Description : This function returns the void * of child key structures
 * * * @param[in] : child class index and logicalport name
 * * * @return    : void * key structure
 * */
void* Kt_LogicalPort::getChildKeyStruct(int child_class,
                                        string logicalport_id,
                                        string controller_name,
                                        string domain_name) {
  switch (child_class) {
    case KIdxLogicalMemberPort: {
      key_logical_member_port_t * obj_child_key = new key_logical_member_port_t;
      memcpy(obj_child_key->logical_port_key.port_id,
             logicalport_id.c_str(),
             logicalport_id.length()+1);
      memcpy(obj_child_key->logical_port_key.domain_key.domain_name,
             domain_name.c_str(),
             domain_name.length()+1);
      memcpy(
          obj_child_key->logical_port_key.domain_key.ctr_key.controller_name,
          controller_name.c_str(),
          controller_name.length()+1);
      // not to fill Switch and physical port id as they can be empty.
      memset(obj_child_key->switch_id, 0, 256);
      memset(obj_child_key->physical_port_id, 0, 32);
      void* child_key = reinterpret_cast<void *>(obj_child_key);
      return child_key;
    }
    default: {
      // Do nothing
      pfc_log_info("Invalid index %d passed to getChildKeyStruct()",
                   child_class);
      PFC_ASSERT(PFC_FALSE);
      return NULL;
    }
  }
}

/** FreeChildKeyStruct
 * * @Description : This function clears the void* of child key structures
 * * * @param[in] : child class index and logicalport name
 * * * @return    : void * key structure
 * */
void Kt_LogicalPort::FreeChildKeyStruct(int child_class,
                                        void *key_struct) {
  switch (child_class) {
    case KIdxLogicalMemberPort: {
      key_logical_member_port_t *obj_child_key =
          reinterpret_cast<key_logical_member_port_t *>(key_struct);
      if (obj_child_key != NULL) {
        delete obj_child_key;
        obj_child_key = NULL;
      }
      return;
    }
    default: {
      pfc_log_info("Invalid index %d passed to FreeChildKeyStruct()",
                   child_class);
      PFC_ASSERT(PFC_FALSE);
      // Do nothing
      return;
    }
  }
}

/** Fill_Attr_Syntax_Map
 * * @Description : This function populates the values to be used for attribute
 * validation
 * * * @param[in] : None
 * * * @return    : None
 * */
void Kt_LogicalPort::Fill_Attr_Syntax_Map() {
  Kt_Class_Attr_Syntax objKeyAttrSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 1, 320, true,  "" };
  attr_syntax_map[LP_PORT_ID] = objKeyAttrSyntax;

  Kt_Class_Attr_Syntax objKeyAttr1Syntax4 =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 32, true, "" };
  attr_syntax_map[DOMAIN_NAME] = objKeyAttr1Syntax4;

  Kt_Class_Attr_Syntax objKeyAttr1Syntax5 =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 32, true, "" };
  attr_syntax_map[CTR_NAME] = objKeyAttr1Syntax5;

  Kt_Class_Attr_Syntax objAttrDescSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 128, false, "" };
  attr_syntax_map[LP_DESCRIPTION] = objAttrDescSyntax;

  Kt_Class_Attr_Syntax objAttrPortTypeSyntax =
  { PFC_IPCTYPE_UINT8, 0, 4, 0, 0, true, "" };
  attr_syntax_map[LP_PORT_TYPE] = objAttrPortTypeSyntax;

  Kt_Class_Attr_Syntax objAttrSwitchIdSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 256, false, "" };
  attr_syntax_map[LP_SWITCH_ID] = objAttrSwitchIdSyntax;

  Kt_Class_Attr_Syntax objAttrPhyPortIdSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 32, false, "" };
  attr_syntax_map[LP_PHYSICAL_PORT_ID] = objAttrPhyPortIdSyntax;

  Kt_Class_Attr_Syntax objAttrOpDownCriteriaSyntax =
  { PFC_IPCTYPE_UINT8, 0, 1, 0, 0, false, "" };
  attr_syntax_map[LP_OPER_DOWN_CRITERIA] = objAttrOpDownCriteriaSyntax;

  Kt_Class_Attr_Syntax objAttrValidSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 5, false, "" };
  attr_syntax_map[LP_CTR_VALID] = objAttrValidSyntax;
}

/** GetAllPortId
 *  * @Description : This function returns all the associated physical/logical
 *   port ids for a given switch
 *  * @param[in] : controller_name, switch_id, port_id flag
 *  * @return    : vector of logical_port_id
 */
void Kt_LogicalPort::GetAllPortId(uint32_t data_type,
                                  string controller_name,
                                  string switch_id,
                                  vector <string> &logical_port_id,
                                  pfc_bool_t is_single_logical_port) {
  DBTableSchema kt_logicalport_dbtableschema;
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  vector<string> vect_prim_keys;
  vector<ODBCMOperator> vect_operators;
  vect_prim_keys.push_back(CTR_NAME);
  vect_prim_keys.push_back(LP_SWITCH_ID);
  vect_prim_keys.push_back(LP_PORT_TYPE);
  vect_operators.push_back(unc::uppl::EQUAL);
  vect_operators.push_back(unc::uppl::EQUAL);
  vect_operators.push_back(unc::uppl::EQUAL);
  // controller_name
  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // switch_id
  PhyUtil::FillDbSchema(LP_SWITCH_ID, switch_id,
                        switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);

  // port_type
  string port_type = "";
  if (is_single_logical_port == true) {
    port_type = "0";  //  UPPL_LP_SWITCH;
  } else {
    port_type = "1";  //  UPPL_LP_PHYSICAL_PORT;
  }
  pfc_log_debug(
      "GetAllPortId is called with is_single_logical_port:%d, port_type:%s",
      static_cast<int>(is_single_logical_port), port_type.c_str());
  PhyUtil::FillDbSchema(LP_PORT_TYPE, port_type,
                        port_type.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);

  string empty_id="";
  // logical_port_id - to be fetched
  PhyUtil::FillDbSchema(LP_PORT_ID, empty_id,
                        empty_id.length(), DATATYPE_UINT8_ARRAY_320,
                        vect_table_attr_schema);
  kt_logicalport_dbtableschema.set_table_name(UPPL_LOGICALPORT_TABLE);
  kt_logicalport_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_logicalport_dbtableschema.set_row_list(row_list);
  // Call GetSibling Rows with operator specification
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  ODBCM_RC_STATUS read_db_status = physical_layer->get_odbc_manager()->
      GetSiblingRows((unc_keytype_datatype_t)data_type, UPPL_MAX_REP_CT,
                     kt_logicalport_dbtableschema,
                     vect_operators,
                     (unc_keytype_operation_t)UNC_OP_READ_SIBLING);

  if (read_db_status != ODBCM_RC_SUCCESS) {
    pfc_log_debug("Get Sibling is not success");
    return;
  }
  // Read all the logical port values
  row_list = kt_logicalport_dbtableschema.get_row_list();
  list < vector<TableAttrSchema> > :: iterator res_logical_port_iter =
      row_list.begin();
  uint32_t row_ct = row_list.size();
  pfc_log_debug("res_logical_port_row_list.size: %d", row_ct);
  for (; res_logical_port_iter != row_list.end();
      ++res_logical_port_iter) {
    vector<TableAttrSchema> res_logical_port_table_attr_schema =
        (*res_logical_port_iter);
    vector<TableAttrSchema> :: iterator vect_logical_port_iter =
        res_logical_port_table_attr_schema.begin();

    for (; vect_logical_port_iter != res_logical_port_table_attr_schema.end();
        ++vect_logical_port_iter) {
      TableAttrSchema tab_schema = (*vect_logical_port_iter);
      string attr_name = tab_schema.table_attribute_name;
      string attr_value;
      if (attr_name == LP_PORT_ID) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT8_ARRAY_320);
        logical_port_id.push_back(attr_value);
        pfc_log_debug("logical_port id: %s", attr_value.c_str());
      }
    }
  }
  return;
}

/** GetLogicalPortValidFlag
 * * @Description : This function reads the valid flag from DB
 * * * @param[in] : Key, value struct and newvalid val
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_LogicalPort::GetLogicalPortValidFlag(
    void *key_struct,
    val_logical_port_st_t &val_logical_port_valid_st) {
  UpplReturnCode return_code = UPPL_RC_SUCCESS;
  // read the value structure from db
  vector<void *> vectVal_logicalport;
  vector<void *> vectkey_logicalport;
  vectkey_logicalport.push_back(key_struct);
  return_code = ReadInternal(vectkey_logicalport,
                             vectVal_logicalport,
                             UNC_DT_STATE, UNC_OP_READ);
  if (return_code == UPPL_RC_SUCCESS) {
    val_logical_port_st_t *obj_new_logicalport_val_vect =
        reinterpret_cast<val_logical_port_st_t*> (vectVal_logicalport[0]);
    if (obj_new_logicalport_val_vect != NULL) {
      val_logical_port_valid_st = *obj_new_logicalport_val_vect;

      delete obj_new_logicalport_val_vect;
      obj_new_logicalport_val_vect = NULL;
      key_logical_port_t *logicalport_key =
          reinterpret_cast<key_logical_port_t*> (vectkey_logicalport[0]);
      if (logicalport_key != NULL) {
        delete logicalport_key;
        logicalport_key = NULL;
      }
    } else {
      pfc_log_info("update logicalport valid ret null val");
    }
  }
  return return_code;
}

/** FrameValidValue
 * * @Description : This function converts the string value from db to uint8
 * * * @param[in] : Attribute value and val_ctr_st
 * * * @return    : Success or associated error code
 * */
void Kt_LogicalPort::FrameValidValue(
    string attr_value,
    val_logical_port_st &obj_val_logical_port_st,
    val_logical_port_t &obj_val_logical_port) {
  obj_val_logical_port_st.valid[kIdxLogicalPortSt] = UNC_VF_VALID;
  for (unsigned int i = 0; i < 6 ; ++i) {
    unsigned int valid = attr_value[i];
    if (attr_value[i] >= 48) {
      valid = attr_value[i] - 48;
    }
    if (i < 5) {
      obj_val_logical_port.valid[i] = valid;
    } else {
      obj_val_logical_port_st.valid[i-4] = valid;
    }
  }
  return;
}

/** GetValidFlag
 * * @Description : This function retrieves the valid flag from DB
 * * * @param[in] : Key and valid_flag
 * * * @return    : Success or associated error code
 * */
UpplReturnCode Kt_LogicalPort::GetValidFlag(
    uint32_t data_type,
    void* key_struct,
    string *valid_flag) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  key_logical_port_t *obj_key_logical_port =
      reinterpret_cast<key_logical_port_t*>(key_struct);
  TableAttrSchema kt_logical_port_table_attr_schema;
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  vector<string> vect_prim_keys;

  string controller_name = (const char*)obj_key_logical_port->domain_key.
      ctr_key.controller_name;

  if (!controller_name.empty()) {
    vect_prim_keys.push_back(CTR_NAME);
  }

  PhyUtil::FillDbSchema(CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  string domain_name = (const char*)obj_key_logical_port->
      domain_key.domain_name;

  if (!domain_name.empty()) {
    vect_prim_keys.push_back(DOMAIN_NAME);
  }

  PhyUtil::FillDbSchema(DOMAIN_NAME, domain_name,
                        domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  string port_id = (const char*)obj_key_logical_port->port_id;

  if (!port_id.empty()) {
    vect_prim_keys.push_back(LP_PORT_ID);
  }

  PhyUtil::FillDbSchema(LP_PORT_ID, port_id,
                        port_id.length(), DATATYPE_UINT8_ARRAY_320,
                        vect_table_attr_schema);

  string valid_empty;
  PhyUtil::FillDbSchema(LP_CTR_VALID, valid_empty,
                        valid_empty.length(),
                        DATATYPE_UINT8_ARRAY_6,
                        vect_table_attr_schema);

  DBTableSchema kt_logicalport_dbtableschema;
  kt_logicalport_dbtableschema.set_table_name(UPPL_LOGICALPORT_TABLE);
  kt_logicalport_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_logicalport_dbtableschema.set_row_list(row_list);

  // Call ODBCManager and read
  ODBCM_RC_STATUS read_db_status =
      physical_layer->get_odbc_manager()->GetOneRow(
          (unc_keytype_datatype_t)data_type,
          kt_logicalport_dbtableschema);
  if (read_db_status != ODBCM_RC_SUCCESS) {
    // log error
    pfc_log_error("valid flag read operation failed");
    return UPPL_RC_ERR_DB_GET;
  }

  // read the valid value
  list < vector<TableAttrSchema> > res_logicalport_row_list =
      kt_logicalport_dbtableschema.get_row_list();
  list < vector<TableAttrSchema> >::iterator res_logicalport_iter =
      res_logicalport_row_list.begin();
  for (; res_logicalport_iter!= res_logicalport_row_list.end();
      ++res_logicalport_iter) {
    vector<TableAttrSchema> res_logicalport_table_attr_schema =
        (*res_logicalport_iter);
    vector<TableAttrSchema>:: iterator vect_logicalport_iter =
        res_logicalport_table_attr_schema.begin();
    for (; vect_logicalport_iter != res_logicalport_table_attr_schema.end();
        ++vect_logicalport_iter) {
      TableAttrSchema tab_schema = (*vect_logicalport_iter);
      string attr_name = tab_schema.table_attribute_name;
      if (attr_name == LP_CTR_VALID) {
        PhyUtil::GetValueFromDbSchema(tab_schema, *valid_flag,
                                      DATATYPE_UINT8_ARRAY_6);
        pfc_log_debug("valid_flag from db: %s", valid_flag->c_str());
        break;
      }
    }
  }
  return UPPL_RC_SUCCESS;
}

