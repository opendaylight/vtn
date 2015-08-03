/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_UPLL_VTN_DATAFLOW_MOMGR_H
#define UNC_UPLL_VTN_DATAFLOW_MOMGR_H

#include <set>
#include <map>
#include <string>
#include "momgr_impl.hh"
#include "uncxx/dataflow.hh"


namespace unc {
namespace upll {
namespace kt_momgr {


using unc::dataflow::kidx_val_vtn_dataflow_cmn;
using unc::dataflow::DataflowCmn;
using unc::dataflow::DataflowDetail;
using unc::dataflow::DataflowUtil;
using unc::dataflow::key_vtn_ctrlr_dataflow;
using unc::dataflow::actions_vect_st;
using unc::dataflow::AddlData;
using unc::dataflow::KeyVtnDataflowCmp;


using unc::upll::dal::DalBindInfo;
using unc::upll::dal::DalDmlIntf;
using unc::upll::dal::DalCursor;
using unc::upll::dal::DalCDataType;
using unc::upll::ipc_util::IpctSt;


class VtnDataflowMoMgr : public MoMgrImpl {
 public:
  VtnDataflowMoMgr() {
    upll_max_dataflow_traversal_ = 0;
      max_dataflow_traverse_count_ = 0;
     ReadConfigFile();
  }
  virtual ~VtnDataflowMoMgr() {
  }
  upll_rc_t ReadMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                     DalDmlIntf *dmi);
  /** Variable to hold max dataflow traversal count to verify during dataflow
   *  traversal, dataflow traversal is limited by this count value
   **/
  uint32_t upll_max_dataflow_traversal_;

  /* This function is used to convert the vexternal name to
   * vbridge name in the path info structure.
   */
  upll_rc_t ConvertVexternaltoVbr(
                                DataflowCmn *df_cmn,
                                const uint8_t *vtn_name,
                                uint8_t *vex_name,
                                uint8_t *vex_if_name,
                                DalDmlIntf *dmi);

  upll_rc_t GetChildConfigKey(ConfigKeyVal *&okey,
                              ConfigKeyVal *ikey);

 private:
   /**
    * @Description : This function reads config from file
    * @param[in] :
    **/
  inline upll_rc_t ReadConfigFile() {
     UPLL_FUNC_TRACE;
     pfc::core::ModuleConfBlock ipcblock("vtn_dataflow");
     upll_max_dataflow_traversal_ = ipcblock.getUint32(
                                    "upll_max_dataflowtraversal", 1000);
     UPLL_LOG_DEBUG("upll_max_dataflow_traversal_ - red from upll.conf = %d",
                  upll_max_dataflow_traversal_);
     return UPLL_RC_SUCCESS;
  }

   /**
    * @Brief   Validates the syntax of the specified key and value structure
    *          for KT_VTN keytype
    *
    * @param[in]  req    This structure contains
    *                     IpcReqRespHeader(first 8 fields of
    *                     input request structure).
    * @param[in]  ikey   ikey contains key and value structure.
    *
    * @retval   UPLL_RC_SUCCESS                       Successful.
    * @retval   UPLL_RC_ERR_GENERIC                   Generic failure.
    * @retval   UPLL_RC_ERR_BAD_REQUEST               key/value struct is
    *                                                 not valid.
    * @retval   UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT   operation not allowed.
    * @retval   UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT   Datatype not allowed.
    * @retval   UPLL_RC_ERR_INVALID_OPTION1           option1 is not valid.
    * @retval   UPLL_RC_ERR_INVALID_OPTION2           option2 is not valid.
    **/
  upll_rc_t ValidateMessage(IpcReqRespHeader *req, ConfigKeyVal *kval);

   /**
    * @Brief Validates the syntax for KT_VTN Keytype Key structure.
    *
    * @param[in]  val_vtn  KT_VTN key structure.
    *
    * @retval  UPLL_RC_SUCCESS         validation succeeded.
    * @retval  UPLL_RC_ERR_CFG_SYNTAX  validation failed.
    *
    **/
  upll_rc_t ValidateVtnDataflowKey(key_vtn_dataflow *vtndataflow);

   /**
    * @Brief Validates the controller capability for VTN dataflow
    *
    * @param[in]  ctrlr_name      controller name.
    * @param[in]  is_first_ctrlr  flag to  specify  ingress controller or not.
    * @param[out] ctrlr_type      specifies the controller type.
    *
    * @retval  UPLL_RC_SUCCESS                      Success.
    * @retval  UPLL_RC_ERR_GENERIC                  Generic Error.
    * @retval  UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR   controller not supported.
    *
    **/
  upll_rc_t ValidateControllerCapability(const char *ctrlr_name,
                                          bool is_first_ctrlr,
                                          unc_keytype_ctrtype_t *ctrlr_type);

   /**
    * @Brief Get the controller and domain id from the specified vtn and
    *        vnode id
    *
    * @param[in]   vtn_id     vtn name.
    * @param[in]   vnode_id   vnode_name.
    * @param[out]  ctrlr_dom  controller domain pointer.
    * @param[out]  dmi         Dal interface pointer.
    *
    * @retval  UPLL_RC_SUCCESS               Success.
    * @retval  UPLL_RC_ERR_GENERIC           Generic Error.
    * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No specified instance in database.
    *
    **/
  upll_rc_t GetVnodeControllerDomain(uint8_t *vtn_id, uint8_t *vnode_id,
                                      std::string &ctrlr, std::string &domain,
                                           DalDmlIntf *dmi);

  upll_rc_t UpdatePathInfoInterfaces(DataflowCmn *df_cmn,
                                       const uint8_t  *vtn_name,
                                       DataflowUtil *df_util,
                                       DalDmlIntf *dmi);
  upll_rc_t MapVexternalToVbridge(const ConfigKeyVal *ckv_df,
                                        DataflowCmn *df_cmn,
                                        DataflowUtil *df_util,
                                        bool *is_vnode_match,
                                        DalDmlIntf *dmi);
  upll_rc_t RenamePathinfoNodes(uint8_t *node_name,
                                              const  uint8_t *pfc_vtn_name,
                                              uint8_t *ctrlr_id,
                                              DataflowUtil *df_util,
                                              DalDmlIntf *dmi);
  upll_rc_t FillCtrlrDomCountMap(uint8_t *vtn_name,
                                       uint32_t  &ctrlr_dom_count,
                                       DalDmlIntf *dmi);

  upll_rc_t CheckBoundaryAndTraverse(ConfigKeyVal *ckv_df,
                                       IpcReqRespHeader *header,
                                       DataflowCmn *source_node,
                                       DataflowCmn *lastPfcNode,
                                      DataflowUtil *df_util,
                                       DalDmlIntf *dmi);

  upll_rc_t GetNeighborInfo(uint8_t *vtn_name,
                              DataflowCmn  *df_cmn,
                              ConfigKeyVal *&ckv_inif,
                              ConfigKeyVal *&ckv_remif,
                              DalDmlIntf *dmi);

  upll_rc_t TraversePFCController(ConfigKeyVal *ckv_df,
                                    IpcReqRespHeader *header,
                                    DataflowCmn *source_node,
                                    DataflowCmn *lastPfcNode,
                                    DataflowUtil *df_util,
                                    DalDmlIntf *dmi,
                                    bool is_first_ctrlr = false);
  upll_rc_t PopulateVnpOrVbypassBoundaryInfo(ConfigKeyVal *&ckv_inif,
                                               ConfigKeyVal *&ckv_remif,
                                               DalDmlIntf *dmi);

  upll_rc_t MapCtrlrNameToUncName(
                                   const uint8_t *pfc_vtn_name,
                                   val_vtn_dataflow_path_info *path_info,
                                   uint8_t *ctrlr_id,
                                   DataflowUtil *df_util,
                                   DalDmlIntf *dmi);

  upll_rc_t MapCtrlrNameToUncName(const uint8_t *vtn_name,
                                   DataflowCmn *df_cmn,
                                   DataflowUtil *df_util,
                                   DalDmlIntf *dmi,
                                   unc_keytype_ctrtype_t ctrlr_type);
  upll_rc_t UpdateReason(DataflowCmn *source_node, upll_rc_t result_code);

  // Check to optimise the path info in case of redirect flow.
  void RedirectCheck(DataflowCmn *df_cmn);

  // To check the given path info's interface is dynamic or not
  // based on direction( ingress or egress).
  upll_rc_t DynamicInterfaceCheck(DataflowCmn *df_cmn,
                                  DataflowUtil *df_util,
                                  val_vtn_dataflow_path_info *path_info,
                                  const uint8_t *vtn_name,
                                  int direction,
                                  DalDmlIntf *dmi);

  upll_rc_t GetParentConfigKey(ConfigKeyVal*&okey, ConfigKeyVal *ikey) {
     return UPLL_RC_SUCCESS;
  }
  upll_rc_t UpdateConfigStatus(ConfigKeyVal *ckv1, unc_keytype_operation_t op,
                               uint32_t count,
                               ConfigKeyVal *ckv2,
                               DalDmlIntf *dmi, ConfigKeyVal *ckv3) {
     UPLL_FUNC_TRACE;
     return UPLL_RC_SUCCESS;
  }
  upll_rc_t UpdateAuditConfigStatus(unc_keytype_configstatus_t st,
                                     uuc::UpdateCtrlrPhase up,
                                     ConfigKeyVal*&ckv,
                                     DalDmlIntf *dmi) {
     UPLL_FUNC_TRACE;
     return UPLL_RC_SUCCESS;
  }
  upll_rc_t ValidateAttribute(ConfigKeyVal*ikey, DalDmlIntf *dmi,
                              IpcReqRespHeader *header) {
  UPLL_FUNC_TRACE;
  return UPLL_RC_SUCCESS;
  }
  upll_rc_t GetValid(void *v1, uint64_t u, uint8_t*& ui,
                     upll_keytype_datatype_t dt, MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  return UPLL_RC_SUCCESS;
  }
  upll_rc_t ValidateCapability(IpcReqRespHeader *header, ConfigKeyVal*ikey,
                               const char* ctrkr) {
  UPLL_FUNC_TRACE;
  return UPLL_RC_SUCCESS;
  }
  upll_rc_t DupConfigKeyVal(ConfigKeyVal*&okey, ConfigKeyVal*&ikey,
                            MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  return UPLL_RC_SUCCESS;
  }
  bool IsValidKey(void*v, uint64_t t, MoMgrTables tbl = MAINTBL) {
  UPLL_FUNC_TRACE;
  return true;
  }
  bool CompareValidValue(void*&v1, void*v2, bool b) {
  UPLL_FUNC_TRACE;
  return true;
  }
  upll_rc_t GetRenamedUncKey(ConfigKeyVal* ikey, upll_keytype_datatype_t dt,
                             DalDmlIntf *dmi, uint8_t *ctrlr) {
  UPLL_FUNC_TRACE;
  return UPLL_RC_SUCCESS;
  }
  bool GetRenameKeyBindInfo(unc_key_type_t dt, BindInfo*& binfo,
                    int&count,
                    MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  return true;
  }
  upll_rc_t CopyToConfigKey(ConfigKeyVal*&okey, ConfigKeyVal*ikey) {
    UPLL_FUNC_TRACE;
    UPLL_LOG_INFO("Not implemented. Returning Generic Error");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t IsReferenced(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                         DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  return UPLL_RC_SUCCESS;
  }
  upll_rc_t AllocVal(ConfigVal *&ckv1, upll_keytype_datatype_t dt,
                     MoMgrTables tbl) {
  UPLL_FUNC_TRACE;
  return UPLL_RC_SUCCESS;
  }

  uint32_t max_dataflow_traverse_count_;
  std::set<std::string> bypass_dom_set;
  std::map<std::string, ConfigKeyVal* > vext_info_map;
  std::map<uint8_t *, uint8_t *> vnode_rename_map;
};
}  //  namespace kt_momgr
}  //  namespace upll
}  //  namespace unc
#endif  // UNC_UPLL_VTN_DATAFLOW_MOMGR_H
