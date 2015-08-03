/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UPLL_FLOWLISTENTRY_MOMGR_HH_
#define UPLL_FLOWLISTENTRY_MOMGR_HH_

#include <string>
#include<set>
#include "momgr_impl.hh"

namespace unc {
namespace upll {
namespace kt_momgr {

/* This file declares interfaces for keyType KT_VBR_FLOWFILER */
/**
 * @brief VbrFlowFilterMoMgr class handles all the request
 *  received from service.
 */
class FlowListEntryMoMgr: public MoMgrImpl {
  private:
    /**
     * Member Variable for FlowListEntryBindInfo.
     */
    static BindInfo flowlistentry_bind_info[];
    /**
     * Member Variable for FlowListEntryCtrlBindInfo.
     */
    static BindInfo flowlistentry_controller_bind_info[];

    static BindInfo rename_flowlist_entry_main_tbl[];
    static BindInfo rename_flowlist_entry_ctrlr_tbl[];

    bool GetRenameKeyBindInfo(unc_key_type_t key_type,
    BindInfo *&binfo, int &nattr, MoMgrTables tbl);

   /**
    * @brief     Methods Used for  Validating Attribute.
    * @param[in]  kval     The pointer to the ConfigKeyVal class
    *
    * @param[in]  dmi      Pointer to the Database Interface.
    *
    * @retval  UPLL_RC_SUCCESS      Validation succeeded.
    * @retval  UPLL_RC_ERR_GENERIC  Validation failure.
    */
    upll_rc_t ValidateAttribute(ConfigKeyVal *kval,
                                DalDmlIntf *dmi,
                                IpcReqRespHeader *req = NULL);

   /**
    * @Brief Checks if the specified key type(KT_FLOWLIST_ENTRY) and
    *        associated attributes are supported on the given controller,
    *        based on the valid flag
    *
    * @param[in] IpcReqRespHeader  contains first 8 fields of input request
    *                              structure
    * @param[in]   ConfigKeyVal    contains key and value structure.
    * @param[in]   ctrlr_name      controller_name
    *
    * @retval  UPLL_RC_SUCCESS             Validation succeeded.
    * @retval  UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR  Validation failure.
    */
    upll_rc_t ValidateCapability(IpcReqRespHeader *req, ConfigKeyVal *key,
                                 const char *ctrlr_name = NULL);

    /**
     * @Brief Checks if the specified key type and
     *        associated attributes are supported on the given controller,
     *        based on the valid flag.
     *
     * @param[in] val_flowlist_entry  KT_FLOWLIST_ENTRY value structure.
     * @param[in] attrs               pointer to controller attribute
     *
     * @retval UPLL_RC_SUCCESS        indicates attribute check completion
     */
    upll_rc_t ValFlowlistEntryAttributeSupportCheck(
                          val_flowlist_entry_t *val_flowlist_entry,
                          const uint8_t* attrs);

   /**
    * @Brief Checks ip_proto attributes is supported
    *  on the given controller, based on the valid flag.
    *
    * @param[in] val_flowlist_entry KT_FLOWLIST_ENTRY value structure
    * @param[in] attrs               pointer to controller attribute
    *
    */
    void ValidateIpProtoAttribute(
      val_flowlist_entry_t *val_flowlist_entry, const uint8_t *attrs);

    /**
    * @Brief Checks Vlan_priority attributes is supported
    *  on the given controller, based on the valid flag.
    *
    * @param[in] val_flowlist_entry KT_FLOWLIST_ENTRY value structure
    * @param[in] attrs               pointer to controller attributee
    */
    void ValidateVlanPriorityAttribute(
      val_flowlist_entry_t *val_flowlist_entry, const uint8_t *attrs);

   /**
    * @Brief Checks dst_mac and src_mac attributes are supported
    *  on the given controller, based on the valid flag.
    *
    * @param[in] val_flowlist_entry KT_FLOWLIST_ENTRY value structure
    * @param[in] attrs               pointer to controller attribute
    */
    void ValidateMacAttribute(val_flowlist_entry_t *val_flowlist_entry,
      const uint8_t *attrs);

   /**
    * @Brief Checks source and destination IPV4 and IPV4 prefix attributes
    *  are supported on the given controller, based on the valid flag.
    *
    * @param[in] val_flowlist_entry KT_FLOWLIST_ENTRY value structure
    * @param[in] attrs               pointer to controller attribute
    */
    void ValidateIPAttribute(val_flowlist_entry_t *val_flowlist_entry,
      const uint8_t *attrs);

   /**
    * @Brief Checks source and destination IPV6 and IPV6 prefix attributes
    *  are supported on the given controller, based on the valid flag.
    *
    * @param[in] val_flowlist_entry KT_FLOWLIST_ENTRY value structure
    * @param[in] attrs               pointer to controller attribute
    */
    void ValidateIPV6Attribute(val_flowlist_entry_t *val_flowlist_entry,
      const uint8_t *attrs);

   /**
    * @Brief Checks source and destination L4 port and portEndpt attributes
    *  are supported on the given controller, based on the valid flag.
    *
    * @param[in] val_flowlist_entry KT_FLOWLIST_ENTRY value structure
    * @param[in] attrs               pointer to controller attribute
    */
    void ValidateL4PortAttribute(val_flowlist_entry_t *val_flowlist_entry,
      const uint8_t *attrs);

   /**
    * @Brief Checks icmp_type, icmp_code, icmpv6_type, icmpv6_code
    *  are supported on the given controller, based on the valid flag.
    *
    * @param[in] val_flowlist_entry KT_FLOWLIST_ENTRY value structure
    * @param[in] attrs               pointer to controller attribute
    */
    void ValidateICMPAttribute(val_flowlist_entry_t *val_flowlist_entry,
     const uint8_t *attrs);
   /**
    * @Brief Checks Mac_eth_type attributes is supported
    *  on the given controller, based on the valid flag.
    *
    * @param[in] val_flowlist_entry KT_FLOWLIST_ENTRY value structure
    * @param[in] attrs               pointer to controller attribute
    */
    void ValidateMacEthTypeAttribute(
      val_flowlist_entry_t *val_flowlist_entry, const uint8_t *attrs);

   /**
    * @Brief Checks DSCP attributes is supported
    *  on the given controller, based on the valid flag.
    *
    * @param[in] val_flowlist_entry KT_FLOWLIST_ENTRY value structure
    * @param[in] attrs               pointer to controller attribute
    */
    void ValidateIpDscpAttribute(
      val_flowlist_entry_t *val_flowlist_entry, const uint8_t *attrs);

   /**
    * @Brief Validates the syntax of the specified key and value structure
    *        for KT_FLOWLIST_ENTRY keytype
    *
    * @param[in] IpcReqRespHeader  contains first 8 fields of input
    *                              request structure
    * @param[in] ConfigKeyVal      key and value structure.
    *
    * @retval UPLL_RC_SUCCESS              Successful.
    * @retval UPLL_RC_ERR_CFG_SYNTAX       Syntax error.
    * @retval UPLL_RC_ERR_NO_SUCH_INSTANCE key_flowlist_entry is not available.
    * @retval UPLL_RC_ERR_GENERIC          Generic failure.
    * @retval UPLL_RC_ERR_INVALID_OPTION1  option1 is not valid.
    * @retval UPLL_RC_ERR_INVALID_OPTION2  option2 is not valid.
    */
    upll_rc_t ValidateMessage(IpcReqRespHeader *req, ConfigKeyVal *key);

   /**
    * @Brief Validates the syntax for KT_FLOWLIST_ENTRY keytype key structure.
    * @param[in] key_flowlist  KT_FLOWLIST_ENTRY key structure.
    * @retval UPLL_RC_SUCCESS         validation succeeded.
    * @retval UPLL_RC_ERR_CFG_SYNTAX  validation failed.
    */
    upll_rc_t ValidateFlowlistEntryKey(ConfigKeyVal *key,
                                       unc_keytype_operation_t op);

   /**
    * @Brief Validates the syntax for KT_FLOWLIST_ENTRY keytype value structure.
    *
    * @param[in] val_flowlist KT_FLOWLIST_ENTRY value structure.
    *
    * @retval UPLL_RC_SUCCESS         validation succeeded.
    * @retval UPLL_RC_ERR_CFG_SYNTAX  validation failed.
    */
    upll_rc_t ValidateFlowlistEntryVal(
       ConfigKeyVal *key, uint32_t operation, uint32_t dt_type);

    upll_rc_t ValidateFlowlistEntryVal(IpcReqRespHeader *req,
                                 ConfigKeyVal *key,
                                 DalDmlIntf *dmi);
   /**
    * @Brief Validates the syntax for mac_dst, mac_src fields
    *
    * @param[in] val_flowlist_entry  KT_FLOWLIST_ENTRY value structure.
    * @param[in] operation           Describes operation code
    *
    * @retval UPLL_RC_SUCCESS         validation succeeded.
    * @retval UPLL_RC_ERR_CFG_SYNTAX  validation failed.
    */
    upll_rc_t ValidateFlowlistMacAddr(val_flowlist_entry_t *val_flowlist_entry,
                                       uint32_t operation);
   /**
    * @Brief Validates the syntax for mac_eth_type field
    *
    * @param[in] val_flowlist_entry  KT_FLOWLIST_ENTRY value structure.
    * @param[in] operation           Describes operation code
    *
    * @retval UPLL_RC_SUCCESS         validation succeeded.
    * @retval UPLL_RC_ERR_CFG_SYNTAX  validation failed.
    */
    upll_rc_t ValidateEthType(val_flowlist_entry_t *val_flowlist_entry,
                               uint32_t operation);
   /**
    * @Brief Validates the syntax for dst_ip, dst_ip_prefix fields
    *
    * @param[in] val_flowlist_entry KT_FLOWLIST_ENTRY value structure.
    * @param[in] operation     Describes operation code
    *
    * @retval UPLL_RC_SUCCESS        validation succeeded.
    * @retval UPLL_RC_ERR_CFG_SYNTAX validation failed.
    */
    upll_rc_t ValidateIPAddress(val_flowlist_entry_t *val_flowlist_entry,
    uint8_t ip_type, unc_keytype_operation_t operation, bool is_src_ip);

   /**
    * @Brief Validates the syntax for valn_priority field
    *
    * @param[in] val_flowlist_entry KT_FLOWLIST_ENTRY value structure.
    * @param[in] operation     Describes operation code
    *
    * @retval UPLL_RC_SUCCESS        validation succeeded.
    * @retval UPLL_RC_ERR_CFG_SYNTAX validation failed.
    */
    upll_rc_t ValidateVlanPriority(val_flowlist_entry_t *val_flowlist_entry,
                                    uint32_t operation);

   /**
    * @Brief Validates the syntax for ip_proto field
    *
    * @param[in] val_flowlist_entry KT_FLOWLIST_ENTRY value structure.
    * @param[in] operation     Describes operation code
    *
    * @retval UPLL_RC_SUCCESS        validation succeeded.
    * @retval UPLL_RC_ERR_CFG_SYNTAX validation failed.
    */
    upll_rc_t ValidateIPProto(val_flowlist_entry_t *val_flowlist_entry,
                              uint32_t operation);

   /**
    * @Brief Validates the syntax for DSCP field
    *
    * @param[in] val_flowlist_entry KT_FLOWLIST_ENTRY value structure.
    * @param[in] operation     Describes operation code
    *
    * @retval UPLL_RC_SUCCESS        validation succeeded.
    * @retval UPLL_RC_ERR_CFG_SYNTAX validation failed.
    */
    upll_rc_t ValidateDSCP(val_flowlist_entry_t *val_flowlist_entry,
                           uint32_t operation);

   /**
    * @Brief Validates the syntax for l4_dst_port_endpt, l4_dst_port fields
    *
    * @param[in] val_flowlist_entry KT_FLOWLIST_ENTRY value structure.
    * @param[in] operation     Describes operation code
    *
    * @retval UPLL_RC_SUCCESS        validation succeeded.
    * @retval UPLL_RC_ERR_CFG_SYNTAX validation failed.
    */
    upll_rc_t ValidateL4Port(val_flowlist_entry_t *val_flowlist_entry,
                             val_flowlist_entry_t *db_val_fle,
                             uint32_t operation,
                             bool is_src_port);

   /**
    * @Brief Validates the syntax for icmp_type, icmp_code fields
    *
    * @param[in] val_flowlist_entry KT_FLOWLIST_ENTRY value structure.
    * @param[in] val_flowlist KT_FLOWLIST value structure
    * @param[in] operation     Describes operation code
    *
    * @retval UPLL_RC_SUCCESS        validation succeeded.
    * @retval UPLL_RC_ERR_CFG_SYNTAX validation failed.
    */

    upll_rc_t ValidateIcmp(val_flowlist_entry_t *val_flowlist_entry,
                           val_flowlist_entry_t *db_val_fle,
                           uint8_t ip_type, uint32_t operation);

   /**
    * @Brief Validates the syntax for DSCP field
    *
    * @param[in] val_flowlist_entry KT_FLOWLIST_ENTRY value structure.
    * @param[in] operation     Describes operation code
    *
    * @retval UPLL_RC_SUCCESS        validation succeeded.
    * @retval UPLL_RC_ERR_CFG_SYNTAX validation failed.
    */
    upll_rc_t ValidateDscp(val_flowlist_entry_t *val_flowlist_entry,
                           uint32_t operation);

   /**
    * @brief  Method GetControllerDomainSpan.
    *
    * @param[out]  ikey     Contains the Pointer to ConfigkeyVal Class
    * @param[in]   dt_type  Describes Datatype.
    * @param[in]   dmi      Describes the Objct type .
    *
    * @retval  UPLL_RT_SUCCESS      Successfull completion.
    * @retval  UPLL_RC_ERR_GENERIC  Failure.
    * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
    * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
    * */

    upll_rc_t GetControllerDomainSpan(ConfigKeyVal *ikey,
                                      upll_keytype_datatype_t dt_type,
                                      DalDmlIntf *dmi);

     upll_rc_t UpdateMainTbl(ConfigKeyVal *fle_key,
                             unc_keytype_operation_t op,
                             uint32_t driver_result,
                             ConfigKeyVal *nreq,
                             DalDmlIntf *dmi);

  public:
    /**
     * @brief FlowlistEntryMoMgr Class Constructor.
     */
    FlowListEntryMoMgr();
    /**
     * @brief FlowlistEntryMoMgr Class Destructor.
     */
    ~FlowListEntryMoMgr() {
      for (int i = 0; i < ntable; i++) {
        if (table[i]) {
          delete table[i];
        }
      }
      delete[] table;
    }

    /**
     * @brief     Methods Used for getting Value Attribute
     *
     * @param[out] valid      Describes the Valid Attribute.
     * @param[in]  val        This Contains the pointer to the class
     *                        for which iValid has to be checked.
     * @param[in]  indx       Describes the Index of the Attribute.
     * @param[in]  dt_type    Describes Configiration Information.
     * @param[in]  tbl        Describes the Destination table Information.
     *
     * @retval  UPLL_RT_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure.
     */
    upll_rc_t GetValid(void *val,
                       uint64_t indx,
                       uint8_t *&valid,
                       upll_keytype_datatype_t dt_type,
                       MoMgrTables tbl);
    /**
     * @brief  Allocates Memory for the Incoming Pointer to the Class.
     *
     * @param[out]  ck_val   This Contains the pointer to the Class for which
     *                       memory has to be allocated.
     * @param[in]   dt_type  Describes Configiration Information.
     * @param[in]   tbl      Describes the Destination table Information.
     *
     * @retval  UPLL_RT_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure.
     */
    upll_rc_t AllocVal(ConfigVal *&ck_val, upll_keytype_datatype_t dt_type,
        MoMgrTables tbl);

    /**
     * @brief  Update config status for commit result and vote result.
     *
     * @param[in,out]  ckv_running  ConfigKeyVal instance.
     * @param[in]      cs_status    either UNC_CS_INVALID or UNC_CS_APPLIED.
     * @param[in]      phase        specify the phase (CREATE,DELETE or UPDATE)
     * @param[in]      dmi          Pointer to the DalDmlIntf(DB Interface)
     *
     * @retval  UPLL_RC_SUCCECSS     Successful Completion
     * @retval  UPLL_RC_ERR_GENERIC  For failue case GENERIC ERROR
     *
     ***/
     upll_rc_t UpdateAuditConfigStatus(
                           unc_keytype_configstatus_t cs_status,
                           uuc::UpdateCtrlrPhase phase,
                           ConfigKeyVal *&ckv_running,
                           DalDmlIntf *dmi);

    /**
     * @brief  Method used to fill the CongigKeyVal with the Parent
     *         Class Information
     *
     * @param[out] okey        This Contains the pointer to the ConfigKeyVal
     *                         Class for which fields have to be updated with
     *                         values from the parent Class.
     * @param[in]  parent_key  This Contains the pointer to the ConfigKeyVal
     *                         Class which is the Parent Class used to fill
     *                         the details.
     *
     * @retval  UPLL_RT_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure.
     */
    upll_rc_t GetChildConfigKey(ConfigKeyVal *&okey, ConfigKeyVal *parent_key);

    /**
     * @brief     Method used to get the RenamedUncKey.
     *
     * @param[out] ctrlr_key  This Contains the pointer to the Class for which
     *                        fields have to be updated with values from the
     *                        parent Class.
     * @param[in]  dt_type    Describes Configiration Information.
     * @param[in]  dmi        Pointer to DalDmlIntf Class.
     * @param[in]  ctrlr_id   Describes the Controller Name.
     *
     * @retval  UPLL_RT_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure.
     */
    upll_rc_t GetRenamedUncKey(ConfigKeyVal *ctrlr_key,
        upll_keytype_datatype_t dt_type, DalDmlIntf *dmi, uint8_t *ctrlr_id);

    /**
     * @brief  Method used to Duplicate the ConfigkeyVal.
     *
     * @param[out] okey  Pointer to the Class for which fields have to be
     *                   updated with values from the Request.
     * @param[in]  req   Pointer to the Class which is used for the Duplication.
     * @param[in]  tbl   Describes the Destination tables.
     *
     * @retval  UPLL_RT_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure.
     */
    upll_rc_t DupConfigKeyVal(ConfigKeyVal *&okey, ConfigKeyVal *&req,
        MoMgrTables tbl);

    /**
     * @brief     Method used for Validation before Merge.
     *
     * @param[in] ikey        This Contains the pointer to the Class for which
     *                        fields have to be Validated before the Merge.
     * @param[in] keytype     Describes the keyType Information.
     * @param[in] dmi         Pointer to DalDmlIntf Class.
     * @param[in] ctrlr_id    Describes the Controller Name.
     *
     * @retval    UPLL_RC_SUCCESS  Successfull completion.
     */
    upll_rc_t MergeValidate(unc_key_type_t keytype, const char *ctrlr_id,
        ConfigKeyVal *ikey, DalDmlIntf *dmi, upll_import_type import_type);

    /**
     * @brief     Method used for Rename Operation.
     *
     * @param[in] req        Describes RequestResponderHeaderClass.
     * @param[in] ikey       Pointer to ConfigKeyVal Class.
     * @param[in] dmi        Pointer to DalDmlIntf Class.
     * @param[in] ctrlr_id   Describes the Controller Name.
     *
     * @retval    UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT
     */
    upll_rc_t RenameMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
        DalDmlIntf *dmi, const char *ctrlr_id);

    /* @brief        Checkes whether the key exists in DB
     *
     * @param[in]  ikey     Pointer to the ConfigKeyval containing the Key and
     *                      Value structure of Import Configuration
     * @param[in]  dt_type  Given UNC Datatype at which reference needs to check
     * @param[in]  dmi      Pointer to the DalDmlIntf(DB Interface)
     *
     * @retval  UPLL_RC_SUCCECSS     Successful Completion
     * @retval  UPLL_RC_ERR_GENERIC  For failue case GENERIC ERROR
     **/
    upll_rc_t IsReferenced(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                           DalDmlIntf *dmi);
    /**
     * @brief  Method GetFlowListKeyVal used for checking
     *         the refernce count for policngprofile object .
     *
     * @param[out]  okey        Contains the Pointer to ConfigkeyVal Class
     *                           and contains the Pfc Name.
     * @param[in]   ikey        Describes Configiration Information.
     * @param[in]   ObjType     Describes the Object type .
     *
     * @retval  UPLL_RT_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure.
     * */

    upll_rc_t GetFlowListKeyVal(ConfigKeyVal *&okey,
                          ConfigKeyVal *&ikey);
    /**
     * @brief  Method used for RenamedControllerkey(PfcName).
     *
     * @param[out] ikey      Contains the Pointer to ConfigkeyVal Class and
     *                       contains the Pfc Name.
     * @param[in] dt_type    Describes Configiration Information.
     * @param[in] dmi        Pointer to DalDmlIntf Class.
     * @param[in] ctrlr_id   Describes the Controller Name.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     */
    upll_rc_t GetRenamedControllerKey(ConfigKeyVal *ikey,
        upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
        controller_domain *ctrlr_dom = NULL);
    /**
     * @brief     Method used for TxCopyCandidateToRunning.
     * @param[in] keytype             Describes the followong keytype
     *                                undergoing the operation.
     * @param[in] ctrlr_commit_status Pointer to the CtrlrCommitStatusList Class
     * @param[in] dmi                 Pointer to DalDmlIntf Class.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     * */
    upll_rc_t TxCopyCandidateToRunning(unc_key_type_t keytype,
        CtrlrCommitStatusList *ctrlr_commit_status, DalDmlIntf *dmi,
        TcConfigMode config_mode, std::string vtn_name);

    bool CompareValidValue(void *&val1, void *val2, bool copy_to_running);

   /**
    *  @brief  Method to compare to keys
    *
    *  @param[in]  key1  Pointer to key structure for comparision
    *  @param[in]  key2  Pointer to key for comparision
    *
    *  @returncode  returns true if both the input parameters match
    *   */
    bool CompareKey(ConfigKeyVal *key1,
                    ConfigKeyVal *key2);

    /**
     * @brief     Method used for UpdateConfigStatus Operation.
     * @param[in] key           Pointer to ConfigKeyVal Class.
     * @param[in] op            Describes the Type of Opeartion.
     * @param[in] driver_result Describes the result of Driver Operation.
     * @param[in] upd_key       Pointer to ConfigKeyVal Class.
     * @param[in] ctrlr_key     Pointer to ConfigKeyVal Class.
     * @retval    RT_SUCCESS    Successfull completion.
     */
    upll_rc_t UpdateConfigStatus(ConfigKeyVal *key, unc_keytype_operation_t op,
        uint32_t driver_result, ConfigKeyVal *nreq, DalDmlIntf *dmi,
        ConfigKeyVal *ctrlr_key);
    /**
     * @brief     Method used for Read Operation.
     * @param[in] req        Describes RequestResponderHeaderClass.
     * @param[in] ikey       Pointer to ConfigKeyVal Class.
     * @param[in] dmi        Pointer to DalDmlIntf Class.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     */
    upll_rc_t ReadMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
        DalDmlIntf *dmi);

    /* @brief      Read the configuration from DB based on the operation code
     *  @param[in]     req    Pointer to IpcResResHeader
     *  @param[in/out] ikey   Pointer to the ConfigKeyVal Structure
     *  @param[in]     begin  boolean variable to decide the sibling operation
     *  @param[in]     dmi    Pointer to the DalDmlIntf(DB Interface)
     *  @return code          UPLL_RC_SUCCECSS Successful Completion
     */
    upll_rc_t ReadSiblingMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
        bool begin, DalDmlIntf *dmi);

    /**
     * @brief     Method used for Read Operation.
     *
     * @param[in] req        Describes RequestResponderHeaderClass.
     * @param[in] ikey       Pointer to ConfigKeyVal Class.
     * @param[in] dmi        Pointer to DalDmlIntf Class.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     * */
    upll_rc_t ReadRecord(IpcReqRespHeader *req, ConfigKeyVal *ikey,
        DalDmlIntf *dmi);

    /**
     * @brief  Method to Create ConfigKeyVal with rename struct as key
     *
     * @param[in]   ikey  Pointer to input ConfigKeyVal
     * @param[out]  okey  Pointer to output ConfigKeyVal.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     */
    upll_rc_t CopyToConfigKey(ConfigKeyVal *&okey,
        ConfigKeyVal *ikey);


   /** @brief Method to Update the Controller Table entry
     *
     * @param[in] ikey     Pointer to ConkeyValClass
     * @param[in] op       Operation code
     * @param[in] dmi      Pointer to DB Interface
     * @param[in] ctrl_id  Controller Name
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure
     *
   */
    upll_rc_t UpdateControllerTable(ConfigKeyVal *ikey,
                                    unc_keytype_operation_t op,
                                    upll_keytype_datatype_t dt_type,
                                    DalDmlIntf *dmi,
                                    char* ctrl_id,
                                    TcConfigMode config_mode,
                                    string vtn_name);
    void SetValidAttributesForController(val_flowlist_entry_t *val);

   /** @brief Method to Validate and Update flowlist in  the Controller Table
     *
     * @param[in] flowlist FlowListName
     * @param[in] dmi      Pointer to DB Interface
     * @param[in] ctrl_id  Controller Name
     * @param[in] op       Operation Code
     *
     * @retval  UPLL_RC_SUCCESS      Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC  Failure
     *
   */
    upll_rc_t AddFlowListToController(char *flowlist_name,
                                      DalDmlIntf *dmi,
                                      char* ctrl_id,
                                      upll_keytype_datatype_t dt_type,
                                      unc_keytype_operation_t op,
                                      TcConfigMode config_mode,
                                      string vtn_name);

    /**
     * @brief  Method to check validity of Key
     *
     * @param[in]   ConfigKeyVal  input COnfigKeyVal
     * @param[out]  index     Column Index
     *
     * @return  TRUE   Success
     * @retval  FALSE  Failure
     * */
    bool IsValidKey(void *key, uint64_t index, MoMgrTables tbl = MAINTBL);

    /**
     * @brief  Method to Set the Consolidated status
     *
     * @param[in]  ikey     Pointer to ConfigKeyVal
     * @param[in]  dmi      Pointer to DalDmlIntf.
     *
     * @retval  UPLL_RC_SUCCESS               Successfull completion.
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  No record found in DB
     * @retval  UPLL_RC_ERR_DB_ACCESS         DB access error
     **/
    upll_rc_t SetConsolidatedStatus(ConfigKeyVal *ikey,
                                    DalDmlIntf *dmi);

    /**
     * @brief  Method to get Parent ConfigKeyVal
     *
     * @param[in]   ConfigKeyVal  parent_key
     * @param[out]  ConfigKeyVal  okey
     *
     * @retval  UPLL_RC_ERR_GENERIC           Failure
     **/
  upll_rc_t GetParentConfigKey(ConfigKeyVal *&okey,
                               ConfigKeyVal *ikey);

  upll_rc_t CreateCandidateMo(IpcReqRespHeader *req,
                                       ConfigKeyVal *ikey,
                                       DalDmlIntf *dmi);

  upll_rc_t UpdateMo(IpcReqRespHeader *req,
                             ConfigKeyVal *ikey,
                             DalDmlIntf *dmi);

  upll_rc_t IsFlowListMatched(ConfigKeyVal *ikey,
                              DalDmlIntf *dmi,
                              IpcReqRespHeader *req);

  upll_rc_t SetValidAudit(ConfigKeyVal *&ikey);

  bool FilterAttributes(void *&val1,
                        void *val2,
                        bool copy_to_running,
                        unc_keytype_operation_t op);

  upll_rc_t Get_Tx_Consolidated_Status(
      unc_keytype_configstatus_t &status,
      unc_keytype_configstatus_t  drv_result_status,
      unc_keytype_configstatus_t current_cs,
      unc_keytype_configstatus_t current_ctrlr_cs);

  upll_rc_t CreateEntryCtrlrTbl(IpcReqRespHeader *req,
                                ConfigKeyVal *ikey,
                                DalDmlIntf *dmi);

  upll_rc_t SetFlowlistEntryConsolidatedStatus(ConfigKeyVal *ikey,
                                               uint8_t *ctrlr_id,
                                               DalDmlIntf *dmi);

  upll_rc_t SetRenameFlag(ConfigKeyVal *ikey,
                          DalDmlIntf *dmi,
                          IpcReqRespHeader *req);
  bool CompareValidVal(void *&val1, void *val2, void *val3,
                       bool copy_to_running);

  bool IsAllAttrInvalid(val_flowlist_entry_t *val);

  upll_rc_t GetOperation(uuc::UpdateCtrlrPhase phase,
                         unc_keytype_operation_t &op);
  // This Function assigns the controller not supported fields to set
  void  GetControllerNotSupportedAttrCol(
                                   set<string> *ctrlr_notsupported_attr_set,
                                   uint8_t valid_index);
  std::string GetQueryStringForCtrlrTable(upll_keytype_datatype_t dt_type);
  std::string SelectColumnsDynamically(char * ctrl_id,
                                upll_keytype_datatype_t dt_type,
                                set<string> *ctrlr_notsupported_attr_set);

  upll_rc_t ChkFlowlistNameInRenameTbl(ConfigKeyVal *ctrlr_key,
     upll_keytype_datatype_t dt_type, DalDmlIntf *dmi, const char *ctrlr_id);

  upll_rc_t GetDomainsForController(
      ConfigKeyVal *ckv_drvr,
      ConfigKeyVal *&ctrlr_ckv,
      DalDmlIntf *dmi);

  bool IsAttributeUpdated(void *val1, void *val2);

  upll_rc_t ClearVirtualKtDirtyInGlobal(DalDmlIntf *dmi);
};

typedef struct val_flowlist_entry_ctrl {
    uint8_t valid[22];
    unc_keytype_configstatus_t cs_row_status;
    unc_keytype_configstatus_t cs_attr[22];
    uint8_t flags;
} val_flowlist_entry_ctrl_t;
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
#endif  // UPLL_FLOWLISTENTRY_MOMGR_HH_
