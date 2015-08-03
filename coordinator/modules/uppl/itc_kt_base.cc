/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   Kt Base implementation
 * @file    itc_kt_base.cc
 *
 */

#include "itc_kt_base.hh"
#include "itc_kt_state_base.hh"
#include "physicallayer.hh"
using unc::uppl::PhysicalLayer;

map<unc_key_type_t, map<string, Kt_Class_Attr_Syntax> > Kt_Base::
attr_syntax_map_all;

/** ValidateRequest
 * * @Description : This function performs syntax and semantic validation
 *                  for a specifed KT
 * * @param[in]   : key_struct - the key for the kt instance
 *                  val_struct - the val for the kt instance
 *                  data_type - UNC_DT_*, read allowed in
 *                  candidate/running/startup/state
 *                  key_type - specifies the kt instance
 * * @return      : UNC_RC_SUCCESS is returned when the response
 *                  is added to ipc session successfully.
 *                  UNC_UPPL_RC_ERR_* is returned when ipc response
 *                  could not be added to sess.
 * */
UncRespCode Kt_Base::ValidateRequest(OdbcmConnectionHandler *db_conn,
                                        void* key_struct,
                                        void* val_struct,
                                        uint32_t operation,
                                        uint32_t data_type,
                                        uint32_t key_type) {
  UncRespCode status = UNC_RC_SUCCESS;
  pfc_log_debug("Inside ValidateRequest of KT_BASE for Key type: %d", key_type);
  switch (key_type) {
    case UNC_KT_ROOT: {
      status = ValidateKtRoot(operation,
                              data_type);
      if (status != UNC_RC_SUCCESS) {
        return status;
      }
    }
    break;
    case UNC_KT_DATAFLOW:
    case UNC_KT_DATAFLOW_V2: {
      status = ValidateKtDataflow(operation,
                              data_type);
      if (status != UNC_RC_SUCCESS) {
        return status;
      }
    }
    break;
    case UNC_KT_CONTROLLER:
    case UNC_KT_BOUNDARY: {
      status = ValidateKtCtrlBdry(operation,
                                  data_type);
      if (status != UNC_RC_SUCCESS) {
        return status;
      }
    }
    break;
    case UNC_KT_CTR_DATAFLOW: {
      status = ValidateKtCtrDataflow(operation,
                              data_type);
      if (status != UNC_RC_SUCCESS) {
        return status;
      }
    }
    break;
    case UNC_KT_CTR_DOMAIN: {
      status = ValidateKtCtrDomain(operation,
                                   data_type);
      if (status != UNC_RC_SUCCESS) {
        return status;
      }
    }
    break;
    case UNC_KT_LOGICAL_PORT:
    case UNC_KT_LOGICAL_MEMBER_PORT:
    case UNC_KT_SWITCH:
    case UNC_KT_PORT:
    case UNC_KT_PORT_NEIGHBOR:
    case UNC_KT_LINK: {
      status = ValidateKtState(operation,
                               data_type);
      if (status != UNC_RC_SUCCESS) {
        return status;
      }
    }
    break;
    default : {
      pfc_log_error("Key type not supported");
      return UNC_UPPL_RC_ERR_KEYTYPE_NOT_SUPPORTED;
    }
  }
  if (operation == UNC_OP_READ || operation == UNC_OP_CREATE ||
      operation == UNC_OP_UPDATE || operation == UNC_OP_DELETE) {
    status = PerformSyntaxValidation(db_conn, key_struct,
                                     val_struct,
                                     operation,
                                     data_type);
    if (status != UNC_RC_SUCCESS) {
      // log error and send error response
      pfc_log_error("Syntax validation failed");
      // return the actual response
      return status;
    }
    if (operation != UNC_OP_READ) {
      status = PerformSemanticValidation(db_conn, key_struct,
                                         val_struct,
                                         operation,
                                         data_type);
      if (status != UNC_RC_SUCCESS) {
        // log error and send error response
        pfc_log_error("Semantic validation failed");
        // return the actual response
        return status;
      }
    }
  }
  return status;
}

/**Read
 * * @Description : This function reads a row of KT in
 *                  the given table of specified data type.
 * * @param[in]   : session_id - ipc session id used for TC validation
 *                  configuration_id - configuration id used for TC validation
 *                  key_struct - the key for the kt instance
 *                  data_type - UNC_DT_*, read allowed in
 *                  candidate/running/startup/state
 *                  sess - ipc server session where the
 *                  response has to be added
 *                  option1/option2 - specifies any additional
 * * @return      : UNC_RC_SUCCESS is returned when the response
 *                  is added to ipc session successfully.
 *                  UNC_UPPL_RC_ERR_* is returned when ipc response could not
 *                  be added to sess.
 * */
UncRespCode Kt_Base::Read(OdbcmConnectionHandler *db_conn,
                             uint32_t session_id,
                             uint32_t configuration_id,
                             void* key_struct,
                             void* val_struct,
                             uint32_t data_type,
                             ServerSession &sess,
                             uint32_t option1,
                             uint32_t option2) {
  return PerformRead(db_conn, session_id,
                     configuration_id,
                     key_struct,
                     val_struct,
                     data_type,
                     UNC_OP_READ,
                     sess,
                     option1,
                     option2,
                     0);
}

/**ReadNext
 * * @Description : This function reads a next row of key instance in
 *                  corresponding table of specified data type.
 * * @param[in]   : session_id - ipc session id used for TC validation
 *                  configuration_id - configuration id used for TC validation
 *                  key_struct - key instance
 *                  data_type - UNC_DT_* ,
 *                  readnext allowed in candidate/running/startup/state
 *                  sess - ipc server session
 *                  where the response has to be added
 * * @return      : UNC_RC_SUCCESS is returned when the response
 *                  is added to ipc session successfully.
 *                  UNC_UPPL_RC_ERR_* is returned when ipc
 *                  response could not be added to sess.
 * */
UncRespCode Kt_Base::ReadNext(OdbcmConnectionHandler *db_conn,
                                 uint32_t client_sess_id,
                                 void* key_struct,
                                 uint32_t data_type,
                                 ReadRequest *read_req) {
  uint32_t max_rep_ct = 1;
  return ReadBulk(db_conn, key_struct,
                  data_type,
                  max_rep_ct,
                  -1,
                  false,
                  true,
                  read_req);
}

/**ReadSiblingBegin
 * * @Description : This function reads sibling rows of given KT object
 *                  for a specified data type from the first instance
 * * @param[in]   : session_id - ipc session id used for TC validation
 *                  configuration_id - configuration id used for TC validation
 *                  key_struct - the key for the kt link instance
 *                  data_type - UNC_DT_*, readsibling allowed in
 *                  candidate/running/startup/state
 *                  sess - ipc server session where the response
 *                  has to be added
 *                  option1/option2 - specifies any additional
 *                  condition for read operation
 *                  max_rep_ct - specifies number of rows to be returned
 * * @return      : UNC_RC_SUCCESS is returned when the response
 *                  is added to ipc session successfully.
 *                  UNC_UPPL_RC_ERR_* is returned when ipc
 *                  response could not be added to sess.
 * */
UncRespCode Kt_Base::ReadSiblingBegin(OdbcmConnectionHandler *db_conn,
                                         uint32_t session_id,
                                         uint32_t configuration_id,
                                         void* key_struct,
                                         void* val_struct,
                                         uint32_t data_type,
                                         ServerSession &sess,
                                         uint32_t option1,
                                         uint32_t option2,
                                         uint32_t &max_rep_ct) {
  UncRespCode read_status = PerformRead(db_conn, session_id,
                                           configuration_id,
                                           key_struct,
                                           val_struct,
                                           data_type,
                                           UNC_OP_READ_SIBLING_BEGIN,
                                           sess,
                                           option1,
                                           option2,
                                           max_rep_ct);
  pfc_log_debug("Read Sibling Begin operation return %d", read_status);
  return read_status;
}

/**ReadSibling
 * * @Description : This function reads sibling rows of KT in
 *                  the given table of specified data type.
 * * @param[in]   : session_id - ipc session id used for TC validation
 *                  configuration_id - configuration id used for TC validation
 *                  key_struct - the key for the kt instance
 *                  data_type - UNC_DT_* , read sibling begin allowed
 *                  in candidate/running/startup/state
 *                  sess - ipc server session where the response has to be added
 *                  option1/option2 - specifies any additional
 *                  condition for read operation
 *                  max_rep_ct - specifies number of rows to be returned
 * * @return      : UNC_RC_SUCCESS is returned when the response
 *                  is added to ipc session successfully.
 *                  UNC_UPPL_RC_ERR_* is returned when ipc response
 *                  could not be added to sess.
 * */
UncRespCode Kt_Base::ReadSibling(OdbcmConnectionHandler *db_conn,
                                    uint32_t session_id,
                                    uint32_t configuration_id,
                                    void* key_struct,
                                    void* val_struct,
                                    uint32_t data_type,
                                    ServerSession &sess,
                                    uint32_t option1,
                                    uint32_t option2,
                                    uint32_t &max_rep_ct) {
  UncRespCode read_status = PerformRead(db_conn, session_id,
                                           configuration_id,
                                           key_struct,
                                           val_struct,
                                           data_type,
                                           UNC_OP_READ_SIBLING,
                                           sess,
                                           option1,
                                           option2,
                                           max_rep_ct);
  pfc_log_debug("Read Sibling operation return %d", read_status);
  return read_status;
}

/**ReadSiblingCount
 * * @Description : This function reads number of KT instances in
 *                  the given table of specified data type.
 * * @param[in]   : session_id - ipc session id used for TC validation
 *                  configuration_id - configuration id used for TC validation
 *                  key_struct - the key for the kt instance
 *                  data_type - UNC_DT_* , read sibling count allowed
 *                  in candidate/running/startup/state
 *                  sess - ipc server session where the response has to be added
 *                  option1/option2 - specifies any additional condition
 *                  for read operation
 *                  max_rep_ct - specifies number of rows to be returned
 * * @return      : UNC_RC_SUCCESS is returned when the response
 *                  is added to ipc session successfully.
 *                  UNC_UPPL_RC_ERR_* is returned when ipc response
 *                  could not be added to sess.
 * */
UncRespCode Kt_Base::ReadSiblingCount(OdbcmConnectionHandler *db_conn,
                                         uint32_t session_id,
                                         uint32_t configuration_id,
                                         void* key_struct,
                                         void* val_struct,
                                         uint32_t key_type,
                                         uint32_t data_type,
                                         ServerSession &sess,
                                         uint32_t option1,
                                         uint32_t option2) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  // Fill the IPC response message in session
  physical_response_header rsh = {session_id,
      configuration_id,
      UNC_OP_READ_SIBLING_COUNT,
      0,
      option1,
      option2,
      data_type,
      0};
  if (option1 != UNC_OPT1_NORMAL) {
    pfc_log_error("Sibling Count is provided on unsupported option1");
    rsh.result_code = UNC_UPPL_RC_ERR_INVALID_OPTION1;
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= AddKeyStructuretoSession(key_type, &sess, key_struct);
    if (err != 0) {
      pfc_log_info("addOutput failed for physical_response_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return (UncRespCode)rsh.result_code;
  }
  uint32_t count = 0;
  // Structure used to send request to ODBC
  DBTableSchema kt_dbtableschema;
  vector<ODBCMOperator> vect_prim_key_operations;
  void* old_value = NULL;
  PopulateDBSchemaForKtTable(db_conn, kt_dbtableschema,
                             key_struct,
                             val_struct,
                             UNC_OP_READ_SIBLING_COUNT, data_type,
                             option1, option2,
                             vect_prim_key_operations,
                             old_value, NOTAPPLIED, false, PFC_FALSE);
  ODBCM_RC_STATUS read_db_status = ODBCM_RC_SUCCESS;
  if (!vect_prim_key_operations.empty()) {
    read_db_status = physical_layer->get_odbc_manager()->
        GetSiblingCount((unc_keytype_datatype_t)data_type,
                        kt_dbtableschema, count,
                        vect_prim_key_operations, db_conn);
  } else {
    read_db_status = physical_layer->get_odbc_manager()-> \
        GetSiblingCount((unc_keytype_datatype_t)data_type,
                        kt_dbtableschema, count, db_conn);
  }
  // count
  if (read_db_status != ODBCM_RC_SUCCESS) {
    if (read_db_status == ODBCM_RC_CONNECTION_ERROR) {
      rsh.result_code = UNC_UPPL_RC_ERR_DB_ACCESS;
    } else {
      rsh.result_code = UNC_UPPL_RC_ERR_DB_GET;
    }
    // log error to log daemon
    pfc_log_error("Sibling Count operation has failed");
    int err = PhyUtil::sessOutRespHeader(sess, rsh);
    err |= AddKeyStructuretoSession(key_type, &sess, key_struct);
    if (err != 0) {
      pfc_log_info("addOutput failed for physical_response_header");
      return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
    }
    return (UncRespCode)rsh.result_code;
  } else {
    pfc_log_info("Count from DB: %d", count);
  }
  rsh.result_code = UNC_RC_SUCCESS;
  int err = PhyUtil::sessOutRespHeader(sess, rsh);
  err |= AddKeyStructuretoSession(key_type, &sess, key_struct);
  if (err != 0) {
    pfc_log_info("addOutput failed for physical_response_header");
    return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  }
  sess.addOutput(count);
  return (UncRespCode)rsh.result_code;
}

/** ConfigurationChangeNotification
 * * @Description : This function frames notification to be sent to NB for
 *                  the configuration changes done
 * * @param[in]   : date_type -  UNC_DT_*
 *                  operation_type - UNC_OP*
 *                  key_struct - specifies the key instance of KT
 *                  old_val_struct - old value struct of kt
 *                  new_val_struct - new value struct of kt
 * * @return      : UNC_RC_SUCCESS is notified to northbound successfully or
 *                  UNC_UPPL_RC_ERR*
 * */
UncRespCode Kt_Base::ConfigurationChangeNotification(
    uint32_t data_type,
    uint32_t key_type,
    uint32_t oper_type,
    void* key_struct,
    void* old_val_struct,
    void* new_val_struct) {
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();
  UncRespCode status = UNC_RC_SUCCESS;
  int err = 0;
  pfc_ipcevtype_t event_type = GetEventType(key_type);
  // Create ServerEvent object to be sent to NB
  ServerEvent ser_evt((pfc_ipcevtype_t)event_type, err);
  northbound_event_header rsh = {oper_type,
      data_type,
      key_type};
  err = PhyUtil::sessOutNBEventHeader(ser_evt, rsh);
  err |= AddKeyStructuretoSession(key_type,
                                  ser_evt,
                                  key_struct);
  err |= AddValueStructuretoSession(key_type,
                                    oper_type,
                                    data_type,
                                    ser_evt,
                                    new_val_struct,
                                    old_val_struct);
  if (err != 0) {
    pfc_log_error(
        "Server Event addOutput failed, return IPC_WRITE_ERROR");
    status = UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  } else {
    status = (UncRespCode) physical_layer->get_ipc_connection_manager()->
        SendEvent(&ser_evt, "", event_type);
    pfc_log_debug(
        "Configuration notification status: %d for operation %d",
        status, oper_type);
  }
  return status;
}

/** GetEventType
 * * @Description : This function gets the notification type for the given kt
 * * @param[in]   : key type - UNC_KT*
 * * @return      : event_type - pfc_ipcevtype_t
 * */
pfc_ipcevtype_t Kt_Base::GetEventType(uint32_t key_type) {
  pfc_ipcevtype_t event_type = 0;
  switch (key_type) {
    case UNC_KT_CONTROLLER: {
      event_type = UPPL_EVENTS_KT_CONTROLLER;
      break;
    }
    case UNC_KT_CTR_DOMAIN: {
      event_type = UPPL_EVENTS_KT_CTR_DOMAIN;
      break;
    }
    case UNC_KT_LOGICAL_PORT: {
      event_type = UPPL_EVENTS_KT_LOGICAL_PORT;
      break;
    }
    case UNC_KT_LOGICAL_MEMBER_PORT: {
      event_type = UPPL_EVENTS_KT_LOGICAL_MEMBER_PORT;
      break;
    }
    case UNC_KT_SWITCH: {
      event_type = UPPL_EVENTS_KT_SWITCH;
      break;
    }
    case UNC_KT_PORT: {
      event_type = UPPL_EVENTS_KT_PORT;
      break;
    }
    case UNC_KT_LINK: {
      event_type = UPPL_EVENTS_KT_LINK;
      break;
    }
    case UNC_KT_BOUNDARY: {
      event_type = UPPL_EVENTS_KT_BOUNDARY;
      break;
    }
    default: {
      // Do nothing
      break;
    }
  }
  return event_type;
}

/** AddKeyStructuretoSession
 * * @Description : This function adds key structure to sever session
 *                  for the given kt
 * * @param[in]   : key type - UNC_KT*
 *                  key_struct - void*
 * * @param[out]  : ServerSession - sess
 * * @return      : Server Session addOutput return value
 * */
int Kt_Base::AddKeyStructuretoSession(uint32_t key_type,
                                      ServerSession *sess,
                                      void *key_struct) {
  int err = sess->addOutput((uint32_t)key_type);
  switch (key_type) {
    case UNC_KT_CONTROLLER: {
      key_ctr_t *obj_key = reinterpret_cast<key_ctr_t*>(key_struct);
      err |= sess->addOutput(*obj_key);
      break;
    }
    case UNC_KT_DATAFLOW: {
      key_dataflow_t *obj_key = reinterpret_cast<key_dataflow_t*>(key_struct);
      err |= sess->addOutput(*obj_key);
      break;
    }
    case UNC_KT_DATAFLOW_V2: {
      key_dataflow_v2_t *obj_key = reinterpret_cast<key_dataflow_v2_t*>
                                   (key_struct);
      err |= sess->addOutput(*obj_key);
      break;
    }
    case UNC_KT_CTR_DOMAIN: {
      key_ctr_domain_t *obj_key =
          reinterpret_cast<key_ctr_domain_t*>(key_struct);
      err |= sess->addOutput(*obj_key);
      break;
    }
    case UNC_KT_LOGICAL_PORT: {
      key_logical_port_t *obj_key =
          reinterpret_cast<key_logical_port_t*>(key_struct);
      err |= sess->addOutput(*obj_key);
      break;
    }
    case UNC_KT_LOGICAL_MEMBER_PORT: {
      key_logical_member_port_t *obj_key =
          reinterpret_cast<key_logical_member_port_t*>(key_struct);
      err |= sess->addOutput(*obj_key);
      break;
    }
    case UNC_KT_SWITCH: {
      key_switch_t *obj_key =
          reinterpret_cast<key_switch_t*>(key_struct);
      err |= sess->addOutput(*obj_key);
      break;
    }
    case UNC_KT_PORT: {
      key_port_t *obj_key =
          reinterpret_cast<key_port_t*>(key_struct);
      err |= sess->addOutput(*obj_key);
      break;
    }
    case UNC_KT_LINK: {
      key_link_t *obj_key = reinterpret_cast<key_link_t*>(key_struct);
      err |= sess->addOutput(*obj_key);
      break;
    }
    case UNC_KT_BOUNDARY: {
      key_boundary_t *obj_key =
          reinterpret_cast<key_boundary_t*>(key_struct);
      err |= sess->addOutput(*obj_key);
      break;
    }
  }
  pfc_log_debug("AddKeyStructuretoSession: key_type %d, return %d",
                key_type, err);
  return err;
}

/** AddKeyStructuretoSession
 * * @Description : This function adds key structure to sever event
 *                  for the given kt
 * * @param[in]   : key type - UNC_KT*
 *                  key_struct - void*
 * * @param[out]  : ServerSession - sess
 * * @return      : Server Event add Output return
 * */
int Kt_Base::AddKeyStructuretoSession(uint32_t key_type,
                                      ServerEvent &ser_evt,
                                      void *key_struct) {
  int err = 0;
  switch (key_type) {
    case UNC_KT_CONTROLLER: {
      key_ctr_t *obj_key =
          reinterpret_cast<key_ctr_t*>(key_struct);
      err = ser_evt.addOutput(*obj_key);
      break;
    }
    case UNC_KT_CTR_DOMAIN: {
      key_ctr_domain_t *obj_key =
          reinterpret_cast<key_ctr_domain_t*>(key_struct);
      err = ser_evt.addOutput(*obj_key);
      break;
    }
    case UNC_KT_LOGICAL_PORT: {
      key_logical_port_t *obj_key =
          reinterpret_cast<key_logical_port_t*>(key_struct);
      err = ser_evt.addOutput(*obj_key);
      break;
    }
    case UNC_KT_LOGICAL_MEMBER_PORT: {
      key_logical_member_port_t *obj_key =
          reinterpret_cast<key_logical_member_port_t*>(key_struct);
      err = ser_evt.addOutput(*obj_key);
      break;
    }
    case UNC_KT_SWITCH: {
      key_switch_t *obj_key = reinterpret_cast<key_switch_t*>(key_struct);
      err = ser_evt.addOutput(*obj_key);
      break;
    }
    case UNC_KT_PORT: {
      key_port_t *obj_key = reinterpret_cast<key_port_t*>(key_struct);
      err = ser_evt.addOutput(*obj_key);
      break;
    }
    case UNC_KT_LINK: {
      key_link_t *obj_key = reinterpret_cast<key_link_t*>(key_struct);
      err = ser_evt.addOutput(*obj_key);
      break;
    }
    case UNC_KT_BOUNDARY: {
      key_boundary_t *obj_key =
          reinterpret_cast<key_boundary_t*>(key_struct);
      err = ser_evt.addOutput(*obj_key);
      break;
    }
    default : {
      // Do nothing
      break;
    }
  }
  pfc_log_debug("Return value AddKeyStructuretoSession %d", err);
  return err;
}

/** AddValueStructuretoSession
 * * @Description : This function adds value structure to sever event
 *                  for the given kt
 * * @param[in]   : key type - UNC_KT*
 *                  operation_type - UNC_OP*
 *                  old_val_struct - void*
 *                  new_val_struct - void*
 * * @param[out]  : ServerEvent - ser_evt
 * * @return      : Server Event add Output return
 * */
int Kt_Base::AddValueStructuretoSession(uint32_t key_type,
                                        uint32_t oper_type,
                                        uint32_t data_type,
                                        ServerEvent &ser_evt,
                                        void *new_val_struct,
                                        void *old_value_struct) {
  int err = 0;
  switch (key_type) {
    case UNC_KT_CONTROLLER: {
      if (oper_type == UNC_OP_CREATE) {
        val_ctr_st_t new_val_st =
            *(reinterpret_cast<val_ctr_st_t*>(new_val_struct));
        err |= ser_evt.addOutput(
            *(reinterpret_cast<val_ctr_t*>(&new_val_st.controller)));
      } else if (oper_type == UNC_OP_UPDATE) {
        val_ctr_st_t new_val_st =
            *(reinterpret_cast<val_ctr_st_t*>(new_val_struct));
        err |= ser_evt.addOutput(
            *(reinterpret_cast<val_ctr_t*>(&new_val_st.controller)));
        val_ctr_st_t old_val_st =
            *(reinterpret_cast<val_ctr_st_t*>(old_value_struct));
        err |= ser_evt.addOutput(
            *(reinterpret_cast<val_ctr_t*>(&old_val_st.controller)));
      }
      break;
    }
    case UNC_KT_CTR_DOMAIN: {
      if (oper_type == UNC_OP_CREATE && data_type == UNC_DT_RUNNING) {
        val_ctr_domain_st_t new_val_st =
            *(reinterpret_cast<val_ctr_domain_st_t*>(new_val_struct));
        err |= ser_evt.addOutput(
            *(reinterpret_cast<val_ctr_domain_t*>(&new_val_st.domain)));
      } else if (oper_type == UNC_OP_CREATE) {
        err |= ser_evt.addOutput(
            *(reinterpret_cast<val_ctr_domain_st_t*>(new_val_struct)));
      }
      if (oper_type == UNC_OP_UPDATE && data_type == UNC_DT_RUNNING) {
        val_ctr_domain_st_t new_val_st =
            *(reinterpret_cast<val_ctr_domain_st_t*>(new_val_struct));
        err |= ser_evt.addOutput(
            *(reinterpret_cast<val_ctr_domain_t*>(&new_val_st.domain)));
        val_ctr_domain_st_t old_val_st =
            *(reinterpret_cast<val_ctr_domain_st_t*>(old_value_struct));
        err |= ser_evt.addOutput(
            *(reinterpret_cast<val_ctr_domain_t*>(&old_val_st.domain)));
      } else if (oper_type == UNC_OP_UPDATE) {
        err |= ser_evt.addOutput(
            *(reinterpret_cast<val_ctr_domain_st_t*>(new_val_struct)));
        err |= ser_evt.addOutput(
            *(reinterpret_cast<val_ctr_domain_st_t*>(old_value_struct)));
      }
      break;
    }
    case UNC_KT_LOGICAL_PORT: {
      if (oper_type == UNC_OP_CREATE) {
        err |= ser_evt.addOutput(
            *(reinterpret_cast<val_logical_port_st_t*>(new_val_struct)));
      } else if (oper_type == UNC_OP_UPDATE) {
        err |= ser_evt.addOutput(
            *(reinterpret_cast<val_logical_port_st_t*>(new_val_struct)));
        err |= ser_evt.addOutput(
            *(reinterpret_cast<val_logical_port_st_t*>(old_value_struct)));
      }
      break;
    }
    case UNC_KT_LOGICAL_MEMBER_PORT: {
      if (oper_type == UNC_OP_CREATE) {
        err |=  ser_evt.addOutput();  // new value structure
      } else if (oper_type == UNC_OP_UPDATE) {
        err |= ser_evt.addOutput();  // new value structure
        err |= ser_evt.addOutput();  // old value structure
      }
      break;
    }
    case UNC_KT_SWITCH: {
      if (oper_type == UNC_OP_CREATE) {
        err |= ser_evt.addOutput(
            *(reinterpret_cast<val_switch_st_t*>(new_val_struct)));
      } else if (oper_type == UNC_OP_UPDATE) {
        err |= ser_evt.addOutput(
            *(reinterpret_cast<val_switch_st_t*>(new_val_struct)));
        err |= ser_evt.addOutput(
            *(reinterpret_cast<val_switch_st_t*>(old_value_struct)));
      }
      break;
    }
    case UNC_KT_PORT: {
      if (oper_type == UNC_OP_CREATE) {
        err |= ser_evt.addOutput(
            *(reinterpret_cast<val_port_st_t*>(new_val_struct)));
      } else if (oper_type == UNC_OP_UPDATE) {
        err |= ser_evt.addOutput(
            *(reinterpret_cast<val_port_st_t*>(new_val_struct)));
        err |= ser_evt.addOutput(
            *(reinterpret_cast<val_port_st_t*>(old_value_struct)));
      }
      break;
    }
    case UNC_KT_LINK: {
      if (oper_type == UNC_OP_CREATE) {
        err |= ser_evt.addOutput(
            *(reinterpret_cast<val_link_st_t*>(new_val_struct)));
      } else if (oper_type == UNC_OP_UPDATE) {
        err |= ser_evt.addOutput(
            *(reinterpret_cast<val_link_st_t*>(new_val_struct)));
        err |= ser_evt.addOutput(
            *(reinterpret_cast<val_link_st_t*>(old_value_struct)));
      }
      break;
    }
    case UNC_KT_BOUNDARY: {
      if (oper_type == UNC_OP_CREATE) {
        val_boundary_st_t new_val_st =
            *(reinterpret_cast<val_boundary_st_t*>(new_val_struct));
        err |= ser_evt.addOutput(
            *(reinterpret_cast<val_boundary_t*>(&new_val_st.boundary)));
      } else if (oper_type == UNC_OP_UPDATE) {
        val_boundary_st_t new_val_st =
            *(reinterpret_cast<val_boundary_st_t*>(new_val_struct));
        err |= ser_evt.addOutput(
            *(reinterpret_cast<val_boundary_t*>(&new_val_st.boundary)));
        val_boundary_st_t old_val_st =
            *(reinterpret_cast<val_boundary_st_t*>(old_value_struct));
        err |= ser_evt.addOutput(
            *(reinterpret_cast<val_boundary_t*>(&old_val_st.boundary)));
      }
      break;
    }
    default : {
      // Do nothing
      break;
    }
  }
  pfc_log_debug("Return value AddValueStructuretoSession %d", err);
  return err;
}

/** ClearValueStructure
 * * @Description : This function clears value structure created in read process
 * * @param[in]   : key type - UNC_KT*
 *                  old_val_struct - void*
 * * @return      : void
 * */
void Kt_Base::ClearValueStructure(uint32_t key_type,
                                  void *&old_value_struct) {
  switch (key_type) {
    case UNC_KT_CONTROLLER: {
      val_ctr_st_t *old_val_kt =
          reinterpret_cast<val_ctr_st_t*>(old_value_struct);
      if (old_val_kt != NULL) {
        delete old_val_kt;
        old_val_kt = NULL;
      }
      break;
    }
    case UNC_KT_CTR_DOMAIN: {
      val_ctr_domain_st_t *old_val_kt =
          reinterpret_cast<val_ctr_domain_st_t*>(old_value_struct);
      if (old_val_kt != NULL) {
        delete old_val_kt;
        old_val_kt = NULL;
      }
      break;
    }
    case UNC_KT_LOGICAL_PORT: {
      val_logical_port_st_t *old_val_kt =
          reinterpret_cast<val_logical_port_st_t*>(old_value_struct);
      if (old_val_kt != NULL) {
        delete old_val_kt;
        old_val_kt = NULL;
      }
      break;
    }
    case UNC_KT_LOGICAL_MEMBER_PORT: {
      break;
    }
    case UNC_KT_SWITCH: {
      val_switch_st_t *old_val_kt =
          reinterpret_cast<val_switch_st_t*>(old_value_struct);
      if (old_val_kt != NULL) {
        delete old_val_kt;
        old_val_kt = NULL;
      }
      break;
    }
    case UNC_KT_PORT: {
      val_port_st_t *old_val_kt =
          reinterpret_cast<val_port_st_t*>(old_value_struct);
      if (old_val_kt != NULL) {
        delete old_val_kt;
        old_val_kt = NULL;
      }
      break;
    }
    case UNC_KT_PORT_NEIGHBOR: {
      val_port_st_neighbor_t *old_val_kt =
          reinterpret_cast<val_port_st_neighbor_t*>(old_value_struct);
      if (old_val_kt != NULL) {
        delete old_val_kt;
        old_val_kt = NULL;
      }
      break;
    }
    case UNC_KT_LINK: {
      val_link_st_t *old_val_kt =
          reinterpret_cast<val_link_st_t*>(old_value_struct);
      if (old_val_kt != NULL) {
        delete old_val_kt;
        old_val_kt = NULL;
      }
      break;
    }
    case UNC_KT_BOUNDARY: {
      val_boundary_st_t *old_val_kt =
          reinterpret_cast<val_boundary_st_t*>(old_value_struct);
      if (old_val_kt != NULL) {
        delete old_val_kt;
        old_val_kt = NULL;
      }
      break;
    }
    default : {
      // Do nothing
      break;
    }
  }
}

UncRespCode Kt_Base::ValidateKtRoot(uint32_t operation,
                                       uint32_t data_type) {
  if ((operation != UNC_OP_READ_NEXT) && (operation != UNC_OP_READ_BULK)) {
    pfc_log_error("Operations other than READ_NEXT and READ_BULK"
        "are not allowed");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  }
  if ((data_type != UNC_DT_CANDIDATE) && (data_type != UNC_DT_STARTUP) &&
      (data_type != UNC_DT_RUNNING) && (data_type != UNC_DT_STATE)) {
    pfc_log_error("Data type other than STARTUP/CANDIDATE/RUNNING/STATE"
        "are not allowed");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  }
  return UNC_RC_SUCCESS;
}

UncRespCode Kt_Base::ValidateKtDataflow(uint32_t operation,
                                       uint32_t data_type) {
  if (operation != UNC_OP_READ) {
    pfc_log_error("Operations other than READ"
        " are not allowed");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  }
  if (data_type != UNC_DT_STATE) {
    pfc_log_error("Data type other than STATE"
        " are not allowed");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  }
  return UNC_RC_SUCCESS;
}

UncRespCode Kt_Base::ValidateKtCtrlBdry(uint32_t operation,
                                           uint32_t data_type) {
  if ( (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE ||
      operation == UNC_OP_DELETE) && data_type != UNC_DT_CANDIDATE) {
    pfc_log_error("Configuration operation only allowed in CANDIDATE DB");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  } else if (operation >= UNC_OP_READ && data_type != UNC_DT_CANDIDATE &&
      data_type != UNC_DT_STARTUP && data_type != UNC_DT_RUNNING &&
      data_type != UNC_DT_STATE) {
    pfc_log_error(
        "Read operations are not allowed in requested data type %d",
        data_type);
    return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  } else if (!(operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE ||
      operation == UNC_OP_DELETE || operation >= UNC_OP_READ)) {
    pfc_log_error("Invalid operation type");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
  }
  return UNC_RC_SUCCESS;
}

UncRespCode Kt_Base::ValidateKtCtrDataflow(uint32_t operation,
                                       uint32_t data_type) {
  if (operation != UNC_OP_READ) {
    pfc_log_error("Operations other than READ"
        "are not allowed");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  }
  if (data_type != UNC_DT_STATE) {
    pfc_log_error("Data type other than STATE "
        "are not allowed");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  }
  return UNC_RC_SUCCESS;
}

UncRespCode Kt_Base::ValidateKtCtrDomain(uint32_t operation,
                                            uint32_t data_type) {
  if ((operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE ||
      operation == UNC_OP_DELETE) && data_type != UNC_DT_CANDIDATE &&
      data_type != UNC_DT_STATE && data_type != UNC_DT_IMPORT) {
    pfc_log_error(
        "Configuration operation only allowed in "
        "CANDIDATE/STATE/IMPORT DB");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  } else if (operation >= UNC_OP_READ && data_type != UNC_DT_CANDIDATE &&
      data_type != UNC_DT_STARTUP && data_type != UNC_DT_RUNNING &&
      data_type != UNC_DT_STATE) {
    pfc_log_error(
        "Read operations are not allowed in requested data type");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  } else if (!(operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE ||
      operation == UNC_OP_DELETE || operation >= UNC_OP_READ)) {
    pfc_log_error("Invalid operation type");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
  }
  return UNC_RC_SUCCESS;
}

UncRespCode Kt_Base::ValidateKtState(uint32_t operation,
                                        uint32_t data_type) {
  if (operation >= UNC_OP_READ && data_type != UNC_DT_STATE) {
    pfc_log_error(
        "Read operations are not allowed in requested data type");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  } else if ( data_type != UNC_DT_STATE &&
      (operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE ||
          operation == UNC_OP_DELETE) &&
          data_type != UNC_DT_IMPORT) {
    pfc_log_error("Configuration operation only allowed in STATE DB");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
  } else if (!(operation >= UNC_OP_READ ||
      operation == UNC_OP_CREATE || operation == UNC_OP_UPDATE ||
      operation == UNC_OP_DELETE)) {
    pfc_log_error("Invalid operation type");
    return UNC_UPPL_RC_ERR_OPERATION_NOT_SUPPORTED;
  }
  return UNC_RC_SUCCESS;
}

UncRespCode Kt_Base::get_oper_status(
    vector<OperStatusHolder> &ref_oper_status,
    unc_key_type_t key_type,
    void* key_struct,
    uint8_t &oper_status) {
  size_t key_struct_size = 0;
  switch (key_type) {
    case UNC_KT_CONTROLLER:
      key_struct_size = sizeof(key_ctr_t);
      break;
    case UNC_KT_CTR_DOMAIN:
      key_struct_size = sizeof(key_ctr_domain_t);
      break;
    case UNC_KT_LOGICAL_PORT:
      key_struct_size = sizeof(key_logical_port_t);
      break;
    case UNC_KT_SWITCH:
      key_struct_size = sizeof(key_switch_t);
      break;
    case UNC_KT_PORT:
      key_struct_size = sizeof(key_port_t);
      break;
    case UNC_KT_LINK:
      key_struct_size = sizeof(key_link_t);
      break;
    case UNC_KT_BOUNDARY:
      key_struct_size = sizeof(key_boundary_t);
      break;
    default:
      break;
  }
  vector<OperStatusHolder>::iterator iter = ref_oper_status.begin();
  for (; iter != ref_oper_status.end(); ++iter) {
    OperStatusHolder kt_oper_status = (*iter);
    if (kt_oper_status.get_key_type() == key_type &&
        (memcmp(kt_oper_status.get_key_struct(),
                key_struct, key_struct_size) == 0) ) {
      oper_status = kt_oper_status.get_oper_status();
      pfc_log_debug("Returning oper_status from internal structure %d",
                    oper_status);
      return UNC_RC_SUCCESS;
    }
  }
  pfc_log_debug("oper_status not found in internal structure");
  return UNC_UPPL_RC_ERR_NO_SUCH_INSTANCE;
}

void Kt_Base::ClearOperStatusHolder(vector<OperStatusHolder> &ref_oper_status) {
  vector<OperStatusHolder>::iterator iter = ref_oper_status.begin();
  for (; iter != ref_oper_status.end(); ++iter) {
    OperStatusHolder kt_oper_status = (*iter);
    switch (kt_oper_status.get_key_type()) {
      case UNC_KT_CONTROLLER: {
        key_ctr_t *ctr_key =
            reinterpret_cast<key_ctr_t*>(kt_oper_status.get_key_struct());
        if (ctr_key != NULL) {
          delete ctr_key;
          ctr_key = NULL;
        }
      }
      break;
      case UNC_KT_CTR_DOMAIN: {
        key_ctr_domain_t *ctr_domain_key =
            reinterpret_cast<key_ctr_domain_t*>
        (kt_oper_status.get_key_struct());
        if (ctr_domain_key != NULL) {
          delete ctr_domain_key;
          ctr_domain_key = NULL;
        }
      }
      break;
      case UNC_KT_LOGICAL_PORT: {
        key_logical_port_t *log_port_key =
            reinterpret_cast<key_logical_port_t*>
        (kt_oper_status.get_key_struct());
        if (log_port_key != NULL) {
          delete log_port_key;
          log_port_key = NULL;
        }
      }
      break;
      case UNC_KT_SWITCH: {
        key_switch_t *switch_key = reinterpret_cast<key_switch_t*>
        (kt_oper_status.get_key_struct());
        if (switch_key != NULL) {
          delete switch_key;
          switch_key = NULL;
        }
      }
      break;
      case UNC_KT_PORT: {
        key_port_t *port_key = reinterpret_cast<key_port_t*>
        (kt_oper_status.get_key_struct());
        if (port_key != NULL) {
          delete port_key;
          port_key = NULL;
        }
      }
      break;
      case UNC_KT_LINK: {
        key_link_t *link_key = reinterpret_cast<key_link_t*>
        (kt_oper_status.get_key_struct());
        if (link_key != NULL) {
          delete link_key;
          link_key = NULL;
        }
      }
      break;
      case UNC_KT_BOUNDARY: {
        key_boundary_t *bdry_key = reinterpret_cast<key_boundary_t*>
        (kt_oper_status.get_key_struct());
        if (bdry_key != NULL) {
          delete bdry_key;
          bdry_key = NULL;
        }
      }
      break;
      default:
        break;
    }
  }
  ref_oper_status.clear();
  return;
}
