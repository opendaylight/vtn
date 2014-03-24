/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _UNC_UPLL_VNODE_MOMGR_H_
#define _UNC_UPLL_VNODE_MOMGR_H_

#include <string>
#include <set>
#include <map>
#include "momgr_impl.hh"

namespace unc {
namespace upll {
namespace kt_momgr {

typedef struct val_db_vbr_st {
  val_vbr_st vbr_val_st;
  uint32_t down_count;
  uint32_t fault_count;
} val_db_vbr_st_t;

typedef struct val_db_vrt_st {
  val_vrt_st vrt_val_st;
  uint32_t down_count;
  uint32_t fault_count;
} val_db_vrt_st_t;

typedef struct val_db_vtep_st {
  val_vtep_st vtep_val_st;
  uint32_t down_count;
  uint32_t fault_count;
} val_db_vtep_st_t;

typedef struct val_db_vtunnel_st {
  val_vtunnel_st vtunnel_val_st;
  uint32_t down_count;
  uint32_t fault_count;
} val_db_vtunnel_st_t;

class VnodeMoMgr : public MoMgrImpl {
 public:
  map<string, ConfigKeyVal *> vnode_oper_map;

  VnodeMoMgr() {
    parent_ck_vtn = NULL;
    cntrl_id = NULL;
  }
  virtual ~VnodeMoMgr() {
    if (parent_ck_vtn) delete parent_ck_vtn;
    parent_ck_vtn = NULL;
  }

  upll_rc_t UpdateOperStatus(ConfigKeyVal *ck_vn,
                             DalDmlIntf *dmi,
                             state_notification notification, bool skip,
                             bool save_to_db);
  upll_rc_t CtrlrTypeAndDomainCheck(ConfigKeyVal *ikey,
                                    IpcReqRespHeader *req);
  /**
   * @Brief  compares controller id and domain id before
   *         updating the value to DB.
   * @param[in]  ikey  ikey contains key and value structure.
   * @param[in]  okey  okey contains key and value structure.
   *
   * @retval  UPLL_RC_SUCCESS            Successful.
   * @retval  UPLL_RC_ERR_CFG_SYNTAX     Syntax error.
   */
  virtual upll_rc_t CtrlrIdAndDomainIdUpdationCheck(ConfigKeyVal *ikey,
                                                    ConfigKeyVal *okey) = 0;

  /**
   * @brief  Perform Semantic Check to check Different vbridges
   *          contain same switch-id and vlan-id
   *
   * @param[in]       ikey        ConfigKeyVal
   * @param[out]      dmi         DataBase Interface
   *
   * @retval     UPLL_RC_ERR_CFG_SEMANTIC     on error
   * @retval     UPLL_RC_SUCCESS              on success
   **/
  virtual upll_rc_t ValidateAttribute(ConfigKeyVal *kval,
                                      DalDmlIntf *dmi,
                                      IpcReqRespHeader *req = NULL);

  /**
   * @Brief This API is to update(Add or delete) the controller
   *
   * @param[in] vtn_name     vtn name pointer
   * @param[in] ctrlr_dom    Controller Domain pointer
   * @param[in] op           UNC Operation Code
   * @param[in] dmi          Database Intereface pointer
   *
   * @retval UPLL_RC_SUCCESS Successful.
   * @retval UPLL_RC_ERR_GENERIC Generic error.
   * @retval UPLL_RC_ERR_NO_SUCH_INSTANCE Record is not available.
   *
   */
  /* upll_rc_t UpdateControllerTableForVtn(uint8_t *vtn_name,
     controller_domain *ctrlr_dom,
     unc_keytype_operation_t op,
     DalDmlIntf *dmi);
     */
  /**
   * @brief  Imtimate POM modules of new Controller addition
   *
   * @param[in]    ikey        ConfigKeyVal
   * @parm[in]     ctrlr_dom   controller_domain pointer
   * @parm[in]     dmi         DataBase Interface pointer
   * @param[in]    op          type of operation
   *
   **/
  upll_rc_t IntimatePOMAboutNewController(ConfigKeyVal *ikey,
                                          controller_domain *ctrlr_dom,
                                          DalDmlIntf *dmi,
                                          unc_keytype_operation_t op,
                                          upll_keytype_datatype_t dt_type);

  template<typename T1, typename T2>
bool SetOperStatus(ConfigKeyVal *ikey,
                   DalDmlIntf *dmi,
                   int notification,
                   bool skip,
                   bool save_to_db);
  /**
   * @brief          Enqueues oper status notifications
   *
   * @param[in]      ikey    pointer to the configkeyval with
   *                 the changed oper status
   *
   * @retval         UPLL_RC_SUCCESS      Successfull completion.
   * @retval         UPLL_RC_ERR_GENERIC  Failure case.
   **/
  bool  EnqueOperStatusNotification(ConfigKeyVal *ikey, bool oper_change);

  /* @brief     To update oper status of vnode
   *
   * @param[in] ktype         keytype
   * @param[in] session_id    session identifier
   * @param[in] config_id     config identifier
   * @param[in] dmi           Pointer to db connection instance
   *
   * @retval  UPLL_RC_SUCCESS                    updated successfully.
   * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
   * @retval  UPLL_RC_ERR_DB_ACCESS              DB Read/Write error.
   *
   **/

  upll_rc_t TxUpdateDtState(unc_key_type_t ktype,
                            uint32_t session_id,
                            uint32_t config_id,
                            DalDmlIntf *dmi);

 protected:
  ConfigKeyVal *parent_ck_vtn;

  /**
   * @brief  Update parent oper status on delete for Transaction commit
   *
   * @param[in]  ikey          ConfigKeyVal instance
   * @param[in]   dmi           Database connection parameter

   * @retval  UPLL_RC_SUCCESS      Completed successfully.
   * @retval  UPLL_RC_ERR_GENERIC  Generic failure.
   */
  virtual upll_rc_t UpdateParentOperStatus(ConfigKeyVal *ikey,
                                           DalDmlIntf *dmi);

  virtual upll_rc_t DeleteMo(IpcReqRespHeader *req,
                             ConfigKeyVal *ikey,
                             DalDmlIntf *dmi);

  upll_rc_t GetVnodeType(const void *key, bool vnode,
                         unc_key_type_t &keytype,
                         ConfigKeyVal *&ck_val, DalDmlIntf *dmi,
                         upll_keytype_datatype_t dt_type);
  /**
   * @brief   set the renamed flag in Vnode
   *          and create an entry in vnode rename table if VTN is renamed
   *
   *
   * @param[in]  ikey       ConfigKeyVal pointer
   * @param[in]  dt_type    specifies the database type
   * @param[in]  ctrlr_id   pointer to the controller name
   * @param[in]  dmi        DalDmlIntf pointer
   *
   * @retval UPLL_RC_SUCCESS      Successful
   * @retval UPLL_RC_ERR_GENERIC  failed to update the VbrIf
   */
  upll_rc_t SetVnodeRenameFlag(ConfigKeyVal *&ikey,
                               upll_keytype_datatype_t dt_type,
                               controller_domain_t *ctrlr_dom,
                               DalDmlIntf *dmi);
  /**
   * @brief  Creates a vnode entry in DB after performing pre-requisite checks
   *
   * @param[in]  ikey            ConfigKeyVal pointer
   * @param[in]  req             IpcReqRespHeader pointer
   * @param[in]  dmi             DalDmlIntf pointer
   * @param[in]  controller_ids  pointer to
   *                             array of controller id pointers
   *
   * @retval UPLL_RC_SUCCESS                Successful
   * @retval UPLL_RC_ERR_NO_SUCH_INSTANCE    is not present in DB
   * @retval UPLL_RC_ERR_GENERIC            generic error
   * @retval UPLL_RC_ERR_INSTANCE_EXISTS    entry already exists in DB
   */
  upll_rc_t RestoreVnode(ConfigKeyVal *ikey,
                         IpcReqRespHeader *req,
                         DalDmlIntf *dmi,
                         controller_domain_t ctrlr_domain[],
                         bool restore_flag);

  /**
   * @brief  Get the specific vtn_name and vnode_name from ConfigKeyVal
   *
   * @param[in]  ikey        ConfigKeyVal pointer
   * @param[out] vtn_name    vnode vtn name
   * @param[out] vnode_name  vnode specific name
   *
   * @retval UPLL_RC_SUCCESS      Successful
   * @retval UPLL_RC_ERR_GENERIC  failed to retrieve the values
   */
  virtual upll_rc_t GetVnodeName(ConfigKeyVal *ikey,
                                 uint8_t *&vtn_name,
                                 uint8_t *&vnode_name) =0;

  /**
   * @brief  Checks and Updates the VbrIf interfaces associated with the vlink with the
   *         information from physical if the VbrIf is part of vExternal
   *
   * @param[in]  dt_type            database type
   * @param[in]  ikey               ConfigKeyVal pointer
   * @param[in]  dmi                DalDmlIntf pointer
   * @param[in]  controller_domain  pointer to array of controller and
   *                                domain  pointers
   * @param[in]  req                IpcRequestresponseHeader pointer
   *
   * @retval UPLL_RC_SUCCESS      Successful
   * @retval UPLL_RC_ERR_GENERIC  failed to update the VbrIf
   */
  /*   virtual upll_rc_t CheckVlinkVbrIf(upll_keytype_datatype_t dt_type,
       ConfigKeyVal *ikey, DalDmlIntf *dmi,
       controller_domain *ctrlr_dom);*/

  /**
   * @brief Update the VbrIf ConfigKeyVal with vExternal details fetched from
   *        physical, if the VbrIf is part of vExternal
   *
   * @param[in]  dt_type  database type
   * @param[in]  ikey     pointer to ConfigKeyVal
   * @param[in]  dmi      pointer to DalDmlIntf
   *
   * @retval UPLL_RC_SUCCESS      Successful
   * @retval UPLL_RC_ERR_GENERIC  Generic error/Failed to update VbrIf
   */
  upll_rc_t UpdateVbrIfExternal(upll_keytype_datatype_t dt_type,
                                ConfigKeyVal *ikey, DalDmlIntf *dmi);

 private:
  uint8_t *cntrl_id;

  /**
   * @brief Used to create a vnode entry in CANDIDATE DB and is invoked
   *         through createMo
   *
   * @param[in]  req    IpcRequestresponseHeader pointer
   * @param[in]  ikey   ConfigKeyVal pointer
   * @param[in]  dmi    DalDmlIntf pointer
   *
   * @retval UPLL_RC_SUCCESS                    Successful
   * @retval UPLL_RC_ERR_NO_SUCH_INSTANCE       parent vtn is not present in DB
   * @retval UPLL_RC_ERR_GENERIC                generic error
   * @retval UPLL_RC_ERR_INSTANCE_EXISTS        entry already exists in DB
   * @retval UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR Not Supported by controller
   * @retval UPLL_RC_ERR_INVALID_OPTION1        Invalid option
   */
  virtual upll_rc_t CreateCandidateMo(IpcReqRespHeader *req,
                                      ConfigKeyVal *ikey, DalDmlIntf *dmi,
                                      bool restore_flag = false);
  virtual upll_rc_t CreateAuditMoImpl(ConfigKeyVal *ikey,
                                      DalDmlIntf *dmi,
                                      const char *ctrlr_id);

  /**
   * @brief  Checks if the parent VTN is already present in the pfc controller
   *
   * @param[in]  ikey       ConfigKeyVal pointer
   * @param[in]  req        IpcReqRespHeader pointer
   * @param[in]  ctrlr_dom  pointer to the controller and domain name
   * @param[in]  dmi        DalDmlIntf pointer
   *
   * @retval UPLL_RC_SUCCESS                Successful
   * @retval UPLL_RC_ERR_GENERIC            generic error
   * @retval UPLL_RC_ERR_INSTANCE_EXISTS    VTN already exists in controller
   */
  upll_rc_t CheckVtnExistenceOnController(ConfigKeyVal *ikey,
                                          IpcReqRespHeader *req,
                                          controller_domain *ctrlr_dom,
                                          DalDmlIntf *dmi);

  /**
   * @brief create entry in Vnode Rename Table,
   *        with the renamed VTN details fetched from VTN rename Table
   *
   *
   * @param[in]  ikey       ConfigKeyVal pointer
   * @param[in]  dt_type    specifies the database type
   * @param[in]  ctrlr_dom   pointer to the controller and domain name
   * @param[in]  dmi        DalDmlIntf pointer
   *
   *
   * @retval UPLL_RC_SUCCESS      Successful
   * @retval UPLL_RC_ERR_GENERIC  failed to update the VbrIf
   */
  upll_rc_t CreateVnodeRenameEntry(ConfigKeyVal *ikey,
                                   upll_keytype_datatype_t dt_type,
                                   controller_domain *ctrlr_dom,
                                   DalDmlIntf *dmi);


  /**
   * @brief  check if any UNC VTN is renamed as this VTN on the given Controller
   *
   *
   * @param[in]  ikey       ConfigKeyVal pointer
   * @param[in]  dt_type    specifies the database type
   * @param[in]  ctrlr_dom   pointer to the controller and domain name
   * @param[in]  dmi        DalDmlIntf pointer
   *
   * @retval UPLL_RC_SUCCESS      Successful
   * @retval UPLL_RC_ERR_GENERIC  failed to update the VbrIf
   */
  upll_rc_t CheckRenamedVtnName(ConfigKeyVal *ikey,
                                upll_keytype_datatype_t dt_type,
                                controller_domain *ctrlr_dom,
                                DalDmlIntf *dmi);

  /* @brief     To control operation on key types
   *
   * @param[in]     header    Pointer to IpcResResHeader
   * @param[in/out] ikey      Pointer to the ConfigKeyVal Structure
   * @param[in]     dmi       Pointer to the DalDmlIntf(DB Interface)
   *
   * @retval  UPLL_RC_SUCCESS                    Completed successfully.
   * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
   * @retval  UPLL_RC_ERR_RESOURCE_DISCONNECTED  Resource disconnected.
   * @retval  UPLL_RC_ERR_DB_ACCESS              DB Read/Write error.
   * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE       Given key does not exist
   *
   **/
  upll_rc_t ControlMo(IpcReqRespHeader *header, ConfigKeyVal *ikey,
                      DalDmlIntf *dmi);
};

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
#endif  // _UNC_UPLL_VNODE_MOMGR_H_

