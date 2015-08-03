/*
 * Copyright (c) 2012-2015 NEC Corporation
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
#include "itc_kt_port_neighbor.hh"
#include "itc_kt_logicalport.hh"
#include "itc_kt_controller.hh"
#include "itc_kt_ctr_domain.hh"
#include "itc_kt_port.hh"
#include "itc_kt_logical_member_port.hh"
#include "itc_kt_boundary.hh"
#include "itc_kt_switch.hh"
#include "odbcm_db_varbind.hh"
#include "ipct_util.hh"
#include "itc_read_request.hh"
#include "phy_util.hh"
#include "itc_kt_state_base.hh"
using unc::uppl::PhysicalLayer;
using unc::uppl::ODBCMOperator;

/**
 * @Description : This function initializes member variables
 *                and fills the attribute syntax map used for validation
 * @param[in]   : None
 * @return      : None
 **/
Kt_LogicalPort::Kt_LogicalPort() {
  for (int i = 0; i < UNC_KT_LOGICAL_PORT_CHILD_COUNT; ++i) {
    child[i] = NULL;
  }
  // Populate structure to be used for syntax validation
  if (attr_syntax_map_all.find(UNC_KT_LOGICAL_PORT) ==
      attr_syntax_map_all.end()) {
    Fill_Attr_Syntax_Map();
  }
}

/**
 * @Description : This function frees the child key instances
 *                instances for kt_logicalport
 * @param[in]   : None
 * @return      : None
 **/
Kt_LogicalPort::~Kt_LogicalPort() {
  // Delete all child objects
  for (int i = 0; i < UNC_KT_LOGICAL_PORT_CHILD_COUNT; ++i) {
    if (child[i] != NULL) {
      delete child[i];
      child[i] = NULL;
    }
  }
}

/**
 * @Description : This function creates a new child class instance
 *                class of KtLogicalPort based on index passed
 * @param[in]   : KIndex - child class index enum
 * @return      : Kt_Base* - The child class pointer
 **/
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

/**
 * @Description : This function is used to delete KT_LOGICAL_PORT instance in
 *                database table using key_ctr provided in IPC request
 * @param[in]   : key_struct - void pointer to be cast into required key type
 *                data type  - UNC_DT_*,delete allowed only in STATE and IMPORT
 *                key_type   - indicates the key type
 * @return      : indicates the delete status of the row -
 *                UNC_RC_SUCCESS is returned when delete is done successfully.
 * 		  UNC_UPPL_RC_ERR_* is returned when delete is failed	
 **/
UncRespCode Kt_LogicalPort::DeleteKeyInstance(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    uint32_t data_type,
    uint32_t key_type) {
  UncRespCode delete_status = UNC_RC_SUCCESS;
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();

  // Check whether operation is allowed on the given DT type
  if (((unc_keytype_datatype_t)data_type != UNC_DT_STATE) &&
      ((unc_keytype_datatype_t)data_type != UNC_DT_IMPORT)) {
    pfc_log_error("Delete operation is provided on unsupported data type");
    delete_status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    return delete_status;
  }
  // Notify port to update logicalportid as invalid
  void *value_struct = NULL;
  vector<uint32_t> vectOperStatus;
  // call below function to fill port and switch id and call port function
  // to update logicalport id as invalid PFC_TRUE is to mention call is
  // from deletekeyinstance
  HandleOperDownCriteriaFromPortStatus(db_conn, data_type,
                                       key_struct,
                                       value_struct,
                                       vectOperStatus,
                                       PFC_TRUE);
  key_logical_port_t *obj_key_logical_port=
      reinterpret_cast<key_logical_port_t*>(key_struct);
  string controller_name =
      (const char*)obj_key_logical_port->domain_key.ctr_key.controller_name;
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
      UncRespCode ch_delete_status = child[child_class]->DeleteKeyInstance(
          db_conn, child_key_struct,
          data_type,
          UNC_KT_LOGICAL_MEMBER_PORT);
      delete child[child_class];
      child[child_class] = NULL;
      FreeChildKeyStruct(child_class, child_key_struct);
      if (ch_delete_status == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE) {
        pfc_log_debug("No child exists");
      } else if (ch_delete_status != UNC_RC_SUCCESS) {
        // child delete failed, so return error
        pfc_log_error("Delete failed for child %d with error %d",
                      child_class, ch_delete_status);
        delete_status = UNC_UPPL_RC_ERR_CFG_SEMANTIC;
        break;
      }
    } else {
      // Free child key struct
      FreeChildKeyStruct(child_class, child_key_struct);
    }
  }

  DBTableSchema kt_logicalport_dbtableschema;

  // Construct Primary key list
  vector<string> vect_prim_keys;
  if (!controller_name.empty()) {
    vect_prim_keys.push_back(CTR_NAME_STR);
  }
  if (!domain_name.empty()) {
    vect_prim_keys.push_back(DOMAIN_NAME_STR);
  }
  if (!logical_port_id.empty()) {
    vect_prim_keys.push_back(LP_PORT_ID_STR);
  }

  vector<TableAttrSchema> vect_table_attr_schema;
  list< vector<TableAttrSchema> > row_list;

  // controller_name
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // domain_name
  PhyUtil::FillDbSchema(unc::uppl::DOMAIN_NAME, domain_name,
                        domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // port_id
  PhyUtil::FillDbSchema(unc::uppl::LP_PORT_ID, logical_port_id,
                        logical_port_id.length(), DATATYPE_UINT8_ARRAY_320,
                        vect_table_attr_schema);

  // Send request to ODBC for logicalport_table delete
  kt_logicalport_dbtableschema.set_table_name(unc::uppl::LOGICALPORT_TABLE);
  kt_logicalport_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_logicalport_dbtableschema.set_row_list(row_list);

  ODBCM_RC_STATUS delete_db_status = physical_layer->get_odbc_manager()-> \
      DeleteOneRow((unc_keytype_datatype_t)data_type,
                   kt_logicalport_dbtableschema, db_conn);
  if (delete_db_status != ODBCM_RC_SUCCESS) {
    if (delete_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log fatal error to log daemon
      UPPL_LOG_FATAL("DB connection not available or cannot access DB");
      delete_status = UNC_UPPL_RC_ERR_DB_ACCESS;
    } else if (delete_db_status == ODBCM_RC_ROW_NOT_EXISTS) {
      pfc_log_error("given instance does not exist");
      delete_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
    } else {
      // log error to log daemon
      pfc_log_error("Delete operation has failed in logicalport table");
      delete_status = UNC_UPPL_RC_ERR_DB_DELETE;
    }
  } else {
    // deletion success send notification to boundary to
    // update operstatus to invalid
    delete_status = UNC_RC_SUCCESS;
    int err = InvokeBoundaryNotifyOperStatus(db_conn, data_type, key_struct);
    if (err != UNC_RC_SUCCESS) {
      pfc_log_error(
          "Delete instance: notify request for boundary is failed %d", err);
    }
  }
  return delete_status;
}

/**
 * @Description : This function is used to read KT_LOGICAL_PORT instance
 *                in database table using key_ctr provided in IPC request
 * @param[in]   : key_val - vector to hold the key struct
 *                val_struct - vector to hold the val struct
 *                data_type - indicates the data base type i.e. UNC_DT_STATE
 *                            or UNC_DT_IMPORT etc
 *                operation_type - indicates the operation type supported i.e.
 *                                 UNC_OP_READ
 * @return      : It returns the read status of a row in logical port table
 *                UNC_RC_SUCCESS - if read operation is success
 *                UNC_UPPL_RC_ERR_* - if read operation is a failure
 * */
UncRespCode Kt_LogicalPort::ReadInternal(OdbcmConnectionHandler *db_conn,
                                            vector<void *> &key_val,
                                            vector<void *> &val_struct,
                                            uint32_t data_type,
                                            uint32_t operation_type) {
  if (operation_type != UNC_OP_READ && operation_type != UNC_OP_READ_SIBLING &&
      operation_type != UNC_OP_READ_SIBLING_BEGIN) {
    pfc_log_trace("This function not allowed for read next/bulk/count");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
  }
  pfc_log_debug("Inside ReadInternal of UNC_KT_LOGICAL_PORT");
  uint32_t max_rep_ct = 1;
  if (operation_type != UNC_OP_READ) {
    max_rep_ct = UPPL_MAX_REP_CT;
  }
  val_logical_port_st_t obj_val;
  memset(&obj_val, '\0', sizeof(val_logical_port_st_t));
  void *key_struct = key_val[0];
  void *void_val_struct = NULL;
  if ((!val_struct.empty()) && (val_struct[0] != NULL)) {
    memcpy(&obj_val, (reinterpret_cast <val_logical_port_st_t*>
                                      (val_struct[0])),
           sizeof(val_logical_port_st_t));
    void_val_struct = reinterpret_cast<void *>(&obj_val);
  }
  // Get read response from database
  UncRespCode read_status = UNC_RC_SUCCESS;
  bool firsttime = true;
  do {
    vector<key_logical_port_t> vect_logicalport_id;
    vector<val_logical_port_st_t> vect_val_logical_port_st;
    read_status = ReadLogicalPortValFromDB(
        db_conn, key_struct,
        void_val_struct,
        data_type,
        operation_type,
        max_rep_ct,
        vect_val_logical_port_st,
        vect_logicalport_id);
    if (firsttime) {
       pfc_log_trace(
           "Clearing key_val and val_struct vectors for the first time");
       key_val.clear();
       val_struct.clear();
       firsttime = false;
    }
    if (read_status == UNC_RC_SUCCESS) {
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
    } else if ((read_status == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE &&
               val_struct.size() != 0)) {
      read_status = UNC_RC_SUCCESS;
    }
    if ((vect_val_logical_port_st.size() == UPPL_MAX_REP_CT) &&
                     (operation_type != UNC_OP_READ)) {
      pfc_log_debug("Op:%d, key.size:%" PFC_PFMT_SIZE_T"fetch_next_set",
                    operation_type, key_val.size());
      key_struct = reinterpret_cast<void *>(key_val[key_val.size() - 1]);
      operation_type = UNC_OP_READ_SIBLING;
      continue;
    } else {
      break;
    }
  } while (true);
  return read_status;
}

/**
 * @Description : This function reads rows from running logical port table
 *                depending on the max_rep_ct number of instances specified
 * @param[in]   : key_struct - void pointer to be cast into logical port type
 *                data_type  - indicates the data base type - read is supported
 *                             on UNC_DT_STATE and UNC_DT_IMPORT
 *                max_rep_ct - specifies the maximum repetition count for read
 *                             bulk operation
 *                child_index - specifies the child index
 *                parent_call - bool variable to check whether parent call
 *                              is made
 *                is_read_next - bool variable to indicate whether this
 *                               function is invoked from readnext
 * @return      : It returns the read bulk operation status in
 *                logical port table
 *                UNC_RC_SUCCESS - if read operation is success
 *                UNC_UPPL_RC_ERR_* - if read operation is a failure
 **/
UncRespCode Kt_LogicalPort::ReadBulk(OdbcmConnectionHandler *db_conn,
                                        void* key_struct,
                                        uint32_t data_type,
                                        uint32_t &max_rep_ct,
                                        int child_index,
                                        pfc_bool_t parent_call,
                                        pfc_bool_t is_read_next,
                                        ReadRequest *read_req) {
  UncRespCode read_status = UNC_RC_SUCCESS;
  key_logical_port_t obj_key_logical_port=
      *(reinterpret_cast<key_logical_port_t*>(key_struct));
  if ((unc_keytype_datatype_t)data_type != UNC_DT_STATE) {
    // Not supported
    pfc_log_debug("ReadBulk operation is not allowed in %d data type",
                  data_type);
    read_status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    return read_status;
  }
  string str_controller_name = reinterpret_cast<char *>(obj_key_logical_port.
      domain_key.ctr_key.controller_name);
  pfc_log_debug("controller name: %s", str_controller_name.c_str());
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
    return UNC_RC_SUCCESS;
  }
  if (child_index == -1 &&
      !str_logicalport_id.empty()) {
    // Check for key existence
    vector<string> vect_key_value;
    vect_key_value.push_back(str_controller_name);
    vect_key_value.push_back(str_domain_name);
    vect_key_value.push_back(str_logicalport_id);
    UncRespCode key_exist_status = IsKeyExists(
        db_conn, (unc_keytype_datatype_t)data_type,
        vect_key_value);
    if (key_exist_status == UNC_RC_SUCCESS) {
      log_port_exists = PFC_TRUE;
    }
  }
  void *val_struct = NULL;
  // Read the logical port values based on given key structure
  read_status = ReadBulkInternal(db_conn, key_struct, val_struct, data_type,
                                 max_rep_ct, vect_val_logical_port_st,
                                 vect_logicalport_id);

  pfc_log_debug("Read status from logical_port is %d with result size %"
                PFC_PFMT_SIZE_T, read_status, vect_logicalport_id.size());
  if (log_port_exists == PFC_TRUE) {
    vect_logicalport_id.insert(vect_logicalport_id.begin(),
                               obj_key_logical_port);
    val_logical_port_st_t dummy_val_log_port;
    vect_val_logical_port_st.insert(vect_val_logical_port_st.begin(),
                                    dummy_val_log_port);
  }
  if (read_status == UNC_RC_SUCCESS ||
      log_port_exists == PFC_TRUE) {
    vector<val_logical_port_st_t>::iterator vect_iter =
        vect_val_logical_port_st.begin();
    vector<key_logical_port_t> ::iterator logicalport_iter =
        vect_logicalport_id.begin();
    for (; logicalport_iter != vect_logicalport_id.end();
        ++logicalport_iter, ++vect_iter) {
      pfc_log_debug("Iterating value entries...");
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
        read_req->AddToBuffer(obj_key_buffer);
        val_logical_port_st_t *val_buffer = new val_logical_port_st_t
            (*vect_iter);
        BulkReadBuffer obj_value_buffer = {
            UNC_KT_LOGICAL_PORT, IS_STATE_VALUE,
            reinterpret_cast<void*>(val_buffer)
        };
        read_req->AddToBuffer(obj_value_buffer);
        BulkReadBuffer obj_sep_buffer = {
            UNC_KT_LOGICAL_PORT, IS_SEPARATOR, NULL
        };
        read_req->AddToBuffer(obj_sep_buffer);
        --max_rep_ct;
        if (max_rep_ct == 0) {
          pfc_log_debug("max_rep_ct reached zero, so returning");
          return UNC_RC_SUCCESS;
        }
      }
      log_port_exists = PFC_FALSE;
      str_logicalport_id =
          reinterpret_cast<char *>((*logicalport_iter).port_id);
      int st_child_index = KIdxLogicalMemberPort;
      pfc_log_debug("child_index=%d st_child_index=%d",
                    child_index, st_child_index);
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
        UncRespCode ch_read_status = child[kIdx]->ReadBulk(
            db_conn, child_key_struct,
            data_type,
            max_rep_ct,
            -1,
            true,
            is_read_next,
            read_req);
        pfc_log_debug("ReadBulk response from child %d is %d",
                      kIdx, ch_read_status);
        delete child[kIdx];
        child[kIdx] = NULL;
        FreeChildKeyStruct(kIdx, child_key_struct);
        if (max_rep_ct == 0) {
          pfc_log_debug("max_rep_ct reached zero, so returning");
          return UNC_RC_SUCCESS;
        }
      }
      // reset child index
      child_index = -1;
    }
  } else if (read_status == UNC_UPPL_RC_ERR_DB_ACCESS) {
    pfc_log_debug("KtLogicalPort ReadBulk - Returning DB Access Error");
    return read_status;
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
        db_conn, reinterpret_cast<void *>(&nextkin_key_struct),
        data_type,
        max_rep_ct,
        0,
        false,
        is_read_next,
        read_req);
    pfc_log_debug("read status from next kin Kt_Domain is %d",
                  read_status);
    return UNC_RC_SUCCESS;
  }
  pfc_log_debug("logical port - reached end of table");
  pfc_log_debug("read_status=%d", read_status);
  if (read_status == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE) {
    read_status = UNC_RC_SUCCESS;
  }
  return read_status;
}

/**
 * @Description : This function reads the max_rep_ct number of instances of
 *                the UNC_KT_LOGICAL_PORT
 * @param[in]   : key_struct - void pointer to be cast into logical port key
 *                type
 *                val_struct - void pointer to be cast into logical port value
 *                type
 *                data_type - indicates the data base type i.e UNC_DT_STATE
 *                or UNC_DT_IMPORT
 *                max_rep_ct - specifies the maximum repetition count for
 *                read bulk operation
 *                vect_val_logical_port - vector to store the value structure
 *                of logical port
 *                vect_logical_port_id - vector to store the logical port id
 *                of logical port
 * @return      : It returns the read bulk operation status in logical port
 *                table
 *                UNC_RC_SUCCESS - if read operation is success
 *                UNC_UPPL_RC_ERR_* - if read operation is a failure
 **/
UncRespCode Kt_LogicalPort::ReadBulkInternal(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t data_type,
    uint32_t max_rep_ct,
    vector<val_logical_port_st_t> &vect_val_logical_port,
    vector<key_logical_port_t> &vect_logical_port_id) {
  if (max_rep_ct <= 0) {
    return UNC_RC_SUCCESS;
  }
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode read_status = UNC_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;
  DBTableSchema kt_logicalport_dbtableschema;
  // Populate DBSchema for logicalport_table
  void* old_value_struct;
  vector<ODBCMOperator> vect_key_operations;
  PopulateDBSchemaForKtTable(db_conn, kt_logicalport_dbtableschema,
                             key_struct,
                             val_struct,
                             UNC_OP_READ_BULK, data_type, 0, 0,
                             vect_key_operations, old_value_struct,
                             NOTAPPLIED, false, PFC_FALSE);
  // Read rows from DB
  read_db_status = physical_layer->get_odbc_manager()-> \
      GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                  kt_logicalport_dbtableschema,
                  (unc_keytype_operation_t)UNC_OP_READ_BULK, db_conn);
  if (read_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
    pfc_log_debug("No record found");
    read_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
    return read_status;
  } else if (read_db_status == ODBCM_RC_CONNECTION_ERROR) {
    read_status = UNC_UPPL_RC_ERR_DB_ACCESS;
    pfc_log_error("Read operation has failed with error %d", read_db_status);
    return read_status;
  } else if (read_db_status != ODBCM_RC_SUCCESS) {
    read_status = UNC_UPPL_RC_ERR_DB_GET;
    // log error to log daemon
    pfc_log_error("Read operation has failed with error %d", read_db_status);
    return read_status;
  }
  // From the values received from DB, populate val_logical_port structure
  FillLogicalPortValueStructure(db_conn, kt_logicalport_dbtableschema,
                                vect_val_logical_port,
                                max_rep_ct,
                                UNC_OP_READ_BULK,
                                vect_logical_port_id);
  return read_status;
}

/**
 * @Description : This function performs syntax validation of logical port key
 *                type of the request received
 * @param[in]   : key_struct - void pointer to be cast into logical port key
 *                type
 *                val_struct - void pointer to be cast into logical port value
 *                type
 *                operation - indicates the type of operation to be performed
 *                            that can be UNC_OP_CREATE or UNC_OP_DELETE etc
 *                data_type - indicates the data base type i.e UNC_DT_STATE
 *                            or UNC_DT_IMPORT
 * @return      : Return code of the syntax validation will be sent
 *                UNC_RC_SUCCESS - if Syntax Validation is success
 *                UNC_UPPL_RC_ERR_CFG_SYNTAX - if syntax validation fails
 * */
UncRespCode Kt_LogicalPort::PerformSyntaxValidation(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t operation,
    uint32_t data_type) {
  UncRespCode ret_code = UNC_RC_SUCCESS;
  pfc_ipcresp_t mandatory = PFC_TRUE;
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map =
      attr_syntax_map_all[UNC_KT_LOGICAL_PORT];
  // Validate key structure
  key_logical_port *key = reinterpret_cast<key_logical_port_t*>(key_struct);
  string value =
      reinterpret_cast<char*>(key->domain_key.ctr_key.controller_name);
  IS_VALID_STRING_KEY(CTR_NAME_STR, value, operation, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }

  value = reinterpret_cast<char*>(key->domain_key.domain_name);
  IS_VALID_STRING_KEY(DOMAIN_NAME_STR, value, operation, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }

  value = reinterpret_cast<char*>(key->port_id);
  IS_VALID_STRING_KEY(LP_PORT_ID_STR, value, operation, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }
  // Validate value structure
  if (val_struct != NULL) {
    unsigned int valid_val = 0;
    val_logical_port *val_lp =
        reinterpret_cast<val_logical_port_t*>(val_struct);

    // validate description
    valid_val = PhyUtil::uint8touint(val_lp->valid[kIdxLogicalPortDescription]);
    string value = reinterpret_cast<char*>(val_lp->description);
    IS_VALID_STRING_VALUE(LP_DESCRIPTION_STR, value, operation,
                          valid_val, ret_code, mandatory);
    if (ret_code != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }

    // Validate port_type
    valid_val = PhyUtil::uint8touint(val_lp->valid[kIdxLogicalPortType]);
    if (valid_val == UNC_VF_VALID) {
      ret_code = ValidatePortType(val_lp->port_type);
      if (ret_code != UNC_RC_SUCCESS) {
        return UNC_UPPL_RC_ERR_CFG_SYNTAX;
      }
    }

    // validate switch_id
    value = reinterpret_cast<char*>(val_lp->switch_id);
    valid_val = PhyUtil::uint8touint(val_lp->valid[kIdxLogicalPortSwitchId]);
    IS_VALID_STRING_VALUE(LP_SWITCH_ID_STR, value, operation,
                          valid_val, ret_code, mandatory);
    if (ret_code != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }

    // validate physical port id
    valid_val = PhyUtil::uint8touint(val_lp->
                                     valid[kIdxLogicalPortPhysicalPortId]);
    value = reinterpret_cast<char*>(val_lp->physical_port_id);
    IS_VALID_STRING_VALUE(LP_PHYSICAL_PORT_ID_STR, value, operation,
                          valid_val, ret_code, mandatory);

    // Validate oper_down_criteria
    valid_val = PhyUtil::uint8touint(val_lp->
                                     valid[kIdxLogicalPortOperDownCriteria]);
    IS_VALID_INT_VALUE(LP_OPER_DOWN_CRITERIA_STR, val_lp->oper_down_criteria,
                       operation, valid_val, ret_code, mandatory);
    if (ret_code != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }
  }
  return ret_code;
}

/**
 * @Description : This function performs semantic validation of the request
 *                received
 * @param[in]   : key_struct - void pointer to be cast into logical port key
 *                type
 *                val_struct - void pointer to be cast into logical port value
 *                type
 *                operation - specifies the operation to be performed
 *                            that can be UNC_OP_CREATE orn UNC_OP_DELETE etc
 *                data_type - indicates the data base type i.e UNC_DT_STATE
 *                            or UNC_DT_IMPORT
 * @return      : return code of the semantic validation will be returned
 *                UNC_RC_SUCCESS - if Semantic Validation is success
 *                UNC_UPPL_RC_ERR_* - if semantic validation is failure
 **/
UncRespCode Kt_LogicalPort::PerformSemanticValidation(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t operation,
    uint32_t data_type) {
  pfc_log_trace("Inside PerformSemanticValidation of KT_LOGICAL_PORT");
  UncRespCode status = UNC_RC_SUCCESS;
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

  UncRespCode KeyStatus = IsKeyExists(
      db_conn, (unc_keytype_datatype_t)data_type,
      sw_vect_key_value);
  pfc_log_debug("IsKeyExists status %d", KeyStatus);

  if (operation == UNC_OP_CREATE) {
    if (KeyStatus == UNC_RC_SUCCESS) {
      pfc_log_error("Key exists,CREATE not allowed");
      status = UNC_UPPL_RC_ERR_INSTANCE_EXISTS;
    } else {
      pfc_log_debug("key instance not exist create operation allowed");
    }
  } else if (operation == UNC_OP_UPDATE || operation == UNC_OP_DELETE ||
      operation == UNC_OP_READ) {
    // In case of update/delete/read operation, key should exist
    if (KeyStatus != UNC_RC_SUCCESS) {
      pfc_log_error("Key not found,U/D/R opern not allowed");
      status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
    } else {
      pfc_log_debug(
          "key instance exist update/del/read operation allowed");
    }
  }
  if (operation == UNC_OP_CREATE && status == UNC_RC_SUCCESS) {
    vector<string> parent_vect_key_value;
    parent_vect_key_value.push_back(controller_name);
    parent_vect_key_value.push_back(domain_name);
    Kt_Ctr_Domain KtObj;
    UncRespCode parent_key_status = KtObj.IsKeyExists(
        db_conn, (unc_keytype_datatype_t)data_type, parent_vect_key_value);
    pfc_log_debug("Parent IsKeyExists status %d", parent_key_status);
    if (parent_key_status != UNC_RC_SUCCESS) {
      status = UNC_UPPL_RC_ERR_PARENT_DOES_NOT_EXIST;
    }
  }
  pfc_log_debug("Return Code SemanticValidation: %d", status);
  return status;
}

/**
 * @Description   : This function processes the alarm notification
 *                  sent by driver for logical port key type
 * @param[in]     : data_type - indicates the data base type i.e UNC_DT_STATE
 *                              or UNC_DT_IMPORT
 *                  alarm type - indicates the alarm type sent by the driver
 *                  oper_type - indicates the operation type i.e. UNC_OP_CREATE
 *                  key_struct - void pointer to be type cast to logical port
 *                  key type
 *                  value_struct - void pointer to be type cast into logical
 *                  port value structure type
 * @return        : the oper status result from db will be returned
 *                  UNC_RC_SUCCESS - in case oper status is received from DB
 *                  UNC_UPPL_RC_ERR_* - if unable to receive oper status from DB
 **/
UncRespCode Kt_LogicalPort::HandleDriverAlarms(
    OdbcmConnectionHandler *db_conn,
    uint32_t data_type,
    uint32_t alarm_type,
    uint32_t oper_type,
    void* key_struct,
    void* val_struct) {
  UncRespCode status = UNC_RC_SUCCESS;
  // Following alarms are sent for kt_logicalport
  if (alarm_type != UNC_SUBDOMAIN_SPLIT) {
    pfc_log_info("%d alarm received for logical port is ignored", alarm_type);
    return UNC_UPPL_RC_ERR_BAD_REQUEST;
  }
  uint8_t oper_status_db = 0;
  UncRespCode read_status = GetOperStatus(db_conn, data_type,
                                             key_struct,
                                             oper_status_db);
  if (read_status != UNC_RC_SUCCESS) {
    pfc_log_info("Unable to get current oper_status from db");
    return UNC_RC_SUCCESS;
  }
  if (oper_status_db == UPPL_LOGICAL_PORT_OPER_UNKNOWN) {
    pfc_log_info("Logical Port oper is unknown, not processing alarm");
    return UNC_RC_SUCCESS;
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
  pfc_log_debug("Oper_status received from db: %d"
                " Oper_status to be set to  db: %d"
                , old_oper_status, new_oper_status);
  if (new_oper_status == old_oper_status) {
    pfc_log_debug("Old and new oper status are same, so do nothing");
    return UNC_RC_SUCCESS;
  }
  if (new_oper_status == UPPL_LOGICAL_PORT_OPER_UP) {
    GetOperStatusFromOperDownCriteria(db_conn, data_type,
                                      key_struct,
                                      val_struct,
                                      new_oper_status);
  }
  pfc_log_info("Oper_status to be set to  db: %d", new_oper_status);
  if (new_oper_status == old_oper_status) {
    pfc_log_debug("Old and new oper status are same, so do nothing");
    return UNC_RC_SUCCESS;
  }
  // Set oper_status update in DB
  status = SetOperStatus(db_conn, data_type, key_struct, NULL, new_oper_status);
  pfc_log_debug("Update oper_status return: %d", status);
  if (status == UNC_RC_SUCCESS) {
    // Call NotifyOperStatus of referred key types
    key_logical_port_t *key_lp = reinterpret_cast<key_logical_port_t*>
    (key_struct);
    string controller_name =
        (const char*)(key_lp->domain_key.ctr_key.controller_name);
    vector<OperStatusHolder> ref_oper_status;
    GET_ADD_CTRL_OPER_STATUS(controller_name, ref_oper_status,
                             data_type, db_conn);
    ADD_LP_PORT_OPER_STATUS(*key_lp,
                            new_oper_status, ref_oper_status);
    status = NotifyOperStatus(db_conn, data_type,
                              key_struct, val_struct, ref_oper_status);
    pfc_log_debug("Notify oper_status return: %d", status);
    ClearOperStatusHolder(ref_oper_status);
  }
  return status;
}


/**
 * @Description : This function handles the oper status changes in logical port
 * @param[in]   : data_type - indicates the data base type i.e UNC_DT_STATE
 *                            or UNC_DT_IMPORT
 *                key_struct - void pointer to be cast into logical port key
 *                type
 *                value struct - void pointer to be cast into logical port
 *                value type
 * @return      : UNC_RC_SUCCESS - if oper status update in db is successful
 *                UNC_UPPL_RC_ERR_* - if oper status update in db is failure
 **/
UncRespCode Kt_LogicalPort::HandleOperStatus(
    OdbcmConnectionHandler *db_conn,
    uint32_t data_type,
    void *key_struct,
    void *value_struct,
    vector<OperStatusHolder> &ref_oper_status,
    unc_key_type_t caller_kt) {
  UncRespCode return_code = UNC_RC_SUCCESS;
  pfc_log_debug("LogicalPort Handle Oper Status Enter");

  if (key_struct == NULL) {
    pfc_log_debug("Key struct is not available in request");
    return_code = UNC_UPPL_RC_ERR_BAD_REQUEST;
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
  string log_port_id =
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
  memset(&obj_val_logical_port_st, '\0', sizeof(obj_val_logical_port_st));
  stringstream ss;
  ss << "Handle operstatus ctr_name:" << controller_name
      << "\tdomain_name:" << domain_name
      << "\tlogical_port_id:" << log_port_id
      << "\tswitch_id:" << switch_id
      << "\tphy_port_id:" << phy_port_id;
  string strName = ss.str();
  pfc_log_debug("Handle operstatus ip vals:%s", strName.c_str());
  vector<key_logical_port_t> vectLogicalPortKey;
  // Get the controller's oper status and decide on the oper_status
  uint8_t ctrl_oper_status = 0;
  key_ctr_t ctr_key;
  memset(&ctr_key, '\0', sizeof(key_ctr_t));
  memcpy(ctr_key.controller_name, controller_name.c_str(),
         (controller_name.length())+1);
  UncRespCode read_oper_status = get_oper_status(
      ref_oper_status,
      UNC_KT_CONTROLLER,
      reinterpret_cast<void*>(&ctr_key),
      ctrl_oper_status);
  if (read_oper_status != UNC_RC_SUCCESS) {
    Kt_Controller controller;
    UncRespCode read_status = controller.GetOperStatus(
        db_conn, data_type, reinterpret_cast<void*>(&ctr_key),
        ctrl_oper_status);
    if (read_status != UNC_RC_SUCCESS) {
      pfc_log_info("Controller's oper_status read returned failure");
      return return_code;
    }
  }
  UpplLogicalPortOperStatus logical_port_oper_status =
      UPPL_LOGICAL_PORT_OPER_UNKNOWN;
  if (ctrl_oper_status ==
      (UpplControllerOperStatus) UPPL_CONTROLLER_OPER_DOWN) {
    // set oper status to unknown
    logical_port_oper_status = UPPL_LOGICAL_PORT_OPER_UNKNOWN;
    return_code = SetOperStatus(
        db_conn, data_type,
        key_struct,
        reinterpret_cast<void*>(&obj_val_logical_port_st),
        logical_port_oper_status);
    // Notify referred classes
    return_code = NotifyOperStatus(db_conn, data_type, key_struct,
                                   NULL, ref_oper_status);
    return return_code;
  }
  pfc_log_debug("Controller's oper_status %d", ctrl_oper_status);
  logical_port_oper_status = UPPL_LOGICAL_PORT_OPER_UP;
  uint8_t switch_oper_status = 0;
  if (!switch_id.empty()) {
    key_switch_t switch_key;
    memset(&switch_key, 0, sizeof(switch_key));
    memcpy(switch_key.ctr_key.controller_name, controller_name.c_str(),
           (controller_name.length())+1);
    pfc_log_debug("switch key %s", switch_id.c_str());
    memcpy(switch_key.switch_id, switch_id.c_str(),
           (switch_id.length())+1);
    read_oper_status = get_oper_status(ref_oper_status,
                                       UNC_KT_SWITCH,
                                       reinterpret_cast<void*>(&switch_key),
                                       switch_oper_status);
    UncRespCode read_status = UNC_RC_SUCCESS;
    if (read_oper_status != UNC_RC_SUCCESS) {
      // get switch oper status:
      logical_port_oper_status =
          UPPL_LOGICAL_PORT_OPER_UNKNOWN;
      Kt_Switch switch_obj;
      read_status = switch_obj.GetOperStatus(
          db_conn, data_type,
          reinterpret_cast<void*>(&switch_key), switch_oper_status);
    }
    if (read_oper_status == UNC_RC_SUCCESS ||
        read_status == UNC_RC_SUCCESS) {
      pfc_log_debug("switch oper_status %d", switch_oper_status);
      if (switch_oper_status ==
          (UpplSwitchOperStatus) UPPL_SWITCH_OPER_UP) {
        logical_port_oper_status = UPPL_LOGICAL_PORT_OPER_UP;
      } else if (switch_oper_status ==
          (UpplSwitchOperStatus) UPPL_SWITCH_OPER_DOWN) {
        logical_port_oper_status = UPPL_LOGICAL_PORT_OPER_DOWN;
      }
    } else {
      pfc_log_error("switch oper_status read returned failure");
    }
  }

  // check oper down criteria
  pfc_log_debug("iterate all logical port in handle oper status");
  // Get all logical ports associated with controller and operdowncriteria
  GetAllLogicalPort(db_conn, controller_name, domain_name, switch_id,
                    phy_port_id, vectLogicalPortKey, data_type);
  vector<key_logical_port_t>::iterator keyItr =
      vectLogicalPortKey.begin();
  for (; keyItr != vectLogicalPortKey.end();) {
    // handle operstatus criteria
    string port_id = (const char*)((*keyItr).port_id);
    if (!log_port_id.empty() && log_port_id != port_id) {
      keyItr = vectLogicalPortKey.erase(keyItr);
      continue;
    }
    pfc_log_debug("Port_Id: %s", port_id.c_str());
    unsigned trunk_port = port_id.find("TP-");
    unsigned sd_port = port_id.find("SD-");
    unsigned pg_port = port_id.find("PG-");
    unsigned mg_port = port_id.find("MG-");
    if (trunk_port == 0 || sd_port == 0 || pg_port == 0 || mg_port == 0) {
      pfc_log_debug("Call oper down criteria function");
      return_code = GetOperStatusFromOperDownCriteria(
          db_conn, data_type,
          reinterpret_cast<void *> (&(*keyItr)),
          value_struct,
          logical_port_oper_status);
      if (return_code != UNC_RC_SUCCESS) {
        ++keyItr;
        continue;
      }
      pfc_log_debug("Handle logical port oper status val: %d",
                    logical_port_oper_status);
    } else if (caller_kt != UNC_KT_SWITCH) {
      // Check oper_status of associated port
      uint8_t port_oper_status = 0;
      UncRespCode port_status = UNC_RC_SUCCESS;
      key_port_t obj_key_port;
      memset(&obj_key_port, '\0', sizeof(key_port_t));
      memcpy(obj_key_port.sw_key.ctr_key.controller_name,
             controller_name.c_str(),
             (controller_name.length())+1);
      memcpy(obj_key_port.sw_key.switch_id,
             switch_id.c_str(),
             (switch_id.length())+1);
      memcpy(obj_key_port.port_id,
             phy_port_id.c_str(),
             (phy_port_id.length())+1);
      read_oper_status = get_oper_status(ref_oper_status,
                                         UNC_KT_PORT,
                                         reinterpret_cast<void*>(&obj_key_port),
                                         port_oper_status);
      if (read_oper_status != UNC_RC_SUCCESS) {
        port_status = GetPortOperStatus(
            db_conn, obj_key_port,
            &port_oper_status, data_type);
      }
      if ((read_oper_status == UNC_RC_SUCCESS ||
          port_status == UNC_RC_SUCCESS) &&
          port_oper_status == UPPL_PORT_OPER_DOWN) {
        logical_port_oper_status = UPPL_LOGICAL_PORT_OPER_DOWN;
      }
    }
    pfc_log_debug("Set logical port oper status: %d",
                  logical_port_oper_status);
    return_code = SetOperStatus(db_conn, data_type,
                                reinterpret_cast<void *> (&(*keyItr)), NULL,
                                logical_port_oper_status);
    pfc_log_debug("Set logical port oper return code: %d",
                  return_code);
    ++keyItr;
  }
  // notify operstatus using logical port siblings as key
  if (!vectLogicalPortKey.empty()) {
    vector<key_logical_port_t>::iterator keyItr =
        vectLogicalPortKey.begin();
    for (; keyItr != vectLogicalPortKey.end(); ++keyItr) {
      if (return_code == UNC_RC_SUCCESS) {
        // Call referred classes' notify oper_status functions
        key_logical_port_t key_lp = (*keyItr);
        ADD_LP_PORT_OPER_STATUS(key_lp,
                                logical_port_oper_status,
                                ref_oper_status);
        return_code = NotifyOperStatus(
            db_conn, data_type, reinterpret_cast<void *> (&(*keyItr)),
            NULL, ref_oper_status);
        pfc_log_debug("logicalport Notify Operstatus ret: %d", return_code);
      } else {
        pfc_log_error("operstatus update failure");
      }
    }
  } else {
    pfc_log_debug("Logicalport sibling count is 0");
  }
  pfc_log_debug("LogicalPort Handle Oper Status End");
  return return_code;
}

/**
 * @Description : This function retrieves the oper status of port based on
 *                operdown criteria
 * @param[in]   : data_type - indicates the data base type i.e UNC_DT_STATE
 *                            or UNC_DT_IMPORT
 *                key_struct - void pointer to be cast into logical port key
 *                type
 *                value struct - void pointer to be cast into logical port
 *                value type
 *                logical_port_oper_status - indicates the logical port oper
 *                status i.e. either UP or DOWN
 * @return      : UNC_RC_SUCCESS - if port oper status is retrieved correctly
 *                UNC_UPPL_RC_ERR_* - if port oper status cannot be retrieved
 **/
UncRespCode Kt_LogicalPort::GetOperStatusFromOperDownCriteria(
    OdbcmConnectionHandler *db_conn,
    uint32_t data_type,
    void *key_struct,
    void *value_struct,
    UpplLogicalPortOperStatus &logical_port_oper_status) {
  uint32_t oper_down_criteria;
  vector<uint32_t> vectOperStatus;
  // get port oper status,PFC_FALSE as call is not from deletekeyinstance
  pfc_log_debug("Enter GetOperStatusFromOperDownCriteria");
  HandleOperDownCriteriaFromPortStatus(db_conn, data_type, key_struct,
                                       value_struct,
                                       vectOperStatus, PFC_FALSE);
  // get oper down from DB
  UncRespCode return_code = UNC_RC_SUCCESS;
  return_code = GetOperDownCriteria(db_conn, data_type,
                                    key_struct, oper_down_criteria);
  if (return_code != UNC_RC_SUCCESS) {
    pfc_log_info("Logicalport OperStatusFromOperDownCriteria read error");
    return return_code;
  }
  pfc_log_debug("vectOperStatus size is %"PFC_PFMT_SIZE_T,
                 vectOperStatus.size());
  // Check for vectOperStatus.size,
  // if it is empty, make logical port operstatus as down
  if (vectOperStatus.size() == 0) {
    logical_port_oper_status = UPPL_LOGICAL_PORT_OPER_DOWN;
  }

  // oper_down_criteria is ALL
  if (oper_down_criteria == (UpplLogicalPortOperDownCriteria)
      UPPL_OPER_DOWN_CRITERIA_ALL) {  // Trunk Port or Port Group port
    vector<uint32_t>::iterator operstatus_itr = vectOperStatus.begin();
    // iterate on the port oper status values
    for (; operstatus_itr != vectOperStatus.end(); ++operstatus_itr) {
      // When any one of the ports in trunk port is up
      // logical port oper_status will be set to UP
      // If all ports are down, then logical port is down
      if ((*operstatus_itr) == 0 || (*operstatus_itr) == 2) {
        logical_port_oper_status = UPPL_LOGICAL_PORT_OPER_DOWN;
      } else if ((*operstatus_itr) == 1) {
        logical_port_oper_status = UPPL_LOGICAL_PORT_OPER_UP;
        break;
      }
    }
    pfc_log_debug("Logical port OperStatusFromOperDownCriteria status: %d",
                  logical_port_oper_status);
  } else if (oper_down_criteria == (UpplLogicalPortOperDownCriteria)
      UPPL_OPER_DOWN_CRITERIA_ANY) {  // Sub Domain
    // oper_down_criteria is ANY
    vector<uint32_t>::iterator operstatus_itr = vectOperStatus.begin();
    // iterate on the port oper status values
    for (; operstatus_itr != vectOperStatus.end(); ++operstatus_itr) {
      // When any one of the ports in subdomain is down,
      // logical port oper_status will be set to down
      // If only all ports are up, subdomain is up
      if ((*operstatus_itr) == 1) {
        logical_port_oper_status = UPPL_LOGICAL_PORT_OPER_UP;
      } else if ((*operstatus_itr) == 0 || (*operstatus_itr) == 2) {
        logical_port_oper_status = UPPL_LOGICAL_PORT_OPER_DOWN;
        break;
      }
    }
    pfc_log_debug("Logicalport OperStatusFromOperDownCriteria Any status:%d",
                  logical_port_oper_status);
  } else {
    return_code = UNC_UPPL_RC_ERR_BAD_REQUEST;
  }
  pfc_log_debug("Exit logical port OperStatusFromOperDownCriteria");
  return return_code;
}

/**
 * @Description : This function retrieves the oper status of port based on
 *                logical member port key values
 * @param[in]   : data_type - indicates the data base type i.e UNC_DT_STATE
 *                            or UNC_DT_IMPORT
 *                key_struct - void pointer to be cast into logical port key
 *                type
 *                valu_struct - void pointer to be cast into logical port
 *                value type
 *                vectOperStatus - vector to hold the oper status of port
 *                is_delete_call - bool variable to check whether delete call
 *                                 is madwe by the parent class
 * @return      : UNC_RC_SUCCESS - if port oper status is read correctly
 *                UNC_UPPL_RC_ERR_* - if port oper status read returned failure
 **/
UncRespCode Kt_LogicalPort::HandleOperDownCriteriaFromPortStatus(
    OdbcmConnectionHandler *db_conn,
    uint32_t data_type,
    void *key_struct,
    void *value_struct,
    vector<uint32_t> &vectOperStatus,
    pfc_bool_t is_delete_call) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode return_code = UNC_RC_SUCCESS;

  pfc_log_debug("Entering Logical port Handle OperDownCriteriaFromPortStatus");
  if (key_struct == NULL) {
    return_code = UNC_UPPL_RC_ERR_BAD_REQUEST;
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
  string switch_id;
  string physical_port_id;
  pfc_log_debug("Handle operdowncriteria ip vals: %s", strName.c_str());
  // Call referred classes' notify oper_status functions
  // Get all domains associated with controller
  while (true) {
    DBTableSchema kt_logicalport_dbtableschema;
    vector<string> vect_prim_keys;
    vector<TableAttrSchema> vect_table_attr_schema;
    list< vector<TableAttrSchema> > row_list;
    vect_prim_keys.push_back(CTR_NAME_STR);
    vect_prim_keys.push_back(DOMAIN_NAME_STR);
    PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                          controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
    PhyUtil::FillDbSchema(unc::uppl::DOMAIN_NAME, domain_name,
                          domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
    vect_prim_keys.push_back(LP_PORT_ID_STR);
    PhyUtil::FillDbSchema(unc::uppl::LMP_LP_PORT_ID, port_id,
                          port_id.length(), DATATYPE_UINT8_ARRAY_320,
                          vect_table_attr_schema);
    vect_prim_keys.push_back(LP_SWITCH_ID_STR);
    PhyUtil::FillDbSchema(unc::uppl::LMP_SWITCH_ID, switch_id,
                          switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                          vect_table_attr_schema);
    PhyUtil::FillDbSchema(unc::uppl::LMP_PHYSICAL_PORT_ID, physical_port_id,
                          physical_port_id.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);

    kt_logicalport_dbtableschema.set_table_name(
        unc::uppl::LOGICAL_MEMBERPORT_TABLE);
    kt_logicalport_dbtableschema.set_primary_keys(vect_prim_keys);

    row_list.push_back(vect_table_attr_schema);
    kt_logicalport_dbtableschema.set_row_list(row_list);

    pfc_log_debug("Iterate to get Switch and Port details");

    // Get number of instances based on member_ports_count
    ODBCM_RC_STATUS db_status = physical_layer->get_odbc_manager()->
        GetBulkRows((unc_keytype_datatype_t)data_type,
                    UPPL_MAX_REP_CT,
                    kt_logicalport_dbtableschema,
                    (unc_keytype_operation_t)UNC_OP_READ_SIBLING_BEGIN,
                    db_conn);
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
        switch (tab_att_schema.table_attribute_name) {
          case unc::uppl::CTR_NAME:
            PhyUtil::GetValueFromDbSchemaStr(
                tab_att_schema,
                logical_memeber_port.logical_port_key.domain_key.\
                ctr_key.controller_name,
                DATATYPE_UINT8_ARRAY_32);
            break;
          case unc::uppl::LMP_SWITCH_ID:
            PhyUtil::GetValueFromDbSchemaStr(tab_att_schema,
                                             logical_memeber_port.switch_id,
                                             DATATYPE_UINT8_ARRAY_256);
            switch_id = (const char*)logical_memeber_port.switch_id;
            break;
          case unc::uppl::LMP_PHYSICAL_PORT_ID:
            PhyUtil::GetValueFromDbSchemaStr(
                tab_att_schema,
                logical_memeber_port.physical_port_id,
                DATATYPE_UINT8_ARRAY_32);
            physical_port_id =
                (const char*)logical_memeber_port.physical_port_id;
            break;
          case unc::uppl::LMP_LP_PORT_ID:
            uint8_t lp_id[ODBCM_SIZE_320];
            PhyUtil::GetValueFromDbSchemaStr(tab_att_schema, lp_id,
                                             DATATYPE_UINT8_ARRAY_320);
            port_id = reinterpret_cast<const char*> (lp_id);
            break;
          default:
            break;
        }
      }

      UncRespCode read_status = UNC_RC_SUCCESS;
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
        memset(&obj_val_port, 0, sizeof(obj_val_port));

        string logical_port_id = (const char*)obj_key_logical_port->port_id;
        memcpy(obj_val_port.logical_port_id,
               logical_port_id.c_str(),
               (logical_port_id.length())+1);
        obj_val_port.valid[kIdxPortLogicalPortId] = UNC_VF_VALID;

        read_status = port.UpdatePortValidFlag(
            db_conn, reinterpret_cast<void*>(&obj_key_port),
            reinterpret_cast<void*>(&obj_val_port),
            obj_val_port,
            UNC_VF_INVALID, data_type);
        continue;
      }
      // get the port oper_status
      uint8_t port_oper_status = 0;

      pfc_log_debug("Logicalport get CriteriaFromPortStatus");
      read_status = port.GetOperStatus(
          db_conn, data_type, reinterpret_cast<void*>(&obj_key_port),
          port_oper_status);
      if (read_status == UNC_RC_SUCCESS) {
        pfc_log_debug("port oper_status in logport: %d", port_oper_status);
        vectOperStatus.push_back(port_oper_status);
      } else {
        pfc_log_info("port oper_status read returned failure");
      }
      return_code = read_status;
    }
    if (kt_logicalport_dbtableschema.row_list_.size() < UPPL_MAX_REP_CT) {
      break;
    }
  }
  pfc_log_debug("Exit Logicalport Handle OperDownCriteriaFromPortStatus");
  return return_code;
}

/**
 * @Description : This function reads the oper down criteria from db
 * @param[in]   : data_type - indicates the data base type i.e UNC_DT_STATE
 *                            or UNC_DT_IMPORT
 *                key_struct - void pointer to be cast into logical port key
 *                type
 *                oper_down_criteria - specifies the oper down criteria
 * @return      : UNC_RC_SUCCESS - if oper down read criteria from db is
 *                success
 *                UNC_UPPL_RC_ERR_* - if oper down read criteria from db is failure
 **/
UncRespCode Kt_LogicalPort::GetOperDownCriteria(
    OdbcmConnectionHandler *db_conn,
    uint32_t data_type,
    void* key_struct,
    uint32_t &oper_down_criteria) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  key_logical_port_t *obj_key_logical_port =
      reinterpret_cast<key_logical_port_t*>(key_struct);
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  vector<string> vect_prim_keys;

  string controller_name = (const char*)obj_key_logical_port->domain_key.
      ctr_key.controller_name;

  if (!controller_name.empty()) {
    vect_prim_keys.push_back(CTR_NAME_STR);
  }

  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  string domain_name = (const char*)obj_key_logical_port->
      domain_key.domain_name;

  if (!domain_name.empty()) {
    vect_prim_keys.push_back(DOMAIN_NAME_STR);
  }

  PhyUtil::FillDbSchema(unc::uppl::DOMAIN_NAME, domain_name,
                        domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  string port_id = (const char*)obj_key_logical_port->port_id;

  if (!port_id.empty()) {
    vect_prim_keys.push_back(LP_PORT_ID_STR);
  }

  PhyUtil::FillDbSchema(unc::uppl::LP_PORT_ID, port_id,
                        port_id.length(), DATATYPE_UINT8_ARRAY_320,
                        vect_table_attr_schema);

  string oper_down_criteria_value;
  PhyUtil::FillDbSchema(unc::uppl::LP_OPER_DOWN_CRITERIA,
                        oper_down_criteria_value,
                        oper_down_criteria_value.length(),
                        DATATYPE_UINT16,
                        vect_table_attr_schema);

  DBTableSchema kt_logicalport_dbtableschema;
  kt_logicalport_dbtableschema.set_table_name(unc::uppl::LOGICALPORT_TABLE);
  kt_logicalport_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_logicalport_dbtableschema.set_row_list(row_list);

  // Call ODBCManager and read
  ODBCM_RC_STATUS read_db_status =
      physical_layer->get_odbc_manager()->GetOneRow(
          (unc_keytype_datatype_t)data_type,
          kt_logicalport_dbtableschema, db_conn);
  if (read_db_status != ODBCM_RC_SUCCESS) {
    // log error
    pfc_log_error("oper down criteria read operation failed");
    return UNC_UPPL_RC_ERR_DB_GET;
  }

  // read the oper_down_criteria value
  list < vector<TableAttrSchema> >& res_logicalport_row_list =
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
      ODBCMTableColumns attr_name = tab_schema.table_attribute_name;
      string attr_value;
      if (attr_name == unc::uppl::LP_OPER_DOWN_CRITERIA) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT16);
        oper_down_criteria = atoi(attr_value.c_str());
        pfc_log_debug("oper_down_criteria: %d", oper_down_criteria);
        break;
      }
    }
  }
  return UNC_RC_SUCCESS;
}

/**
 * @Description : This function invokes the notifyoperstatus of boundary
 * @param[in]   : data_type - indicates the data base type i.e UNC_DT_STATE
 *                            or UNC_DT_IMPORT
 *                key_struct - void pointer to be type cast into logical port
 *                key type
 * @return      : UNC_RC_SUCCESS - if boundary oper status is changed in DB
 *                UNC_UPPL_RC_ERR_* - if there are no boundaries associated with
 *                the controller name
 **/
UncRespCode Kt_LogicalPort::InvokeBoundaryNotifyOperStatus(
    OdbcmConnectionHandler *db_conn,
    uint32_t data_type,
    void *key_struct) {
  UncRespCode return_code = UNC_RC_SUCCESS;
  key_logical_port_t *obj_key_logical_port =
      reinterpret_cast<key_logical_port_t*>(key_struct);

  string controller_name =
      (const char*)obj_key_logical_port->domain_key.ctr_key.controller_name;
  string domain_name =
      (const char*)obj_key_logical_port->domain_key.domain_name;
  string port_id =
      (const char*)obj_key_logical_port->port_id;
  vector<OperStatusHolder> ref_oper_status;
  // Get controller oper status
  GET_ADD_CTRL_OPER_STATUS(controller_name, ref_oper_status,
                          data_type, db_conn);
  uint8_t lp_oper_status = 0;
  read_status = GetOperStatus(
      db_conn, data_type, key_struct, lp_oper_status);
  ADD_LP_PORT_OPER_STATUS(*obj_key_logical_port,
                          lp_oper_status,
                          ref_oper_status);
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
  return_code = boundary.HandleOperStatus(
      db_conn, data_type, NULL, reinterpret_cast<void *> (&obj_val_boundary1),
      ref_oper_status);
  pfc_log_debug("Invoke Notify for boundary class C1D1L1 %d", return_code);

  val_boundary_t obj_val_boundary2;
  memset(&obj_val_boundary2, 0, sizeof(val_boundary_t));
  memset(obj_val_boundary2.valid, 0, 7);
  memcpy(obj_val_boundary2.controller_name2,
         controller_name.c_str(), controller_name.length()+1);
  obj_val_boundary2.valid[kIdxBoundaryControllerName2] = UNC_VF_VALID;
  memcpy(obj_val_boundary2.domain_name2,
         domain_name.c_str(), domain_name.length()+1);
  obj_val_boundary2.valid[kIdxBoundaryDomainName2] = UNC_VF_VALID;
  memcpy(obj_val_boundary2.logical_port_id2,
         port_id.c_str(), port_id.length()+1);
  obj_val_boundary2.valid[kIdxBoundaryLogicalPortId2] = UNC_VF_VALID;
  return_code = boundary.HandleOperStatus(
      db_conn, data_type, NULL, reinterpret_cast<void *> (&obj_val_boundary2),
      ref_oper_status);
  pfc_log_debug("Invoke Notify for boundary class C2D2L2 %d", return_code);
  ClearOperStatusHolder(ref_oper_status);
  return return_code;
}

/**
 * @Description : This function notifies key type boundary when oper status of
 *                logical port changes
 * @param[in]   : data_type - specifies the data base type i.e UNC_DT_STATE
 *                            or UNC_DT_IMPORT
 *                key_struct - void pointer type to be type cast into logical
 *                port key type
 *                value_struct - void pointer type to be type cast into logical
 *                port key type
 * @return      : UNC_RC_SUCCESS - if the oper status notification in key type
 *                boundary is changed successfully
 *                UNC_UPPL_RC_ERR_* - if the oper status notification in key type
 *                boundary fails
 **/
UncRespCode Kt_LogicalPort::NotifyOperStatus(
    OdbcmConnectionHandler *db_conn,
    uint32_t data_type,
    void *key_struct,
    void *value_struct,
    vector<OperStatusHolder> &ref_oper_status) {
  UncRespCode return_code = UNC_RC_SUCCESS;
  key_logical_port_t *obj_key_logical_port =
      reinterpret_cast<key_logical_port_t*>(key_struct);

  string controller_name =
      (const char*)obj_key_logical_port->domain_key.ctr_key.controller_name;
  string domain_name =
      (const char*)obj_key_logical_port->domain_key.domain_name;
  string port_id =
      (const char*)obj_key_logical_port->port_id;
  pfc_log_debug(
      "NotifyOperStatus: controller_name: %s, domain_name: %s, port_id: %s",
      controller_name.c_str(), domain_name.c_str(), port_id.c_str());
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
  if (!port_id.empty()) {
    obj_val_boundary1.valid[kIdxBoundaryLogicalPortId1] = UNC_VF_VALID;
  }
  obj_val_boundary1.valid[kIdxBoundaryControllerName1] = UNC_VF_VALID;
  if (!domain_name.empty()) {
    obj_val_boundary1.valid[kIdxBoundaryDomainName1] = UNC_VF_VALID;
    pfc_log_debug("Domain Name1 is set as Valid");
  }
  return_code = boundary.HandleOperStatus(
      db_conn, data_type, NULL,
      reinterpret_cast<void *> (&obj_val_boundary1), ref_oper_status);
  pfc_log_debug("HandleOperStatus C1D1L1 from boundary returned %d",
               return_code);
  val_boundary_t obj_val_boundary2;
  memset(&obj_val_boundary2, 0, sizeof(val_boundary_t));
  memcpy(obj_val_boundary2.controller_name2,
         controller_name.c_str(),
         (controller_name.length())+1);
  memcpy(obj_val_boundary2.domain_name2,
         domain_name.c_str(),
         (domain_name.length())+1);
  memcpy(obj_val_boundary2.logical_port_id2,
         port_id.c_str(), port_id.length()+1);
  if (!port_id.empty()) {
    obj_val_boundary2.valid[kIdxBoundaryLogicalPortId2] = UNC_VF_VALID;
  }
  obj_val_boundary2.valid[kIdxBoundaryControllerName2] = UNC_VF_VALID;
  if (!domain_name.empty()) {
    obj_val_boundary2.valid[kIdxBoundaryDomainName2] = UNC_VF_VALID;
    pfc_log_debug("Domain Name2 is set as Valid");
  }
  return_code = boundary.HandleOperStatus(
      db_conn, data_type, NULL,
      reinterpret_cast<void *> (&obj_val_boundary2), ref_oper_status);
  pfc_log_debug("HandleOperStatus C2D2L2 from boundary returned %d",
               return_code);
  return return_code;
}

/**
 * @Description : This function gets the oper status of logical port key type
 *                from DB
 * @param[in]   : data_type - indicates the data base type i.e UNC_DT_STATE
 *                            or UNC_DT_IMPORT
 *                key_struct - void pointer type to be type cast into logical
 *                port key type
 *                oper_status - Stores the oper status received from DB
 * @return      : UNC_RC_SUCCESS - if oper status is reads successfully  from
 *                the DB
 *                UNC_UPPL_RC_ERR_DB_GET if oper staus read operation failed from DB
 **/
UncRespCode Kt_LogicalPort::GetOperStatus(OdbcmConnectionHandler *db_conn,
                                             uint32_t data_type,
                                             void* key_struct,
                                             uint8_t &oper_status) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  key_logical_port *obj_key_logical_port =
      reinterpret_cast<key_logical_port_t*>(key_struct);
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  vector<string> vect_prim_keys;
  string controller_name = (const char*)obj_key_logical_port->domain_key.
      ctr_key.controller_name;
  string domain_name = (const char*)obj_key_logical_port->
      domain_key.domain_name;
  string port_id = (const char*)obj_key_logical_port->port_id;
  if (!controller_name.empty()) {
    vect_prim_keys.push_back(CTR_NAME_STR);
    PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                          controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
  }
  if (!domain_name.empty()) {
    vect_prim_keys.push_back(DOMAIN_NAME_STR);
    PhyUtil::FillDbSchema(unc::uppl::DOMAIN_NAME, domain_name,
                          domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
  }
  if (!port_id.empty()) {
    vect_prim_keys.push_back(LP_PORT_ID_STR);
    PhyUtil::FillDbSchema(unc::uppl::LP_PORT_ID, port_id,
                          port_id.length(), DATATYPE_UINT8_ARRAY_320,
                          vect_table_attr_schema);
  }

  string oper_value;
  PhyUtil::FillDbSchema(unc::uppl::LP_OPER_STATUS, oper_value,
                        oper_value.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);

  DBTableSchema kt_logicalport_dbtableschema;
  kt_logicalport_dbtableschema.set_table_name(unc::uppl::LOGICALPORT_TABLE);
  kt_logicalport_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_logicalport_dbtableschema.set_row_list(row_list);

  // Call ODBCManager and update
  ODBCM_RC_STATUS read_db_status =
      physical_layer->get_odbc_manager()->GetOneRow(
          (unc_keytype_datatype_t)data_type,
          kt_logicalport_dbtableschema, db_conn);
  if (read_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
    pfc_log_info("Logical Port is not present in DB");
    oper_status = UPPL_LOGICAL_PORT_OPER_UNKNOWN;
    return UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
  } else if (read_db_status != ODBCM_RC_SUCCESS) {
    // log error
    pfc_log_error("oper_status read operation failed");
    return UNC_UPPL_RC_ERR_DB_GET;
  }

  // read the oper_status value
  list < vector<TableAttrSchema> >& res_logicalport_row_list =
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
      ODBCMTableColumns attr_name = tab_schema.table_attribute_name;
      string attr_value;
      if (attr_name == unc::uppl::LP_OPER_STATUS) {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT16);
        oper_status = atoi(attr_value.c_str());
        pfc_log_debug("oper_status: %d", oper_status);
        break;
      }
    }
  }
  return UNC_RC_SUCCESS;
}

/**
 * @Description : This function updates the oper status in logical port db and
 *                notifies the change to northbound
 * @param[in]   : data_type - specifies the data base type i.e UNC_DT_STATE
 *                            or UNC_DT_IMPORT
 *                key_struct - void pointer type to be type casted in logical
 *                port key type
 *                val_struct - void pointer type to be type cast in logical
 *                port value type
 *                oper_statuus - value of oper status to be updated in DB
 * @return      : Success or associated error code
 **/
UncRespCode Kt_LogicalPort::SetOperStatus(
    OdbcmConnectionHandler *db_conn,
    uint32_t data_type,
    void* key_struct,
    void* val_struct,
    UpplLogicalPortOperStatus oper_status) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  key_logical_port_t *obj_key_logical_port=
      reinterpret_cast<key_logical_port_t*>(key_struct);
  val_logical_port_st_t *obj_val_logical_port=
      reinterpret_cast<val_logical_port_st_t*>(val_struct);
  // Get existing oper_status
  uint8_t old_oper_status = 0;
  UncRespCode read_status = GetOperStatus(db_conn, data_type,
                                             key_struct,
                                             old_oper_status);
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  vector<string> vect_prim_keys;
  string controller_name = (const char*)obj_key_logical_port->domain_key.
      ctr_key.controller_name;
  pfc_log_debug("logical port set oper_status");
  if (!controller_name.empty()) {
    vect_prim_keys.push_back(CTR_NAME_STR);
  }
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
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
    vect_prim_keys.push_back(DOMAIN_NAME_STR);
    PhyUtil::FillDbSchema(unc::uppl::DOMAIN_NAME, domain_name,
                          domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
  }
  if (!port_id.empty()) {
    vect_prim_keys.push_back(LP_PORT_ID_STR);
    PhyUtil::FillDbSchema(unc::uppl::LP_PORT_ID, port_id,
                          port_id.length(), DATATYPE_UINT8_ARRAY_320,
                          vect_table_attr_schema);
  }

  if (!switch_id.empty()) {
    vect_prim_keys.push_back(LP_SWITCH_ID_STR);
    PhyUtil::FillDbSchema(unc::uppl::LP_SWITCH_ID, switch_id,
                          switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                          vect_table_attr_schema);
  }
  if (!phy_port_id.empty()) {
    vect_prim_keys.push_back(LP_PHYSICAL_PORT_ID_STR);
    PhyUtil::FillDbSchema(unc::uppl::LP_PHYSICAL_PORT_ID, phy_port_id,
                          phy_port_id.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
  }

  string oper_value = PhyUtil::uint8tostr(oper_status);
  PhyUtil::FillDbSchema(unc::uppl::LP_OPER_STATUS, oper_value,
                        oper_value.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);
  string valid_flag;
  // Update valid flag as well
  UncRespCode get_ret = GetValidFlag(db_conn, data_type,
                                        key_struct, &valid_flag);
  if (get_ret == UNC_RC_SUCCESS) {
    string new_valid = valid_flag.substr(0, 5);
    new_valid += "1";
    pfc_log_debug("New Valid to be set: %s", new_valid.c_str());
    PhyUtil::FillDbSchema(unc::uppl::LP_CTR_VALID, new_valid,
                          new_valid.length(), DATATYPE_UINT8_ARRAY_6,
                          vect_table_attr_schema);
  }

  DBTableSchema kt_logicalport_dbtableschema;
  kt_logicalport_dbtableschema.set_table_name(unc::uppl::LOGICALPORT_TABLE);
  kt_logicalport_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_logicalport_dbtableschema.set_row_list(row_list);

  // Call ODBCManager and update
  ODBCM_RC_STATUS update_db_status =
      physical_layer->get_odbc_manager()->UpdateOneRow(
          (unc_keytype_datatype_t)data_type,
          kt_logicalport_dbtableschema, db_conn, true);
  if (update_db_status == ODBCM_RC_ROW_NOT_EXISTS) {
    pfc_log_debug("No instance available for update");
  } else if (update_db_status != ODBCM_RC_SUCCESS) {
    // log error
    pfc_log_error("oper_status update operation failed");
    return UNC_UPPL_RC_ERR_DB_UPDATE;
  } else if (old_oper_status != oper_status) {
    // Notify operstatus change to northbound
    if (read_status == UNC_RC_SUCCESS && data_type != UNC_DT_IMPORT) {
      val_logical_port_st old_val_logical_port, new_val_logical_port;
      memset(&old_val_logical_port, 0, sizeof(val_logical_port_st));
      memset(&new_val_logical_port, 0, sizeof(val_logical_port_st));
      old_val_logical_port.oper_status = old_oper_status;
      old_val_logical_port.valid[kIdxLogicalPortStOperStatus] = UNC_VF_VALID;
      new_val_logical_port.oper_status = oper_status;
      new_val_logical_port.valid[kIdxLogicalPortStOperStatus] = UNC_VF_VALID;
      int err = 0;
      // Send notification to Northbound
      ServerEvent ser_evt((pfc_ipcevtype_t)UPPL_EVENTS_KT_LOGICAL_PORT, err);
      northbound_event_header rsh = {UNC_OP_UPDATE,
          data_type,
          UNC_KT_LOGICAL_PORT};
      err = PhyUtil::sessOutNBEventHeader(ser_evt, rsh);
      err |= ser_evt.addOutput(*obj_key_logical_port);
      err |= ser_evt.addOutput(new_val_logical_port);
      err |= ser_evt.addOutput(old_val_logical_port);
      if (err == 0) {
        pfc_log_debug("%s",
                      (IpctUtil::get_string(*obj_key_logical_port)).c_str());
        pfc_log_debug("%s",
                      (IpctUtil::get_string(new_val_logical_port)).c_str());
        pfc_log_debug("%s",
                      (IpctUtil::get_string(old_val_logical_port)).c_str());
        PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
        // Notify operstatus modifications
        UncRespCode status = (UncRespCode) physical_layer
            ->get_ipc_connection_manager()->SendEvent(&ser_evt, controller_name,
                                                 UPPL_EVENTS_KT_LOGICAL_PORT);
        pfc_log_debug("Event notification status %d", status);
      } else {
        pfc_log_error("Server Event addOutput failed");
      }
    }
  }
  return UNC_RC_SUCCESS;
}

/**
 * @Description : This function checks whether the logicalport_id exists in DB
 * @param[in]   : data type - specifies the data base type i.e UNC_DT_STATE
 *                            or UNC_DT_IMPORT
 *                key value - vector to hold the key structure values to be
 *                            checked in DB
 * @return      : UNC_RC_SUCCESS - if the row exist in DB
 *                UNC_UPPL_RC_NO_SUCH_INSTANCE - if the row doesnt exist in DB
 **/
UncRespCode Kt_LogicalPort::IsKeyExists(
    OdbcmConnectionHandler *db_conn,
    unc_keytype_datatype_t data_type,
    const vector<string> &key_values) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode check_status = UNC_RC_SUCCESS;
  if (key_values.empty()) {
    // No key given, return failure
    pfc_log_error("No key given. Returning error");
    return UNC_UPPL_RC_ERR_BAD_REQUEST;
  }
  string controller_name = key_values[0];
  string domain_name = key_values[1];
  string logicalport_id = key_values[2];
  // Structure used to send request to ODBC
  DBTableSchema kt_logicalport_dbtableschema;

  // Construct Primary key list
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back(CTR_NAME_STR);
  if (!domain_name.empty()) {
    vect_prim_keys.push_back(DOMAIN_NAME_STR);
  }
  if (!logicalport_id.empty()) {
    vect_prim_keys.push_back(LP_PORT_ID_STR);
  }

  // Construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list< vector<TableAttrSchema> > row_list;

  // controller_name
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // domain_name
  PhyUtil::FillDbSchema(unc::uppl::DOMAIN_NAME, domain_name,
                        domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // port_id
  PhyUtil::FillDbSchema(unc::uppl::LP_PORT_ID, logicalport_id,
                        logicalport_id.length(), DATATYPE_UINT8_ARRAY_320,
                        vect_table_attr_schema);

  kt_logicalport_dbtableschema.set_table_name(unc::uppl::LOGICALPORT_TABLE);
  kt_logicalport_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_logicalport_dbtableschema.set_row_list(row_list);

  // Send request to ODBC for controlle_table
  ODBCM_RC_STATUS check_db_status = physical_layer->get_odbc_manager()->\
      IsRowExists(data_type, kt_logicalport_dbtableschema, db_conn);
  if (check_db_status == ODBCM_RC_ROW_EXISTS) {
    pfc_log_debug("DB returned success for Row exists");
  } else {
    pfc_log_debug("DB Returned failure for IsRowExists");
    check_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
  }
  return check_status;
}

/**
 * @Description : This function is used to populate the db schema using the
 *                given key struct, value struct, data_type, operation,
 *                option1 and option
 * @param[in]   : kt_logicalport_dbtableschema - object of type DBTableSchema
 *        key_struct - void pointer to be type cast into logical port key type
 *        val_struct - void pointer to be type cast into logical port value type
 *        operation_type - specifies the operation type i.e
 *                         UNC_OP_READ or
 *                         UNC_OP_READ_SIBLING_BEGIN etc
 *        option1/option2 - specifies any additional option for populating in DB
 *        vect_key_operations - vector of type ODBCMOperator
 *                old_value_struct - holds the old value structure of the
 *                logical port key type
 *                row_status - enum indicating the row status of logicalport
 *                type entries in db
 *                is_filtering/is_state - bool variables
 * @return      : None
 * */
void Kt_LogicalPort::PopulateDBSchemaForKtTable(
    OdbcmConnectionHandler *db_conn,
    DBTableSchema &kt_logicalport_dbtableschema,
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
  // controller_name
  string controller_name =
      (const char*)obj_key_logical_port->domain_key.ctr_key.controller_name;
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // domain_name
  string domain_name =
      (const char*)obj_key_logical_port->domain_key.domain_name;
  pfc_log_info("ctrr name:%s,dom name%s,op:%d", controller_name.c_str(),
                     domain_name.c_str(), operation_type);
  PhyUtil::FillDbSchema(unc::uppl::DOMAIN_NAME, domain_name,
                        domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // port_id
  string port_id = (const char*)obj_key_logical_port->port_id;
  if (operation_type == UNC_OP_READ_SIBLING_BEGIN ||
      operation_type == UNC_OP_READ_SIBLING_COUNT) {
    // ignore port_id
    port_id = "";
  }
  pfc_log_debug("port_id : %s", port_id.c_str());
  PhyUtil::FillDbSchema(unc::uppl::LP_PORT_ID, port_id,
                        port_id.length(), DATATYPE_UINT8_ARRAY_320,
                        vect_table_attr_schema);

  if (!controller_name.empty()) {
    vect_prim_keys.push_back(CTR_NAME_STR);
  }
  if (!domain_name.empty()) {
    vect_prim_keys.push_back(DOMAIN_NAME_STR);
  }
  val_logical_port_st_t *val_logical_port_valid_st = NULL;
  if (operation_type == UNC_OP_UPDATE) {
    // get valid array for update req
    pfc_log_debug("Get Valid value from logicalport Update Valid Flag");
    val_logical_port_valid_st = new val_logical_port_st_t();
    GetLogicalPortValidFlag(db_conn, key_struct, *val_logical_port_valid_st,
                            data_type);
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
  PhyUtil::FillDbSchema(unc::uppl::LP_DESCRIPTION, LP_DESCRIPTION_STR, value,
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
  PhyUtil::FillDbSchema(unc::uppl::LP_PORT_TYPE, LP_PORT_TYPE_STR, value,
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
  PhyUtil::FillDbSchema(unc::uppl::LP_SWITCH_ID, LP_SWITCH_ID_STR, value,
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
  PhyUtil::FillDbSchema(unc::uppl::LP_PHYSICAL_PORT_ID, LP_PHYSICAL_PORT_ID_STR,
                        value, value.length(), DATATYPE_UINT8_ARRAY_32,
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
  PhyUtil::FillDbSchema(unc::uppl::LP_OPER_DOWN_CRITERIA,
                        LP_OPER_DOWN_CRITERIA_STR, value,
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
  PhyUtil::FillDbSchema(unc::uppl::LP_OPER_STATUS, LP_OPER_STATUS_STR, value,
                        value.length(), DATATYPE_UINT16,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  // valid
  stringstream dummy_valid;
  valid_val = UPPL_NO_VAL_STRUCT;
  prev_db_val = 0;
  PhyUtil::FillDbSchema(unc::uppl::LP_CTR_VALID, LP_CTR_VALID_STR, valid.str(),
                        ODBCM_SIZE_6, DATATYPE_UINT8_ARRAY_6,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, dummy_valid);
  // For read_bulk/ read_next port_id is required
  if ((operation_type == UNC_OP_READ && !port_id.empty()) ||
      operation_type != UNC_OP_READ) {
    vect_prim_keys.push_back(LP_PORT_ID_STR);
  }
  PhyUtil::reorder_col_attrs(vect_prim_keys, vect_table_attr_schema);
  kt_logicalport_dbtableschema.set_table_name(unc::uppl::LOGICALPORT_TABLE);
  kt_logicalport_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_logicalport_dbtableschema.set_row_list(row_list);
  return;
}
/**
 * @Description : This function is used to fill the ipc response structure
 *                based on the db schema retrieved from DB
 * @param[in]   : kt_logicalport_dbtableschema - object of type DBTableSchema
 *                vect_obj_val_logical_port - vector to store the value
 *                structure of logical port
 *                max_rep_ct - specifies the maximum repetition count
 *                operation_type - indicates the operation type
 *                vect_logical_port_id - vector to store the logical port id
 * @return      : None
 **/
void Kt_LogicalPort::FillLogicalPortValueStructure(
    OdbcmConnectionHandler *db_conn,
    DBTableSchema &kt_logicalport_dbtableschema,
    vector<val_logical_port_st_t> &vect_obj_val_logical_port,
    uint32_t &max_rep_ct,
    uint32_t operation_type,
    vector<key_logical_port_t> &vect_logical_port_id) {
  // populate IPC value structure based on the response received from DB
  list < vector<TableAttrSchema> >& res_logical_port_row_list =
      kt_logicalport_dbtableschema.get_row_list();

  list < vector<TableAttrSchema> > :: iterator res_logical_port_iter =
      res_logical_port_row_list.begin();

  max_rep_ct = res_logical_port_row_list.size();
  pfc_log_debug("res_logical_port_row_list.size: %d", max_rep_ct);
  key_logical_port_t obj_key_logical_port;
  memset(&obj_key_logical_port, '\0', sizeof(obj_key_logical_port));

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
      ODBCMTableColumns attr_name = tab_schema.table_attribute_name;
      string attr_value;
      switch (attr_name) {
        case unc::uppl::LP_PORT_ID:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_key_logical_port.port_id,
                                           DATATYPE_UINT8_ARRAY_320);
          pfc_log_debug("logical_port id: %s", reinterpret_cast<char *>
          (&obj_key_logical_port.port_id));
          break;

        case unc::uppl::DOMAIN_NAME:
          PhyUtil::GetValueFromDbSchemaStr(
              tab_schema,
              obj_key_logical_port.domain_key.domain_name,
              DATATYPE_UINT8_ARRAY_32);
          pfc_log_debug("domain_name: %s", reinterpret_cast<char *>
          (&obj_key_logical_port.domain_key.domain_name));
          break;

        case unc::uppl::CTR_NAME:
          PhyUtil::GetValueFromDbSchemaStr(
              tab_schema,
              obj_key_logical_port.domain_key.ctr_key.controller_name,
              DATATYPE_UINT8_ARRAY_32);
          pfc_log_debug("controller_name: %s", reinterpret_cast<char *>
          (&obj_key_logical_port.domain_key.ctr_key.controller_name));
          break;

        case unc::uppl::LP_DESCRIPTION:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_val_logical_port.description,
                                           DATATYPE_UINT8_ARRAY_128);
          pfc_log_debug("description: %s", attr_value.c_str());
          break;

        case unc::uppl::LP_PORT_TYPE:
          PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                        DATATYPE_UINT16);
          obj_val_logical_port.port_type = atoi(attr_value.c_str());
          pfc_log_debug("port_type: %s", attr_value.c_str());
          break;

        case unc::uppl::LP_SWITCH_ID:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_val_logical_port.switch_id,
                                           DATATYPE_UINT8_ARRAY_256);
          pfc_log_debug("switch_id: %s", attr_value.c_str());
          break;

        case unc::uppl::LP_PHYSICAL_PORT_ID:
          PhyUtil::GetValueFromDbSchemaStr(
              tab_schema,
              obj_val_logical_port.physical_port_id,
              DATATYPE_UINT8_ARRAY_32);
          pfc_log_debug("physical_port_id: %s", attr_value.c_str());
          break;

        case unc::uppl::LP_OPER_DOWN_CRITERIA:
          PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                        DATATYPE_UINT16);
          obj_val_logical_port.oper_down_criteria =
              atoi(attr_value.c_str());
          pfc_log_debug("oper_down_criteria: %s", attr_value.c_str());
          break;

        case unc::uppl::LP_OPER_STATUS:
          PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                        DATATYPE_UINT16);
          obj_val_logical_port_st.oper_status =
              atoi(attr_value.c_str());
          pfc_log_debug("oper_status: %s", attr_value.c_str());
          break;

        case unc::uppl::LP_CTR_VALID:
          uint8_t lp_valid[ODBCM_SIZE_6];
          memset(lp_valid, '\0', sizeof(lp_valid));
          PhyUtil::GetValueFromDbSchemaStr(tab_schema, lp_valid,
                                           DATATYPE_UINT8_ARRAY_6);
          memset(obj_val_logical_port_st.valid,
                 '\0',
                 sizeof(obj_val_logical_port_st.valid));
          memset(obj_val_logical_port.valid,
                 '\0',
                 sizeof(obj_val_logical_port.valid));
          FrameValidValue(reinterpret_cast<const char*> (lp_valid),
                          obj_val_logical_port_st,
                          obj_val_logical_port);
          pfc_log_debug("valid: %s", lp_valid);
          break;

        default:
          pfc_log_info("Ignoring LogicalPort attribute %d", attr_name);
          break;
      }
    }
    // copy val_t to val_st_t structure
    memcpy(&obj_val_logical_port_st.logical_port,
           &obj_val_logical_port,
           sizeof(val_logical_port_t));
    vect_obj_val_logical_port.push_back(obj_val_logical_port_st);
    // populate key structure
    vect_logical_port_id.push_back(obj_key_logical_port);
    pfc_log_debug("result - vect_obj_val_logical_port size: %"
                  PFC_PFMT_SIZE_T, vect_obj_val_logical_port.size());
    pfc_log_debug("result - vect_logical_port_id size: %"
                  PFC_PFMT_SIZE_T, vect_logical_port_id.size());
  }
  return;
}

/**
 * @Description : This function is used to read connected domain and 
 *                fill val_logical_port_boundary structure
 * @param[in]   : OdbcmConnectionHandler - db conn obj
 *                val_port_st_neighbor - port neighbor struct
 *                val_logical_port_boundary - response struct
 *                date_type
 * @param[out]  : val_logical_port_boundary
 * @return      : UncRespCode
 **/
UncRespCode Kt_LogicalPort::FillBoundaryCandidateDetails(
                                   OdbcmConnectionHandler *db_conn,
                                   val_port_st_neighbor &obj_neighbor,
                                   val_logical_port_boundary &val_lp_boundary,
                                   uint32_t data_type) {
  bool set_boundary_candidate_false = false;
  UncRespCode read_status = UNC_RC_SUCCESS;
  if ((obj_neighbor.valid[kIdxPortConnectedControllerId] == UNC_VF_INVALID) ||
     (obj_neighbor.valid[kIdxPortConnectedPortId] == UNC_VF_INVALID) ||
     (obj_neighbor.valid[kIdxPortConnectedSwitchId] == UNC_VF_INVALID) ||
     (strlen((const char*)obj_neighbor.connected_controller_id) == 0) ||
     (strlen((const char*)obj_neighbor.connected_switch_id) == 0) ||
     (strlen((const char*)obj_neighbor.connected_port_id) == 0)) {
    // if invalid set for boundary candidate.
    pfc_log_debug(
      "%s - boundarycandidate is false due to null/invalid", __FUNCTION__);
    set_boundary_candidate_false = true;
  } else {
    // connectedcontroller is valid convert controllerid to controller name.
    string controller_name = "";
    read_status = PhyUtil::ConvertToControllerName(db_conn,
                          reinterpret_cast<const char*>(
                          obj_neighbor.connected_controller_id),
                          controller_name);
    if (read_status != UNC_RC_SUCCESS || controller_name == "") {
      pfc_log_debug(
       "ConvertToControllerName is failed, boundarycandidate is false");
      set_boundary_candidate_false = true;
      read_status = UNC_RC_SUCCESS;
    } else {
      // conversion success fill for switch table.
      Kt_Switch switch_lp;
      key_switch_t switch_key;
      memset(&switch_key, 0, sizeof(key_switch_t));
      memcpy(switch_key.ctr_key.controller_name,
             controller_name.c_str(),
             controller_name.length()+1);
      memcpy(switch_key.switch_id,
             obj_neighbor.connected_switch_id,
             sizeof(obj_neighbor.connected_switch_id));
      vector<void *>vectval_switch;
      vector<void *>vectkey_switch;
      vectkey_switch.push_back(
                        reinterpret_cast<void *>(&switch_key));
      read_status= switch_lp.ReadInternal(db_conn, vectkey_switch,
                                          vectval_switch, data_type,
                                          UNC_OP_READ);
      pfc_log_debug("Return value for read operation %d", read_status);
      if (read_status != UNC_RC_SUCCESS) {
        pfc_log_debug("switch read is failed, boundarycandidate is false");
        set_boundary_candidate_false = true;
        read_status = UNC_RC_SUCCESS;
      } else {
        val_switch_st_t *obj_switch_val =
                  reinterpret_cast<val_switch_st_t *>(vectval_switch[0]);
        string domain_name=(const char*)
                             obj_switch_val->switch_val.domain_name;
        // get the domain name.if domain is not there,set an invalid flag.
        if (domain_name.empty()) {
          pfc_log_debug("domain_name is empty, boundarycandidate is false");
          set_boundary_candidate_false = true;
        } else {
          val_lp_boundary.boundary_candidate = UPPL_LP_BDRY_CANDIDATE_YES;
          memcpy(val_lp_boundary.connected_domain,
                  domain_name.c_str(), domain_name.length()+1);
          memcpy(val_lp_boundary.connected_controller,
                  controller_name.c_str(), controller_name.length()+1);
          val_lp_boundary.valid[kIdxLogicalPortBoundaryCandidate]
                                                  = UNC_VF_VALID;
          val_lp_boundary.valid[kIdxLogicalPortBoundaryConnectedController]
                                                  = UNC_VF_VALID;
          val_lp_boundary.valid[kIdxLogicalPortBoundaryConnectedDomain]
                                                   = UNC_VF_VALID;
        }
        delete obj_switch_val;
        obj_switch_val = NULL;
        key_switch_t* obj_key_switch =
           reinterpret_cast<key_switch_t *>(vectkey_switch[0]);
        delete obj_key_switch;
        obj_key_switch = NULL;
      }
    }
  }
  if (set_boundary_candidate_false  == true) {
    // Default values would be retained, UPPL_LP_BDRY_CANDIDATE_NO
    val_lp_boundary.boundary_candidate = UPPL_LP_BDRY_CANDIDATE_NO;
    val_lp_boundary.valid[kIdxLogicalPortBoundaryCandidate] = UNC_VF_VALID;
    val_lp_boundary.valid[kIdxLogicalPortBoundaryConnectedController] =
                                                             UNC_VF_INVALID;
    val_lp_boundary.valid[kIdxLogicalPortBoundaryConnectedDomain] =
                                                                UNC_VF_INVALID;
  }
  return read_status;
}
/**
 * @Description : This function is used to read KT_LOGICAL_PORT instance in
 *                database table using key_ctr provided in IPC request
 *                The IPC response would be filled in IPC session
 * @param[in]   : ipc session id - ipc session id used for TC validation
 *                configuration id - configuration id used for TC validation
 *                key_struct - void pointer to be type cast into logical port
 *                key type
 *                value_struct - void pointer to be type cast into logical port
 *                value structure
 *                data_type - specifies the data base type i.e UNC_DT_STATE
 *                            or UNC_DT_IMPORT
 *                operation type - indicates the operation type
 *                sess - ipc server session where the response has to be added
 *                option1,option2 - specifies any additional condition for read
 *                operation
 *                max_rep_ct - indicates the maximum repetition count
 * @return      : UNC_RC_SUCCESS - if the read operation is successful
 *                UNC_UPPL_RC_ERR_* - read operation failed
 **/
UncRespCode Kt_LogicalPort::PerformRead(
    OdbcmConnectionHandler *db_conn,
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
  pfc_log_debug("Inside PerformRead");
  if (operation_type == UNC_OP_READ) {
    max_rep_ct = 1;
  }
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
  if (option1 != UNC_OPT1_NORMAL) {
    pfc_log_error("PerformRead provided on unsupported option1");
    rsh.result_code = UNC_UPPL_RC_ERR_INVALID_OPTION1;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_LOGICAL_PORT);
    if (err != 0) {
      pfc_log_info("addOutput failed for physical_response_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UNC_RC_SUCCESS;
  }

  if (option2 != UNC_OPT2_NONE && option2 != UNC_OPT2_BOUNDARY) {
    pfc_log_error("PerformRead provided on unsupported option2");
    rsh.result_code = UNC_UPPL_RC_ERR_INVALID_OPTION2;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_LOGICAL_PORT);
    if (err != 0) {
      pfc_log_info("addOutput failed for physical_response_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UNC_RC_SUCCESS;
  }

  if ((unc_keytype_datatype_t)data_type != UNC_DT_STATE) {
    pfc_log_error("Read operation is provided on unsupported data type");
    rsh.result_code = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= sess.addOutput((uint32_t)UNC_KT_LOGICAL_PORT);
    err |= sess.addOutput(*obj_key_logical_port);
    if (err != 0) {
      pfc_log_info("addOutput failed for physical_response_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UNC_RC_SUCCESS;
  }

  UncRespCode read_status = UNC_RC_SUCCESS;
  vector<key_logical_port_t> vect_logicalport_id;
  vector<val_logical_port_st_t> vect_val_logical_port_st;
  // Reading port val structure values
  read_status = ReadLogicalPortValFromDB(db_conn,
                  key_struct, val_struct, data_type, operation_type,
                  max_rep_ct, vect_val_logical_port_st,
                  vect_logicalport_id);

  rsh.max_rep_count = max_rep_ct;
  rsh.result_code = read_status;
  if (read_status != UNC_RC_SUCCESS) {
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    if (err != 0) {
      pfc_log_error("Failure in addOutput");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    pfc_log_error("Read operation on dt_state failed with status: %d",
           read_status);
    err |= sess.addOutput((uint32_t) UNC_KT_LOGICAL_PORT);
    err |= sess.addOutput(*obj_key_logical_port);
    if (err != 0) {
      pfc_log_error("Failure in addOutput");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return UNC_RC_SUCCESS;
  }

  int err = 0;
  err = PhyUtil::sessOutRespHeader(sess, rsh);
  if (err != 0) {
    pfc_log_error("Failure in addOutput");
    return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  }
  if (option1 == UNC_OPT1_NORMAL && option2 == UNC_OPT2_NONE) {
    for (unsigned int index = 0; index < vect_logicalport_id.size();
                                                         ++index) {
      err |= sess.addOutput((uint32_t)UNC_KT_LOGICAL_PORT);
      err |= sess.addOutput((key_logical_port_t)vect_logicalport_id[index]);
      err |= sess.addOutput(vect_val_logical_port_st[index]);
      if (index < vect_logicalport_id.size() -1) {
        err |= sess.addOutput();  //  Seperator
      }
      if (err != 0) {
        pfc_log_error("Failure in addOutput");
        return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
      }
    }
  } else if (option1 == UNC_OPT1_NORMAL && option2 == UNC_OPT2_BOUNDARY) {
    uint32_t rep_count = 1;
    bool IsNeighborFound = false;
    for (unsigned int index = 0; index < vect_logicalport_id.size();
                                                         ++index) {
      err |= sess.addOutput((uint32_t)UNC_KT_LOGICAL_PORT);
      val_logical_port_t *obj_val_logical_port =
           reinterpret_cast<val_logical_port_t*>(
                &vect_val_logical_port_st[index].logical_port);
      val_logical_port_boundary val_lp_boundary;
      memset(&val_lp_boundary, 0, sizeof(val_logical_port_boundary));
      // setting default values, applicable for port_type UPPL_LP_SWITCH
      //  as well UPPL_LP_TUNNEL_ENDPOINT, UPPL_LP_PORT_GROUP
      val_logical_port_st_t *vlp_st = &vect_val_logical_port_st[index];
      memcpy(&val_lp_boundary.logical_port_st_val, vlp_st,
                                      sizeof(val_logical_port_st_t));
      val_lp_boundary.valid[kIdxLogicalPortBSt] = UNC_VF_VALID;
      val_lp_boundary.boundary_candidate = UPPL_LP_BDRY_CANDIDATE_NO;
      val_lp_boundary.valid[kIdxLogicalPortBoundaryCandidate] = UNC_VF_VALID;
      val_lp_boundary.valid[kIdxLogicalPortBoundaryConnectedController] =
                                                                UNC_VF_INVALID;
      val_lp_boundary.valid[kIdxLogicalPortBoundaryConnectedDomain] =
                                                                UNC_VF_INVALID;
      key_port_t port_key;  // port key to find neighbor information
      memset(&port_key, 0, sizeof(key_port_t));
      val_port_st_neighbor_t obj_neighbor;
      memset(&obj_neighbor, '\0', sizeof(val_port_st_neighbor_t));
      vector<key_port_t> vect_key_port;
      vector<val_port_st_neighbor_t> vect_val_port_nbr;
      rep_count = 1;
      Kt_Port_Neighbor kt_port_Neigh_obj;
      IsNeighborFound = false;
      UpplLogicalPortType porttype = (UpplLogicalPortType)
                    vect_val_logical_port_st[index].logical_port.port_type;
      if ((porttype == UPPL_LP_TRUNK_PORT) ||
                 (porttype == UPPL_LP_SUBDOMAIN) ||
                 (porttype == UPPL_LP_MAPPING_GROUP)) {
        Kt_LogicalMemberPort kt_lmp;
        key_logical_member_port_t lmp_key;
        memset(&lmp_key, 0, sizeof(key_logical_member_port_t));
        key_logical_port_t *lp_key = &vect_logicalport_id[index];
        memcpy(&lmp_key.logical_port_key, lp_key, sizeof(key_logical_port_t));
        vector<void *> vect_logical_mem_key;
        vector<void *> vect_logical_mem_port_val;
        vect_logical_mem_key.push_back(reinterpret_cast<void *>(&lmp_key));
        read_status = kt_lmp.ReadInternal(db_conn, vect_logical_mem_key,
                                       vect_logical_mem_port_val, data_type,
                                       UNC_OP_READ_SIBLING_BEGIN);
        if (read_status != UNC_RC_SUCCESS) {
          IsNeighborFound = false;
        } else {
          uint32_t lmpindex = 0;
          for (; lmpindex < vect_logical_mem_key.size(); ++lmpindex) {
            key_logical_member_port_t *key_mem_port =
                        reinterpret_cast< key_logical_member_port_t *>
                        (vect_logical_mem_key[lmpindex]);
            memcpy(port_key.sw_key.ctr_key.controller_name,
               key_mem_port->
               logical_port_key.domain_key.ctr_key.controller_name,
               sizeof(key_mem_port->logical_port_key.
               domain_key.ctr_key.controller_name));
            memcpy(port_key.sw_key.switch_id,
               key_mem_port->switch_id,
               sizeof(key_mem_port->switch_id));
            memcpy(port_key.port_id,
               key_mem_port->physical_port_id,
               sizeof(key_mem_port->physical_port_id));
            pfc_log_trace("case TP/SD/MG calling ReadPortNeighbor");
            read_status= kt_port_Neigh_obj.ReadPortNeighbor(db_conn,
                     data_type, reinterpret_cast<void *>(&port_key),
                     UNC_OP_READ, rep_count, vect_val_port_nbr, vect_key_port);
            if (read_status == UNC_RC_SUCCESS) {
              obj_neighbor = vect_val_port_nbr[0];
              if (!((obj_neighbor.valid[kIdxPortConnectedControllerId]
                 == UNC_VF_INVALID) ||
                (obj_neighbor.valid[kIdxPortConnectedPortId]
                 == UNC_VF_INVALID) ||
                (obj_neighbor.valid[kIdxPortConnectedSwitchId]
                   == UNC_VF_INVALID) ||
                (strlen((const char*)obj_neighbor.connected_controller_id) == 0)
                || (strlen((const char*)obj_neighbor.connected_switch_id) == 0)
                || (strlen((const char*)obj_neighbor.connected_port_id)
                                                                 == 0))) {
                IsNeighborFound = true;
                break;
              }
              IsNeighborFound = false;
            } else {
              IsNeighborFound = false;
              //  continue;
            }
          }
          lmpindex = 0;
          for (; lmpindex < vect_logical_mem_key.size(); ++lmpindex) {
            key_logical_member_port_t *key_mem_port =
                        reinterpret_cast< key_logical_member_port_t *>
                            (vect_logical_mem_key[lmpindex]);
            delete key_mem_port;
            key_mem_port =  NULL;
          }
        }
      } else if (porttype == UPPL_LP_PHYSICAL_PORT) {
        string controller_name=
         (const char*) obj_key_logical_port->domain_key.ctr_key.controller_name;
        string switch_id=(const char*)obj_val_logical_port->switch_id;
        string port_id=(const char*)obj_val_logical_port->physical_port_id;

        memcpy(port_key.sw_key.ctr_key.controller_name,
               controller_name.c_str(), controller_name.length()+1);
        memcpy(port_key.sw_key.switch_id,
               switch_id.c_str(), switch_id.length()+1);
        memcpy(port_key.port_id,
               port_id.c_str(), port_id.length()+1);
        pfc_log_trace("case PP calling ReadPortNeighbor");
        read_status = kt_port_Neigh_obj.ReadPortNeighbor(db_conn,
                      data_type, reinterpret_cast<void *> (&port_key),
                      UNC_OP_READ, rep_count, vect_val_port_nbr, vect_key_port);
        if (read_status == UNC_RC_SUCCESS) {
          obj_neighbor = vect_val_port_nbr[0];
          pfc_log_trace("ReadPortNeighbor is success");
          IsNeighborFound = true;
        } else {
          pfc_log_trace("neighbor domain NOT found");
          IsNeighborFound = false;
        }
      }
      if (IsNeighborFound == true) {
        pfc_log_trace("neighbor domain found");
        read_status = FillBoundaryCandidateDetails(
                      db_conn, obj_neighbor,
                      val_lp_boundary, data_type);
        // read_status will be success always.
      }
      err |= sess.addOutput(vect_logicalport_id[index]);
      err |= sess.addOutput(val_lp_boundary);
      if (index < vect_logicalport_id.size() -1) {
        err |= sess.addOutput();  //  Seperator
      }
      if (err != 0) {
        pfc_log_error("Failure in addOutput");
        return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
      }
      pfc_log_debug(" %s", IpctUtil::get_string(val_lp_boundary).c_str());
      pfc_log_info("successfully send the neighnor to vtnservices");
    }  // end of for loop
  }
  return UNC_RC_SUCCESS;
}
/** ReadLogicalPortValFromDB
 * @Description : This function is used to read KT_LOGICAL_PORT instance in
 *                database table using key_ctr provided in IPC request
 *                and operation_type
 * @param[in]   : key_struct - void pointer to be type cast into logical
 *                port key structure
 *                value_struct - void pointer to be type cast into logical port
 *                value structure
 *                data_type - indicates the data base type
 *                operation_type - indicates the operation type UNC_OP*
 *                max_rep_ct - indicates the maximum repetition count
 *                vect_val_logical_port - vector to store the val_logical_port_t
 *                structure
 *                vect_val_logical_port_st - vector to store the
 *                val_logical_port_st_t structure
 *                logicalport_id - vector of type key_logical_port_t to store
 *                the logical port id
 * @return      : UNC_RC_SUCCESS - read operation is success
 *                UNC_UPPL_RC_ERR_DB_GET - read operation is failure
 **/
UncRespCode Kt_LogicalPort::ReadLogicalPortValFromDB(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t data_type,
    uint32_t operation_type,
    uint32_t &max_rep_ct,
    vector<val_logical_port_st_t> &vect_val_logical_port_st,
    vector<key_logical_port_t> &logicalport_id) {
  if (operation_type < UNC_OP_READ) {
    // Unsupported operation type for this function
    return UNC_RC_SUCCESS;
  }
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode read_status = UNC_RC_SUCCESS;
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;
  DBTableSchema kt_logicalport_dbtableschema;
  void* old_value_struct;
  vector<ODBCMOperator> vect_key_operations;
  PopulateDBSchemaForKtTable(db_conn, kt_logicalport_dbtableschema,
                             key_struct,
                             val_struct,
                             operation_type, data_type,
                             0, 0, vect_key_operations,
                             old_value_struct,
                             NOTAPPLIED, false, PFC_FALSE);
  if (operation_type == UNC_OP_READ) {
    read_db_status = physical_layer->get_odbc_manager()->
        GetOneRow((unc_keytype_datatype_t)data_type,
                  kt_logicalport_dbtableschema, db_conn);
  } else {
    read_db_status = physical_layer->get_odbc_manager()->
        GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                    kt_logicalport_dbtableschema,
                    (unc_keytype_operation_t)operation_type, db_conn);
  }
  if (read_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
    pfc_log_debug("No record found");
    read_status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
    return read_status;
  } else if (read_db_status == ODBCM_RC_CONNECTION_ERROR) {
    read_status = UNC_UPPL_RC_ERR_DB_ACCESS;
    pfc_log_error("Read operation has failed with error %d", read_db_status);
    return read_status;
  } else if (read_db_status != ODBCM_RC_SUCCESS) {
    read_status = UNC_UPPL_RC_ERR_DB_GET;
    // log error to log daemon
    pfc_log_error("Read operation has failed with error %d", read_db_status);
    return read_status;
  }
  FillLogicalPortValueStructure(db_conn, kt_logicalport_dbtableschema,
                                vect_val_logical_port_st,
                                max_rep_ct,
                                operation_type,
                                logicalport_id);
  pfc_log_debug("vect_val_logical_port_st size: %"
                PFC_PFMT_SIZE_T, vect_val_logical_port_st.size());
  pfc_log_debug("logicalport_id size: %" PFC_PFMT_SIZE_T,
                 logicalport_id.size());
  if (vect_val_logical_port_st.empty()) {
    // Read failed , return error
    read_status = UNC_UPPL_RC_ERR_DB_GET;
    // log error to log daemon
    pfc_log_error("Read operation has failed after reading response");
    return read_status;
  }
  pfc_log_debug("Read operation Completed with result: %d", read_status);
  return read_status;
}
/**
 * @Description : This function returns the pointer to the child key structures
 * @param[in]   : child_class - variable to store the child class enum
 *                logicalport_id - string to store the logical port id
 *                controller_name - string to store the controller name
 *                domain_name - string to store the domain name
 * @return      : returns the pointer to the child key structure
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

/**
 * @Description : This function clears the pointer to the child key structures
 * @param[in]   : child class - indicates the child  class
 *                key_struct - void pointer to be type cast to the child
 *                key type
 * @return      : void
 **/
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

/**
 * @Description : This function populates the values to be used for attribute
 *                validation
 * @param[in]   : None
 * @return      : None
 **/
void Kt_LogicalPort::Fill_Attr_Syntax_Map() {
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map;
  Kt_Class_Attr_Syntax objKeyAttrSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 1, 320, true,  "" };
  attr_syntax_map[LP_PORT_ID_STR] = objKeyAttrSyntax;

  Kt_Class_Attr_Syntax objKeyAttr1Syntax4 =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 32, true, "" };
  attr_syntax_map[DOMAIN_NAME_STR] = objKeyAttr1Syntax4;

  Kt_Class_Attr_Syntax objKeyAttr1Syntax5 =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 32, true, "" };
  attr_syntax_map[CTR_NAME_STR] = objKeyAttr1Syntax5;

  Kt_Class_Attr_Syntax objAttrDescSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 128, false, "" };
  attr_syntax_map[LP_DESCRIPTION_STR] = objAttrDescSyntax;

  Kt_Class_Attr_Syntax objAttrSwitchIdSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 256, false, "" };
  attr_syntax_map[LP_SWITCH_ID_STR] = objAttrSwitchIdSyntax;

  Kt_Class_Attr_Syntax objAttrPhyPortIdSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 32, false, "" };
  attr_syntax_map[LP_PHYSICAL_PORT_ID_STR] = objAttrPhyPortIdSyntax;

  Kt_Class_Attr_Syntax objAttrOpDownCriteriaSyntax =
  { PFC_IPCTYPE_UINT8, 0, 1, 0, 0, false, "" };
  attr_syntax_map[LP_OPER_DOWN_CRITERIA_STR] = objAttrOpDownCriteriaSyntax;

  Kt_Class_Attr_Syntax objAttrValidSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 5, false, "" };
  attr_syntax_map[LP_CTR_VALID_STR] = objAttrValidSyntax;
  attr_syntax_map_all[UNC_KT_LOGICAL_PORT] = attr_syntax_map;
}

/**
 * @Description : This function returns all the associated physical/logical
 *                port ids for a given switch
 * @param[in]   : data_type - indicates the data type
 *                controller_name - string to store the controller_name
 *                switch_id - string to store the switch_id
 *                logical_port_id - vector of type string to store the
 *                logical port id
 *                is_single_logical_port - indicates type of logical port -
 *                                         true - UPPL_LP_SWITCH
 *                                         false - UPPL_LP_PHYSICAL_PORT
 * @return      : None
 **/
void Kt_LogicalPort::GetAllPortId(OdbcmConnectionHandler *db_conn,
                                  uint32_t data_type,
                                  string controller_name,
                                  string switch_id,
                                  string &domain_name,
                                  vector <string> &logical_port_id,
                                  pfc_bool_t is_single_logical_port) {
  DBTableSchema kt_logicalport_dbtableschema;
  vector<TableAttrSchema> vect_table_attr_schema;
  vector<string> vect_prim_keys;
  vector<ODBCMOperator> vect_operators;
  list < vector<TableAttrSchema> > row_list_in;
  vect_prim_keys.push_back(CTR_NAME_STR);
  vect_prim_keys.push_back(LP_SWITCH_ID_STR);
  vect_prim_keys.push_back(LP_PORT_TYPE_STR);
  vect_operators.push_back(unc::uppl::EQUAL);
  vect_operators.push_back(unc::uppl::EQUAL);
  vect_operators.push_back(unc::uppl::EQUAL);
  // controller_name
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // switch_id
  PhyUtil::FillDbSchema(unc::uppl::LP_SWITCH_ID, switch_id,
                        switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);

  // port_type
  string port_type = "";
  if (is_single_logical_port == true) {
    port_type = "1";  //  UPPL_LP_SWITCH;
  } else {
    port_type = "2";  //  UPPL_LP_PHYSICAL_PORT;
  }
  pfc_log_debug(
      "GetAllPortId is called with is_single_logical_port:%d, port_type:%s",
      static_cast<int>(is_single_logical_port), port_type.c_str());
  PhyUtil::FillDbSchema(unc::uppl::LP_PORT_TYPE, port_type,
                        port_type.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);

  string empty_id="";
  // logical_port_id - to be fetched
  PhyUtil::FillDbSchema(unc::uppl::LP_PORT_ID, empty_id,
                        empty_id.length(), DATATYPE_UINT8_ARRAY_320,
                        vect_table_attr_schema);
  // domain_name - to be fetched
  PhyUtil::FillDbSchema(unc::uppl::DOMAIN_NAME, empty_id,
                        empty_id.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  kt_logicalport_dbtableschema.set_table_name(unc::uppl::LOGICALPORT_TABLE);
  kt_logicalport_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list_in.push_back(vect_table_attr_schema);
  kt_logicalport_dbtableschema.set_row_list(row_list_in);
  // Call GetSibling Rows with operator specification
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  ODBCM_RC_STATUS read_db_status = physical_layer->get_odbc_manager()->
      GetSiblingRows((unc_keytype_datatype_t)data_type, UPPL_MAX_REP_CT,
                     kt_logicalport_dbtableschema,
                     vect_operators,
                     (unc_keytype_operation_t)UNC_OP_READ_SIBLING, db_conn);

  if (read_db_status != ODBCM_RC_SUCCESS) {
    pfc_log_debug("Get Sibling is not success");
    return;
  }
  // Read all the logical port values
  list < vector<TableAttrSchema> >& row_list =
      kt_logicalport_dbtableschema.get_row_list();
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
      ODBCMTableColumns attr_name = tab_schema.table_attribute_name;
      uint8_t port_id[ODBCM_SIZE_320];
      uint8_t domain_val[ODBCM_SIZE_32];
      switch (attr_name) {
        case unc::uppl::LP_PORT_ID:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema, port_id,
                                           DATATYPE_UINT8_ARRAY_320);
          logical_port_id.push_back(reinterpret_cast<const char*> (port_id));
          pfc_log_debug("logical_port id: %s", port_id);
          break;
        case unc::uppl::DOMAIN_NAME:
          PhyUtil::GetValueFromDbSchemaStr(tab_schema, domain_val,
                                           DATATYPE_UINT8_ARRAY_32);
          domain_name = reinterpret_cast<const char*> (domain_val);
          pfc_log_debug("domain_name: %s", domain_val);
          break;
        default:
          break;
      }
    }
  }
  return;
}

/**
 * @Description : This function reads the valid flag from DB
 * @param[in]   : key_struct - void pointer to be type cast to
 *                logical port key type
 *                val_logical_port_valid_st - value structure of logical port
 *                key type
 * @return      : UNC_RC_SUCCESS - value structure read from DB is success
 *                UNC_UPPL_RC_ERR_* - value structure read from DB is failure
 **/
UncRespCode Kt_LogicalPort::GetLogicalPortValidFlag(
    OdbcmConnectionHandler *db_conn,
    void *key_struct,
    val_logical_port_st_t &val_logical_port_valid_st,
    uint32_t data_type) {
  UncRespCode return_code = UNC_RC_SUCCESS;
  // read the value structure from db
  vector<void *> vectVal_logicalport;
  vector<void *> vectkey_logicalport;
  vectkey_logicalport.push_back(key_struct);
  return_code = ReadInternal(db_conn, vectkey_logicalport,
                             vectVal_logicalport,
                             data_type, UNC_OP_READ);
  if (return_code == UNC_RC_SUCCESS) {
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
      pfc_log_debug("update logicalport valid ret null val");
    }
  }
  return return_code;
}

/**
 * @Description : This function converts the string value from db to uint8
 * @param[in]   : attr_value - string to store attribute value
 *                obj_val_logical_port_st/obj_val_logical_portt
 *                - struct variables of the logical port val structure
 * @return      : None
 **/
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

/**
 * @Description : This function gets the valid flag value from DB
 * @param[in]   : data_type - indicates the data base type
 *                key_struct - void pointer type to be type cast to logical port
 *                key structure
 *                valid_flag - string pointer to hold the valid flag
 * @return      : UNC_RC_SUCCESS - if valid flag is received from db
 *                UNC_UPPL_RC_DB_GET - if there is error in receiving the valid
 *                flag from DB
 **/
UncRespCode Kt_LogicalPort::GetValidFlag(
    OdbcmConnectionHandler *db_conn,
    uint32_t data_type,
    void* key_struct,
    string *valid_flag) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  key_logical_port_t *obj_key_logical_port =
      reinterpret_cast<key_logical_port_t*>(key_struct);
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  vector<string> vect_prim_keys;

  string controller_name = (const char*)obj_key_logical_port->domain_key.
      ctr_key.controller_name;

  if (!controller_name.empty()) {
    vect_prim_keys.push_back(CTR_NAME_STR);
    PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                          controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
  }

  string domain_name = (const char*)obj_key_logical_port->
      domain_key.domain_name;

  if (!domain_name.empty()) {
    vect_prim_keys.push_back(DOMAIN_NAME_STR);
    PhyUtil::FillDbSchema(unc::uppl::DOMAIN_NAME, domain_name,
                          domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
  }
  string port_id = (const char*)obj_key_logical_port->port_id;

  if (!port_id.empty()) {
    vect_prim_keys.push_back(LP_PORT_ID_STR);
    PhyUtil::FillDbSchema(unc::uppl::LP_PORT_ID, port_id,
                          port_id.length(), DATATYPE_UINT8_ARRAY_320,
                          vect_table_attr_schema);
  }

  string valid_empty;
  PhyUtil::FillDbSchema(unc::uppl::LP_CTR_VALID, valid_empty,
                        valid_empty.length(),
                        DATATYPE_UINT8_ARRAY_6,
                        vect_table_attr_schema);

  DBTableSchema kt_logicalport_dbtableschema;
  kt_logicalport_dbtableschema.set_table_name(unc::uppl::LOGICALPORT_TABLE);
  kt_logicalport_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_logicalport_dbtableschema.set_row_list(row_list);

  // Call ODBCManager and read
  ODBCM_RC_STATUS read_db_status =
      physical_layer->get_odbc_manager()->GetOneRow(
          (unc_keytype_datatype_t)data_type,
          kt_logicalport_dbtableschema, db_conn);
  if (read_db_status != ODBCM_RC_SUCCESS) {
    // log error
    pfc_log_error("valid flag read operation failed");
    return UNC_UPPL_RC_ERR_DB_GET;
  }

  // read the valid value
  list < vector<TableAttrSchema> >& res_logicalport_row_list =
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
      ODBCMTableColumns attr_name = tab_schema.table_attribute_name;
      if (attr_name == unc::uppl::LP_CTR_VALID) {
        uint8_t valid_val[ODBCM_SIZE_6];
        memset(valid_val, '\0', sizeof(valid_val));
        PhyUtil::GetValueFromDbSchemaStr(tab_schema, valid_val,
                                         DATATYPE_UINT8_ARRAY_6);
        *valid_flag = reinterpret_cast<const char*> (valid_val);
        pfc_log_debug("valid_flag from db: %s", valid_flag->c_str());
        break;
      }
    }
  }
  return UNC_RC_SUCCESS;
}

/**
 * @Description : This function gets the oper status for the assicated
 *                physical port
 * @param[in]   : logical_port_key - structure variable of type
 *                key_logical_port_t
 *                port_oper_status - poiter to store the oper status
 *                data_type - indicates the data base type
 * @return      : UNC_RC_SUCCESS - if oper status value is received from DB
 *                UNC_UPPL_RC_ERR_* - if there is failure in receiving oper status
 *                value from DB
 **/
UncRespCode Kt_LogicalPort::GetPortOperStatus(
    OdbcmConnectionHandler *db_conn,
    key_port_t &obj_key_port,
    uint8_t *port_oper_status,
    uint32_t data_type) {
  pfc_log_debug("Decide oper_status based on physical port oper_status");
  Kt_Port port;
  UncRespCode read_status = port.GetOperStatus(
      db_conn, data_type, reinterpret_cast<void*>(&obj_key_port),
      *port_oper_status);
  pfc_log_debug("Oper Status return %d, value %d", read_status,
                PhyUtil::uint8touint(*port_oper_status));
  return read_status;
}

/**
 * @Description : This function gets all the logical ports associated with
 *                given controller/domain/switch
 * @param[in]   : controller_name - string to store the controller_name
 *      domain_name - string to store the domain_name
 *      switch_id - string to store the switch_id
 *      phy_port_id - string to store the physical port id
 *      vectLogicalPortKey - vector of type key_logical_port_t
 *      data_type - indicates the data base type UNC_DT_*
 * @return      : None
 **/
void Kt_LogicalPort::GetAllLogicalPort(
    OdbcmConnectionHandler *db_conn,
    string controller_name,
    string domain_name,
    string switch_id,
    string phy_port_id,
    vector<key_logical_port_t> &vectLogicalPortKey,
    uint32_t data_type) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  string port_id;
  while (true) {
    DBTableSchema kt_ctr_domain_dbtableschema;
    vector<string> vect_prim_keys;
    vect_prim_keys.push_back(CTR_NAME_STR);

    vector<TableAttrSchema> vect_table_attr_schema;
    list< vector<TableAttrSchema> > row_list;
    PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                          controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
    if (!domain_name.empty()) {
      vect_prim_keys.push_back(DOMAIN_NAME_STR);
    }
    PhyUtil::FillDbSchema(unc::uppl::DOMAIN_NAME, domain_name,
                          domain_name.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
    if (!switch_id.empty()) {
      vect_prim_keys.push_back(LP_SWITCH_ID_STR);
    }
    PhyUtil::FillDbSchema(unc::uppl::LP_SWITCH_ID, switch_id,
                          switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                          vect_table_attr_schema);

    if (!phy_port_id.empty()) {
      vect_prim_keys.push_back(LP_PHYSICAL_PORT_ID_STR);
    }
    PhyUtil::FillDbSchema(unc::uppl::LP_PHYSICAL_PORT_ID, phy_port_id,
                          phy_port_id.length(), DATATYPE_UINT8_ARRAY_32,
                          vect_table_attr_schema);
    vect_prim_keys.push_back(LP_PORT_ID_STR);
    PhyUtil::FillDbSchema(unc::uppl::LP_PORT_ID, port_id,
                          port_id.length(), DATATYPE_UINT8_ARRAY_320,
                          vect_table_attr_schema);
    PhyUtil::reorder_col_attrs(vect_prim_keys, vect_table_attr_schema);
    kt_ctr_domain_dbtableschema.set_table_name(unc::uppl::LOGICALPORT_TABLE);
    kt_ctr_domain_dbtableschema.set_primary_keys(vect_prim_keys);
    row_list.push_back(vect_table_attr_schema);
    kt_ctr_domain_dbtableschema.set_row_list(row_list);
    ODBCM_RC_STATUS db_status = physical_layer->get_odbc_manager()->
        GetBulkRows((unc_keytype_datatype_t)data_type, UPPL_MAX_REP_CT,
                    kt_ctr_domain_dbtableschema,
                    (unc_keytype_operation_t)UNC_OP_READ_SIBLING_BEGIN,
                    db_conn);
    if (db_status != ODBCM_RC_SUCCESS) {
      // No other logical port available
      pfc_log_debug("No other logical port available");
      break;
    }
    list<vector<TableAttrSchema> > ::iterator iter_list;
    for (iter_list = kt_ctr_domain_dbtableschema.row_list_.begin();
        iter_list != kt_ctr_domain_dbtableschema.row_list_.end();
        ++iter_list) {
      vector<TableAttrSchema> attributes_vector = *iter_list;
      vector<TableAttrSchema> :: iterator iter_vector;
      key_logical_port_t logical_port;
      memset(&logical_port, '\0', sizeof(key_logical_port_t));
      for (iter_vector = attributes_vector.begin();
          iter_vector != attributes_vector.end();
          ++iter_vector) {
        /* Get attribute name of a row */
        TableAttrSchema tab_att_schema = (*iter_vector);
        switch (tab_att_schema.table_attribute_name) {
          case unc::uppl::CTR_NAME:
            PhyUtil::GetValueFromDbSchemaStr(
                tab_att_schema,
                logical_port.domain_key.ctr_key.controller_name,
                DATATYPE_UINT8_ARRAY_32);
            controller_name = reinterpret_cast<const char*>
            (logical_port.domain_key.ctr_key.controller_name);
            break;
          case unc::uppl::DOMAIN_NAME:
            PhyUtil::GetValueFromDbSchemaStr(
                tab_att_schema,
                logical_port.domain_key.domain_name,
                DATATYPE_UINT8_ARRAY_32);
            domain_name = reinterpret_cast<const char*>
            (logical_port.domain_key.domain_name);
            break;
          case unc::uppl::LP_PORT_ID:
            PhyUtil::GetValueFromDbSchemaStr(tab_att_schema,
                                             logical_port.port_id,
                                             DATATYPE_UINT8_ARRAY_320);
            port_id = reinterpret_cast<const char*> (logical_port.port_id);
            break;
          default:
            break;
        }
      }
      vectLogicalPortKey.push_back(logical_port);
    }
    if (kt_ctr_domain_dbtableschema.row_list_.size() < UPPL_MAX_REP_CT) {
      // No other logical port available
      pfc_log_debug("No other logical port available");
      break;
    }
  }
}

/**
 * @Description : This function validates the logical port type
 * @param[in]   : port_type - Logical Port's type
 * @return      : Success if port type is within allowed values or
 *                else return failure
 **/
UncRespCode Kt_LogicalPort::ValidatePortType(uint8_t port_type) {
  if (port_type == UPPL_LP_SWITCH ||
      port_type == UPPL_LP_PHYSICAL_PORT ||
      (port_type >= UPPL_LP_TRUNK_PORT &&
      port_type <= UPPL_LP_MAPPING_GROUP)) {
    return UNC_RC_SUCCESS;
  }
  pfc_log_info("Invalid Logical Port Type provided %d", port_type);
  return UNC_UPPL_RC_ERR_CFG_SYNTAX;
}

/**
 * @Description: This function updates the domain name for already existing
 *               trunk port.
 * @param[in]  : key_struct - void pointer to be type cast into logical port
 *               key type
 *               value_struct - void pointer to be type cast into logical port
 *               value structure
 *               data_type - specifies the data base type i.e UNC_DT_STATE
 *                           or UNC_DT_IMPORT
 *               key_type   - indicates the key type
 * @return     : UNC_RC_SUCCESS - if the update of a domain name is successful
 * @return     : UNC_UPPL_RC_ERR_* if ther is any failure while updating domain
 **/
UncRespCode Kt_LogicalPort::UpdateDomainNameForTP(
                   OdbcmConnectionHandler *db_conn,
                   void* key_struct,
                   void* val_struct,
                   uint32_t data_type,
                   uint32_t key_type) {
  // Extact the val structre and check for the port type
  UncRespCode status = UNC_RC_SUCCESS;
  UncRespCode read_status = UNC_RC_SUCCESS;
  key_logical_port *obj_key_port =
     reinterpret_cast<key_logical_port_t*>(key_struct);
  val_logical_port_st_t *obj_val_port =
       reinterpret_cast<val_logical_port_st_t*>(val_struct);
  pfc_log_debug("Port type is %d", obj_val_port->logical_port.port_type);
  // if port type id not UPPL_LP_SWITCH return success.
  if (obj_val_port->logical_port.port_type != UPPL_LP_SWITCH) {
    pfc_log_debug("Received event port_type is not UPPL_LP_SWITCH ");
    return UNC_RC_SUCCESS;
  }
  // store controller_name, domain_name and switch_id locally.
  string controller_name = reinterpret_cast<const char*>
          (obj_key_port->domain_key.ctr_key.controller_name);
  string newdomain_name = reinterpret_cast<const char*>
          (obj_key_port->domain_key.domain_name);
  string switch_id = reinterpret_cast<const char*>
          (obj_val_port->logical_port.switch_id);
  pfc_log_debug("controller_name = %s \t,new domain_name = %s \t,"
                "switch_id = %s", controller_name.c_str(),
                newdomain_name.c_str(), switch_id.c_str());
  // Check for trunk port records with the received switch_id and ctr_name
  // in logical_member_port table
  Kt_LogicalMemberPort kt_lm_port;
  vector<void *> vect_lmp_key_struct, vect_lmp_val_struct;
  key_logical_member_port_t key_lmp;
  memset(&key_lmp, 0, sizeof(key_lmp));
  memcpy(key_lmp.logical_port_key.domain_key.ctr_key.controller_name,
         controller_name.c_str(), (controller_name.length())+1);
  memcpy(key_lmp.switch_id, switch_id.c_str(),
        (switch_id.length())+1);
  vect_lmp_key_struct.push_back(reinterpret_cast<void *>(&key_lmp));
  read_status = kt_lm_port.ReadInternal(db_conn,
                                      vect_lmp_key_struct, vect_lmp_val_struct,
                                      UNC_DT_STATE,
                                      UNC_OP_READ_SIBLING);
  if (read_status == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE) {
    pfc_log_debug("No member ports, so continue with next logical port");
    vect_lmp_key_struct.clear();
    return UNC_RC_SUCCESS;
  }
  vector<string> vect_processed_LPs;
  for (uint32_t lm_port_Index = 0; lm_port_Index < vect_lmp_key_struct.size();
             lm_port_Index ++) {
    key_logical_member_port_t *key_lm_port =
                     reinterpret_cast<key_logical_member_port_t*>
                     (vect_lmp_key_struct[lm_port_Index]);
    if (key_lm_port == NULL) continue;
    string port_id = reinterpret_cast<const char*>
                     (key_lm_port->logical_port_key.port_id);
    delete key_lm_port;
    key_lm_port = NULL;
    if (std::find(vect_processed_LPs.begin(), vect_processed_LPs.end(), port_id)
            != vect_processed_LPs.end()) {
      pfc_log_debug("port_id already processed, so continue");
      continue;
    } else {
      vect_processed_LPs.push_back(port_id);
    }
    // 3.GetOneRow = CTR_NAME, PORT_ID
    // Populate FillDBSchema for logicalport_table and call GetOneRow
    DBTableSchema kt_logicalport_dbtableschema;
    void* old_value_struct;
    vector<ODBCMOperator> vect_key_operations;
    //  fill controller_name and port_id in key struct
    key_logical_port_t o_key_port;
    val_logical_port_st_t o_val_port;
    memset(&o_key_port, '\0', sizeof(key_logical_port_t));
    memcpy(o_key_port.domain_key.ctr_key.controller_name,
              controller_name.c_str(), (controller_name.length())+1);
    memcpy(o_key_port.port_id, port_id.c_str(), (port_id.length()+1));
    memset(&o_val_port, 0, sizeof(val_logical_port_st_t));

    PopulateDBSchemaForKtTable(db_conn, kt_logicalport_dbtableschema,
                          reinterpret_cast<void*>(&o_key_port),
                          reinterpret_cast<void*>(&o_val_port),
                          UNC_OP_READ, data_type, 0, 0,
                          vect_key_operations, old_value_struct,
                          NOTAPPLIED, false, PFC_FALSE);
    // Read row from DB
    PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
    ODBCM_RC_STATUS read_db_status =
      physical_layer->get_odbc_manager()->GetOneRow(
          (unc_keytype_datatype_t)data_type,
          kt_logicalport_dbtableschema, db_conn);
    if (read_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
      pfc_log_debug("No record found, nothing to do, return success");
      read_status = UNC_RC_SUCCESS;
      return read_status;
    } else if (read_db_status == ODBCM_RC_CONNECTION_ERROR) {
      read_status = UNC_UPPL_RC_ERR_DB_ACCESS;
      pfc_log_error("Read operation has failed with error %d", read_db_status);
      return read_status;
    } else if (read_db_status != ODBCM_RC_SUCCESS) {
      read_status = UNC_UPPL_RC_ERR_DB_GET;
      // log error to log daemon
      pfc_log_error("Read operation has failed with error %d", read_db_status);
      return read_status;
    }
    vector<val_logical_port_st_t> vect_val_logical_port;
    vector<key_logical_port_t> vect_logical_port_id;
    uint32_t max_rep_ct = 0;
    // From the values received from DB, populate logical_port structure
    FillLogicalPortValueStructure(db_conn, kt_logicalport_dbtableschema,
                                  vect_val_logical_port,
                                  max_rep_ct,
                                  UNC_OP_READ,
                                  vect_logical_port_id);
    // 4. Check if the received domain name and the existing dom_name in DB
    // are equal or not
    vector <val_logical_port_st_t>::iterator vect_val_lport_iter =
                vect_val_logical_port.begin();
    vector <key_logical_port_t>::iterator vect_lport_iter =
                vect_logical_port_id.begin();
    // 5. iterate the db fetched logical port values
    for (; (vect_val_lport_iter != vect_val_logical_port.end()) &&
            (vect_lport_iter != vect_logical_port_id.end());
            vect_val_lport_iter++, vect_lport_iter++) {
      key_logical_port_t key_lport = (*vect_lport_iter);
      string olddomain_name = reinterpret_cast<const char*>(key_lport.
                        domain_key.domain_name);
      val_logical_port_st_t val_lport = (*vect_val_lport_iter);
      // 6. compare domain names, if equal continue else move rest actions
      if (newdomain_name.compare(olddomain_name) == 0)
        continue;  // if equals no action required
      // 7. check for port_type, if it is not UPPL_LP_TRUNK_PORT or
      // UPPL_LP_PORT_GROUP continue else move rest actions
      if (val_lport.logical_port.port_type != UPPL_LP_TRUNK_PORT &&
          val_lport.logical_port.port_type != UPPL_LP_PORT_GROUP ) {
        pfc_log_debug("Existing entry is not trunk port entry");
        continue;
      }

      pfc_log_debug("domain names are not equal ");
      // 8. fetch and store the corresponding logicalmember port values
      vector<void *> vect_lmp_key, vect_lmp_val;
      key_logical_member_port_t key_lmp_obj;
      memset(&key_lmp_obj, 0, sizeof(key_lmp_obj));
      memcpy(key_lmp_obj.logical_port_key.domain_key.ctr_key.controller_name,
             controller_name.c_str(), (controller_name.length())+1);
      memcpy(key_lmp_obj.logical_port_key.domain_key.domain_name,
             olddomain_name.c_str(), (olddomain_name.length())+1);
      memcpy(key_lmp_obj.logical_port_key.port_id,
             key_lport.port_id, sizeof(key_lport.port_id));
      memcpy(key_lmp_obj.switch_id, switch_id.c_str(),
            (switch_id.length())+1);
      vect_lmp_key.push_back(reinterpret_cast<void *>(&key_lmp_obj));
      read_status = kt_lm_port.ReadInternal(db_conn,
                                            vect_lmp_key, vect_lmp_val,
                                            UNC_DT_STATE,
                                            UNC_OP_READ_SIBLING_BEGIN);
      if (read_status != UNC_RC_SUCCESS) {
        if (read_status == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE) {
          pfc_log_debug("No member ports, so continue with next logical port");
          continue;
        } else {
          pfc_log_info("Unable to retrieve the logical_member_port entry");
          continue;
        }
      }
      // 9. Delete the trunk port/port group record in logicalport_table
      UncRespCode delete_status = DeleteKeyInstance(db_conn,
                            reinterpret_cast<void *>(&key_lport),
                          (uint32_t)UNC_DT_STATE, key_type);
      if (delete_status != UNC_RC_SUCCESS) {
        pfc_log_error("Unable to delete Trunk port record from DB");
        status = delete_status;
      }
      //  10. create the same trunk port/port group record in logical_port_table
      //  with new domain name
      // Fill the key with the received domain name
      memset(&(key_lport.domain_key.domain_name), '\0',
            sizeof(key_lport.domain_key.domain_name));
      memcpy(key_lport.domain_key.domain_name,
            newdomain_name.c_str(), (newdomain_name.length())+1);
      UncRespCode create_status = CreateKeyInstance(db_conn,
                          reinterpret_cast<void *>(&key_lport),
                          reinterpret_cast<void *>(&val_lport),
                          (uint32_t)UNC_DT_STATE, key_type);
      if (create_status == UNC_RC_SUCCESS) {
        pfc_log_debug(" Trunk port/Port group record is created in"
                      " logical_port_table");
        // 11. using backup of logicalmemberport values,
        // Create the same logical_member_port records with the
        // received domain name
        for (uint32_t lmpIndex = 0; lmpIndex < vect_lmp_key.size();
               lmpIndex ++) {
          Kt_LogicalMemberPort logical_member_port;
          key_logical_member_port_t *key_lmp1 =
               reinterpret_cast<key_logical_member_port_t*>
               (vect_lmp_key[lmpIndex]);
          memset(key_lmp1->logical_port_key.domain_key.domain_name, '\0',
               sizeof(key_lmp1->logical_port_key.domain_key.domain_name));
          memcpy(key_lmp1->logical_port_key.domain_key.domain_name,
               newdomain_name.c_str(), (newdomain_name.length())+1);
          uint32_t key_type_child = UNC_KT_LOGICAL_MEMBER_PORT;
          void* val_struct_child = NULL;
          status = logical_member_port.UpdateDomainNameForTP(db_conn,
                      key_lmp1, val_struct_child, data_type, key_type_child);
          if (status == UNC_RC_SUCCESS) {
            pfc_log_debug("Logical_member_port record is created");
          } else {
            pfc_log_error("Create of Logical_member_port failed with error %d",
                          status);
            continue;
          }
          if (key_lmp1 != NULL) {
            delete key_lmp1;
            key_lmp1 = NULL;
          }
          // no delete required for val struct,
          // since logicalmember port does not have val struct
        }
      } else {
        pfc_log_error("Create of trunkport/port group failed with error %d",
                      create_status);
        status = create_status;
      }
      vect_lmp_key.clear();
      vect_lmp_val.clear();
    }
  }
  vect_processed_LPs.clear();
  vect_lmp_key_struct.clear();
  vect_lmp_val_struct.clear();
  return status;
}
