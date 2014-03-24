/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   KT Root implementation
 * @file    itc_kt_root.cc
 *
 */

#include "itc_kt_root.hh"
#include "itc_kt_controller.hh"
#include "itc_kt_boundary.hh"


/** Constructor
 * @Description : This function instantiates child key instances for
 * kt_Root
 * @param[in] : None
 * @return    : None
 * */
Kt_Root::Kt_Root() {
  for (int i = 0; i < KT_ROOT_CHILD_COUNT; ++i) {
    child[i] = NULL;
  }
}


/** Destructor
 * @Description : This function frees the child key instances
 *  for Kt_Root
 * @param[in] : None
 * @return    : None
 * */
Kt_Root::~Kt_Root() {
  for (int i = 0; i < KT_ROOT_CHILD_COUNT; ++i) {
    if (child[i] != NULL) {
      delete child[i];
      child[i] = NULL;
    }
  }
}

/** GetChildClassPointer
 * @Description : This function creates the instances of child classes of KtRoot
 * @param[in] : KIndex-any value of KtRootChildClass
 * @return    : Kt_Base*
 ** */
Kt_Base* Kt_Root::GetChildClassPointer(KtRootChildClass KIndex) {
  switch (KIndex) {
    case KIdxController: {
      return (new Kt_Controller());
    }
    case KIdxBoundary: {
      return (new Kt_Boundary());
    }
    default: {
      pfc_log_info("Invalid index %d passed to GetChildClassPointer()",
                   KIndex);
      PFC_ASSERT(PFC_FALSE);
      return NULL;
    }
  }
}


/** Create
 * @Description : This function is not supported on KT_ROOT
 * @param[in] : key_struct-void * to kt key structure
 * value_struct-void* to kt value structure
 * session_id -ipc session id used for TC validation
 * configuration_id-ipc configuration id used for TC validation
 * data_type-UNC_DT_*,type of database
 * sess-ServerSession object where the request argument present
 * @return    : UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED
 * */

UncRespCode Kt_Root::Create(OdbcmConnectionHandler *db_conn,
                               uint32_t session_id,
                               uint32_t configuration_id,
                               void* key_struct,
                               void* val_struct,
                               uint32_t data_type,
                               ServerSession &sess) {
  physical_response_header rsh = {session_id,
      configuration_id,
      UNC_OP_CREATE,
      0,
      0,
      0,
      data_type,
      UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED};
  pfc_log_debug("Create Operation not allowed on KT_ROOT");
  int err = PhyUtil::sessOutRespHeader(sess, rsh);
  err |= sess.addOutput((uint32_t) UNC_KT_ROOT);
  err |= sess.addOutput(*reinterpret_cast<key_root_t*>(key_struct));
  if (err != 0) {
    return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  }
  return UNC_RC_SUCCESS;
}


/** Update
 * @Description : This function is not supported on KT_ROOT
 * @param[in] : key_struct-void* to ket key structure
 * value_struct-void* to kt value structure
 * session_id-ipc session id used for TC validation
 * configuration_id-ipc configuration id used for TC validation
 * data_type-UNC_DT_*,type of database
 * sess-ServerSession object where the request argument present
 * @return    : UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED
 * */
UncRespCode Kt_Root::Update(OdbcmConnectionHandler *db_conn,
                               uint32_t session_id,
                               uint32_t configuration_id,
                               void* key_struct,
                               void* val_struct,
                               uint32_t data_type,
                               ServerSession &sess) {
  pfc_log_error("Update Operation not allowed on KT_ROOT");
  physical_response_header rsh = {session_id,
      configuration_id,
      UNC_OP_UPDATE,
      0,
      0,
      0,
      data_type,
      UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED};
  pfc_log_debug("Update Operation not allowed on KT_ROOT");
  int err = PhyUtil::sessOutRespHeader(sess, rsh);
  err |= sess.addOutput((uint32_t) UNC_KT_ROOT);
  err |= sess.addOutput(*reinterpret_cast<key_root_t*>(key_struct));
  if (err != 0) {
    return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  }
  return UNC_RC_SUCCESS;
}


/** Delete
 * @Description : This function is not supported on KT_ROOT
 * @param[in] : key_struct-void* to ket key structure
 * value_struct-void* to kt value structure
 * session_id-ipc session id used for TC validation
 * configuration_id-ipc configuration id used for TC validation
 * data_type-UNC_DT_*,type of database
 * sess-ServerSession object where the request argument present
 * @return    : UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED
 * */
UncRespCode Kt_Root::Delete(OdbcmConnectionHandler *db_conn,
                               uint32_t session_id,
                               uint32_t configuration_id,
                               void* key_struct,
                               uint32_t data_type,
                               ServerSession &sess) {
  pfc_log_error("Delete Operation not allowed on KT_ROOT");
  physical_response_header rsh = {session_id,
      configuration_id,
      UNC_OP_DELETE,
      0,
      0,
      0,
      data_type,
      UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED};
  pfc_log_debug("Delete Operation not allowed on KT_ROOT");
  int err = PhyUtil::sessOutRespHeader(sess, rsh);
  err |= sess.addOutput((uint32_t) UNC_KT_ROOT);
  err |= sess.addOutput(*reinterpret_cast<key_root_t*>(key_struct));
  if (err != 0) {
    return UNC_UPPL_RC_ERR_IPC_WRITE_ERROR;
  }
  return UNC_RC_SUCCESS;
}

/**ReadBulk
 * @Description : This function reads a bulk rows of KT_Controller in
 *  Controller table of specified data type.
 *  Order of ReadBulk response
 *  val_ctr -> val_ctr_domain -> val_logical_port ->
 *  val_logical_member_port -> val_switch ->  val_port ->
 *  val_link -> val_boundary
 * @param[in] :
 * key_struct - no key for the kt Root instance
 * data_type - UNC_DT_* , read allowed in candidate/running/startup/state
 * max_rep_ct-maximum repetition count
 * child_index-index indicating children of KtController
 * bool parent_call-flag to indicate whether function called by parent
 * bool is_read_next-flag to indicate whether requested operation is 
 *                 UNC_OP_READ_NEXT
 * @return    : UNC_RC_SUCCESS is returned when the response
 * is added to ipc session successfully.
 * UNC_UPPL_RC_ERR_* is returned when ipc response could not be added to sess.
 * */
UncRespCode Kt_Root::ReadBulk(OdbcmConnectionHandler *db_conn,
                                 void* key_struct,
                                 uint32_t data_type,
                                 uint32_t &max_rep_ct,
                                 int child_index,
                                 pfc_bool_t parent_call,
                                 pfc_bool_t is_read_next,
                                 ReadRequest *read_req) {
  pfc_log_debug("Inside ReadBulk of Kt_ROOT");
  UncRespCode read_status = UNC_RC_SUCCESS;
  if (max_rep_ct == 0) {
    // Count is 0, return UNC_RC_SUCCESS
    pfc_log_debug("KT_ROOT - Count is 0, return Success");
    return UNC_RC_SUCCESS;
  }
  if ((unc_keytype_datatype_t)data_type == UNC_DT_CANDIDATE ||
      (unc_keytype_datatype_t)data_type == UNC_DT_RUNNING ||
      (unc_keytype_datatype_t)data_type == UNC_DT_STATE ||
      (unc_keytype_datatype_t)data_type == UNC_DT_STARTUP) {
    pfc_log_debug("Calling Child class ReadBulk");
    std::string str_controller_name = "";
    int st_child_index =
        (child_index >= 0 && child_index <= KIdxBoundary) ? child_index+1 \
                                                          : KIdxController;
    for (int kIdx = st_child_index; kIdx <= KIdxBoundary; ++kIdx) {
      // Filling key_struct corresponding to tht key type
      void *child_key_struct = getChildKeyStruct(kIdx);
      child[kIdx] = GetChildClassPointer((KtRootChildClass)kIdx);
      if (child[kIdx] == NULL) {
        // Free Key struct
        FreeKeyStruct(child_key_struct, kIdx);
        continue;
      }
      read_status = child[kIdx]->ReadBulk(db_conn, child_key_struct,
                                          data_type,
                                          max_rep_ct,
                                          -1,
                                          true,
                                          is_read_next,
                                          read_req);
      pfc_log_debug(
          "KT_ROOT - read status from child %d is %d",
          kIdx, read_status);
      FreeKeyStruct(child_key_struct, kIdx);
      delete child[kIdx];
      child[kIdx] = NULL;
      if (max_rep_ct <= 0) {
        // Count reached 0, return UNC_RC_SUCCESS
        return UNC_RC_SUCCESS;
      }
    }
  } else {
    pfc_log_debug(
        "READ_NEXT and READ_BULK operations are not allowed on the data types"
        " other than DT_CANDIDATE, DT_RUNNING, DT_STARTUP and DT_STATE");
    read_status = UNC_UPPL_RC_ERR_OPERATION_NOT_ALLOWED;
    return UNC_RC_SUCCESS;
  }
  pfc_log_debug("read_status=%d", read_status);
  return read_status;
}

/** getChildKeyStruct
 * @Description : This function returns the void * of child key structures
 * @param[in] : child_class-index indicating children of Controller
 * @return    : void * key structure
 * */

void* Kt_Root::getChildKeyStruct(uint32_t child_class) {
  switch (child_class) {
    case 0: {
      key_ctr_t *obj_child_key = new key_ctr_t;
      memset(obj_child_key->controller_name, '\0',
             sizeof(obj_child_key->controller_name));
      void* child_key = reinterpret_cast<void *>(obj_child_key);
      return child_key;
    }
    case 1: {
      key_boundary_t *obj_child_key = new key_boundary_t;
      memset(obj_child_key->boundary_id, '\0',
             sizeof(obj_child_key->boundary_id));
      void* child_key = reinterpret_cast<void *>(obj_child_key);
      return child_key;
    }
    default: {
      pfc_log_info("Invalid index %d passed to getChildKeyStruct()",
                   child_class);
      PFC_ASSERT(PFC_FALSE);
      return NULL;
    }
  }
}

/** FreeKeyStruct
 * @Description : This function clears the void * of child key structures
 * @param[in] : child_class index indicating the children of KtController
 * key_struct-void* to any child's key structure
 * @return    : void 
 * */

void Kt_Root::FreeKeyStruct(void *key_struct,
                            uint32_t child_class) {
  switch (child_class) {
    case 0: {
      key_ctr_t *obj_child_key = reinterpret_cast<key_ctr_t*>(key_struct);
      if (obj_child_key != NULL) {
        delete obj_child_key;
        obj_child_key = NULL;
      }
      break;
    }
    case 1: {
      key_boundary_t *obj_child_key =
          reinterpret_cast<key_boundary_t*>(key_struct);
      if (obj_child_key != NULL) {
        delete obj_child_key;
        obj_child_key = NULL;
      }
      break;
    }
    default: {
      // do nothing
      pfc_log_info("Invalid index %d passed to FreeKeyStruct()",
                   child_class);
      PFC_ASSERT(PFC_FALSE);
      break;
    }
  }
  return;
}
