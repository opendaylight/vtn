/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _UNC_UPLL_VNODE_CHILD_MOMGR_H_
#define _UNC_UPLL_VNODE_CHILD_MOMGR_H_

#include <string>
#include <set>
#include "momgr_impl.hh"


#define IS_VBRIDGE_CHILD_KEY_TYPE(keytype) ((keytype == UNC_KT_VBR_VLANMAP) || \
                                         (keytype == UNC_KT_VBR_NWMONITOR) || \
                                         (keytype == UNC_KT_VBR_NWMONITOR_HOST))

enum rename_key {
  UNC_RENAME_KEY, CTRLR_RENAME_KEY
};

enum alarm_status {
  ALARM_OPER_UP = 0,
  ALARM_OPER_DOWN,
  ALARM_NOT_SET
};

namespace unc {
namespace upll {
namespace kt_momgr {

typedef struct val_db_vbr_if_st {
        val_vbr_if_st       vbr_if_val_st;
        uint32_t             down_count;
} val_db_vbr_if_st_t;
typedef struct val_db_vbr_portmap_st {
    val_vbr_portmap_st vbr_portmap_val_st;
    uint32_t down_count;
} val_db_vbr_portmap_st_t;


typedef struct val_db_vterm_if_st {
        val_vterm_if_st       vterm_if_val_st;
        uint32_t             down_count;
} val_db_vterm_if_st_t;

typedef struct val_db_vrt_if_st {
    val_vrt_if_st vrt_if_val_st;
    uint32_t down_count;
} val_db_vrt_if_st_t;

typedef struct val_db_vtep_if_st {
    val_vtep_if_st vtep_if_val_st;
    uint32_t down_count;
} val_db_vtep_if_st_t;

typedef struct val_db_vtunnel_if_st {
    val_vtunnel_if_st vtunnel_if_val_st;
    uint32_t down_count;
} val_db_vtunnel_if_st_t;



class VnodeChildMoMgr : public MoMgrImpl {
    controller_domain cntrl_dom;
    virtual upll_rc_t SetRenameField(ConfigKeyVal *&ikey,
                                     ConfigKeyVal *parent_ck_vnode);
    upll_rc_t GetRenamedKey(ConfigKeyVal *ikey, upll_keytype_datatype_t dt_type,
                            DalDmlIntf *dmi, controller_domain *ctrlr_dom,
                            rename_key flag);

    /* @brief         Read the configuration either from RDBMS and/or from the controller
     *
     * @param[in]     req    Pointer to IpcResResHeader
     * @param[in/out] ikey   Pointer to the ConfigKeyVal Structure
     * @param[in]     dmi    Pointer to the DalDmlIntf(DB Interface)
     *
     * @retval  UPLL_RC_SUCCESS                    Completed successfully.
     * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
     * @retval  UPLL_RC_ERR_RESOURCE_DISCONNECTED  Resource disconnected.
     * @retval  UPLL_RC_ERR_DB_ACCESS              DB Read/Write error.
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE       Given key does not exist
     *
     **/
    upll_rc_t ReadMo(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                     DalDmlIntf *dmi);

    /* @brief    Populate val_vtn_neighbor for the READ/READ_SIBLING operations
     *
     * @param[in/out] key   Pointer to the ConfigKeyVal Structure
     * @param[in]     dmi    Pointer to the DalDmlIntf(DB Interface)
     *
     * @retval  UPLL_RC_SUCCESS                    Completed successfully.
     * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
     * @retval  UPLL_RC_ERR_RESOURCE_DISCONNECTED  Resource disconnected.
     * @retval  UPLL_RC_ERR_DB_ACCESS              DB Read/Write error.
     * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE       Given key does not exist
     *
     **/
    upll_rc_t PopulateValVtnNeighbor(ConfigKeyVal *&key,
                                     DalDmlIntf *dmi);

  protected:
    ConfigKeyVal *parent_ck_vnode;

   /**
    * @brief  Update parent oper status on delete for Transaction commit
    *
    * @param[in]  ikey          ConfigKeyVal instance
    * @param[in]   dmi           Database connection parameter

    * @retval  UPLL_RC_SUCCESS      Completed successfully.
    * @retval  UPLL_RC_ERR_GENERIC  Generic failure.
    */
    virtual upll_rc_t UpdateParentOperStatus(ConfigKeyVal *ikey,
                                             DalDmlIntf *dmi,
                                            uint32_t driver_result);


    virtual upll_rc_t CreateAuditMoImpl(ConfigKeyVal *ikey,
                                        DalDmlIntf *dmi,
                                        const char *ctrlr_id);
    upll_rc_t SwapKeyVal(ConfigKeyVal *ikey, ConfigKeyVal *&okey,
                         DalDmlIntf *dmi, uint8_t *ctrlr) {
      return UPLL_RC_ERR_NO_SUCH_OPERATION;
    }
    template<typename T1, typename T2>
    upll_rc_t SetOperStatus(ConfigKeyVal *ikey,
                       state_notification &notification,
                       unc_keytype_operation_t op,
                       DalDmlIntf *dmi, bool &oper_change = false);

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

  public:
    VnodeChildMoMgr() {
      parent_ck_vnode = NULL;
      memset(&cntrl_dom, 0, sizeof(controller_domain));
    }
    virtual ~VnodeChildMoMgr() {
      if (parent_ck_vnode) delete parent_ck_vnode;
      parent_ck_vnode = NULL;
    }
    upll_rc_t RestoreVnode(ConfigKeyVal *ikey, ConfigKeyVal *parent_ck_vnode,
                           IpcReqRespHeader *req, DalDmlIntf *dmi);

    upll_rc_t RenameMo(IpcReqRespHeader *req, ConfigKeyVal *key,
                       DalDmlIntf *dmi, const char *ctrlr_id) {
      return UPLL_RC_ERR_NO_SUCH_OPERATION;
    }
    upll_rc_t GetRenamedUncKey(ConfigKeyVal *ikey,
                               upll_keytype_datatype_t dt_type, DalDmlIntf *dmi,
                               uint8_t *ctrlr_id);
    upll_rc_t GetRenamedControllerKey(ConfigKeyVal *ikey,
                                      upll_keytype_datatype_t dt_type,
                                      DalDmlIntf *dmi,
                                      controller_domain *ctrlr_dom);
    virtual upll_rc_t GetControllerDomainId(ConfigKeyVal *ck_vbr,
                                            upll_keytype_datatype_t dt_type,
                                            controller_domain *ctrlr_dom,
                                            DalDmlIntf *dmi);

    /**
     * @Brief Compare if Vtep member ControllerId and VtepGrp ControllerId are same
     *        If same throw an error
     *
     * @param[in] req                       pointer IpcReqResp header
     * @param[in] ikey                      ikey contains key and value structure.
     * @param[in] dt_type                   database type
     * @param[in] dmi                       pointer to DalDmlIntf
     *
     * @retval UPLL_RC_SUCCESS                      Successful.
     * @retval UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT  Operation not allowed
     * @retval UPLL_RC_ERR_GENERIC          Generic failure.
     */
    upll_rc_t CompareContollers(ConfigKeyVal *ikey,
                                upll_keytype_datatype_t datatye,
                                DalDmlIntf *dmi);
    virtual upll_rc_t ConverttoDriverPortMap(ConfigKeyVal *ck_port_map,
                                             DalDmlIntf *dmi) {
      return UPLL_RC_ERR_GENERIC;
    }

  /* @brief         This is semantic check for KEY_VBR_IF key type
   *                in the update operation.
   *
   * @param[in/out] ikey     Pointer to the ConfigKeyVal Structure
   * @param[in]     datatype DB type.
   * @param[in]     dmi      Pointer to the DalDmlIntf(DB Interface)
   *
   * @retval  UPLL_RC_SUCCESS                    Completed successfully.
   * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
   *
   **/

    upll_rc_t IsReferenced(IpcReqRespHeader *req, ConfigKeyVal *ikey,
                           DalDmlIntf *dmi);

    virtual upll_rc_t SetVlinkPortmapConfiguration(ConfigKeyVal *ikey,
        upll_keytype_datatype_t dt_type,
        DalDmlIntf *dmi,
        InterfacePortMapInfo flag) {
      return UPLL_RC_ERR_GENERIC;
    }
    upll_rc_t UpdateOperStatus(ConfigKeyVal *ikey,
                               DalDmlIntf *dmi,
                               state_notification notification,
                               unc_keytype_operation_t op,
                               bool skip,
                               bool propagate = true);
    upll_rc_t UpdateVnodeIfOperStatus(
        ConfigKeyVal *ck_vnif,
        DalDmlIntf *dmi);

    /* VlanmapOnBoundary change */
    virtual upll_rc_t CreateCandidateMo(IpcReqRespHeader *req,
                                        ConfigKeyVal *ikey,
                                        DalDmlIntf *dmi);

    /* @brief   Gets the interface type (mapped,boundary,linked,unbound)
     *          expects the interface key/val to be populated
     *
     * @param[in]  ck_vnif   Pointer to the ConfigKeyVal Structure
     * @param[in]  valid_pm  whether portmap is set to valid
     * @param[out] vnif_type type of the interface
     *
     * @retval  UPLL_RC_SUCCESS      Completed successfully.
     * @retval  UPLL_RC_ERR_GENERIC  Generic failure.
     *
     **/
    upll_rc_t GetInterfaceType(ConfigKeyVal *ck_vnif,
                               uint8_t valid_pm,
                               if_type &vnif_type);

    template<typename T3>
upll_rc_t IsLogicalPortAndVlanIdInUse(ConfigKeyVal *ckv,
                                      DalDmlIntf *dmi,
                                      IpcReqRespHeader *req);
    upll_rc_t PortStatusHandler(ConfigKeyVal *ikey,
                                const char *ctrlr_name,
                                const char *domain_name,
                                const char *logical_port_id,
                                uint8_t oper_status,
                                uuc::UpllDbConnMgr* db_con,
                                uuc::ConfigLock* cfg_lock);
    upll_rc_t PortStatusHandler(ConfigKeyVal *ikey, uint8_t oper_status,
                                const char *logical_port_id, DalDmlIntf *dmi);
    upll_rc_t PortStatusHandler(ConfigKeyVal *ikey,
                                const char *logical_port_id,
                                uint8_t oper_status,
                                std::set<std::string> *vtn_list,
                                DalDmlIntf *dmi);
    upll_rc_t SetInterfaceOperStatus(ConfigKeyVal *&ck_vnif,
                                     DalDmlIntf *dmi,
                                     unc_keytype_operation_t op,
                                     bool propagate,
                                     uint32_t driver_result);
    /* @brief      Retrieve oper status of logical portid from physical
     *
     * @param[in]  pm            pointer to portmap structure
     * @param[in]  ctr_domain    holds pointers to controller and domain id
     * @param[in]  logical_port_operStatus operstatus of logical port
     *
     * @retval  UPLL_RC_SUCCESS      Completed successfully.
     * @retval  UPLL_RC_ERR_GENERIC  Generic failure.
     *
     **/
    virtual upll_rc_t GetPortStatusFromPhysical(val_port_map_t *pm,
        controller_domain_t ctr_domain,
        val_oper_status &logical_port_operStatus);
    upll_rc_t RecomputeVlinkAndIfoperStatus(ConfigKeyVal *ck_vlink,
                          ConfigKeyVal *ck_rem_if,
                          DalDmlIntf *dmi, state_notification notfn);
    upll_rc_t GetCtrlrStatusFromPhysical(uint8_t *ctrlr_name,
                                         val_oper_status &ctrlr_operstatus);
    upll_rc_t GetGatewayPortStatus(ConfigKeyVal *ck_vnif,
                    uint8_t *oper_status, DalDmlIntf *dmi);
    upll_rc_t GetConvertedIfStatus(ConfigKeyVal *ck_vnif,
                    uint8_t *oper_status, DalDmlIntf *dmi);
    upll_rc_t GetUninfiedvBrIf(ConfigKeyVal *&ck_vnif_tmp,
                        ConfigKeyVal *ck_vnif, DalDmlIntf *dmi);
};

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
#endif  // _UNC_UPLL_VNODE_CHILD_MOMGR_H_
