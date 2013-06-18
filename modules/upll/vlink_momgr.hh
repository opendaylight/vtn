/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_UPLL_VLINK_MOMGR_H
#define UNC_UPLL_VLINK_MOMGR_H

#include <string>
#include <sstream>
#include "vnode_momgr.hh"
#include "unc/uppl_common.h"

namespace unc {
namespace upll {
namespace kt_momgr {

#define VNODE1_RENAME 0x04
#define VNODE2_RENAME 0x08
#define VNODE_RENAME 0x0C
#define NO_VN1_RENAME 0xFB
#define NO_VN2_RENAME 0xF7


typedef struct key_vlink_user_data {
    key_user_data_t user_data1;
    key_user_data_t user_data2;
} key_vlink_user_data_t;

typedef struct val_db_vlink_st {
        val_vlink_st       vlink_val_st;
        uint32_t           down_count;
} val_db_vlink_st_t;

#define SET_USER_DATA_CTRLR2(ckey, ctrlr2) { \
  GET_USER_DATA(ckey)      \
  if (ctrlr && strlen(reinterpret_cast<char *>(ctrlr))) { \
  key_vlink_user_data *user_data = reinterpret_cast<key_vlink_user_data *>\
                                    (ckey->get_user_data()); \
  uuu::upll_strncpy(user_data->ctrlr2_id, ctrlr2, (kMaxLenCtrlrId+1)); \
  } else {  \
  return UPLL_RC_ERR_GENERIC; \
  }\
}

#define SET_USER_DATA_DOMAIN2(ckey, domain2) { \
  GET_USER_DATA(ckey)      \
  if ( domain2 && strlen(reinterpret_cast<char *>(domain2))) { \
  key_vlink_user_data *user_data = reinterpret_cast<key_vlink_user_data *> \
                                                  (ckey->get_user_data()); \
  uuu::upll_strncpy(user_data->domain2_id, domain2, (kMaxLenCtrlrId+1)); \
  } else { \
  return UPLL_RC_ERR_GENERIC; \
  }\
}

class VlinkMoMgr : public VnodeMoMgr {
  private:
    static unc_key_type_t vlink_child[];
    static BindInfo vlink_bind_info[];
    static BindInfo vlink_rename_bind_info[];
    static BindInfo key_vlink_maintbl_bind_info[];
    static BindInfo key_vlink_renametbl_update_bind_info[];
    ConfigKeyVal *ck_boundary;

    /**
     * @brief  Gets the associated config and state information for interfaces
     *         that constitute the vlink
     *
     * @param[in]     vlink    ConfigKeyVal pointer to the vlink
     * @param[out]    vnif     array of pointers to the vlink member interfaces
     * @param[in]     dmi      Database connection params
     *
     * @retval UPLL_RC_SUCCESS     validation succeeded.
     * @retval UPLL_RC_ERR_GENERIC Illegal operation/ invalid paramters
     **/
    upll_rc_t GetVnodeIfFromVlink(ConfigKeyVal *vlink, 
                                  ConfigKeyVal **vnif,
                                  DalDmlIntf   *dmi) ;

    /**
     * @brief  Gets the keytype of the vlink member interface specified by pos
     *
     * @param[in]     ck_vlink    ConfigKeyVal pointer to the vlink
     * @param[in]     pos         0/1 corresponding to the first/second member 
     *                            interface
     *
     * @retval unc_key_type_t     keytype of the specified member interface
     *                            KT_ROOT if unsupported iftype.
     **/
    unc_key_type_t GetVlinkVnodeIfKeyType(ConfigKeyVal *ck_vlink,
                                          int pos ) {
      uint8_t vnif_type;
      uint8_t vlink_flag = 0;
      GET_USER_DATA_FLAGS(ck_vlink,vlink_flag);
      if (pos == 0)
        vnif_type = GET_VLINK_NODE1_TYPE(vlink_flag);
      else 
        vnif_type = GET_VLINK_NODE2_TYPE(vlink_flag);
      unc_key_type_t if_ktype[] = {UNC_KT_ROOT,UNC_KT_VBR_IF, UNC_KT_VRT_IF,
                             UNC_KT_VUNK_IF, UNC_KT_VTEP_IF,UNC_KT_VTUNNEL_IF};
      if (vnif_type >= sizeof(if_ktype)/sizeof(if_ktype[0]))
        return UNC_KT_ROOT;
      return if_ktype[vnif_type];
    }

    /**
     * @brief  Gets the valid array position of the variable in the value 
     *         structure from the table in the specified configuration  
     *
     * @param[in]     val      pointer to the value structure 
     * @param[in]     indx     database index for the variable
     * @param[out]    valid    position of the variable in the valid array - 
     *                          NULL if valid does not exist.
     * @param[in]     dt_type  specifies the configuration
     * @param[in]     tbl      specifies the table containing the given value 
     *
     **/
    upll_rc_t GetValid(void *val,
                       uint64_t indx,
                       uint8_t *&valid,
                       upll_keytype_datatype_t dt_type,
                       MoMgrTables tbl);
    /**
     * @Brief Validates the syntax of the specified key and value structure
     *        for KT_VLINK keytype
     *
     * @param[in] req                       This structure contains
     *                                      IpcReqRespHeader(first 8 fields of input request structure).
     * @param[in] ikey                      ikey contains key and value structure.
     *
     * @retval UPLL_RC_SUCCESS              Successful.
     * @retval UPLL_RC_ERR_CFG_SYNTAX       Syntax error.
     * @retval UPLL_RC_ERR_NO_SUCH_INSTANCE key_vlink is not available.
     * @retval UPLL_RC_ERR_GENERIC          Generic failure.
     * @retval UPLL_RC_ERR_INVALID_OPTION1  option1 is not valid.
     * @retval UPLL_RC_ERR_INVALID_OPTION2  option2 is not valid.
     */
    upll_rc_t ValidateMessage(IpcReqRespHeader *req,
                              ConfigKeyVal *ikey);

    /**
     * @Brief Validates the syntax for KT_VLINK keytype key structure.
     *
     * @param[in] key_vlink KT_VLINK key structure.
     *
     * @retval UPLL_RC_SUCCESS        validation succeeded.
     * @retval UPLL_RC_ERR_CFG_SYNTAX validation failed.
     */
    upll_rc_t ValidateVlinkKey(
        key_vlink_t *key_vlink,
        unc_keytype_operation_t operation = UNC_OP_INVALID);

    /**
     * @Brief Validates the syntax for KT_VLINK keytype value structure.
     *
     * @param[in] val_vlink KT_VLINK value structure.
     *
     * @retval UPLL_RC_ERR_GENERIC    Generic failure.
     * @retval UPLL_RC_SUCCESS        validation succeeded.
     * @retval UPLL_RC_ERR_CFG_SYNTAX validation failed.
     */
    upll_rc_t ValidateVlinkValue(val_vlink_t *val_vlink,
        unc_keytype_operation_t operation =  UNC_OP_INVALID);

    /**
     * @Brief Checks if the specified key type(KT_VLINK) and
     *        associated attributes are supported on the given controller,
     *        based on the valid flag
     *
     * @param[in] req               This structure contains
     *                              IpcReqRespHeader(first 8 fields of input
     *                              request structure).
     * @param[in] ikey              ikey contains key and value structure.
     *
     * @retval  UPLL_RC_SUCCESS             Validation succeeded.
     * @retval  UPLL_RC_ERR_GENERIC         Validation failure.
     * @retval  UPLL_RC_ERR_INVALID_OPTION1 Option1 is not valid.
     * @retval  UPLL_RC_ERR_INVALID_OPTION2 Option2 is not valid.
     */

    upll_rc_t ValidateCapability(IpcReqRespHeader *req,
                                 ConfigKeyVal *ikey,
                                 const char *ctrlr_name = NULL);

    /**
     * @Brief Checks if the specified key type and
     *        associated attributes are supported on the given controller,
     *        based on the valid flag.
     *
     * @param[in] crtlr_name      Controller name.
     * @param[in] ikey            Corresponding key and value structure.
     *
     * @retval UPLL_RC_SUCCESS                    validation succeeded.
     * @retval UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR Attribute NOT_SUPPORTED.
     * @retval UPLL_RC_ERR_GENERIC                Generic failure.
     */
    upll_rc_t ValVlinkAttributeSupportCheck(const char * crtlr_name,
                                            ConfigKeyVal *ikey,
                                            uint32_t operation);

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
    upll_rc_t GetVnodeName(ConfigKeyVal *ikey,
                           uint8_t *&vtn_name,
                           uint8_t *&vnode_name);

    upll_rc_t UpdateConfigStatus(ConfigKeyVal *req,
                                 unc_keytype_operation_t op,
                                 uint32_t driver_result,
                                 ConfigKeyVal *upd_key, DalDmlIntf *dmi,
                                 ConfigKeyVal *ctrlr_key = NULL);
    /**
     * @brief  Update config status for commit result and vote result.
     *
     * @param[in/out]  ckv_running  ConfigKeyVal instance.
     * @param[in]      cs_status    either UNC_CS_INVALID or UNC_CS_APPLIED.
     * @param[in]      phase        specify the phase (CREATE,DELETE or UPDATE)
     *
     **/
    upll_rc_t UpdateAuditConfigStatus(unc_keytype_configstatus_t cs_status,
                                      uuc::UpdateCtrlrPhase phase,
                                      ConfigKeyVal *&ckv_running);
    /**
     * @brief  Duplicates the input configkeyval including the key and val.  
     * based on the tbl specified.
     *
     * @param[in]  okey   Output Configkeyval - allocated within the function
     * @param[in]  req    Input ConfigKeyVal to be duplicated.
     * @param[in]  tbl    specifies if the val structure belongs to the main table/ controller table or rename table.
     *
     * @retval         UPLL_RC_SUCCESS      Successfull completion.
     * @retval         UPLL_RC_ERR_GENERIC  Failure case.
     **/
    upll_rc_t DupConfigKeyVal(ConfigKeyVal *&okey, ConfigKeyVal *&req,
                              MoMgrTables tbl = MAINTBL);
    /**
     * @brief  Allocates for the specified val in the given configuration in the     * specified table.   
     *
     * @param[in/out]  ck_val   Reference pointer to configval structure 
     *                          allocated. 
     * @param[in]      dt_type  specifies the configuration candidate/running/
     *                          state 
     * @param[in]      tbl      specifies if the corresponding table is the  
     *                          main table / controller table or rename table.
     *
     * @retval         UPLL_RC_SUCCESS      Successfull completion.
     * @retval         UPLL_RC_ERR_GENERIC  Failure case.
     **/
    upll_rc_t AllocVal(ConfigVal *&ck_val, upll_keytype_datatype_t dt_type,
                       MoMgrTables tbl = MAINTBL);
    /**
     * @brief      Method to get a configkeyval of a specified keytype from an input configkeyval
     *
     * @param[in/out]  okey                 pointer to output ConfigKeyVal 
     * @param[in]      parent_key           pointer to the configkeyval from which the output configkey val is initialized.
     *
     * @retval         UPLL_RC_SUCCESS      Successfull completion.
     * @retval         UPLL_RC_ERR_GENERIC  Failure case.
     */
    upll_rc_t GetChildConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *parent_key);
    /**
     * @brief      Method to get a configkeyval of the parent keytype 
     *
     * @param[in/out]  okey           pointer to parent ConfigKeyVal 
     * @param[in]      ikey           pointer to the child configkeyval from 
     * which the parent configkey val is obtained.
     *
     * @retval         UPLL_RC_SUCCESS      Successfull completion.
     * @retval         UPLL_RC_ERR_GENERIC  Failure case.
     **/
    upll_rc_t GetParentConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *ikey);
    upll_rc_t GetControllerKey(ConfigKeyVal *ikey, ConfigKeyVal *&okey,
                               unc_keytype_datatype_t dt_type,
                               char *ctrlr_name);
    upll_rc_t SwapVnodes(ConfigKeyVal *&temp_ck_vlink);
    upll_rc_t SwapKeyVal(ConfigKeyVal *ikey, ConfigKeyVal *&okey,
                         DalDmlIntf *dmi, uint8_t *ctrlr_id, bool &no_rename);
    upll_rc_t GetRenamedControllerKey(ConfigKeyVal *ikey,
                                      upll_keytype_datatype_t dt_type,
                                      DalDmlIntf *dmi,
                                      controller_domain *ctrlr_dom);
    upll_rc_t GetRenamedUncKey(ConfigKeyVal *ctrlr_key,
                               upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
                               uint8_t *ctrlr_id);

    upll_rc_t GetControllerDomainId(ConfigKeyVal *ikey,
                                    controller_domain_t *ctrlr_dom);

    upll_rc_t ValidateAttribute(ConfigKeyVal *kval, 
                                DalDmlIntf *dmi,
                                IpcReqRespHeader *req = NULL); 

    /**
     * @brief Validate whether the boundary exists in Physical
     *        -boundary data read from physical is stored in class variable
     * @param[in]       boundary_name  BoundaryId
     * @param[out]      upll_rc_t      UPLL_RC_ERR_CFG_SEMANTIC on error
     *                                    UPLL_RC_SUCCESS on success
     *
     * @retval UPLL_RC_SUCCESS      Successful
     * @retval UPLL_RC_ERR_GENERIC  Failed to fetch Boundary Data
     */
    upll_rc_t ValidateBoundary(uint8_t *boundary_name,IpcReqRespHeader *req);

    /* Rename */

    bool GetRenameKeyBindInfo(unc_key_type_t key_type, BindInfo *&binfo,
                              int &nattr, MoMgrTables tbl);
    upll_rc_t GetRenameInfo(ConfigKeyVal *ikey,
                            ConfigKeyVal *okey,
                            ConfigKeyVal *&rename_info,
                            DalDmlIntf *dmi,
                            const char *ctrlr_id,
                            bool &renamed);
    upll_rc_t CopyToConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *ikey);
    upll_rc_t UpdateVnodeVal(ConfigKeyVal *rename_info, DalDmlIntf *dmi,
                             upll_keytype_datatype_t data_type, bool &no_rename);
    upll_rc_t IsReferenced(ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type,
                           DalDmlIntf *dmi);

    /**
     * @brief Check if Vlink interfaces are unique and update accordingly
     *
     * @param[in]  dt_type  database type
     * @param[in]  ikey     pointer to ConfigKeyVal
     * @param[in]  dmi      pointer to DalDmlIntf  
     * @param[in]  controller_ids      pointer to array of controller domains 
     *
     * @retval UPLL_RC_SUCCESS      Successful
     * @retval UPLL_RC_ERR_GENERIC  Generic error/Failed to update VbrIf
     */
    upll_rc_t UpdateVlinkIf(upll_keytype_datatype_t dt_type,
                              ConfigKeyVal *ikey, DalDmlIntf *dmi,
                              controller_domain_t *ctrlr_dom);
    /**
     * @brief Updates the flag bits of the member interfacs
     *
     * @param[in]  dt_type  database type
     * @param[in]  ckv_if   pointer to member interface ConfigKeyVal
     * @param[in]  dmi      pointer to DalDmlIntf  
     * @param[in]  mgr       corresponding if momgr
     * @param[out] vnif_type  interface type encoded in 6 bits of vlink flag
     *
     * @retval UPLL_RC_SUCCESS      Successful
     * @retval UPLL_RC_ERR_GENERIC  Generic error/Failed to update member if
     */

     upll_rc_t UpdateVlinkMemIfFlag(upll_keytype_datatype_t dt_type,
                                   ConfigKeyVal *ckv_if, 
                                   DalDmlIntf *dmi,
                                   vnode_if_type &vnif_type,
                                   MoMgrImpl *mgr,
                                   unc_keytype_operation_t op);
    /**
     * @brief Update the VbrIf ConfigKeyVal with vExternal details fetched from
     *        physical, if the VbrIf is part of vExternal
     *
     * @param[in]  dt_type  database type
     * @param[in]  ikey     pointer to ConfigKeyVal
     * @param[in]  ck_vbrif array of pointers to component vbrifs
     * @param[in]  dmi      pointer to DalDmlIntf  
     *
     * @retval UPLL_RC_SUCCESS      Successful
     * @retval UPLL_RC_ERR_GENERIC  Generic error/Failed to update VbrIf
     */
    upll_rc_t UpdateVbrIfExternal(upll_keytype_datatype_t dt_type,
             ConfigKeyVal *ikey, ConfigKeyVal **cv_vbrif, DalDmlIntf *dmi);

    /**
     * @brief Check if the Vlink interface is a VbrIf. If it is VbrIf,
     *        Update portmap if port on switch in not preconfigured
     with settings different from physical data
     *
     * @param[in]  ikey           pointer to ConfigKeyVal
     * @param[in]  ikey           pointer to ConfigKeyVal
     * @param[in]  dmi            pointer to DalDmlIntf  
     * @param[in]  rename_vbr_if  uint8_t  
     *
     * @retval UPLL_RC_SUCCESS               Successful
     * @retval UPLL_RC_ERR_NO_SUCH_INSTANCE  vLink Interface is not a VbrIf Interface
     * @retval UPLL_RC_ERR_GENERIC           Generic error/Failed to update VbrIfi
     */
    upll_rc_t CheckPortmapValidandUpdateVbrIf(ConfigKeyVal *ikey,
                                              ConfigKeyVal *ck_drv_vbr_if,
                                              DalDmlIntf *dmi,
                                              upll_keytype_datatype_t dt_type);

    bool GetBoundaryData(ConfigKeyVal *ikey,
                         IpcRequest *ipc_req,
                         IpcResponse *ipc_resp);

    /**
     * @brief If portmap is valid, compare the switch_id, port_name and vlan_id 
     *        of physical boundary data with the existing VbrIf portmap details
     *        and throw error if mismatch
     *
     * @param[in]  req   pointer to IpcReqRespHeader
     * @param[in]  ikey  pointer to ConfigKeyVal
     * @param[in]  dmi   pointer to DalDmlIntf  
     *
     * @retval true      Successful
     * @retval false     Failure/MisMatch  
     */
    bool CompareVbrIfWithPhysicalData(ConfigKeyVal *ck_drv_vbr_if,
                                      ConfigKeyVal *ck_boundary_data,
                                      ConfigKeyVal *ikey);

    /**
     * @brief Create a Driver port map structure which is filled with data from
     *                                                     Boundary 
     *
     * @param[in]  iokey              pointer to vbrif ConfigKeyVal
     * @param[in]  ikey               pointer to vlink ConfigKeyVal
     *
     */
    upll_rc_t ConverttoDriverPortMap(ConfigKeyVal *iokey, ConfigKeyVal *ikey);

    upll_rc_t CreateVnodeConfigKey(ConfigKeyVal *ikey,
                                   ConfigKeyVal *&okey);
    /* @brief         This method invoke when the VTN merge hapeening between
     *                Running and DT import. This will checks the vnode name 
     *                unique or not. 
     *              
     * @param[in]     keytype       UNC KEY TYPE
     * @param[in/out] ctrlr_id      Controller ID                    
     * @param[in]     conflict_ckv  key and value structure 
     * @param[in]     dal    Pointer to the DalDmlIntf(DB Interface)
     * 
     * @retval  UPLL_RC_SUCCESS                    Completed successfully.
     * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
     * @retval  UPLL_RC_ERR_RESOURCE_DISCONNECTED  Resource disconnected.
     * @retval  UPLL_RC_ERR_DB_ACCESS              DB Read/Write error.
     * @retval  UPLL_RC_ERR_MERGE_CONFLICT         Semantic check error.
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE       Given key does not exist 
     *
     **/
    upll_rc_t MergeValidate(unc_key_type_t keytype,
                            const char *ctrlr_id,
                            ConfigKeyVal *ikey,
                            DalDmlIntf *dmi);

    /* @brief     To convert the value structure read from DB to 
     *            VTNService during READ operations
     * @param[in/out] ikey      Pointer to the ConfigKeyVal Structure           
     * 
     * @retval  UPLL_RC_SUCCESS                    Completed successfully.
     * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
     *
     **/
    upll_rc_t AdaptValToVtnService(ConfigKeyVal *ikey);
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
    upll_rc_t CreateCandidateMo(IpcReqRespHeader *req,
                                ConfigKeyVal *ikey,
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
#if 0
    upll_rc_t RestoreVnode(ConfigKeyVal *ikey,
                           IpcReqRespHeader *req,
                           DalDmlIntf *dmi,
                           controller_domain_t ctrlr_domain[]);
#endif
    /**
     * @brief Set the Consolidated Configstatus of boundary vlink after 
     *        retreiving the component if status from individual controllers
     * @param[in]      vlink     ConfigKeyVal pointer
     * @param[in]      dmi       Pointer to DalDmlIntf Class.
     *
     */
    upll_rc_t SetConsolidatedStatus(ConfigKeyVal *vlink,
                                    unc_keytype_operation_t op,
                                    unc_keytype_configstatus_t cs_status,
                                    DalDmlIntf *dmi);

  public:
    VlinkMoMgr();
    virtual ~VlinkMoMgr() {
      for (int i = 0; i < ntable; i++)
        if (table[i]) {
          delete table[i];
        }
      delete[] table;
      if (ck_boundary)
        delete ck_boundary;
      ck_boundary = NULL;
    }
    /**
     * @brief      Method to check if individual portions of a key are valid
     *
     * @param[in/out]  ikey                 pointer to ConfigKeyVal referring to a UNC resource
     * @param[in]      index                db index associated with the variable
     *
     * @retval         true                 input key is valid
     * @retval         false                input key is invalid.
     **/
    bool IsValidKey(void *tkey,
                    uint64_t index);

    /**
     * @brief Exposed to Physical to Verify whether particular Key
     *                            is in Use in Logical
     *

     * @param[in/out]   operation      Operation to check if a boundary is referenced 
     *                                            in UPLL configuration
     * @param[in]       dt_type        Data Type
     * @param[in]       key_type       KeyType of the key to be verified
     * @param[in]       ckv            ConfigKeyVal of the KeyType
     * @param[out]      in_use         true if key is in use, else false
     * @param[out]      upll_rc_t      UPLL_RC_ERR_CFG_SEMANTIC on error
     *                                    UPLL_RC_SUCCESS on success
     *
     */
    upll_rc_t IsKeyInUse(upll_keytype_datatype_t dt_type,
                         const ConfigKeyVal *ckv, bool *in_use,
                         DalDmlIntf *dmi);

    /**
     * @brief  Filters the attributes which need not be sent to controller
     *
     * @param[in/out]  val1   first record value instance.
     * @param[in]      val2   second record value instance.
     * @param[in]      audit  Not used for VTN
     * @param[in]      op     Operation to be performed
     *
     **/
    bool FilterAttributes(void *&val1,
                          void *val2,
                          bool audit_status,
                          unc_keytype_operation_t op);
    /**
     * @brief  Compares the valid value between two database records.
     * 	     if both the values are same, update the valid flag for corresponding
     * 	     attribute as invalid in the first record. 
     *
     * @param[in/out]  val1   first record value instance.
     * @param[in]      val2   second record value instance.
     * @param[in]      audit  if true, CompareValidValue called from audit process.
     *
     **/
    bool CompareValidValue(void *&val1,
                           void *val2,
                           bool audit);

    upll_rc_t ValidateRenameVlinkValue(val_rename_vlink_t *val);
    upll_rc_t CheckVnodeInfo(ConfigKeyVal *ikey,
                             upll_keytype_datatype_t dt_type,
                             DalDmlIntf *dmi);
    upll_rc_t SetOperStatus(ConfigKeyVal *ikey,
                            DalDmlIntf *dmi,
                            int notification,
                            bool skip = false);

    upll_rc_t GetNodeType(void *key, bool vnode,
                          unc_key_type_t &keytype,
                          ConfigKeyVal *&ck_val, DalDmlIntf *dmi);
    /**
     * @brief Returns success if member of Boundary vlink
     *
     * @param[in]       ck_vbrif         ConfigKeyVal of the vbrif
     * @param[in]       dt_type        Configuration type 
     * @param[in/out]   ck_vlink       ConfigKeyVal of the vlink key formed
     * @param[in]       dmi            DB Connection
     * @param[out]      upll_rc_t      UPLL_RC_SUCCESS if member
     *                                 UPLL_RC_ERR_NO_SUCH_INSTANCE if not 
     *                                 UPLL_RC_SUCCESS on success
     *
     */
    upll_rc_t CheckIfMemberOfVlink(ConfigKeyVal *ck_vbrif, 
                                 upll_keytype_datatype_t dt_type,
                                 ConfigKeyVal *&ck_vlink,
                                 DalDmlIntf *dmi,
                                 vn_if_type &if_type);
    /**
     * @brief Creates a vlink key from a vbrif key that is a boundary.  
     *
     * @param[in]       keyVal         ConfigKeyVal of the vbrif
     * @param[in]       dt_type        Configuration type 
     * @param[in/out]   ck_vlink       ConfigKeyVal of the vlink key formed
     * @param[in]       dmi            DB Connection
     * @param[out]      upll_rc_t      UPLL_RC_ERR_CFG_SEMANTIC on error
     *                                    UPLL_RC_SUCCESS on success
     *
     */
    upll_rc_t GetVlinkKeyVal(ConfigKeyVal *keyVal,
                           upll_keytype_datatype_t dt_type,
                           ConfigKeyVal *&ck_vlink,
                           DalDmlIntf *dmi);
    /**
     * @brief Determines if a given vlink is a boundary 
     *
     * @param[in/out]   ck_vlink    ConfigKeyVal of the vlink key with 
     *                              populated user data
     * @retval           bool       true if it is aboundary         
     *                   false      if it is not
     */
    inline upll_rc_t BoundaryVlink(ConfigKeyVal        *ck_main,
                              controller_domain_t *ctrlr_dom,
                              bool            &bound_vlink) {
       upll_rc_t result_code = GetControllerDomainId(ck_main, ctrlr_dom);
       if (result_code != UPLL_RC_SUCCESS) {
         UPLL_LOG_DEBUG("Returning %d\n", result_code);
         return result_code;
       }
       if((memcmp(ctrlr_dom[0].ctrlr, ctrlr_dom[1].ctrlr,kMaxLenCtrlrId)) || 
          (memcmp(ctrlr_dom[0].domain, ctrlr_dom[1].domain,kMaxLenDomainId))) 
          bound_vlink = true;
       else
          bound_vlink = false;
       return UPLL_RC_SUCCESS;
    }
    upll_rc_t DeleteMo(IpcReqRespHeader *req,
                               ConfigKeyVal *ikey,
                               DalDmlIntf *dmi);
    upll_rc_t UpdateMo(IpcReqRespHeader *req,
                              ConfigKeyVal *ikey,
                              DalDmlIntf *dmi);
    upll_rc_t TxUpdateDtState(unc_key_type_t ktype,
                              uint32_t session_id,
                              uint32_t config_id,
                              DalDmlIntf *dmi);
};

typedef struct val_db_rename_vlink {
    uint8_t valid[2];
    uint8_t ctrlr_vtn_name[(kMaxLenVtnName + 1)];
    uint8_t ctrlr_vlink_name[(kMaxLenVlinkName + 1)];
} val_db_rename_vlink_t;

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
#
#endif
