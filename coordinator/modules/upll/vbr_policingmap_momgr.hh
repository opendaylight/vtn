/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef MODULES_UPLL_VBR_POLICINGMAP_MOMGR_HH_
#define MODULES_UPLL_VBR_POLICINGMAP_MOMGR_HH_

#include <string>
#include "momgr_impl.hh"

namespace unc {
namespace upll {
namespace kt_momgr {

enum vbrpolicingmapMoMgrTables {
  VBRPOLICINGMAPTBL = 0, NVBRPOLICINGMAPTABLES
};

/* This file declares interfaces for keyType KT_VBR_POLICINGMAP */
/**
 * @Brief  VbrPolicingMapMoMgr class handles all the request
 *         received from service.
 */
class VbrPolicingMapMoMgr : public MoMgrImpl {
  private:
    /**
     * Member Variable for PolicingProfileBindInfo.
     */
    static BindInfo vbr_policingmap_bind_info[];
    static BindInfo key_vbrpm_maintbl_rename_bind_info[];
    static BindInfo key_vbrpm_policyname_maintbl_rename_bind_info[];

  public:
    /**
     * @Brief  PolicingProfileMoMgr Class Constructor.
     */
    VbrPolicingMapMoMgr();
    /**
     * @Brief  PolicingProfileMoMgr Class Destructor.
     */
    ~VbrPolicingMapMoMgr() {
      for (int i = 0; i < ntable; i++) {
        if (table[i]) {
          delete table[i];
        }
      }
      delete[] table;
    }

    /**
     * @Brief This API is used to create the record (Vbr name with
     * Policer name) in VbrPolicingMap table and Increment the refcount
     * or insert the record based on the incoming policer name's availability
     * in policingprofilectrl table
     *
     * @param[in] req                          Describes
     *                                         RequestResponderHeaderClass.
     * @param[in] ikey                         Pointer to ConfigKeyVal Class.
     * @param[in] dmi                          Pointer to DalDmlIntf Class.
     *
     * @retval    UPLL_RC_ERR_INSTANCE_EXISTS  Policymap Record Already Exists.
     * @retval    UPLL_RC_ERR_CFG_SEMANTIC     Policy Profile Record not
     *                                         available.
     * @retval    UPLL_RC_SUCCESS              Successful completion.
     * @retval    UPLL_RC_ERR_GENERIC          Generic Errors.
     */
    upll_rc_t CreateCandidateMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                                DalDmlIntf *dm);

    /**
     * @Brief This API is used to delete the record (Vbr name with
     * Policer name) in VbrPolicingMap table and decrement the refcount
     * based on the record availability in policingprofilectrl table
     *
     * @param[in] req                           Describes
     *                                          RequestResponderHeaderClass.
     * @param[in] ikey                          Pointer to ConfigKeyVal Class.
     * @param[in] dmi                           Pointer to DalDmlIntf Class.
     *
     * @retval    UPLL_RC_ERR_NO_SUCH_INSTANCE  Record Not available.
     * @retval    UPLL_RC_SUCCESS               Successful completion.
     * @retval    UPLL_RC_ERR_GENERIC           Generic Errors.
     */
    upll_rc_t DeleteMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                       DalDmlIntf *dmi);

    /**
     * @Brief This API is used to Update the record (Vbr name with
     * Policer name) in VbrPolicingMap table and Increment the refcount
     * or insert the record based on the incoming policer name's availability
     * in policingprofilectrl table. Get the existing policer name in
     * vtnpolicingmap table and decrement the refcount in policingprofilectrl
     * table in respective vtn associated controller name
     *
     * @param[in] req                           Describes
     *                                          RequestResponderHeaderClass.
     * @param[in] ikey                          Pointer to ConfigKeyVal Class.
     * @param[in] dmi                           Pointer to DalDmlIntf Class.
     *
     * @retval    UPLL_RC_ERR_NO_SUCH_INSTANCE  Record Not available.
     * @retval    UPLL_RC_ERR_CFG_SEMANTIC      Policy Profile Record not
     *                                          available.
     * @retval    UPLL_RC_SUCCESS               Successful completion.
     * @retval    UPLL_RC_ERR_GENERIC           Generic Errors.
     */
    upll_rc_t UpdateMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                       DalDmlIntf *dmi);

    /**
     * @Brief This API is used to check the policymap object availability
     * in vbrpolicingmaptbl CANDIDATE DB
     *
     * @param[in] ikey                          Pointer to ConfigKeyVal Class.
     * @param[in] dt_type                       Configuration information.
     * @param[in] dmi                           Pointer to DalDmlIntf Class.
     *
     * @retval    UPLL_RC_SUCCESS               Successful completion.
     * @retval    UPLL_RC_ERR_GENERIC           Generic Error code.
     * @retval    UPLL_RC_ERR_NO_SUCH_INSTANCE  No Record in DB.
     * @retval    UPLL_RC_ERR_INSTANCE_EXISTS   Record exists in DB.
     */
    upll_rc_t IsReferenced(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                           DalDmlIntf *dmi);

    /**
     * @Brief This API is used to check the policer name availability
     * in policingprofiletbl CANDIDATE DB
     *
     * @param[in] ikey                          Pointer to ConfigKeyVal Class.
     * @param[in] dt_type                       Configuration information.
     * @param[in] dmi                           Pointer to DalDmlIntf Class.
     * @param[in] op                            Describes the Type of Opeartion.
     *
     * @retval    UPLL_RC_SUCCESS               Successful completion.
     * @retval    UPLL_RC_ERR_GENERIC           Generic Error code.
     * @retval    UPLL_RC_ERR_NO_SUCH_INSTANCE  No Record in DB.
     * @retval    UPLL_RC_ERR_INSTANCE_EXISTS   Record exists in DB.
     */
    upll_rc_t IsPolicyProfileReferenced(ConfigKeyVal *ikey,
                                        upll_keytype_datatype_t dt_type,
                                        DalDmlIntf* dmi,
                                        unc_keytype_operation_t op);

    /**
     * @Brief This API is used to get the vbr associated controller name
     * and update (increment/decrement) the refcount in policingprofilectrlrtbl
     *
     * @param[in] ikey                   Pointer to ConfigKeyVal Class.
     * @param[in] dt_type                Configuration information.
     * @param[in] dmi                    Pointer to DalDmlIntf Class.
     * @param[in] op                     Describes the Type of Opeartion.
     *
     * @retval    UPLL_RC_SUCCESS        Successful completion.
     * @retval    UPLL_RC_ERR_GENERIC    Generic Errors.
     * @retval    UPLL_RC_ERR_DB_ACCESS  Error accessing DB.
     */
    upll_rc_t UpdateRefCountInPPCtrlr(ConfigKeyVal *ikey,
                                      upll_keytype_datatype_t dt_type,
                                      DalDmlIntf *dmi,
                                      unc_keytype_operation_t op,
                                      TcConfigMode config_mode,
                                      string vtn_name);

    /**
     *  @Brief Method used to get ctrlrid .
     *
     *  @param[out] okey        Pointer to class ConfigKeyVal
     *  @param[in]  ikey        Pointer to ConfigKeyVal Class.
     *  @param[in]  dmi         Pointer to DalDmlIntf Class.
     *
     *  @retval     UPLL_RC_SUCCESS  Successfull completion.
     *  */
    upll_rc_t GetControllerId(ConfigKeyVal *ikey, ConfigKeyVal *&okey,
                              upll_keytype_datatype_t dt_type, DalDmlIntf *dmi);

    /**
     * @Brief This API is used to read the configuration and statistics
     *
     * @param[in]  req                           Describes
     *                                           RequestResponderHeaderClass.
     * @param[out] ikey                          Pointer to ConfigKeyVal Class.
     * @param[in]  dmi                           Pointer to DalDmlIntf Class.
     *
     * @retval     UPLL_RC_ERR_NO_SUCH_INSTANCE  Record Not available
     * @retval     UPLL_RC_ERR_CFG_SEMANTIC      Policy Profile Record not
     *                                           available
     * @retval     UPLL_RC_SUCCESS               Successful completion.
     * @retval     UPLL_RC_ERR_GENERIC           Generic Errors.
     * @retval     UPLL_RC_ERR_DB_ACCESS         Error accessing DB.
     */
    upll_rc_t ReadMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                     DalDmlIntf *dmi);

    /**
     * @Brief This API is used to read the siblings configuration
     *            and statistics
     *
     * @param[in]  req                           Describes
     *                                           RequestResponderHeaderClass.
     * @param[out] ikey                          Pointer to ConfigKeyVal Class.
     * @param[in]  begin                         Describes read from begin or not
     * @param[in]  dmi                           Pointer to DalDmlIntf Class.
     *
     * @retval     UPLL_RC_SUCCESS               Successful completion.
     * @retval     UPLL_RC_ERR_GENERIC           Generic Errors.
     * @retval     UPLL_RC_ERR_NO_SUCH_INSTANCE  Record Not available
     * @retval     UPLL_RC_ERR_CFG_SEMANTIC      PolicyProfile Record not found
     * @retval     UPLL_RC_ERR_DB_ACCESS         Error accessing DB.
     */
    upll_rc_t ReadSiblingMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                            bool begin, DalDmlIntf *dmi);

    /**
     * @Brief Method used to get the Bind Info Structure for Rename Purpose.
     *
     * @param[in] key_type   Describes the KT Information.
     * @param[in] binfo
     * @param[in] nattr      Describes the Tbl For which the Operation is
     *                       Targeted.
     * @param[in] tbl        Describes the Table Information.
     *
     * @retval    pfc_true   Successful Completion.
     * @retval    pfc_fasle  Failure.
     */
    bool GetRenameKeyBindInfo(unc_key_type_t key_type, BindInfo *&binfo,
                              int &nattr, MoMgrTables tbl);

    /**
     * @Brief Method to Copy The ConfigkeyVal with the Input Key.
     *
     * @param[out]  okey                 Pointer to ConfigKeyVal Class for
     *                                   which attributes have to be copied.
     * @param[in]   ikey                 Pointer to ConfigKeyVal Class.
     *
     * @retval      UPLL_RC_SUCCESS      Successfull Completion.
     * @retval      UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */
    upll_rc_t CopyToConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *ikey);

    /**
     * @Brief This API is used to read the policing map record in IMPORT's
     * vbrpolicingmap table and check with CANDIDATES's policingprofile table
     * If record exists return merge conflict
     *
     * @param[in] keytype                     Describes the keyType Information.
     * @param[in] ctrlr_id                    Describes the Controller Name.
     * @param[in] ikey                        This Contains the pointer to the
     *                                        Class for which fields have to be
     *                                        Validated before the Merge.
     * @param[in] dmi                         Pointer to DalDmlIntf Class.
     *
     * @retval    UPLL_RC_SUCCESS             Successful completion.
     * @retval    UPLL_RC_ERR_GENERIC         Generic Errors.
     * @retval    UPLL_RC_ERR_MERGE_CONFLICT  Record already avilable
     */
    upll_rc_t MergeValidate(unc_key_type_t keytype, const char *ctrlr_id,
                            ConfigKeyVal *ikey, DalDmlIntf *dmi,
                            upll_import_type import_type);

    /**
     * @Brief This API is used to get the unc key name
     * @param[out] ikey                 This Contains the pointer to the Class
     for which fields have to be updated
     with values from the parent Class.
     *
     * @param[in]  dt_type              Configuration Information.
     * @param[in]  dmi                  Pointer to DalDmlIntf Class.
     * @param[in]  ctrlr_id             Controller name.
     *
     * @retval     UPLL_RC_SUCCESS      Successful completion.
     * @retval     UPLL_RC_ERR_GENERIC  Generic Errors.
     */
    upll_rc_t GetRenamedUncKey(ConfigKeyVal *ikey,
                               upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
                               uint8_t *ctrlr_id);

    /**
     * @Brief This API is used to get the renamed Controller's key
     *
     * @param[out] ikey                Contains the Pointer to ConfigkeyVal
     *                                 Class and contains the Pfc Name.
     * @param[in] dt_type              Configuratin information.
     * @param[in] dmi                  Pointer to DalDmlIntf Class.
     * @param[in] ctrlr_name           Controller name.
     *
     * @retval    UPLL_RC_SUCCESS      Successful completion.
     * @retval    UPLL_RC_ERR_GENERIC  Generic Errors.
     */
    upll_rc_t GetRenamedControllerKey(ConfigKeyVal *ikey,
                                      upll_keytype_datatype_t dt_type,
                                      DalDmlIntf *dmi,
                                      controller_domain *ctrlr_dom = NULL);

    /**
     * @Brief Method To Compare the Valid Check of Attributes
     *
     * @param[In/out] val1             Pointer to ConfigKeyVal Class which
     *                                 contains only Valid Attributes.
     * @param[in]     val2             Pointer to ConfigKeyVal Class.
     * @param[in]     audit            Audit purpose.
     *
     * @retval        UPLL_RC_SUCCESS  Successful completion.
     */
    bool CompareValidValue(void *&val1, void *val2, bool audit);

    /**
     ** @Brief This method used to update the configstatus in Db during
     **        Trasaction Operation
     **
     ** @param[in]  ckv                  Pointer to ConfigkeyVal class.
     ** @param[in]  op                   Type of operation
     ** @param[in]  driver_result        Result code from Driver.
     ** @param[in]  nreq                 Pointer to ConfigkeyVal.
     ** @param[out] ctrlr_key            Pointer to ConfigkeyVal for controller.
     **
     ** @retval     UPLL_RC_SUCCESS      Successful completion.
     ** @retval     UPLL_RC_ERR_GENERIC  Generic Errors.
     **/
     upll_rc_t UpdateConfigStatus(ConfigKeyVal *ckv,
              unc_keytype_operation_t op, uint32_t driver_result,
              ConfigKeyVal *nreq, DalDmlIntf *dmi,
              ConfigKeyVal *ctrlr_key);

    /**
     * @Brief This API updates the Configuration status for AuditConfigiration todo
     *
     * @param[in]  cs_status            either UNC_CS_INVALID or UNC_CS_APPLIED.
     * @param[in]  phase                Describes the phase of controller.
     * @param[in]  ckv_running          Pointer to ConfigkeyVal.
     * @param[in]  dmi                  Pointer to the DalDmlIntf(DB Interface)
     *
     * @retval     UPLL_RC_SUCCESS      Successful completion.
     * @retval     UPLL_RC_ERR_GENERIC  Generic Errors.
     */
    upll_rc_t UpdateAuditConfigStatus(
              unc_keytype_configstatus_t cs_status,
              uuc::UpdateCtrlrPhase phase,
              ConfigKeyVal *&ckv_running,
              DalDmlIntf *dmi);

    /**
     * @Brief Method to compare to keys
     *
     * @param[in]  key1  Pointer to key structure for comparision
     * @param[in]  key2  Pointer to key for comparision
     *
     * @return  TRUE   If keys match
     * @return  FALSE  If keys dont match
     */
    bool CompareKey(void *key1, void *key2);

    /**
     * @Brief This API is used to know the value availability of
     *         val structure attributes
     *
     * @param[in]  val                  pointer to the value structure.
     * @param[out] valid                reference to the enum containing
     *                                  the possible values of Valid flag.
     * @param[in]  indx                 Gives the index of attribute for
     *                                  validity.
     * @param[in]  dt_type              configuration for validity check.
     * @param[in]  tbl                  Table name.
     *
     * @retval     UPLL_RC_SUCCESS      Successful completion.
     * @retval     UPLL_RC_ERR_GENERIC  Generic Errors.
     */
    upll_rc_t GetValid(void *val, uint64_t indx, uint8_t *&valid,
                       upll_keytype_datatype_t dt_type, MoMgrTables tbl);

    /**
     * @Brief This API is used to allocate the memory for incoming in configval
     *
     * @param[out] ck_val               This Contains the pointer to the Class
     *                                  for which memory has to be allocated.
     * @param[in]  dt_type              Configuration information.
     * @param[in]  tbl                  Table name.
     *
     * @retval     UPLL_RC_SUCCESS      Successful completion.
     * @retval     UPLL_RC_ERR_GENERIC  Generic Errors.
     */
    upll_rc_t AllocVal(ConfigVal *&ck_val, upll_keytype_datatype_t dt_type,
                       MoMgrTables tbl);

    /**
     * @Brief This API is used to get the duplicate configkeyval
     *
     * @param[out] okey                 This Contains the pointer to the Class
     *                                  for which fields have to be updated
     *                                  with values from the Request.
     * @param[in]  req                  This Contains the pointer to the Class
     *                                  which is used for the Duplication.
     * @param[in]  tbl                  Table name
     *
     * @retval     UPLL_RC_SUCCESS      Successful completion.
     * @retval     UPLL_RC_ERR_GENERIC  Generic Errors.
     */
    upll_rc_t DupConfigKeyVal(ConfigKeyVal *&okey, ConfigKeyVal *&req,
                              MoMgrTables tbl);

    /**
     * @Brief Method used to fill the CongigKeyVal with the Parent Class
     *            Information.
     *
     * @param[out] okey                This Contains the pointer to the
     *                                 ConfigKeyVal Class for which fields have
     *                                 to be updated with values from the
     *                                 parent Class.
     * @param[in] parent_key           This Contains the pointer to the
     *                                 ConfigKeyVal Class which is the
     *                                 Parent class used to fill the details.
     *
     * @retval    UPLL_RC_SUCCESS      Successful completion.
     * @retval    UPLL_RC_ERR_GENERIC  Generic Errors.
     */
    upll_rc_t GetChildConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *parent_key);

    /**
     * @Brief Validates the syntax of the specified key and value structure
     *        for KT_VBR_POLICINGMAP/KT_VBR_POLICINGMAP_ENTRY keytype
     *
     * @param[in] IpcReqRespHeader contains first 8 fields of input request structure
     * @param[in] ConfigKeyVal key and value structure.
     *
     * @retval UPLL_RC_SUCCESS               Successful.
     * @retval UPLL_RC_ERR_CFG_SYNTAX        Syntax error.
     * @retval UPLL_RC_ERR_NO_SUCH_INSTANCE  key_vbr is not available.
     * @retval UPLL_RC_ERR_GENERIC           Generic failure.
     * @retval UPLL_RC_ERR_INVALID_OPTION1   option1 is not valid.
     * @retval UPLL_RC_ERR_INVALID_OPTION2   option2 is not valid.
     */
    upll_rc_t ValidateMessage(IpcReqRespHeader *req, ConfigKeyVal *key);

    /**
     * @Brief Checks if the specified key type(KT_VBR_POLICINGMAP) and
     *        associated attributes are supported on the given controller,
     *        based on the valid flag
     *
     * @param[in] IpcReqRespHeader  contains first 8 fields of input request structure
     * @param[in] ConfigKeyVal      contains key and value structure.
     * @param[in] ctrlr_name        controller name
     *
     * @retval  UPLL_RC_SUCCESS              Validation succeeded.
     * @retval  UPLL_RC_ERR_GENERIC          Validation failure.
     * @retval  UPLL_RC_ERR_INVALID_OPTION1  Option1 is not valid.
     * @retval  UPLL_RC_ERR_INVALID_OPTION2  Option2 is not valid.
     */
    upll_rc_t ValidateCapability(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                                 const char *ctrlr_name = NULL);
    /**
     * @Brief Method used to fill the CongigKeyVal with the Parent Class
     *            Information.
     *
     * @param[out] okey                This Contains the pointer to the
     *                                 ConfigKeyVal Class for which fields have
     *                                 to be updated with values from the
     *                                 parent Class.
     * @param[in] parent_key           This Contains the pointer to the
     *                                 ConfigKeyVal Class which is the
     *                                 Parent class used to fill the details.
     *
     * @retval    UPLL_RC_SUCCESS      Successful completion.
     * @retval    UPLL_RC_ERR_GENERIC  Generic Errors.
     */
    upll_rc_t GetParentConfigKey(ConfigKeyVal *&okey,
                                 ConfigKeyVal *ikey);

    /**
     * @brief  Method used for Restoring PolicingProfile in the Controller Table
     *
     * @param[in]      ikey       Pointer to ConfigKeyVal Class
     * @param[in]      dt_type    Describes Configiration Information.
     * @param[in]      tbl        Describe the destination table
     * @param[in]      dmi        Pointer to DalDmlIntf Class.
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_DB_ACCESS              DB Read/Write error.
     * @retval  UPLL_RC_ERR_INSTANCE_EXISTS       Record already exists
     * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error.
     */

    upll_rc_t RestorePOMInCtrlTbl(ConfigKeyVal *ikey,
                                  upll_keytype_datatype_t dt_type,
                                  MoMgrTables tbl,
                                  DalDmlIntf* dmi);

    upll_rc_t ConstructReadDetailResponse(ConfigKeyVal *ikey,
                                          ConfigKeyVal *drv_resp_ckv,
                                          upll_keytype_datatype_t dt_type,
                                          unc_keytype_operation_t op,
                                          DbSubOp dbop,
                                          DalDmlIntf *dmi,
                                          ConfigKeyVal **okey,
                                          controller_domain ctrlr_dom);

    bool IsValidKey(void *key, uint64_t index, MoMgrTables tbl = MAINTBL);

    upll_rc_t ConstructReadDetailEntryResponse(ConfigKeyVal *ikey,
                                               ConfigKeyVal *dup_key,
                                               ConfigKeyVal *drv_resp_ckv,
                                               upll_keytype_datatype_t dt_type,
                                               unc_keytype_operation_t op,
                                               DbSubOp dbop,
                                               DalDmlIntf *dmi,
                                               ConfigKeyVal **okey,
                                               controller_domain ctrlr_dom);


    upll_rc_t GetReadVbrEntryKey(ConfigKeyVal *&dup_key,
                                 ConfigKeyVal *ikey);
    upll_rc_t SwapKeyVal(ConfigKeyVal *ikey, ConfigKeyVal *&okey,
                         DalDmlIntf *dmi, uint8_t *ctrlr) {
      return UPLL_RC_SUCCESS;
    }
    upll_rc_t ReadDTsateNormal(ConfigKeyVal *ikey,
                               ConfigKeyVal* dup_key,
                               upll_keytype_datatype_t  dt_type,
                               DbSubOp dbop,
                               DalDmlIntf *dmi,
                               ConfigKeyVal**okey);

    upll_rc_t ReadDetailRecord(IpcReqRespHeader *req,
                             ConfigKeyVal *&ikey,
                             DalDmlIntf *dmi);

    upll_rc_t ValidateAttribute(ConfigKeyVal *kval,
                                DalDmlIntf *dmi,
                                IpcReqRespHeader *req = NULL);
    upll_rc_t ReadSiblingCount(IpcReqRespHeader *req,
                               ConfigKeyVal* ikey,
                               DalDmlIntf *dmi);
    upll_rc_t IsKeyInUse(upll_keytype_datatype_t dt_type,
                     const ConfigKeyVal *ckv,
                     bool *in_use,
                     DalDmlIntf *dmi);
    upll_rc_t GetReadVbrKey(ConfigKeyVal *&dup_key, ConfigKeyVal *ikey);


    upll_rc_t OnPolicerFullAlarm(string ctrlr_name ,
                                 string domain_id,
                                 const key_vtn_t &key_vtn,
                                 const pfcdrv_policier_alarm_data_t &alarm_data,
                                 bool &alarm_raised,
                                 DalDmlIntf *dmi);

    upll_rc_t OnPolicerFailAlarm(string ctrlr_name ,
                                 string domain_id,
                                 const key_vtn_t &key_vtn,
                                 const pfcdrv_policier_alarm_data_t &alarm_data,
                                 bool &alarm_raised,
                                 DalDmlIntf *dmi);

    upll_rc_t CreateAuditMoImpl(ConfigKeyVal *ikey,
                                 DalDmlIntf *dmi,
                                 const char *ctrlr_id);

    upll_rc_t DeleteChildrenPOM(ConfigKeyVal *ikey,
                                upll_keytype_datatype_t dt_type,
                                DalDmlIntf *dmi,
                                TcConfigMode config_mode,
                                string vtn_name);

    upll_rc_t IsPolicingProfileConfigured(const char* policingprofile_name,
                                          upll_keytype_datatype_t dt_type,
                                          DalDmlIntf *dmi);

    upll_rc_t SetValidAudit(ConfigKeyVal *&ikey);

    bool FilterAttributes(void *&val1,
                          void *val2,
                          bool copy_to_running,
                          unc_keytype_operation_t op);

    upll_rc_t UpdateVnodeVal(ConfigKeyVal *ikey,
                             DalDmlIntf *dmi,
                             upll_keytype_datatype_t data_type,
                             bool &no_rename);

    upll_rc_t IsRenamed(ConfigKeyVal *ikey,
                        upll_keytype_datatype_t dt_type,
                        DalDmlIntf *dmi,
                        uint8_t &rename);

    upll_rc_t GetPolicingProfileConfigKey(
          const char *pp_name, ConfigKeyVal *&okey,
          DalDmlIntf *dmi);

    upll_rc_t SetRenameFlag(ConfigKeyVal *ikey,
                            DalDmlIntf *dmi,
                            IpcReqRespHeader *req);
};
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
#endif  // MODULES_UPLL_VBR_POLICINGMAP_MOMGR_HH_
