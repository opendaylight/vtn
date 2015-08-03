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
 * @file     itc_kt_port_neighbor.cc
 *
 */

#include "itc_kt_port.hh"
#include "itc_kt_port_neighbor.hh"
#include "itc_kt_controller.hh"
#include "itc_kt_switch.hh"
#include "itc_kt_logicalport.hh"
#include "itc_kt_link.hh"
#include "odbcm_utils.hh"
#include "ipct_util.hh"
#include "itc_read_request.hh"
#include "odbcm_db_varbind.hh"
using unc::uppl::PhysicalLayer;
using unc::uppl::IPCClientDriverHandler;

/** 
 * @Description : This function initializes member variables
 *                and fills the attribute syntax map used for validation
 * @param[in]   : None
 * @return      : None
 **/
Kt_Port_Neighbor::Kt_Port_Neighbor() {
  // Populate structure to be used for syntax validation
  if (attr_syntax_map_all.find(UNC_KT_PORT_NEIGHBOR) ==
        attr_syntax_map_all.end()) {
    Fill_Attr_Syntax_Map();
  }
}

/** 
 * @Description : This function does nothing
 * @param[in]   : None
 * @return      : None
 **/
Kt_Port_Neighbor::~Kt_Port_Neighbor() {
}

/**
 * @Description : This function is used to populate the db schema using the
 *                given key struct, value struct, data_type, operation,
 *                option1 and option
 * @param[in]   : kt_port_dbtableschema - object of type DBTableSchema
 *                key_struct - void pointer to be type cast into port key type
 *                val_struct - void pointer to be type cast into port value type
 *                operation_type - specifies the operation type i.e
 *                                 UNC_OP_READ or UNC_OP_READ_SIBLING_BEGIN etc
 *                option1/option2 - specifies any additional option for
 *                populating in DB
 *                vect_key_operations - vector of type ODBCMOperator
 *                old_value_struct - holds the old value structure of the
 *                port key type
 *                row_status - enum indicating the row status of port type
 *                entries in db
 *                is_filtering/is_state - bool variables
 * @return      : None
 **/
void Kt_Port_Neighbor::PopulateDBSchemaForKtTable(
    OdbcmConnectionHandler *db_conn,
    DBTableSchema &kt_port_dbtableschema,
    void* key_struct,
    void* val_struct,
    uint8_t operation_type,
    uint32_t data_type,
    uint32_t option1,
    uint32_t option2,
    vector<ODBCMOperator> &vect_key_operations,
    void* &old_val_struct,
    CsRowStatus row_status,
    pfc_bool_t is_filtering,
    pfc_bool_t is_state) {
  // Construct Primary key list
  vector<string> vect_prim_keys;
  // construct TableAttrSchema structure
  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  key_port_t *obj_key_port = reinterpret_cast<key_port_t*>(key_struct);
  val_port_st_neighbor_t *obj_val_port =
         reinterpret_cast<val_port_st_neighbor_t*>(val_struct);

  // controller_name
  string controller_name = (const char*)obj_key_port
      ->sw_key.ctr_key.controller_name;
  vect_prim_keys.push_back(CTR_NAME_STR);
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // switch_id
  string switch_id = (const char*)obj_key_port->sw_key.switch_id;
  vect_prim_keys.push_back(SWITCH_ID_STR);
  PhyUtil::FillDbSchema(unc::uppl::SWITCH_ID, switch_id,
                        switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);
  // port_id
  string port_id = (const char*)obj_key_port->port_id;
  pfc_log_info("ctr name:%s,switch id:%s,port id:%s,oper_type:%d",
                 controller_name.c_str(),
                 switch_id.c_str(),
                 port_id.c_str(),
                 operation_type);
  PhyUtil::FillDbSchema(unc::uppl::PORT_ID, port_id,
                        port_id.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  vect_prim_keys.push_back(PORT_ID_STR);
  stringstream valid;
  PopulatePortStateNeighborValStructure(
      db_conn, obj_val_port,
      vect_table_attr_schema,
      vect_prim_keys,
      operation_type,
      reinterpret_cast<val_port_st_neighbor_t*>(old_val_struct),
      valid);
  PhyUtil::reorder_col_attrs(vect_prim_keys, vect_table_attr_schema);
  kt_port_dbtableschema.set_table_name(unc::uppl::PORT_TABLE);
  kt_port_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_port_dbtableschema.set_row_list(row_list);
  return;
}

/** 
 * @Description : This function reads the values given in val_port_st structure
 * @param[in]   : obj_val_port - pointer to structure val_port_st_t
 *                vect_table_attr_schema - vector to store the table attribute
 *                schema of key type port
 *                vect_prim_keys - vector to store the primary keys of port key
 *                type
 *                operation_type - indicates the operation type
 *                val_port_valid_st - pointer to the structure val_port_st_neighbor_t
 *                valid - reference variable to store the valid flags of port
 *                value structure
 * @return      : None
 **/
void Kt_Port_Neighbor::PopulatePortStateNeighborValStructure(
    OdbcmConnectionHandler *db_conn,
    val_port_st_neighbor_t *obj_val_port,
    vector<TableAttrSchema> &vect_table_attr_schema,
    vector<string> &vect_prim_keys,
    uint8_t operation_type,
    val_port_st_neighbor_t *val_port_nbr_st,
    stringstream &valid) {
  string value = "";
  uint16_t valid_val = 0, prev_db_val = 0;
  // connected switch id
  if (obj_val_port != NULL) {
    valid_val = PhyUtil::uint8touint(
         obj_val_port->valid[kIdxPortConnectedSwitchId]);
    if (valid_val == UNC_VF_VALID) {
      value = reinterpret_cast<const char*>(obj_val_port->connected_switch_id);
    }
    // val_port_nbr_st(old value structure) is NULL in case of DELETE
    if (operation_type == UNC_OP_UPDATE && val_port_nbr_st != NULL) {
      prev_db_val =
          PhyUtil::uint8touint(
              val_port_nbr_st->valid[kIdxPortConnectedSwitchId]);
    }
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(unc::uppl::PORT_CONNECTED_SWITCH_ID,
                        PORT_CONNECTED_SWITCH_ID_STR,
                        value, value.length(), DATATYPE_UINT8_ARRAY_256,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // connected port_id
  if (obj_val_port != NULL) {
    valid_val = PhyUtil::uint8touint(
                 obj_val_port->valid[kIdxPortConnectedPortId]);
    if (valid_val == UNC_VF_VALID) {
      value = reinterpret_cast<const char*>(obj_val_port->connected_port_id);
    }
    // val_port_nbr_st(old value structure) is NULL in case of DELETE
    if (operation_type == UNC_OP_UPDATE && val_port_nbr_st != NULL) {
      prev_db_val =
          PhyUtil::uint8touint(
              val_port_nbr_st->valid[kIdxPortConnectedPortId]);
    }
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(unc::uppl::PORT_CONNECTED_PORT_ID,
                        PORT_CONNECTED_PORT_ID_STR, value,
                        value.length(), DATATYPE_UINT8_ARRAY_32,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();
  // connected controller_id
  if (obj_val_port != NULL) {
    valid_val = PhyUtil::uint8touint(obj_val_port->
                                     valid[kIdxPortConnectedControllerId]);
    if (valid_val == UNC_VF_VALID) {
      value = reinterpret_cast<const char*>(
                obj_val_port->connected_controller_id);
    }
    // val_port_nbr_st(old value structure) is NULL in case of DELETE
    if (operation_type == UNC_OP_UPDATE && val_port_nbr_st != NULL) {
      prev_db_val =
          PhyUtil::uint8touint(
              val_port_nbr_st->valid[kIdxPortConnectedControllerId]);
    }
  } else {
    valid_val = UPPL_NO_VAL_STRUCT;
  }
  PhyUtil::FillDbSchema(unc::uppl::PORT_CONNECTED_CONTROLLER_ID,
                        PORT_CONNECTED_CONTROLLER_ID_STR,
                        value, value.length(), DATATYPE_UINT8_ARRAY_32,
                        operation_type, valid_val, prev_db_val,
                        vect_table_attr_schema, vect_prim_keys, valid);
  value.clear();

  // connectedneighbor_valid
  valid_val = UPPL_NO_VAL_STRUCT;
  prev_db_val = 0;
  stringstream dummy_valid;
  PhyUtil::FillDbSchema(unc::uppl::PORT_CONNECTEDNEIGHBOR_VALID,
                        PORT_CONNECTEDNEIGHBOR_VALID_STR, valid.str(),
                        valid.str().length(), DATATYPE_UINT8_ARRAY_3,
                        operation_type, valid_val, 0,
                        vect_table_attr_schema, vect_prim_keys, dummy_valid);
  return;
}


/** 
 * @Description : This function performs syntax validation for key type kt_port
 * @param[in]   : key_struct - void pointer to be type cast into port key type
 *                val_struct - void pointer to be type cast into va structure of
 *                key type port
 *                operation_type - UNC_OP* - indicates the operation type
 *                data_type - UNC_DT_* - indicates the data base type
 * @return      : UNC_RC_SUCCESS is returned when the validation is successful
 *                UNC_UPPL_RC_ERR_* is returned when validation is failure
 **/
UncRespCode Kt_Port_Neighbor::PerformSyntaxValidation(
                                                OdbcmConnectionHandler *db_conn,
                                                void* key_struct,
                                                void* val_struct,
                                                uint32_t operation,
                                                uint32_t data_type) {
  UncRespCode ret_code = UNC_RC_SUCCESS;
  pfc_bool_t mandatory = PFC_TRUE;
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map =
      attr_syntax_map_all[UNC_KT_PORT_NEIGHBOR];
  // Validate key structure
  // validate controller_name
  key_port *key = reinterpret_cast<key_port_t*>(key_struct);
  string value = reinterpret_cast<char*>(key->sw_key.ctr_key.controller_name);
  IS_VALID_STRING_KEY(CTR_NAME_STR, value, operation, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }

  // validate switch_id
  value = reinterpret_cast<char*>(key->sw_key.switch_id);
  IS_VALID_STRING_KEY(SWITCH_ID_STR, value, operation, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }

  // validate port_id
  value = reinterpret_cast<char*>(key->port_id);
  IS_VALID_STRING_KEY(PORT_ID_STR, value, operation, ret_code, mandatory);
  if (ret_code != UNC_RC_SUCCESS) {
    return UNC_UPPL_RC_ERR_CFG_SYNTAX;
  }

  // Validate value structure
  if (val_struct != NULL) {
    unsigned int valid_val = 0;
    val_port_st_neighbor *value_port =
      reinterpret_cast<val_port_st_neighbor_t*>(val_struct);

    // validate connected_controller
    valid_val = PhyUtil::uint8touint(
        value_port->valid[kIdxPortConnectedControllerId]);
    value = reinterpret_cast<char*>(value_port->connected_controller_id);
    IS_VALID_STRING_VALUE(PORT_CONNECTED_CONTROLLER_ID_STR, value, operation,
                       valid_val, ret_code, mandatory);
    if (ret_code != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }

    // validate connected_switch
    valid_val = PhyUtil::uint8touint(
                   value_port->valid[kIdxPortConnectedSwitchId]);
    value = reinterpret_cast<char*>(value_port->connected_switch_id);
    IS_VALID_STRING_VALUE(PORT_CONNECTED_SWITCH_ID_STR, value, operation,
                          valid_val, ret_code, mandatory);
    if (ret_code != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }
    // validate connected_port
    valid_val = PhyUtil::uint8touint(
        value_port->valid[kIdxPortConnectedPortId]);
    value = reinterpret_cast<char*>(value_port->connected_port_id);
    IS_VALID_STRING_VALUE(PORT_CONNECTED_PORT_ID_STR, value,
                       operation, valid_val, ret_code, mandatory);
    if (ret_code != UNC_RC_SUCCESS) {
      return UNC_UPPL_RC_ERR_CFG_SYNTAX;
    }
  }
  return ret_code;
}

/** 
 * @Description : This function performs semantic validation
 *                for UNC_KT_PORT_NEIGHBOR
 * @param[in]   : key_struct - void pointer to be type cast into port key type
 *                val_struct - void pointer to be type cast into val structure
 *                of key type port
 *                operation_type - UNC_OP* - indicates the operation type
 *                data_type - UNC_DT_* - indicates the data base type
 * @return      : UNC_RC_SUCCESS is returned when the semantic validation is
 *                successful
 *                UNC_UPPL_RC_ERR_* is returned when semantic validation is failure
 * */
UncRespCode Kt_Port_Neighbor::PerformSemanticValidation(
    OdbcmConnectionHandler *db_conn,
    void* key_struct,
    void* val_struct,
    uint32_t operation,
    uint32_t data_type) {
  UncRespCode status = UNC_RC_SUCCESS;
  pfc_log_debug("Inside PerformSemanticValidation of KT_PORT_NEIGHBOR");
  // Check whether the given instance of port exists in DB
  key_port *obj_key_port = reinterpret_cast<key_port_t*>(key_struct);
  string controller_name = (const char*)obj_key_port->
      sw_key.ctr_key.controller_name;
  string switch_id = (const char*)obj_key_port->sw_key.switch_id;
  string port_id = (const char*)obj_key_port->port_id;

  // call Switch validation function which in turn invokes ctrl validation

  vector<string> vect_key_value;
  vect_key_value.push_back(controller_name);
  vect_key_value.push_back(switch_id);
  vect_key_value.push_back(port_id);
  Kt_Port Obj_Port;
  UncRespCode key_status = Obj_Port.IsKeyExists(
      db_conn, (unc_keytype_datatype_t)data_type, vect_key_value);
  pfc_log_debug("IsKeyExists status %d", key_status);
  // In case of create/update/delete/read operation, key should exist
  if (operation == UNC_OP_CREATE ||
             operation == UNC_OP_UPDATE ||
             operation == UNC_OP_DELETE) {
    if (key_status != UNC_RC_SUCCESS) {
      pfc_log_error("Key not found, C/U/D/R opern not allowed");
      status = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
    } else {
      pfc_log_debug("key exists, C/U/D/R operation allowed");
    }
  }
  if (operation == UNC_OP_CREATE && status == UNC_RC_SUCCESS) {
    vector<string> parent_vect_key_value;
    parent_vect_key_value.push_back(controller_name);
    parent_vect_key_value.push_back(switch_id);
    Kt_Switch KtObj;
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

/** ReadPortNeighbor
 * @Description : This function reads the connectedneighbor info of the given port key
 * @param[in] : key_struct
 * data_type-UNC_DT_*,type of database
 * param[out]:oper_status-oper status of Controller whether up/down/auditing
 * @return    : Success or associated error code
 * UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 */
UncRespCode Kt_Port_Neighbor::ReadPortNeighbor(OdbcmConnectionHandler *db_conn,
            uint32_t data_type, void* key_struct, uint32_t operation_type,
            uint32_t &max_rep_ct, vector<val_port_st_neighbor> &vect_val_port_nbr,
                               vector<key_port_t> &vect_key_port) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode return_code = UNC_RC_SUCCESS;
  vector<string> vect_prim_keys;
  vector<TableAttrSchema> vect_table_attr_schema;
  list < vector<TableAttrSchema> > row_list;
  DBTableSchema kt_port_dbtableschema;

  key_port_t *obj_key_port = reinterpret_cast<key_port_t*>(key_struct);
  // Controller_name
  string controller_name =
    (const char*)obj_key_port->sw_key.ctr_key.controller_name;
  pfc_log_info("controller name: %s", controller_name.c_str());
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  if (!controller_name.empty()) {
    vect_prim_keys.push_back(CTR_NAME_STR);
  }
  // switch_id
  string switch_id = (const char*)obj_key_port->sw_key.switch_id;
  if (!switch_id.empty()) {
    vect_prim_keys.push_back(SWITCH_ID_STR);
  }
  PhyUtil::FillDbSchema(unc::uppl::SWITCH_ID, switch_id,
                        switch_id.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);
  // port_id
  string port_id = (const char*)obj_key_port->port_id;
  if (operation_type == UNC_OP_READ_SIBLING_BEGIN ||
      operation_type == UNC_OP_READ_SIBLING_COUNT) {
    // Ignore port_id
    port_id = "";
  }
  PhyUtil::FillDbSchema(unc::uppl::PORT_ID, port_id,
                        port_id.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  vect_prim_keys.push_back(PORT_ID_STR);

  pfc_log_info("ctr name:%s,switch id:%s,port id:%s",
                 controller_name.c_str(),
                 switch_id.c_str(),
                 port_id.c_str());
  // connected switch id
  string value = "";
  PhyUtil::FillDbSchema(unc::uppl::PORT_CONNECTED_SWITCH_ID, value,
                        value.length(), DATATYPE_UINT8_ARRAY_256,
                        vect_table_attr_schema);
  // connected port id
  PhyUtil::FillDbSchema(unc::uppl::PORT_CONNECTED_PORT_ID, value,
                        value.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  // connected controller id
  PhyUtil::FillDbSchema(unc::uppl::PORT_CONNECTED_CONTROLLER_ID, value,
                        value.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  // connectedneighbor valid
  PhyUtil::FillDbSchema(unc::uppl::PORT_CONNECTEDNEIGHBOR_VALID, value,
                        value.length(), DATATYPE_UINT8_ARRAY_3,
                        vect_table_attr_schema);


  kt_port_dbtableschema.set_table_name(unc::uppl::PORT_TABLE);
  kt_port_dbtableschema.set_primary_keys(vect_prim_keys);
  row_list.push_back(vect_table_attr_schema);
  kt_port_dbtableschema.set_row_list(row_list);
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;
  if (operation_type == UNC_OP_READ) {
    read_db_status = physical_layer->get_odbc_manager()->
      GetOneRow((unc_keytype_datatype_t)data_type,
                    kt_port_dbtableschema, db_conn);
  } else {
    read_db_status = physical_layer->get_odbc_manager()->
        GetBulkRows((unc_keytype_datatype_t)data_type, max_rep_ct,
                    kt_port_dbtableschema,
                    (unc_keytype_operation_t)operation_type, db_conn);

  }
  if (read_db_status == ODBCM_RC_SUCCESS) {
    // populate IPC value structure based on the response received from DB
    list < vector<TableAttrSchema> >& res_port_row_list =
      kt_port_dbtableschema.get_row_list();
    list < vector<TableAttrSchema> > :: iterator res_port_iter =
      res_port_row_list.begin();

    max_rep_ct = res_port_row_list.size();
    pfc_log_debug("res_port_row_list.size: %d", max_rep_ct);
    // populate IPC value structure based on the response received from DB
    for (; res_port_iter != res_port_row_list.end(); ++res_port_iter) {

      vector<TableAttrSchema> res_table_attr_schema = (*res_port_iter);
        /*kt_port_dbtableschema.get_row_list().front()*/;
      vector<TableAttrSchema> ::iterator vect_iter =
          res_table_attr_schema.begin();
      key_port_t obj_key_port;
      memset(&obj_key_port, '\0', sizeof(obj_key_port));
      val_port_st_neighbor_t val_struct;
      memset(&val_struct, '\0', sizeof(val_port_st_neighbor_t));
      for (; vect_iter != res_table_attr_schema.end(); ++vect_iter) {
        TableAttrSchema tab_schema = (*vect_iter);
        ODBCMTableColumns attr_name = tab_schema.table_attribute_name;
        switch (attr_name) {
          case unc::uppl::CTR_NAME:
          {
            PhyUtil::GetValueFromDbSchemaStr(
                tab_schema,
                obj_key_port.sw_key.ctr_key.controller_name,
                DATATYPE_UINT8_ARRAY_32);
            pfc_log_debug("controller_name: %s", reinterpret_cast<char *>
            (&obj_key_port.sw_key.ctr_key.controller_name));
            break;
          }
          case unc::uppl::SWITCH_ID:
          {
            PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_key_port.sw_key.switch_id,
                                           DATATYPE_UINT8_ARRAY_256);
            pfc_log_debug("switch_id: %s", reinterpret_cast<char *>
            (&obj_key_port.sw_key.switch_id));
            break;
          }
          case unc::uppl::PORT_ID:
          {
            PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                           obj_key_port.port_id,
                                           DATATYPE_UINT8_ARRAY_32);
            pfc_log_debug("port_id: %s", reinterpret_cast<char *>
            (&obj_key_port.port_id));
            break;
          }
          case unc::uppl::PORT_CONNECTED_SWITCH_ID:
          {
            PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                val_struct.connected_switch_id,
                                      DATATYPE_UINT8_ARRAY_256);
            pfc_log_info("connected_switch_id from DB: %s",
            val_struct.connected_switch_id);
            break;
          }
          case unc::uppl::PORT_CONNECTED_PORT_ID:
          {
            PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                   val_struct.connected_port_id,
                                      DATATYPE_UINT8_ARRAY_32);
            pfc_log_info("connected_port_id from DB: %s",
            val_struct.connected_port_id);
            break;
          }
          case unc::uppl::PORT_CONNECTED_CONTROLLER_ID:
          {
            PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                         val_struct.connected_controller_id,
                                      DATATYPE_UINT8_ARRAY_32);
            pfc_log_info("connected_controller_id from DB: %s",
              val_struct.connected_controller_id);
            break;
          }
          case unc::uppl::PORT_CONNECTEDNEIGHBOR_VALID:
          {
            uint8_t nbr_valid[ODBCM_SIZE_3];
            memset(nbr_valid, '\0', ODBCM_SIZE_3);
            PhyUtil::GetValueFromDbSchemaStr(tab_schema, nbr_valid,
                                      DATATYPE_UINT8_ARRAY_3);
            FrameCValidValue(reinterpret_cast<const char*>(nbr_valid),
                          val_struct);
            break;
          }
          default:
          {
            pfc_log_info("Ignoring Port attribute %d", attr_name);
            break;
          }
        }
      }
      vect_val_port_nbr.push_back(val_struct);
      vect_key_port.push_back(obj_key_port);
    }
  } else if (read_db_status == ODBCM_RC_RECORD_NOT_FOUND) {
    pfc_log_debug("No record to read");
    return_code = UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
  } else if (read_db_status == ODBCM_RC_CONNECTION_ERROR) {
    return_code = UNC_UPPL_RC_ERR_DB_ACCESS;
    pfc_log_error("Read operation has failed with %d", read_db_status);
  } else {
    return_code = UNC_UPPL_RC_ERR_DB_GET;
    pfc_log_error("Read operation has failed with %d", read_db_status);
  }
  return return_code;
}

/** 
 * @Description : This function reads the given  instance of KT_Port_Neighbor
 * @param[in]   : key_val - vector to hold the primary key values 
 *                val_struct - vector to hold the value structure
 *                data_type - indicates the data base type, UNC_DT_* ,
 *                read allowed in DT_STATE
 *                operation_type - indicates the operation type for
 *                read operation
 * @return      : UNC_RC_SUCCESS is returned when read operation is success on
 *                DT_STATE db
 *                UNC_UPPL_RC_ERR_* is returned when read operation is failure
 **/

UncRespCode Kt_Port_Neighbor::ReadInternal(OdbcmConnectionHandler *db_conn,
           vector<void *> &key_val, vector<void *> &val_struct,
           uint32_t data_type, uint32_t operation_type) {
  if (operation_type != UNC_OP_READ && operation_type != UNC_OP_READ_SIBLING &&
      operation_type != UNC_OP_READ_SIBLING_BEGIN) {
    pfc_log_trace("This function not allowed for read next/bulk/count");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
  }
  pfc_log_debug("Processing Kt_Port::ReadInternal");
  vector<key_port_t> vect_port_id;
  vector<val_port_st_neighbor_t> vect_val_port_nbr;
  uint32_t max_rep_ct = 1;
  if (operation_type != UNC_OP_READ) {
    max_rep_ct = UPPL_MAX_REP_CT;
  }
  // Get read response from database
  val_port_st_neighbor_t obj_port_val;
  memset(&obj_port_val, '\0', sizeof(val_port_st_neighbor_t));
  void *key_struct = key_val[0];
  UncRespCode read_status = UNC_RC_SUCCESS;
  bool firsttime = true;
  do {
    read_status = ReadPortNeighbor(db_conn, data_type, key_struct,
                 operation_type, max_rep_ct, vect_val_port_nbr, vect_port_id);
    if (firsttime) {
      pfc_log_trace(
          "Clearing key_val and val_struct vectors for the first time");
      key_val.clear();
      val_struct.clear();
      firsttime = false;
    }
    if (read_status == UNC_RC_SUCCESS) {
      pfc_log_debug("ReadPortNeighbor returned %d with response size %"
                  PFC_PFMT_SIZE_T, read_status, vect_val_port_nbr.size());
      for (unsigned int iIndex = 0 ; iIndex < vect_val_port_nbr.size();
          ++iIndex) {
        key_port_t *key_port = new key_port_t(vect_port_id[iIndex]);
        val_port_st_neighbor_t *val_port = new val_port_st_neighbor_t(
                                                vect_val_port_nbr[iIndex]);
        key_val.push_back(reinterpret_cast<void *>(key_port));
        val_struct.push_back(reinterpret_cast<void *>(val_port));
      }
    } else if ((read_status == UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE &&
               val_struct.size() != 0)) {
      read_status = UNC_RC_SUCCESS;
    }
    if ((vect_val_port_nbr.size() == UPPL_MAX_REP_CT) &&
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

/** FrameCValidValue 
 * @Description : This function converts the string value from db to uint8
 * @param[in] : attr_value-attribute value in string
 * obj_cv_ctr-instance of val_ctr_commit_ver_t
 * @return    : void 
 * */
void Kt_Port_Neighbor::FrameCValidValue(string attr_value,
                    val_port_st_neighbor_t &obj_port_nbr) {
  if (attr_value.length() == 0) {
    obj_port_nbr.valid[kIdxPortConnectedSwitchId] = UNC_VF_INVALID;
    obj_port_nbr.valid[kIdxPortConnectedPortId] = UNC_VF_INVALID;
    obj_port_nbr.valid[kIdxPortConnectedControllerId] = UNC_VF_INVALID;
    return;
  }
  for (unsigned int index = 0; index < attr_value.length(); ++index) {
    if (attr_value[index] >= 48) {
      obj_port_nbr.valid[index+1] = attr_value[index] - 48;
      pfc_log_debug("FrameCValidValue: valid flag %d is %d", index,
                                      obj_port_nbr.valid[index+1]);
    }
  }
}


/** UpdateKeyInstance
 * @Description : This function updates a row of KT in
 * candidate  table.
 * @param[in] :
 * key_struct - the key for the new kt instance
 * value_struct - the values for the new kt instance
 * data_type - UNC_DT_* , update only allowed in running/state/import
 * key_type-UNC_KT_*,any value of unc_key_type_t
 * old_val_struct-void * to switch value structure
 * @return    : UNC_RC_SUCCESS is returned when the update
 * is done successfully.
 * UNC_UPPL_RC_ERR_* is returned when the update is error
 * */
UncRespCode Kt_Port_Neighbor::UpdateKeyInstance(OdbcmConnectionHandler *db_conn,
                                                void* key_struct,
                                                void* val_struct,
                                                uint32_t data_type,
                                                uint32_t key_type,
                                                void* &old_val_struct) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode update_status = UNC_RC_SUCCESS;
  // Check whether operation is allowed on the given DT type
  if (((unc_keytype_datatype_t)data_type != UNC_DT_STATE) &&
      ((unc_keytype_datatype_t)data_type != UNC_DT_IMPORT)) {
    pfc_log_error("Update operation is provided on unsupported data type %d",
                  data_type);
    update_status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    return update_status;
  }
  val_port_st_neighbor_t *old_val_port = new val_port_st_neighbor_t();
  vector<key_port_t> vect_key_port;
  vector<val_port_st_neighbor_t> vect_val_port_nbr;
  uint32_t max_rep_ct = 1;
  update_status = ReadPortNeighbor(db_conn, data_type,
                        key_struct, UNC_OP_READ,
                        max_rep_ct,
                        vect_val_port_nbr,
                        vect_key_port);
  if (update_status != UNC_RC_SUCCESS) {
    pfc_log_info("Read  of port neighbor info is not success:%d",
        update_status);
    if (old_val_port != NULL) {
      delete old_val_port;
      old_val_port = NULL;
    }
    return update_status;
  }
  *old_val_port = vect_val_port_nbr[0];
  old_val_struct = reinterpret_cast<void*>(old_val_port);
  // Structure used to send request to ODBC
  DBTableSchema kt_dbtableschema;
  vector<ODBCMOperator> vect_key_operations;
  // Create DBSchema structure for table
  PopulateDBSchemaForKtTable(db_conn, kt_dbtableschema,
                             key_struct,
                             val_struct,
                             UNC_OP_UPDATE, data_type, 0, 0,
                             vect_key_operations, old_val_struct,
                             NOTAPPLIED, PFC_FALSE, PFC_FALSE);
  if (old_val_port != NULL) {
      delete old_val_port;
      old_val_port = NULL;
  }
  if (old_val_struct != NULL) {
      old_val_struct = NULL;
  }

  if (!((kt_dbtableschema.get_row_list()).empty())) {
    // Send request to ODBC for kt_table update
    ODBCM_RC_STATUS update_db_status = physical_layer->get_odbc_manager()-> \
        UpdateOneRow((unc_keytype_datatype_t)data_type,
                     kt_dbtableschema, db_conn, true);
    if (update_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log error error to log daemon
      pfc_log_error("DB connection not available or cannot access DB");
      update_status = UNC_UPPL_RC_ERR_DB_ACCESS;
    } else if (update_db_status != ODBCM_RC_SUCCESS) {
      // log error to log daemon
      pfc_log_error("UpdateOneRow error response from DB is %d",
                    update_db_status);
      update_status = UNC_UPPL_RC_ERR_DB_UPDATE;
    } else {
      pfc_log_info("Update of kt in data type %d is success",
                   data_type);
    }
  }
  return update_status;
}


/** DeleteKeyInstance
 * @Description : This function deletes neighbor info of KT in
 * candidate  table.
 * @param[in] :
 * key_struct - the key for the new kt instance
 * value_struct - the values for the new kt instance
 * data_type - UNC_DT_* , update only allowed in running/state/import
 * key_type-UNC_KT_*,any value of unc_key_type_t
 * old_val_struct-void * to switch value structure
 * @return    : UNC_RC_SUCCESS is returned when the update
 * is done successfully.
 * UNC_UPPL_RC_ERR_* is returned when the update is error
 * */

UncRespCode Kt_Port_Neighbor::DeleteKeyInstance(OdbcmConnectionHandler *db_conn,
                                           void* key_struct,
                                           uint32_t data_type,
                                           uint32_t key_type) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode delete_status = UNC_RC_SUCCESS;
  // Check whether operation is allowed on the given DT type
  if (((unc_keytype_datatype_t)data_type != UNC_DT_STATE) &&
      ((unc_keytype_datatype_t)data_type != UNC_DT_IMPORT)) {
    pfc_log_error("Delete operation is provided on unsupported data type %d",
                  data_type);
    delete_status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    return delete_status;
  }
  // Structure used to send request to ODBC
  DBTableSchema kt_dbtableschema;
  vector<ODBCMOperator> vect_key_operations;
  val_port_st_neighbor_t *val_port_nbr = new val_port_st_neighbor_t;
  memset(val_port_nbr, '\0', sizeof(val_port_st_neighbor_t));
  memset(val_port_nbr->valid, UNC_VF_VALID_NO_VALUE,
          sizeof(val_port_nbr->valid));
  val_port_nbr->valid[kIdxPort] = UNC_VF_INVALID;
  void* old_val_struct = NULL;
  // Create DBSchema structure for table
  PopulateDBSchemaForKtTable(db_conn, kt_dbtableschema,
                             key_struct,
                             reinterpret_cast<void*>(val_port_nbr),
                             UNC_OP_UPDATE, data_type, 0, 0,
                             vect_key_operations,
                             old_val_struct,
                             NOTAPPLIED, PFC_FALSE, PFC_FALSE);
  if (val_port_nbr != NULL) {
      delete val_port_nbr;
      val_port_nbr = NULL;
  }
  if (!((kt_dbtableschema.get_row_list()).empty())) {
    // Send request to ODBC for kt_table update
    ODBCM_RC_STATUS update_db_status = physical_layer->get_odbc_manager()-> \
        UpdateOneRow((unc_keytype_datatype_t)data_type,
                     kt_dbtableschema, db_conn, true);
    if (update_db_status == ODBCM_RC_CONNECTION_ERROR) {
      // log error to log daemon
      pfc_log_error("DB connection not available or cannot access DB");
      delete_status = UNC_UPPL_RC_ERR_DB_ACCESS;
    } else if (update_db_status != ODBCM_RC_SUCCESS) {
      // log error to log daemon
      pfc_log_error("UpdateOneRow error response from DB is %d",
                    update_db_status);
      delete_status = UNC_UPPL_RC_ERR_DB_UPDATE;
    } else {
      pfc_log_info("Update of kt in data type %d is success",
                   data_type);
    }
  }
  return delete_status;
}

/** 
 * @Description : This function populates the values to be used for attribute
 *                validation
 * @param[in] : None
 * @return    : None
 **/
void Kt_Port_Neighbor::Fill_Attr_Syntax_Map() {
  map<string, Kt_Class_Attr_Syntax> attr_syntax_map;

  Kt_Class_Attr_Syntax objKeyAttrCtrSyntax =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 32, true, "" };
  attr_syntax_map[CTR_NAME_STR] = objKeyAttrCtrSyntax;

  Kt_Class_Attr_Syntax objKeyAttrSwitchSyntax =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 256, true, "" };
  attr_syntax_map[SWITCH_ID_STR] = objKeyAttrSwitchSyntax;

  Kt_Class_Attr_Syntax objKeyAttrPortSyntax =
  { PFC_IPCTYPE_UINT8, 0, 0, 1, 32, true, "" };
  attr_syntax_map[PORT_ID_STR] = objKeyAttrPortSyntax;

  Kt_Class_Attr_Syntax objAttrConnectedControllerIdSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 1, 32, true, "" };
  attr_syntax_map[PORT_CONNECTED_CONTROLLER_ID_STR] =
         objAttrConnectedControllerIdSyntax;

  Kt_Class_Attr_Syntax objAttrConnectedSwitchIdSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 1, 256, true, "" };
  attr_syntax_map[PORT_CONNECTED_SWITCH_ID_STR] =
          objAttrConnectedSwitchIdSyntax;

  Kt_Class_Attr_Syntax objAttrConnectedPortIdSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 1, 32, true, "" };
  attr_syntax_map[PORT_CONNECTED_PORT_ID_STR] = objAttrConnectedPortIdSyntax;

  Kt_Class_Attr_Syntax objAttrValidSyntax =
  { PFC_IPCTYPE_STRING, 0, 0, 0, 4, true, "" };
  attr_syntax_map[PORT_CONNECTEDNEIGHBOR_VALID_STR] = objAttrValidSyntax;
  attr_syntax_map_all[UNC_KT_PORT_NEIGHBOR] = attr_syntax_map;
}

