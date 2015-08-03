/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#include<gtest/gtest.h>
#include<vlanmap_momgr.hh>
#include <capa_module_stub.hh>
#include <config_mgr.hh>
#include <ctrlr_mgr.hh>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dal/dal_dml_intf.hh>
#include <dal/dal_odbc_mgr.hh>
#include <vlanmap_stub.hh>
#include "ut_util.hh"
#define INVALID_LOG_PORT_ID_VALID 0xFF

using ::testing::TestWithParam;
using ::testing::Values;
using namespace std;
using namespace unc::upll::dal;
using namespace unc::capa;
using namespace unc::upll::config_momgr;
using namespace unc::upll::test;
using namespace unc::upll::kt_momgr;
using namespace pfc::core;
using namespace unc::upll;
using namespace unc::tclib;

class VlanMapTest : public UpllTestEnv
{
};

/* ===================*GetRenameKeyBindInfo*======================== */
TEST_F(VlanMapTest, TblEqMaintbl) {
/*Checking if tbl equals maintabl and returning true */
  VlanMapMoMgr obj;
  unc_key_type_t key_type = UNC_KT_VBR_VLANMAP;
  BindInfo *binfo = NULL;
  int nattr = 0;
  MoMgrTables tbl = MAINTBL;
  EXPECT_TRUE(obj.GetRenameKeyBindInfo(key_type, binfo, nattr, tbl));
}
TEST_F(VlanMapTest, TblNotMaintbl) {
/*Checking if tbl is other than maintabl and returning true */
  VlanMapMoMgr obj;
  unc_key_type_t key_type = UNC_KT_VBR_VLANMAP;
  BindInfo *binfo = NULL;
  int nattr = 0;
  MoMgrTables tbl = CTRLRTBL;
  EXPECT_FALSE(obj.GetRenameKeyBindInfo(key_type, binfo, nattr, tbl));
}
/* =========================*GetValid*================================= */
TEST_F(VlanMapTest, valNull) {
/*Checking if configkeyval is null and returning error */
  VlanMapMoMgr obj;
  void *val = NULL;
  unsigned long indx = 0;
  uint8_t *valid = NULL;
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  MoMgrTables tbl= MAINTBL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetValid(val, indx, valid, dt_type, tbl));
}
TEST_F(VlanMapTest, DbtypeState) {
/* Checking if database type is state and returning error */
  VlanMapMoMgr obj;
  val_vlan_map *val =ZALLOC_TYPE(val_vlan_map);
  unsigned long indx = 0;
  uint8_t *valid = NULL;
  upll_keytype_datatype_t dt_type = UPLL_DT_STATE;
  MoMgrTables tbl= MAINTBL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetValid(val, indx, valid, dt_type, tbl));
}
TEST_F(VlanMapTest, indx_not_vbridge_vlanmap_Vlanid) {
/*Checking if index is not vbridge_vlanmap_Vlanid and returning error */
  VlanMapMoMgr obj;
  val_vlan_map *val =ZALLOC_TYPE(val_vlan_map);
  unsigned long indx =0;
  uint8_t *valid = NULL;
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  MoMgrTables tbl= MAINTBL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetValid(val, indx, valid, dt_type, tbl));
}
TEST_F(VlanMapTest, indx_vbridge_vlanmap_Vlanid) {
/*Checking if index is vbridge_vlanmap_Vlanid and returning success */
  VlanMapMoMgr obj;
  val_vlan_map *val =ZALLOC_TYPE(val_vlan_map);
  unsigned long indx = uudst::vbridge_vlanmap::kDbiVlanid;
  uint8_t *valid = NULL;
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  MoMgrTables tbl= MAINTBL;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetValid(val, indx, valid, dt_type, tbl));
}
/* ===========================*UpdateConfigStatus*=========================== */
TEST_F(VlanMapTest, ikeyvalNull) {
/*Checking if val is null and returning error */
  VlanMapMoMgr obj;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                                  IpctSt::kIpcStValVlanMap);
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigKeyVal *ctrlr_key= NULL;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  uint32_t driver_result = 0;
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                                  IpctSt::kIpcStKeyVlanMap);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
                    obj.UpdateConfigStatus(ikey, op, driver_result, upd_key, dmi, ctrlr_key));
}
TEST_F(VlanMapTest, ikey_val_NotNull) {
/* Checking if val is not null and returning success when op is create*/
  VlanMapMoMgr obj;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                          IpctSt::kIpcStKeyVlanMap, NULL, tmp);
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigKeyVal *ctrlr_key= NULL;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  uint32_t driver_result = 0;
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                           IpctSt::kIpcStKeyVlanMap, NULL, tmp);
  EXPECT_EQ(UPLL_RC_SUCCESS,
                      obj.UpdateConfigStatus(ikey, op, driver_result, upd_key, dmi, ctrlr_key));
}
TEST_F(VlanMapTest, driverresultZero) {
/* Check if cs_status is UNC_CS_APPLIED when driver_result is zero and
                                                          returning success  */
  VlanMapMoMgr obj;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigKeyVal *ctrlr_key= NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                           IpctSt::kIpcStKeyVlanMap, NULL, tmp);
  unc_keytype_operation_t op = UNC_OP_CREATE;
  uint32_t driver_result = UPLL_RC_SUCCESS;
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                           IpctSt::kIpcStKeyVlanMap, NULL, tmp);
  EXPECT_EQ(UPLL_RC_SUCCESS,
                      obj.UpdateConfigStatus(ikey, op, driver_result, upd_key, dmi, ctrlr_key));
}
TEST_F(VlanMapTest, driverresultNotZero) {
/* Check if cs_status is UNC_CS_NOT_APPLIED when driver result is non zero
                                                        and returning success */
  VlanMapMoMgr obj;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                           IpctSt::kIpcStKeyVlanMap, NULL, tmp);
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigKeyVal *ctrlr_key= NULL;
  unc_keytype_operation_t op = UNC_OP_CREATE;
  uint32_t driver_result = 1;
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                           IpctSt::kIpcStKeyVlanMap, NULL, tmp);
  EXPECT_EQ(UPLL_RC_SUCCESS,
                      obj.UpdateConfigStatus(ikey, op, driver_result, upd_key, dmi, ctrlr_key));
}
TEST_F(VlanMapTest, Op_Neither_Create_Nor_Update) {
/* Check if op is other than create and update and return error */
  VlanMapMoMgr obj;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                          IpctSt::kIpcStKeyVlanMap, NULL, tmp);
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigKeyVal *ctrlr_key= NULL;
  unc_keytype_operation_t op = UNC_OP_READ;
  uint32_t driver_result = 1;
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                           IpctSt::kIpcStKeyVlanMap, NULL, tmp);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
                      obj.UpdateConfigStatus(ikey, op, driver_result, upd_key, dmi, ctrlr_key));
}
TEST_F(VlanMapTest, OpCreate) {
/* Check if op is create and retrun success */
  VlanMapMoMgr obj;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigKeyVal *ctrlr_key= NULL;
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                           IpctSt::kIpcStKeyVlanMap, NULL, tmp);
  unc_keytype_operation_t op = UNC_OP_CREATE;
  uint32_t driver_result = 1;
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                           IpctSt::kIpcStKeyVlanMap, NULL, tmp);
  EXPECT_EQ(UPLL_RC_SUCCESS,
                      obj.UpdateConfigStatus(ikey, op, driver_result, upd_key, dmi, ctrlr_key));
}
TEST_F(VlanMapTest, OpCreate_valid_UNCVFVALID) {
/* Check if op is create and valid flag is valid and return success */
  VlanMapMoMgr obj;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigKeyVal *ctrlr_key= NULL;
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                           IpctSt::kIpcStKeyVlanMap, NULL, tmp);
  unc_keytype_operation_t op = UNC_OP_CREATE;
  uint32_t driver_result = 0;
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                           IpctSt::kIpcStKeyVlanMap, NULL, tmp);
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM]= UNC_VF_VALID;
  EXPECT_EQ(UPLL_RC_SUCCESS,
                      obj.UpdateConfigStatus(ikey, op, driver_result, upd_key, dmi, ctrlr_key));
}
TEST_F(VlanMapTest,
                               OpCreate_updkeyNotNull_valid_UNCVFNOTSUPPORTED) {
/* Check if op is create and valid flag is not supported and return success */
  VlanMapMoMgr obj;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigKeyVal *ctrlr_key= NULL;
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                           IpctSt::kIpcStKeyVlanMap, NULL, tmp);
  unc_keytype_operation_t op = UNC_OP_CREATE;
  uint32_t driver_result = 0;
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                           IpctSt::kIpcStKeyVlanMap, NULL, tmp);
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM]= UNC_VF_NOT_SUPPORTED;
  EXPECT_EQ(UPLL_RC_SUCCESS,
                      obj.UpdateConfigStatus(ikey, op, driver_result, upd_key, dmi, ctrlr_key));
}
#if 0   //NUll checks to be provided for test case to pass
TEST_F(VlanMapTest, OpUpdate_updkeyNull) {
/* Checking if upd_key is null (not to be null) */
  VlanMapMoMgr obj;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigKeyVal *ctrlr_key= NULL;
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                           IpctSt::kIpcStKeyVlanMap, NULL, tmp);
  unc_keytype_operation_t op = UNC_OP_UPDATE;
  uint32_t driver_result = 0;
  ConfigKeyVal *upd_key = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
                     obj.UpdateConfigStatus(ikey, op, driver_result, upd_key, dmi, ctrlr_key));
}
#endif
TEST_F(VlanMapTest, OpUpdate_valid_UNCVFVALID) {
/* Check if op is update and valid flag is valid and return success */
  VlanMapMoMgr obj;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigKeyVal *ctrlr_key= NULL;
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                           IpctSt::kIpcStKeyVlanMap, NULL, tmp);
  unc_keytype_operation_t op = UNC_OP_UPDATE;
  uint32_t driver_result = 0;
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                           IpctSt::kIpcStKeyVlanMap, NULL, tmp);
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM]= UNC_VF_VALID;
  EXPECT_EQ(UPLL_RC_SUCCESS,
                      obj.UpdateConfigStatus(ikey, op, driver_result, upd_key, dmi, ctrlr_key));
}

TEST_F(VlanMapTest,
                               OpUpdate_updkeyNotNull_valid_UNCVFNOTSUPPORTED) {
/* Check if op is update and valid flag is not supported and return success */
  VlanMapMoMgr obj;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                           IpctSt::kIpcStKeyVlanMap, NULL, tmp);
  DalDmlIntf *dmi(getDalDmlIntf());
  ConfigKeyVal *ctrlr_key= NULL;
  unc_keytype_operation_t op = UNC_OP_UPDATE;
  uint32_t driver_result = 0;
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                           IpctSt::kIpcStKeyVlanMap, NULL, tmp);
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM]= UNC_VF_NOT_SUPPORTED;
  EXPECT_EQ(UPLL_RC_SUCCESS,
                      obj.UpdateConfigStatus(ikey, op, driver_result, upd_key, dmi, ctrlr_key));
}
TEST_F(VlanMapTest, ctrlr_key) {
//Test case checking by changing default value for ctrlr key
  VlanMapMoMgr obj;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                       IpctSt::kIpcStKeyVlanMap, NULL, tmp);
  unc_keytype_operation_t op = UNC_OP_UPDATE;
  DalDmlIntf *dmi(getDalDmlIntf());
  uint32_t driver_result = 0;
  ConfigKeyVal *upd_key = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                         IpctSt::kIpcStKeyVlanMap, NULL, tmp);
  ConfigKeyVal *ctrlr_key = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                         IpctSt::kIpcStKeyVlanMap, NULL, tmp);
  EXPECT_EQ(UPLL_RC_SUCCESS,
           obj.UpdateConfigStatus(ikey, op, driver_result, upd_key, dmi, ctrlr_key));
}
/* ==========================*UpdateAuditConfigStatus*======================= */
TEST_F(VlanMapTest, val_ckvrunning_NULL) {
 /*Returning error code if configkeyval is null which implies val is null */
  VlanMapMoMgr obj;
  unc_keytype_configstatus_t cs_status = UNC_CS_INVALID;
  uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpCreate;
  ConfigKeyVal *ckv_running = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
                  obj.UpdateAuditConfigStatus(cs_status, phase, ckv_running, dmi));
}
TEST_F(VlanMapTest, val_NotNul) {
/*Returning success if configkeyval is not null */
  VlanMapMoMgr obj;
  unc_keytype_configstatus_t cs_status = UNC_CS_INVALID;
  uuc::UpdateCtrlrPhase phase =uuc::kUpllUcpInvalid;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ckv_running = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                          IpctSt::kIpcStKeyVlanMap, NULL, tmp);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_SUCCESS,
                   obj.UpdateAuditConfigStatus(cs_status, phase, ckv_running, dmi));
}
TEST_F(VlanMapTest, val_NotNULL_phaseCreate_csInvalid) {
/*Checking for phase is create and cs_status is invalid and valid flag is
                                                 valid and returning success */
  VlanMapMoMgr obj;
  unc_keytype_configstatus_t cs_status = UNC_CS_INVALID;
  uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpCreate;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM]= UNC_VF_VALID;
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ckv_running = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                           IpctSt::kIpcStKeyVlanMap, NULL, tmp);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_SUCCESS,
                    obj.UpdateAuditConfigStatus(cs_status, phase, ckv_running, dmi));
}

TEST_F(VlanMapTest,
                                         val_NotNULL_phaseCreate_csNotInvalid) {
/*Checking for phase is create and cs_status is other than invalid and valid
                                          flag is valid and returning success */
  VlanMapMoMgr obj;
  unc_keytype_configstatus_t cs_status = UNC_CS_NOT_SUPPORTED;
  uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpCreate;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM]= UNC_VF_VALID;
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ckv_running = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                          IpctSt::kIpcStKeyVlanMap, NULL, tmp);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_SUCCESS,
                   obj.UpdateAuditConfigStatus(cs_status, phase, ckv_running, dmi));
}
TEST_F(VlanMapTest,
                                         phaseCreate_csNotInvalid_valdInvalid) {
/*Checking for phase is create and cs_status is other than invalid and valid
                               flag is other than valid and returning success */
  VlanMapMoMgr obj;
  unc_keytype_configstatus_t cs_status = UNC_CS_NOT_SUPPORTED;
  uuc::UpdateCtrlrPhase phase = uuc::kUpllUcpCreate;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM]= UNC_VF_INVALID;
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ckv_running = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                          IpctSt::kIpcStKeyVlanMap, NULL, tmp);
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_EQ(UPLL_RC_SUCCESS,
                    obj.UpdateAuditConfigStatus(cs_status, phase, ckv_running, dmi));
}
/* =========================*ValidateVlanmapKey*============================= */
TEST_F(VlanMapTest, Stvlanmapkey) {
/*Returning success if vlanmapkey is valid */
  VlanMapMoMgr obj;
  unc_keytype_operation_t operation = UNC_OP_READ_SIBLING_BEGIN;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                     "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                               "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                              "PORT", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateVlanmapKey(vlanmap_key, operation));
}

TEST_F(VlanMapTest, StvlanmapkeyNovtnname) {

/*Returning cfg_syntax error when vtn_name for valnmapkey is not valid */
  VlanMapMoMgr obj;
  unc_keytype_operation_t operation = UNC_OP_READ_SIBLING_BEGIN;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                      "_VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                 "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateVlanmapKey(vlanmap_key, operation));
}
TEST_F(VlanMapTest, StvlanmapkeyNovbridgename) {

/*Returning cfg_syntax error when vbridge_name for valnmapkey is not valid */
  VlanMapMoMgr obj;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  unc_keytype_operation_t operation = UNC_OP_READ_SIBLING_BEGIN;
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                 "_VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                 "PORT", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_FALSE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateVlanmapKey(vlanmap_key, operation));
}
TEST_F(VlanMapTest, vlanmapkey_strt_char_invald_logclportid) {
/*Returning cfg_syntax error when logical_portid for valnmapkey is not valid */
  VlanMapMoMgr obj;
  unc_keytype_operation_t operation = UNC_OP_READ_SIBLING_BEGIN;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                         "", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                 "", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_FALSE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateVlanmapKey(vlanmap_key, operation));
}
/*TEST_F(VlanMapTest, StvlanmapkeyNologicalportid) {

// Returning cfg_syntax error when logical_portid for valnmapkey is not valid 
  VlanMapMoMgr obj;
  unc_keytype_operation_t operation = UNC_OP_READ_SIBLING_BEGIN;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                         "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "vbridge1", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                 "", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_FALSE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateVlanmapKey(vlanmap_key, operation));
}*/
TEST_F(VlanMapTest, StvlanmapkeyNULL) {

/*Returning cfg_syntax error when no key structure */
  VlanMapMoMgr obj;
  unc_keytype_operation_t operation = UNC_OP_READ_SIBLING_BEGIN;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateVlanmapKey(vlanmap_key, operation));
}

TEST_F(VlanMapTest, StvlanmapKeylvalidlogicalportid) {
/*Returning cfg_syntax error if logical_port_id for vlanmapkey is
                                                       not valid name */
/*checking toupper condition */
  VlanMapMoMgr obj;
  unc_keytype_operation_t operation = UNC_OP_CREATE;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRDIGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "pPort", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateVlanmapKey(vlanmap_key, operation));
}
TEST_F(VlanMapTest, Stvlanmapkey_logicalportidvalid_invalid) {
/*Returning error if vlanmapkey logical portid valid is invalid */
  VlanMapMoMgr obj;
  unc_keytype_operation_t operation = UNC_OP_CREATE;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                     "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                               "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                              "PORT", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid = 5;  //random number
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateVlanmapKey(vlanmap_key, operation));
}

/*=========================*ValidateVlanmapValue*=========================== */
TEST_F(VlanMapTest, vlanmapValNull) {
/* Retrun error generic if vlanmap val is NULL */
  VlanMapMoMgr obj;
  val_vlan_map *vlanmap_val = NULL;
  uint32_t op = UNC_OP_CREATE;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateVlanmapValue(vlanmap_val, op));
}
TEST_F(VlanMapTest, vlanmapVal_Valid) {
/* Retrun success when vlanmapvalue valid flag is UNC_VF_VALID and
                                               vlanid is in valid  range */
  VlanMapMoMgr obj;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );

  uint32_t op = UNC_OP_CREATE;
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  vlanmap_val->vlan_id = 44;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateVlanmapValue(vlanmap_val, op));
}
TEST_F(VlanMapTest, vlanmapVal_NotValidvlanid) {
/* Retrun cfg_syntax error if vlanmapvalue valid flag is UNC_VF_VALID
                                            and vlanid is not in valid range */
  VlanMapMoMgr obj;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );

  uint32_t op = UNC_OP_CREATE;
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  vlanmap_val->vlan_id = 4096;
//  vlanmap_val->vlan_id = -4096;  //passes for both negative values and zero
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateVlanmapValue(vlanmap_val, op));
}

TEST_F(VlanMapTest, vlanmapVal_validflag_novalue_opcreate) {
/* Retrun success if vlanmapvalue valid flag is UNC_VF_VALID_NO_VALUE
                                                      and op is UNC_OP_CREATE */
  VlanMapMoMgr obj;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );

  uint32_t op = UNC_OP_CREATE;
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID_NO_VALUE;
  EXPECT_EQ(UNC_UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateVlanmapValue(vlanmap_val, op));
}
TEST_F(VlanMapTest, vlanmapVal_validflag_novalue_opupdate) {
/* Retrun success if vlanmapvalue valid flag is UNC_VF_VALID_NO_VALUE
                                                     and op is UNC_OP_UPDATE */
  VlanMapMoMgr obj;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );

  uint32_t op = UNC_OP_UPDATE;
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID_NO_VALUE;
  EXPECT_EQ(UNC_UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateVlanmapValue(vlanmap_val, op));
}
TEST_F(VlanMapTest, vlanmapVal_validflag_Invalid) {
/* Retrun success if vlanmapvalue valid flag is other than
                             UNC_VF_VALID and UNC_VF_VALID_NO_VALUE */
  VlanMapMoMgr obj;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );

  uint32_t op = UNC_OP_UPDATE;
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_INVALID;
  //vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_NOT_SUPPORTED;
             //passes for not supported also
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateVlanmapValue(vlanmap_val, op));
}
TEST_F(VlanMapTest, vlanmapVal_validflag_novalue_opread) {
/* Retrun success if vlanmapvalue valid flag is UNC_VF_VALID_NO_VALUE
                             and op is other than UNC_OP_UPDATE or CREATE */
  VlanMapMoMgr obj;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );

  uint32_t op = UNC_OP_READ;
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID_NO_VALUE;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateVlanmapValue(vlanmap_val, op));
}
/* ============================*ValidateMessage*============================= */
TEST_F(VlanMapTest, IPcreqkeyNULL) {
/* Returning error generic if req is NULL */
  VlanMapMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  IpcReqRespHeader *req = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateMessage(req, ikey));
}

TEST_F(VlanMapTest, ikeystnum_invalidkeyStnum) {
/*Returning error with no such instance if key struct num  is
                                               other than kIpcStKeyVbrVlanMap */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                                          IpctSt::kIpcStKeyVbrIf, vlanmap_key);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));
}

TEST_F(VlanMapTest, ikeystnum_invalidkeytype) {
/*Returning error with no such instance if key type is other
                                                       than UNC_KT_VBR_VLANMAP*/
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                                       IpctSt::kIpcStKeyVlanMap, vlanmap_key);
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, ikeystnum_validate_vlanmapkey_cfg_err) {
/*Returning error cfg_syntax if valnmapkey is not valid*/
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key =ZALLOC_TYPE(key_vlan_map);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                         IpctSt::kIpcStKeyVlanMap, vlanmap_key);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                       "_VTN", kMaxLenVtnName);
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, opCreate_dtCandidate_cfgval_not_vlanmap) {
/*Returning error cfg_syntax if val struct is not vlanmap*/
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVbr, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                    IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_CREATE;
  req->datatype = UPLL_DT_CANDIDATE;
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, valSt_null_opCreate_dtCandidate) {
/*Returning error cfg_syntax if vlammap val struct is NULL*/
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, NULL);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                   IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_CREATE;
  req->datatype = UPLL_DT_CANDIDATE;
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, valSt_NotValid_opCreate_dtCandidate) {
/*Returning error no such instance if val structure is other than vlanmap */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );

  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                                         IpctSt::kIpcStKeyVlanMap, vlanmap_key);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, validatevlanmapvalue_error) {
/*Returning error cfg syntax if vlammapvalue is not valid */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                     IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_CREATE;
  req->datatype = UPLL_DT_CANDIDATE;
  vlanmap_val->vlan_id = 4096 ;//out of range
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;  /*validate
                                                                vlanmap value */
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, valSt_Notnull_op_create_dt_candidate) {
/*Returning success if vlammapvalue is valid for op create and dt candidate*/
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                     IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_CREATE;
  req->datatype = UPLL_DT_CANDIDATE;
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID_NO_VALUE;  /*validate
                                                                vlanmap value */
  EXPECT_EQ(UNC_UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, valSt_Notnull_op_update_dt_candidate) {
/*Returning success if vlammapvalue is valid for op update and dt candidate*/
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                     IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_UPDATE;
  req->datatype = UPLL_DT_CANDIDATE;
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID_NO_VALUE;  /* validate
                                                                vlanmap value */
  EXPECT_EQ(UNC_UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, valSt_Notnull_op_update_dt_others) {
/*Returning error no such instance if vlammapvalue is valid for op
                                 update and dt other than candidate*/
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                    IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_UPDATE;
  req->datatype = UPLL_DT_RUNNING;
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;  /* validate
                                                                vlanmap value */
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, Op_read_Dt_Candidate_Opt1_invalid) {
/*Returning error invalid option1 if req->option1 if other than NORMAL
                                                  for op Read and Dt candidate*/
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                        IpctSt::kIpcStKeyVlanMap, vlanmap_key);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_DETAIL;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, Op_read_Dt_Candidate_Opt2_invalid) {
/*Returning error invalid option2 if vlammapvalue is option2 is other
                                       than None for op Read and Dt candidate */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                        IpctSt::kIpcStKeyVlanMap, vlanmap_key);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_IP_ROUTE;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, obj.ValidateMessage(req, ikey));
}

TEST_F(VlanMapTest, Op_read_Dt_Candidate_Opt1_Opt2_valid) {
/*Returning Success if option1 and option2 are valid  for op Read and
                                                              Dt candidate */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map);
  
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                              IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, Op_read_candidate_validOpts_invalid_valSt) {
/* Returning error if option1 and option2 are valid but val struct is invalid
                                             for op Read and  Dt candidate */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVbr, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                  IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  EXPECT_EQ(UPLL_RC_ERR_BAD_REQUEST, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, Op_read_candidate_validOpts_Val_Null) {
/* Returning success if option1 and option2 are valid but val is null
                                             for op Read and  Dt candidate */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );

  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, NULL);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                  IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, Op_read_Dt_candidate_validateval_error) {
/* Returning error if option1 and option2 are valid but validate  val error
                                             for op Read and  Dt candidate */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                  IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  vlanmap_val->vlan_id = 4096 ;//out of range
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;  /*validate
                                                                vlanmap value */
  req->operation = UNC_OP_READ;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, OpRead_Dt_Invalid) {
/* Returning error if option1 and option2 are valid
                                        for op read and  Dt invalid */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                  IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ;
  req->datatype = UPLL_DT_INVALID;
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, Op_ReadNext_DtInvalid) {
/* Returning error if option1 and option2 are valid
                                        for op read_Next and  Dt invalid */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                  IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ_NEXT;
  req->datatype = UPLL_DT_INVALID;
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, Op_ReadSibling_Dt_Candidate_Opt1_invalid) {
/*Returning error invalid option1 if req->option1 if other than NORMAL for
                                             op ReadSibling and Dt candidate */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                        IpctSt::kIpcStKeyVlanMap, vlanmap_key);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ_SIBLING;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_DETAIL;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, Op_ReadSibling_Dt_Candidate_Opt2_invalid) {
/*Returning error invalid option2 if option2 is invalid for op ReadSibling
                                                            and Dt candidate */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                        IpctSt::kIpcStKeyVlanMap, vlanmap_key);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ_SIBLING;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_IP_ROUTE;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, obj.ValidateMessage(req, ikey));
}

TEST_F(VlanMapTest, Op_ReadSibling_Dt_Candidate_Opt1_Opt2_valid) {
/*Returning Success if option1 and option2 are valid for op ReadSibling
                                                             and Dt candidate */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                     IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ_SIBLING;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest,
                               Op_ReadSiblingBegin_Dt_Candidate_Opt1_invalid) {
/*Returning error invalid option1 if req->option1 if other than NORMAL for
                                         op ReadSiblingBegin and Dt candidate */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                  IpctSt::kIpcStKeyVlanMap, vlanmap_key, NULL);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_DETAIL;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest,
                                Op_ReadSiblingBegin_Dt_Candidate_Opt2_invalid) {
/*Returning error invalid option2 if option2 is invalid for op ReadSiblingBegin
                                                             and Dt candidate */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                        IpctSt::kIpcStKeyVlanMap, vlanmap_key);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_IP_ROUTE;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, obj.ValidateMessage(req, ikey));
}

TEST_F(VlanMapTest,
                             Op_ReadSiblingBegin_Dt_Candidate_Opt1_Opt2_valid) {
/*Returning Success if option1 and option2 are valid for op ReadSiblingBegin
                                                             and Dt candidate */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                    IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest,
                              Op_ReadSiblingCount_Dt_Candidate_Opt1_invalid) {
/*Returning error invalid option1 if req->option1 if other than NORMAL
                                    for op ReadSiblingCount and Dt candidate */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                        IpctSt::kIpcStKeyVlanMap, vlanmap_key);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_DETAIL;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest,
                                Op_ReadSiblingCount_Dt_Candidate_Opt2_invalid) {
/*Returning error invalid option2 if option2 is invalid for op
                                            ReadSiblingCount and Dt candidate */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                        IpctSt::kIpcStKeyVlanMap, vlanmap_key);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_IP_ROUTE;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest,
                           Op_ReadSiblingCount_Dt_Candidate_Opt1_Opt2_valid) {
/*Returning Success if option1 and option2 are valid for op ReadSiblingCount
                                                           and Dt candidate */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                   IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->datatype = UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, Op_ReadSiblingCount_Dt_Running_Opt1_invalid) {
/*Returning error invalid option1 if req->option1 if other than NORMAL for
                                           op ReadSiblingCount and Dt running */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                        IpctSt::kIpcStKeyVlanMap, vlanmap_key);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->datatype = UPLL_DT_RUNNING;
  req->option1 = UNC_OPT1_DETAIL;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, Op_ReadSiblingCount_Dt_Running_Opt2_invalid) {
/*Returning error invalid option2 if option2 is invalid for op
                                              ReadSiblingCount and Dt Running */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                        IpctSt::kIpcStKeyVlanMap, vlanmap_key);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->datatype = UPLL_DT_RUNNING;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_IP_ROUTE;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, obj.ValidateMessage(req, ikey));
}

TEST_F(VlanMapTest,
                               Op_ReadSiblingCount_Dt_Running_Opt1_Opt2_valid) {
/*Returning Success if option1 and option2 are valid for op ReadSiblingCount
                                                               and Dt Running */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                   IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->datatype = UPLL_DT_RUNNING;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, Op_ReadSiblingCount_Dt_StartUp_Opt1_invalid) {
/*Returning error invalid option1 if req->option1 if other than NORMAL for
                                         op ReadSiblingCount and Dt startup */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                        IpctSt::kIpcStKeyVlanMap, vlanmap_key);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->datatype = UPLL_DT_STARTUP;
  req->option1 = UNC_OPT1_DETAIL;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, Op_ReadSiblingCount_Dt_StartUp_Opt2_invalid) {
/*Returning error invalid option2 if option2 is invalid for op
                                              ReadSiblingCount and Dt Startup */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                        IpctSt::kIpcStKeyVlanMap, vlanmap_key);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->datatype = UPLL_DT_STARTUP;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_IP_ROUTE;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest,
                               Op_ReadSiblingCount_Dt_Startup_Opt1_Opt2_valid) {
/*Returning Success if option1 and option2 are valid for op ReadSiblingCount
                                                             and Dt StartUP */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                   IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->datatype = UPLL_DT_STARTUP;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, Op_ReadSiblingCount_Dt_State_Opt1_invalid) {
/*Returning error invalid option1 if req->option1 if other than NORMAL
                                        for op ReadSiblingCount and Dt state */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                        IpctSt::kIpcStKeyVlanMap, vlanmap_key);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->datatype = UPLL_DT_STATE;
  req->option1 = UNC_OPT1_DETAIL;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION1, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, Op_ReadSiblingCount_Dt_State_Opt2_invalid) {
/*Returning error invalid option2 if option2 is invalid for op
                                              ReadSiblingCount and Dt State */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                         IpctSt::kIpcStKeyVlanMap, vlanmap_key);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->datatype = UPLL_DT_STATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_IP_ROUTE;
  EXPECT_EQ(UPLL_RC_ERR_INVALID_OPTION2, obj.ValidateMessage(req, ikey));
}

TEST_F(VlanMapTest,
                                 Op_ReadSiblingCount_Dt_State_Opt1_Opt2_valid) {
/*Returning Success if option1 and option2 are valid for op ReadSiblingCount
                                                                 and Dt State */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                   IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->datatype = UPLL_DT_STATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, Op_ReadNext_Dt_StartUp) {
/*Returning Success if op is Readnext and Dt Startup */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ_NEXT;
  req->datatype = UPLL_DT_STARTUP;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));
}

TEST_F(VlanMapTest, Op_ReadBulk_Dt_StartUp) {
/*Returning Success if op is ReadBulk and Dt Startup */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                   IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ_BULK;
  req->datatype = UPLL_DT_STARTUP;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, Op_ReadNext_Dt_Candidate) {
/*Returning Success if op is ReadNext and Dt candidate */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                   IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ_NEXT;
  req->datatype = UPLL_DT_CANDIDATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, Op_ReadBulk_Dt_Candidate) {
/*Returning Success if op is ReadBulk and Dt candidate */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ_BULK;
  req->datatype = UPLL_DT_CANDIDATE;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, Op_ReadNext_Dt_Running) {
/*Returning Success if op is ReadNext and Dt running */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                     IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ_NEXT;
  req->datatype = UPLL_DT_RUNNING;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, Op_ReadBulk_Dt_Running) {
/*Returning Success if op is ReadBulk and Dt running */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                    IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_READ_BULK;
  req->datatype = UPLL_DT_RUNNING;
 EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateMessage(req, ikey));
}
TEST_F(VlanMapTest, Op_notValid_Dt_invalid) {
/*Returning error no such instance if op is not valid and Dt is not valid */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                    IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  req->operation = UNC_OP_CONTROL;
  req->datatype = UPLL_DT_RUNNING;
  EXPECT_EQ(UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT, obj.ValidateMessage(req, ikey));
}
/* =============================*AllocVal*================================= */
TEST_F(VlanMapTest, configval_NotNull) {
/*Returning error generic if configval is not null */
  VlanMapMoMgr obj;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  ConfigVal *ck_val = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  MoMgrTables tbl = MAINTBL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.AllocVal(ck_val, dt_type, tbl));
}
TEST_F(VlanMapTest, configval_Null_alloc_val_tbl_Maintbl) {
/*Returning success after allocating val if configval is null  */
  VlanMapMoMgr obj;
  ConfigVal *ck_val = NULL;
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  MoMgrTables tbl = MAINTBL;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.AllocVal(ck_val, dt_type, tbl));
  EXPECT_EQ(IpctSt::kIpcStPfcdrvValVlanMap, ck_val->get_st_num()); //st_num 39 =>kIpcStValVlanMap
}

TEST_F(VlanMapTest, configval_Null_alloc_val_tbl_NotMaintbl) {
/* Returning success allocating val to zero if table is not maintable  */
  VlanMapMoMgr obj;
  ConfigVal *ck_val = NULL;
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  MoMgrTables tbl = RENAMETBL;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.AllocVal(ck_val, dt_type, tbl));
}

/* ==========================*IsValidKey*=================================== */
TEST_F(VlanMapTest, Index_Valid_Vtnname) {
/*Returning true if key->vtnname is valid */
  VlanMapMoMgr obj;
  uint64_t index = uudst::vbridge_vlanmap::kDbiVtnName;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  EXPECT_TRUE(obj.IsValidKey(vlanmap_key, index));
}
TEST_F(VlanMapTest, Index_Invalid_Vtnname) {
/*Returning false if key->vtnname is not valid  */
  VlanMapMoMgr obj;
  uint64_t index = uudst::vbridge_vlanmap::kDbiVtnName;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                       "_VTN", kMaxLenVtnName);
  EXPECT_FALSE(obj.IsValidKey(vlanmap_key, index));
}

TEST_F(VlanMapTest, Index_Valid_Vbridgename) {
/*Returning true if key->vbridgename is valid  */
  VlanMapMoMgr obj;
  uint64_t index = uudst::vbridge_vlanmap::kDbiVbrName;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                   "VBRIDGE", kMaxLenVnodeName);
  EXPECT_TRUE(obj.IsValidKey(vlanmap_key, index));
}
TEST_F(VlanMapTest, Index_Invalid_Vbridgename) {
/*Returning false if key->vbridgename is not valid */
  VlanMapMoMgr obj;
  uint64_t index =  uudst::vbridge_vlanmap::kDbiVbrName;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                 "_VBRIDGE", kMaxLenVnodeName);
  EXPECT_FALSE(obj.IsValidKey(vlanmap_key, index));
}
TEST_F(VlanMapTest, Index_Valid_LogicalPortID) {
/*Returning true if  key->logicalportid is valid*/
  VlanMapMoMgr obj;
  uint64_t index =  uudst::vbridge_vlanmap::kDbiLogicalPortId;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                 "PORT", kMaxLenLogicalPortId);
// vlanmap_key->logical_port_id_valid =PFC_TRUE;   //validate vlanmap key
  EXPECT_TRUE(obj.IsValidKey(vlanmap_key, index));
}
TEST_F(VlanMapTest, Index_Invalid_LogicalPortId) {
/*Returning false if key->logicalportid is not valid */
  VlanMapMoMgr obj;
  uint64_t index =   uudst::vbridge_vlanmap::kDbiLogicalPortId;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                 "_PORT", kMaxLenLogicalPortId);
  EXPECT_TRUE(obj.IsValidKey(vlanmap_key, index));
}
TEST_F(VlanMapTest, Index_Valid_LogicalPortIdValid) {
/*Returning true if  key->logicalportidvalid is valid*/
  VlanMapMoMgr obj;
  uint64_t index =  uudst::vbridge_vlanmap::kDbiLogicalPortIdValid;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  vlanmap_key->logical_port_id_valid = PFC_TRUE;
  EXPECT_TRUE(obj.IsValidKey(vlanmap_key, index));
}
TEST_F(VlanMapTest, Index_Invalid_LogicalPortIdValid) {
/*Returning false if key->logicalportidvalid is not valid */
  VlanMapMoMgr obj;
  uint64_t index =   uudst::vbridge_vlanmap::kDbiLogicalPortIdValid;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  vlanmap_key->logical_port_id_valid =5;   //random number
  EXPECT_FALSE(obj.IsValidKey(vlanmap_key, index));
}

TEST_F(VlanMapTest, Index_NotValid) {
/*Returning true if index is not valid */
  VlanMapMoMgr obj;
  uint64_t index = 10; //random number
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  EXPECT_TRUE(obj.IsValidKey(vlanmap_key, index));
}
/*========================*GetChildConfigKey*================================ */

TEST_F(VlanMapTest, Parentkey_Null) {
/*Returning success after allocationg new configkeyval if ckv
                                                      parentkey is null  */
  VlanMapMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *parent_key = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetChildConfigKey(okey, parent_key));
  key_vlan_map *output_vlmkey =reinterpret_cast<key_vlan_map *>
                                                            (okey->get_key());
/* check  has all set to 0 (memset to 0 being done in code) */
  EXPECT_STREQ("", reinterpret_cast<char *>(output_vlmkey->
                                                    vbr_key.vtn_key.vtn_name));
  EXPECT_STREQ("", reinterpret_cast<char *>(output_vlmkey->
                                                    vbr_key.vbridge_name));
  EXPECT_STREQ("", reinterpret_cast<char *>(output_vlmkey->logical_port_id));
  EXPECT_EQ(0xFF, output_vlmkey->logical_port_id_valid);
}
TEST_F(VlanMapTest, okey_NULL_parentkey_keyStructNULL) {
/*Return error generic if parentkey is not null but key struct is null */
  VlanMapMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *parent_key = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                              IpctSt::kIpcStKeyVlanMap);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetChildConfigKey(okey, parent_key));
}
TEST_F(VlanMapTest, okey_NULL_parentkey_keystructNotNULL) {
/* Return success if parentkey key struct is not null */
  VlanMapMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigKeyVal *parent_key = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                         IpctSt::kIpcStKeyVlanMap, vlanmap_key);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetChildConfigKey(okey, parent_key));
}
TEST_F(VlanMapTest, okey_NULL_pkey_keystvalid_kype_NotVlanmap) {
/* Return error generic if okey and parentkey key struct are not null and
                                                  okey keytype is not vlanmap */
  VlanMapMoMgr obj;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  key_vbr_if *vbr_key = ZALLOC_TYPE(key_vbr_if );
  ConfigKeyVal *parent_key = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                         IpctSt::kIpcStKeyVlanMap, vlanmap_key);
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBR_IF,
                                               IpctSt::kIpcStKeyVbrIf, vbr_key);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetChildConfigKey(okey, parent_key));
}
TEST_F(VlanMapTest, okey_NULL_pkey_keystvalid_kype_Vlanmap) {
/* Return success if okey and parentkey key struct are not null and
                                                  okey keytype is vlanmap */
  VlanMapMoMgr obj;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigKeyVal *parent_key = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                         IpctSt::kIpcStKeyVlanMap, vlanmap_key);
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                        IpctSt::kIpcStKeyVlanMap, vlanmap_key);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetChildConfigKey(okey, parent_key));
}

TEST_F(VlanMapTest,
                okey_NULL_pkey_keystValid_pkey_ktype_vlanmap_portidvalid_true) {
/* Return success if okey is null and parent key is valid struct with key type
                                          vlanmap and logicalportidvalid true */
  VlanMapMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigKeyVal *parent_key = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                         IpctSt::kIpcStKeyVlanMap, vlanmap_key);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                         "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                   "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                   "PORT", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid = PFC_TRUE;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetChildConfigKey(okey, parent_key));
  key_vlan_map *output_vlmkey = reinterpret_cast<key_vlan_map *>
                                                              (okey->get_key());
  EXPECT_STREQ("VTN", reinterpret_cast<char *>(output_vlmkey->
                                                     vbr_key.vtn_key.vtn_name));
  EXPECT_STREQ("VBRIDGE", reinterpret_cast<char *>(output_vlmkey->
                                                         vbr_key.vbridge_name));
  EXPECT_STREQ("PORT",
                      reinterpret_cast<char *>(output_vlmkey->logical_port_id));
  EXPECT_TRUE(output_vlmkey->logical_port_id_valid);
}
TEST_F(VlanMapTest,
                 okey_NULL_pkey_keystValid_pkey_ktype_vlanmap_portvalid_false) {
/* Return success if okey is null and parent key is valid struct with key type
                                         vlanmap and logicalportidvalid false */
  VlanMapMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigKeyVal *parent_key = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                         IpctSt::kIpcStKeyVlanMap, vlanmap_key);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                         "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                   "VBRIDGE", kMaxLenVnodeName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                                  "PORT", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid = PFC_FALSE;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetChildConfigKey(okey, parent_key));
  key_vlan_map *output_vlmkey = reinterpret_cast<key_vlan_map *>
                                                            (okey->get_key());
  EXPECT_STREQ("VTN", reinterpret_cast<char *>(output_vlmkey->
                                                   vbr_key.vtn_key.vtn_name));
  EXPECT_STREQ("VBRIDGE", reinterpret_cast<char *>(output_vlmkey->
                                                        vbr_key.vbridge_name));
/* port is set to '/0' when portidvalid is false */
  EXPECT_STREQ("", reinterpret_cast<char *>(output_vlmkey->logical_port_id));
  EXPECT_FALSE(output_vlmkey->logical_port_id_valid);
}

TEST_F(VlanMapTest,
                                okey_NULL_pkey_keystValid_pkey_ktype_vbridge) {
/* Return success if okey is null and parent key is valid struct
                                                      with key type vbridge*/
  VlanMapMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vbr_t *key_vbr = ZALLOC_TYPE(key_vbr_t );
  ConfigKeyVal *parent_key = new ConfigKeyVal(UNC_KT_VBRIDGE,
                                                IpctSt::kIpcStKeyVbr, key_vbr);
  strncpy(reinterpret_cast<char *>(key_vbr->vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(key_vbr->vbridge_name),
                                                  "VBRIDGE", kMaxLenVnodeName);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetChildConfigKey(okey, parent_key));
  key_vlan_map *output_vlmkey =(key_vlan_map *)(okey->get_key());
  EXPECT_STREQ("VTN", reinterpret_cast<char *>(output_vlmkey->
                                                     vbr_key.vtn_key.vtn_name));
  EXPECT_STREQ("VBRIDGE", reinterpret_cast<char *>(output_vlmkey->
                                                         vbr_key.vbridge_name));
}
TEST_F(VlanMapTest,
                                okey_NULL_pkey_keystValid_pkey_ktype_vtn) {
/* Return success if okey is null and parent key is valid struct
                                                      with key type vtn*/
  VlanMapMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vtn_t *key_vtn = ZALLOC_TYPE(key_vtn_t );
  ConfigKeyVal *parent_key = new ConfigKeyVal(UNC_KT_VTN,
                                                IpctSt::kIpcStKeyVtn, key_vtn);
  strncpy(reinterpret_cast<char *>(key_vtn->vtn_name),
                                                        "VTN", kMaxLenVtnName);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetChildConfigKey(okey, parent_key));
  key_vlan_map *output_vlmkey =(key_vlan_map *)(okey->get_key());
  EXPECT_STREQ("VTN", reinterpret_cast<char *>(output_vlmkey->
                                                     vbr_key.vtn_key.vtn_name));
}
/* =========================*GetParentConfigKey*============================= */

TEST_F(VlanMapTest, ikey_NULL) {
/* Return error generic if ikey is null */
  VlanMapMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetParentConfigKey(okey, ikey));
}

TEST_F(VlanMapTest, ikey_NotNULL_keyst_Notvbrvlanmap) {
/* Return error generic if ikey_type is not vbrvlanmap when
                                                        child key is not null */
  VlanMapMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_IF,
                                        IpctSt::kIpcStKeyVbrIf);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetParentConfigKey(okey, ikey));
}
TEST_F(VlanMapTest, ikey_NotNULL_keyst_vbrvlanmap_pkey_null) {
/* Return error generic if pkey is null and ikey_type is vbrvlanmap when
                                                        child key is not null */
  VlanMapMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                        IpctSt::kIpcStKeyVlanMap);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.GetParentConfigKey(okey, ikey));
}

TEST_F(VlanMapTest, ikey_NotNULL_keyst_vbrvlanmap) {
/* Return success if pkey is not null and ikey_type is vbrvlanmap when
                                                        child key is not null */
  VlanMapMoMgr obj;
  key_vlan_map *vlanmap_key =ZALLOC_TYPE(key_vlan_map );
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                     IpctSt::kIpcStKeyVlanMap, vlanmap_key);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                        "VTN", kMaxLenVtnName);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetParentConfigKey(okey, ikey));
/* check if child key struct is assigned to pkey when key type is vbr_vlanmap */
  key_vlan_map_t *pkey = reinterpret_cast<key_vlan_map *>(ikey->get_key());
  EXPECT_STREQ("VTN", reinterpret_cast<char *>(pkey->vbr_key.vtn_key.vtn_name));
}
TEST_F(VlanMapTest, ikey_NotNULL_vbrkey_valid) {
/* Return success after getting parent config key */
  VlanMapMoMgr obj;
  key_vlan_map *vlanmap_key =ZALLOC_TYPE(key_vlan_map );
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                   IpctSt::kIpcStKeyVlanMap, vlanmap_key, NULL);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                         "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vbridge_name),
                                                   "VBRIDGE", kMaxLenVnodeName);
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.GetParentConfigKey(okey, ikey));
/*check if vlanmap key struct is copied to vbr key struct */
  key_vbr *vbr_key = reinterpret_cast<key_vbr *>(ikey->get_key());
  EXPECT_STREQ("VTN", reinterpret_cast<char *>(vbr_key->vtn_key.vtn_name));
  EXPECT_STREQ("VBRIDGE", reinterpret_cast<char *>(vbr_key->vbridge_name));
  EXPECT_EQ(16, okey->get_key_type());  //parent key is 18-UNC_KT_VBRIDGE
}
/*=========================*DupConfigKeyVal*==================================*/
TEST_F(VlanMapTest, req_Null) {
/* Return error generic when input configkeyval(req) to be duplicated is null */
  VlanMapMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *req = NULL;
  MoMgrTables tbl = MAINTBL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DupConfigKeyVal(okey, req, tbl));
}
TEST_F(VlanMapTest, OkeyNotNull) {
/* Return error generic when output configkeyval(okey) is not null */
  VlanMapMoMgr obj;
  ConfigKeyVal *okey = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                             IpctSt::kIpcStKeyVlanMap);
  MoMgrTables tbl = MAINTBL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DupConfigKeyVal(okey, req, tbl));
}
TEST_F(VlanMapTest, req_invalid_keytype) {
/* Return error generic when input configkeyval(req) is invalid keytype */
  VlanMapMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcStKeyVbr);
  MoMgrTables tbl = MAINTBL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DupConfigKeyVal(okey, req, tbl));
}
TEST_F(VlanMapTest, val_struct_Null) {
/* Return error generic when req val struct is Null */
  VlanMapMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVlanMap, NULL);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                   IpctSt::kIpcStKeyVlanMap, NULL, cfg_val);
  MoMgrTables tbl = MAINTBL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.DupConfigKeyVal(okey, req, tbl));
}

TEST_F(VlanMapTest,
                           req_valid_valNotNull_TblMaintblSuccess_valcheck) {
/* Return success after duplicating configkey */
  VlanMapMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                            IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  vlanmap_val->vlan_id = 0;
  MoMgrTables tbl = MAINTBL;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.DupConfigKeyVal(okey, req, tbl));
/* check if val struct of  okey is been duplicated to req */
  val_vlan_map *output_vlmval = ZALLOC_TYPE(val_vlan_map );
  EXPECT_EQ(0, output_vlmval->vlan_id);
}
TEST_F(VlanMapTest,
                              req_valid_valNotNull_TblMaintblSuccess_keycheck) {
/* Return success after duplicating configkey */
  VlanMapMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                            IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                  "VTN", kMaxLenVtnName);
  MoMgrTables tbl = MAINTBL;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.DupConfigKeyVal(okey, req, tbl));
/* check if key struct of okey is been duplicated to req */
  key_vlan_map *output_vlmkey = reinterpret_cast<key_vlan_map *>
                                                         (okey->get_key());
  EXPECT_STREQ("VTN", reinterpret_cast<char *>(output_vlmkey->
                                                     vbr_key.vtn_key.vtn_name));
}
TEST_F(VlanMapTest, req_valid_valNotNull_TblRenametblSuccess){
/* Return success after duplicating configkey when table is other than main
                                                                        table */
  VlanMapMoMgr obj;
  ConfigKeyVal *okey = NULL;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *req = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                            IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  MoMgrTables tbl = RENAMETBL;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.DupConfigKeyVal(okey, req, tbl));
}
/*==========================*CopyToConfigKey*================================ */
TEST_F(VlanMapTest, ikeyNull ){
/* Returning error generic when ikey is null */
  VlanMapMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CopyToConfigKey(okey, ikey));
}
TEST_F(VlanMapTest, ikey_getkeyNull ){
/* Returning error generic when ikey->get_key() is null */
  VlanMapMoMgr obj;
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                        IpctSt::kIpcStKeyVlanMap);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CopyToConfigKey(okey, ikey));
}
TEST_F(VlanMapTest, ikeyNotNull_Novtnname) {
/* Return error when there is no vtn name */
  VlanMapMoMgr obj;
  key_rename_vnode_info_t *key_rename = ZALLOC_TYPE(key_rename_vnode_info_t);
  memcpy(&(key_rename->old_unc_vtn_name), "", kMaxLenVtnName);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                      IpctSt::kIpcStKeyVlanMap, key_rename);
  ConfigKeyVal *okey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CopyToConfigKey(okey, ikey));
}


/*TEST_F(VlanMapTest, ikeyNotNull_copyvtnname) {
 Return success after copytoconfig key is done
  VlanMapMoMgr obj;
  key_rename_vnode_info_t *key_rename = ZALLOC_TYPE(key_rename_vnode_info_t );
  memcpy(&(key_rename->old_unc_vtn_name), "VTN", kMaxLenVtnName);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                       IpctSt::kIpcStKeyVlanMap, key_rename);
  ConfigKeyVal *okey = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.CopyToConfigKey(okey, ikey));
  check if config key is copied to okey 
  key_vlan_map *output_vlmkey = reinterpret_cast<key_vlan_map *>
                                                           (okey->get_key());
  EXPECT_STREQ("VTN", reinterpret_cast<char *>(output_vlmkey->
                                                  vbr_key.vtn_key.vtn_name));
}*/


TEST_F(VlanMapTest, ikeyNotNull_no_vnodename) {
// Return error when vnodename is not present
  VlanMapMoMgr obj;
  key_rename_vnode_info_t *key_rename = ZALLOC_TYPE(key_rename_vnode_info_t );

  memcpy(&(key_rename->old_unc_vnode_name), "", kMaxLenVnodeName);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                                           IpctSt::kIpcStKeyVbr, key_rename);
  ConfigKeyVal *okey = NULL;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.CopyToConfigKey(okey, ikey));
}

#if 0
TEST_F(VlanMapTest, ikeyNotNull_copyvnodename) {
 Return success after copytoconfig key is done 
  VlanMapMoMgr obj;
  key_rename_vnode_info_t *key_rename = ZALLOC_TYPE(key_rename_vnode_info_t );

//  memcpy(&(key_rename->old_unc_vtn_name), "VTN", kMaxLenVtnName);
  memcpy(&(key_rename->old_unc_vnode_name), "VBRIDGE", kMaxLenVnodeName);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE,
                                           IpctSt::kIpcStKeyVbr, key_rename);
  ConfigKeyVal *okey = NULL;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.CopyToConfigKey(okey, ikey));
 check if config key is copied to okey vnode name
  key_vlan_map *output_vlmkey = reinterpret_cast<key_vlan_map *>
                                                           (okey->get_key());
  EXPECT_STREQ("VBRIDGE", reinterpret_cast<char *>(output_vlmkey->
                                                      vbr_key.vbridge_name));
//  EXPECT_STREQ("", reinterpret_cast<char *>(output_vlmkey->
  //                                               vbr_key.vtn_key.vtn_name));
}
#endif

TEST_F(VlanMapTest, Keytype_Notvalid) {
// Return Null when keytype is not valid
  VlanMapMoMgr obj;
  ConfigKeyVal *ck_vbr = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                    IpctSt::kIpcStKeyVlanMap);
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  DalDmlIntf *dmi(getDalDmlIntf());
  EXPECT_STREQ(NULL, reinterpret_cast<char *>
                                    (obj.GetControllerId(ck_vbr, dt_type, dmi)));
}
/*
TEST_F(VlanMapTest, Keytype_valid_ReadDb_error) {
// Return Null when keytype is valid but ReadConfigDb returns error
  VlanmapStub obj;
  ConfigKeyVal *ck_vbr = new ConfigKeyVal(UNC_KT_VBRIDGE,
                                    IpctSt::kIpcStKeyVbr);
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  DalDmlIntf *dmi(getDalDmlIntf());
  VlanmapStub::stub_result[ReadConfigDB1] = UPLL_RC_ERR_GENERIC;
  EXPECT_STREQ(NULL, reinterpret_cast<char *>
                                    (obj.GetControllerId(ck_vbr, dt_type, dmi)));
}

TEST_F(VlanMapTest, Keytype_valid_ReadDb_Success_valNull) {
// Return Null when keytype is valid, ReadConfigDb returns Success
// but val is Null
  VlanmapStub obj;
  ConfigKeyVal *ck_vbr = new ConfigKeyVal(UNC_KT_VBRIDGE,
                                    IpctSt::kIpcStKeyVbr);
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  DalDmlIntf *dmi(getDalDmlIntf());
  VlanmapStub::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;
  EXPECT_STREQ(NULL, reinterpret_cast<char *>
                                    (obj.GetControllerId(ck_vbr, dt_type, dmi)));
}
TEST_F(VlanMapTest, Keytype_valid_ReadDb_Success_valNotNull) {
// Return controllerid when keytype is valid, ReadconfigDb returns success
                                                      // when val is ot null 
  VlanmapStub obj;
  val_vbr *vbr_val = ZALLOC_TYPE(val_vbr );
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVbr, vbr_val);
  ConfigKeyVal *ck_vbr = new ConfigKeyVal(UNC_KT_VBRIDGE,
                                    IpctSt::kIpcStKeyVbr, NULL, cfg_val);
  strncpy(reinterpret_cast<char *>(vbr_val->controller_id), "CONTROLLER",
                                                          kMaxLenCtrlrId);
  upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  DalDmlIntf *dmi(getDalDmlIntf());  
  VlanmapStub::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;
  EXPECT_STREQ(NULL, reinterpret_cast<char *>
                                    (obj.GetControllerId(ck_vbr, dt_type, dmi)));
}
*/
/*====================*ValidateAttribute*=================================== */
TEST_F(VlanMapTest, iKey_NULL) {
/* Return error when ikey is null */
  VlanMapMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  DalDmlIntf *dmi(getDalDmlIntf());  
  IPC_REQ_RESP_HEADER_DECL(req);
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateAttribute(ikey, dmi, req));
}
TEST_F(VlanMapTest, iKey_getkeytype_Invalid) {
/* Return error when jeytype is not vbr_vlanmap */
  VlanMapMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());  
  IPC_REQ_RESP_HEADER_DECL(req);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBRIDGE);
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateAttribute(ikey, dmi, req));
}
TEST_F(VlanMapTest, iKey_val_struct_Null) {
/* Return error when ikey val is null */
  VlanMapMoMgr obj;
  DalDmlIntf *dmi(getDalDmlIntf());  
  IPC_REQ_RESP_HEADER_DECL(req);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                           IpctSt::kIpcStKeyVlanMap);
  EXPECT_EQ(UPLL_RC_ERR_CFG_SYNTAX, obj.ValidateAttribute(ikey, dmi, req));
}
TEST_F(VlanMapTest, iKey_valid_struct_readconfg_success) {
/* Return success when ikey is not null and valid struct and
                                        readDb is success for 2 records */
  VlanmapStub obj;
  DalDmlIntf *dmi(getDalDmlIntf());  
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  IPC_REQ_RESP_HEADER_DECL(req);
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                            IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                  "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                              "PORT", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                  "VTN1", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                              "PORT1", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;
  VlanmapStub::stub_result[ReadConfigDB1] = UPLL_RC_SUCCESS;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateAttribute(ikey, dmi, req));
}
TEST_F(VlanMapTest, ReadConfigDb_read_error) {
/* Return success when readconfigdb returns error */
  VlanmapStub obj;
  DalDmlIntf *dmi(getDalDmlIntf());  
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  IPC_REQ_RESP_HEADER_DECL(req);
  ConfigVal *tmp = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                            IpctSt::kIpcStKeyVlanMap, vlanmap_key, tmp);
  strncpy(reinterpret_cast<char *>(vlanmap_key->vbr_key.vtn_key.vtn_name),
                                                  "VTN", kMaxLenVtnName);
  strncpy(reinterpret_cast<char *>(vlanmap_key->logical_port_id),
                                              "PORT", kMaxLenLogicalPortId);
  vlanmap_key->logical_port_id_valid =PFC_TRUE;
  VlanmapStub::stub_result[ReadConfigDB1] = UPLL_RC_ERR_NO_SUCH_INSTANCE;
  EXPECT_EQ(UPLL_RC_SUCCESS, obj.ValidateAttribute(ikey, dmi, req));
}

/* ============================*ValidateCapability*========================== */

TEST_F(VlanMapTest, req_Null1) {
/* Return error generic when req is null */
  VlanMapMoMgr obj;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC, obj.ValidateCapability(NULL, NULL, NULL));
}
TEST_F(VlanMapTest, ikey_Null) {
/* Return error generic when ikey is null */
  VlanMapMoMgr obj;
  const char * ctrlr_name = NULL;
  IPC_REQ_RESP_HEADER_DECL(req);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
                     obj.ValidateCapability(req, NULL, ctrlr_name));
}
TEST_F(VlanMapTest, ikey_getkey_Null) {
/* Return error generic when ikey_getkey is null */
  VlanMapMoMgr obj;
  const char * ctrlr_name = NULL;
  IPC_REQ_RESP_HEADER_DECL(req);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                                         IpctSt::kIpcStKeyVlanMap);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
                   obj.ValidateCapability(req, ikey, ctrlr_name));
}
TEST_F(VlanMapTest, OpCreate_DtCandidate) {
/*Return success when op is create and dt is candidate */
 VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_CREATE;
  req->datatype = UPLL_DT_CANDIDATE;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  vlanmap_val->vlan_id = 567;     // between 1 and 4095
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                           IpctSt::kIpcStKeyVlanMap, vlanmap_key, config_val);
  CapaModuleStub::stub_clearStubData();
  uint32_t max_instance_count = 3;
  uint32_t num_attrs = 0;
  uint8_t attrs[1];
  attrs[unc::capa::vlan_map::kCapVlanId] = 1;
  const char* cntrlr_name("cntrlr_name");
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(cntrlr_name, UNC_CT_PFC, version, 0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_loadCapaModule();
  CapaModuleStub::stub_setCreatecapaParameters(max_instance_count,
                                                       &num_attrs, attrs);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_CREATE_CAPABILITY,
                                                                   true);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.ValidateCapability(req, ikey, cntrlr_name));
}

TEST_F(VlanMapTest, OpCreate_DtCandidate_attrcheckfail) {
  /* Return error when attribute support check fail for attrs::vlanid is zero */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_CREATE;
  req->datatype =  UPLL_DT_CANDIDATE;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  vlanmap_val->vlan_id = 440;     // between 1 and 4095
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVlanMap,
                                                               vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                   IpctSt::kIpcStKeyVlanMap, vlanmap_key, config_val);
  CapaModuleStub::stub_clearStubData();
  uint32_t max_instance_count = 3;
  uint32_t num_attrs = 0;
  uint8_t attrs[1];
  attrs[unc::capa::vlan_map::kCapVlanId] = 0;
  const char* cntrlr_name("cntrlr_name");
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(cntrlr_name, UNC_CT_PFC, version, 0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_loadCapaModule();
  CapaModuleStub::stub_setCreatecapaParameters(max_instance_count,
                                                  &num_attrs, attrs);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_CREATE_CAPABILITY,
                                                                     true);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR,
                          obj.ValidateCapability(req, ikey, cntrlr_name));
}
TEST_F(VlanMapTest, OpCreate_DtOthers) {
/* Return error when op is create and dt is other than candidate */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_CREATE;
  req->datatype = UPLL_DT_IMPORT;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                           IpctSt::kIpcStKeyVlanMap, vlanmap_key, config_val);
  const char* cntrlr_name("cntrlr_name");
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR,
                            obj.ValidateCapability(req, ikey, cntrlr_name));
}
TEST_F(VlanMapTest, OpUpdate_DtCandidate) {
/* Return success when op is update and dt is candidate */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_UPDATE;
  req->datatype =  UPLL_DT_CANDIDATE;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  vlanmap_val->vlan_id = 567;// between 1 and 4095
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVlanMap,
                                                                 vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                     IpctSt::kIpcStKeyVlanMap, vlanmap_key, config_val);
   CapaModuleStub::stub_clearStubData();
  uint32_t max_instance_count = 3;
  uint32_t num_attrs = 0;
  uint8_t attrs[1];
  attrs[unc::capa::vlan_map::kCapVlanId] = 1;
  const char* cntrlr_name("cntrlr_name");
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(cntrlr_name, UNC_CT_PFC, version, 0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_loadCapaModule();
  CapaModuleStub::stub_setCreatecapaParameters(max_instance_count,
                                                  &num_attrs, attrs);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_UPDATE_CAPABILITY,
                                                                      true);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.ValidateCapability(req, ikey, cntrlr_name));
}
TEST_F(VlanMapTest, OpUpdate_DtCandidate_attrcheckfail) {
/* Return error when attribute support check fail for attrs::vlanid is zero */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_UPDATE;
  req->datatype =  UPLL_DT_CANDIDATE;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  vlanmap_val->vlan_id = 567;     // between 1 and 4095
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVlanMap,
                                                               vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                   IpctSt::kIpcStKeyVlanMap, vlanmap_key, config_val);
  CapaModuleStub::stub_clearStubData();
  uint32_t max_instance_count = 3;
  uint32_t num_attrs = 0;
  uint8_t attrs[1];
  attrs[unc::capa::vlan_map::kCapVlanId] = 0;
  const char* cntrlr_name("cntrlr_name");
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(cntrlr_name, UNC_CT_PFC, version, 0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_loadCapaModule();
  CapaModuleStub::stub_setCreatecapaParameters(max_instance_count,
                                                  &num_attrs, attrs);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_UPDATE_CAPABILITY,
                                                                     true);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR,
                          obj.ValidateCapability(req, ikey, cntrlr_name));
}
TEST_F(VlanMapTest, OpUpdate_DtOthers) {
/* Return error when op is update and dt is other than candidate */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_UPDATE;
  req->datatype = UPLL_DT_IMPORT;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                           IpctSt::kIpcStKeyVlanMap, vlanmap_key, config_val);
  const char* cntrlr_name("cntrlr_name");
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR,
                            obj.ValidateCapability(req, ikey, cntrlr_name));
}
TEST_F(VlanMapTest, OpRead_DtCandidate) {
/* Return success when op is read and dt is candidate and option 1 and 2
                                                             are valid */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ;
  req->datatype =  UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  vlanmap_val->vlan_id = 567;     // between 1 and 4095
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVlanMap,
                                                               vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                   IpctSt::kIpcStKeyVlanMap, vlanmap_key, config_val);
  CapaModuleStub::stub_clearStubData();
  uint32_t max_instance_count = 3;
  uint32_t num_attrs = 0;
  uint8_t attrs[1];
  attrs[unc::capa::vlan_map::kCapVlanId] = 1;
  const char* cntrlr_name("cntrlr_name");
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(cntrlr_name, UNC_CT_PFC, version, 0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_loadCapaModule();
  CapaModuleStub::stub_setCreatecapaParameters(max_instance_count,
                                                  &num_attrs, attrs);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY,
                                                                    true);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.ValidateCapability(req, ikey, cntrlr_name));
}
TEST_F(VlanMapTest, OpRead_DtCandidate_Opt1_invalid) {
/* Return error option1 when op is read, dt is candidate and opt1 is invalid */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ;
  req->datatype =  UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_DETAIL;
  req->option2 = UNC_OPT2_NONE;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  vlanmap_val->vlan_id = 567;     // between 1 and 4095
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVlanMap,
                                                                vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                     IpctSt::kIpcStKeyVlanMap, vlanmap_key, config_val);
  CapaModuleStub::stub_clearStubData();
  uint32_t max_instance_count = 3;
  uint32_t num_attrs = 0;
  uint8_t attrs[1];
  attrs[unc::capa::vlan_map::kCapVlanId] = 1;
  const char* cntrlr_name("cntrlr_name");
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(cntrlr_name, UNC_CT_PFC, version, 0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_loadCapaModule();
  CapaModuleStub::stub_setCreatecapaParameters(max_instance_count,
                                                  &num_attrs, attrs);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR,
                        obj.ValidateCapability(req, ikey, cntrlr_name));
}
TEST_F(VlanMapTest, OpRead_DtCandidate_Opt2_Invalid) {
/*Return error opt2 when op is read, dt is candidate and opt2 is invalid */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ;
  req->datatype =  UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_L2DOMAIN;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  vlanmap_val->vlan_id = 567;     // between 1 and 4095
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVlanMap,
                                                               vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                   IpctSt::kIpcStKeyVlanMap, vlanmap_key, config_val);
  CapaModuleStub::stub_clearStubData();
  uint32_t max_instance_count = 3;
  uint32_t num_attrs = 0;
  uint8_t attrs[1];
  attrs[unc::capa::vlan_map::kCapVlanId] = 1;
  const char* cntrlr_name("cntrlr_name");
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(cntrlr_name, UNC_CT_PFC, version, 0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_loadCapaModule();
  CapaModuleStub::stub_setCreatecapaParameters(max_instance_count,
                                                &num_attrs, attrs);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY,
                                                                  true);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR,
                          obj.ValidateCapability(req, ikey, cntrlr_name));
}
TEST_F(VlanMapTest, OpReadSibling_dtStartup_opts_valid) {
/* Return succes when op is read sibling, dt is startup and
                                                      options are valid */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ_SIBLING;
  req->datatype =  UPLL_DT_STARTUP;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  vlanmap_val->vlan_id = 567;     // between 1 and 4095
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVlanMap, NULL);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                    IpctSt::kIpcStKeyVlanMap, vlanmap_key, config_val);
  CapaModuleStub::stub_clearStubData();
  uint32_t max_instance_count = 3;
  uint32_t num_attrs = 0;
  uint8_t attrs[1];
  attrs[unc::capa::vlan_map::kCapVlanId] = 1;
  const char* cntrlr_name("cntrlr_name");
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(cntrlr_name, UNC_CT_PFC, version, 0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_loadCapaModule();
  CapaModuleStub::stub_setCreatecapaParameters(max_instance_count,
                                                 &num_attrs, attrs);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY,
                                                                  true);
EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR,
                          obj.ValidateCapability(req, ikey, cntrlr_name));
}
TEST_F(VlanMapTest, OpReadSiblingbegin_Dtstate_Opts_Valid) {
/* Return Success when op is read sibling begin, dt is state and options
                                                                  are valid */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ_SIBLING_BEGIN;
  req->datatype =  UPLL_DT_STATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  vlanmap_val->vlan_id = 567;     // between 1 and 4095
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcStValVlanMap, vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                            IpctSt::kIpcStKeyVlanMap, vlanmap_key, cfg_val );
  CapaModuleStub::stub_clearStubData();
  uint32_t max_instance_count = 3;
  uint32_t num_attrs = 0;
  uint8_t attrs[1];
  attrs[unc::capa::vlan_map::kCapVlanId] = 1;
  const char* cntrlr_name("cntrlr_name");
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(cntrlr_name, UNC_CT_PFC, version, 0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_loadCapaModule();
  CapaModuleStub::stub_setCreatecapaParameters(max_instance_count,
                                                  &num_attrs, attrs);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY,
                                                                  true);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR, obj.ValidateCapability(req, ikey, cntrlr_name));
}
TEST_F(VlanMapTest, Opreadsiblingcount_dtRunning_opts_Valid) {
/* Return Success when op is readsibling count, dt is running and options
                                                                are valid */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ_SIBLING_COUNT;
  req->datatype =  UPLL_DT_RUNNING;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  vlanmap_val->vlan_id = 567;     // between 1 and 4095
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVlanMap,
                                                                vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                    IpctSt::kIpcStKeyVlanMap, vlanmap_key, config_val);
  CapaModuleStub::stub_clearStubData();
  uint32_t max_instance_count = 3;
  uint32_t num_attrs = 0;
  uint8_t attrs[1];
  attrs[unc::capa::vlan_map::kCapVlanId] = 0;
  const char* cntrlr_name("cntrlr_name");
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(cntrlr_name, UNC_CT_PFC, version, 0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_loadCapaModule();
  CapaModuleStub::stub_setCreatecapaParameters(max_instance_count,
                                                  &num_attrs, attrs);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY,
                                                                    true);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR,
                          obj.ValidateCapability(req, ikey, cntrlr_name));
}
TEST_F(VlanMapTest, OPRead_DtImport_opts_valid) {
/* Return error when op is read and dt is import and options are valid */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_READ;
  req->datatype =  UPLL_DT_IMPORT;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  vlanmap_val->vlan_id = 567;     // between 1 and 4095
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVlanMap,
                                                               vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                    IpctSt::kIpcStKeyVlanMap, vlanmap_key, config_val);
  CapaModuleStub::stub_clearStubData();
  uint32_t max_instance_count = 3;
  uint32_t num_attrs = 0;
  uint8_t attrs[1];
  attrs[unc::capa::vlan_map::kCapVlanId] = 1;
  const char* cntrlr_name("cntrlr_name");
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(cntrlr_name, UNC_CT_PFC, version, 0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_loadCapaModule();
  CapaModuleStub::stub_setCreatecapaParameters(max_instance_count,
                                                  &num_attrs, attrs);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY, true);
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR,
                        obj.ValidateCapability(req, ikey, cntrlr_name));
}
TEST_F(VlanMapTest, OpRename_Dtcandidate_Opts_valid) {
/* Return error when op is others */
  VlanMapMoMgr obj;
  IPC_REQ_RESP_HEADER_DECL(req);
  req->operation = UNC_OP_RENAME;
  req->datatype =  UPLL_DT_CANDIDATE;
  req->option1 = UNC_OPT1_NORMAL;
  req->option2 = UNC_OPT2_NONE;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  vlanmap_val->vlan_id = 567;     // between 1 and 4095
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  key_vlan_map *vlanmap_key = ZALLOC_TYPE(key_vlan_map );
  ConfigVal *config_val= new ConfigVal(IpctSt::kIpcStValVlanMap,
                                                                vlanmap_val);
  ConfigKeyVal *ikey = new ConfigKeyVal(UNC_KT_VBR_VLANMAP,
                    IpctSt::kIpcStKeyVlanMap, vlanmap_key, config_val);
  CapaModuleStub::stub_clearStubData();
  uint32_t max_instance_count = 3;
  uint32_t num_attrs = 0;
  uint8_t attrs[1];
  attrs[unc::capa::vlan_map::kCapVlanId] = 1;
  const char* cntrlr_name("cntrlr_name");
  const char* version("version");
  CtrlrMgr::Ctrlr ctrl(cntrlr_name, UNC_CT_PFC, version, 0);
  CtrlrMgr::Ctrlr* ctrl1( new CtrlrMgr::Ctrlr(ctrl, UPLL_DT_RUNNING));
  CtrlrMgr::GetInstance()->ctrlrs_.push_back(ctrl1);
  CapaModuleStub::stub_loadCapaModule();
  CapaModuleStub::stub_setCreatecapaParameters(max_instance_count,
                                                  &num_attrs, attrs);
  CapaModuleStub::stub_setResultcode(CapaModuleStub::GET_READ_CAPABILITY,
  true);
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
                          obj.ValidateCapability(req, ikey, cntrlr_name));
}
/* ============================*FilterAttributes*============================ */
TEST(VlanMapMoMgr_FilterAttributes, update_auditstatus_test) {
  VlanMapMoMgr obj;
  pfc_log_set_level(PFC_LOGLVL_VERBOSE);

  pfcdrv_val_vlan_map_t *vlanmap_val1 =
    reinterpret_cast<pfcdrv_val_vlan_map_t *>
    (malloc(sizeof(pfcdrv_val_vlan_map_t)));
  vlanmap_val1->vm.vlan_id = 567;     //  between 1 and 4095
  vlanmap_val1->vm.valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  pfcdrv_val_vlan_map_t *vlanmap_val2 =
    reinterpret_cast<pfcdrv_val_vlan_map_t *>
    (malloc(sizeof(pfcdrv_val_vlan_map_t)));
  vlanmap_val2->vm.vlan_id = 567;     //  between 1 and 4095
  vlanmap_val2->vm.valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;

  void *ptr =
    reinterpret_cast<void *>(vlanmap_val1);
  EXPECT_EQ(true, obj.FilterAttributes((*&ptr), (void*)vlanmap_val2,
                                             false, UNC_OP_UPDATE));
  pfcdrv_val_vlan_map_t *drv_val_vlan_map1 =
    reinterpret_cast<pfcdrv_val_vlan_map_t *>(ptr);
  val_vlan_map_t *val_vlan_map1 = &(drv_val_vlan_map1->vm);

  EXPECT_EQ(UNC_VF_VALUE_NOT_MODIFIED,
     val_vlan_map1->valid[UPLL_IDX_VLAN_ID_VM]);
  free(vlanmap_val1); free(vlanmap_val2);
}

TEST(VlanMapMoMgr_FilterAttributes, update_auditstatus_valid) {
  VlanMapMoMgr obj;
  pfc_log_set_level(PFC_LOGLVL_VERBOSE);

  pfcdrv_val_vlan_map_t *vlanmap_val1 =
    reinterpret_cast<pfcdrv_val_vlan_map_t *>
      (malloc(sizeof(pfcdrv_val_vlan_map_t)));
  vlanmap_val1->vm.vlan_id = 567;     //  between 1 and 4095
  vlanmap_val1->vm.valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  pfcdrv_val_vlan_map_t *vlanmap_val2 =
    reinterpret_cast<pfcdrv_val_vlan_map_t *>
      (malloc(sizeof(pfcdrv_val_vlan_map_t)));
  vlanmap_val2->vm.vlan_id = 1;     //  between 1 and 4095
  vlanmap_val2->vm.valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;

  void *ptr =
    reinterpret_cast<void *>(vlanmap_val1);
  EXPECT_FALSE(obj.FilterAttributes((*&ptr), (void*)vlanmap_val2,
                                       false, UNC_OP_UPDATE));
  pfcdrv_val_vlan_map_t *drv_val_vlan_map1 =
    reinterpret_cast<pfcdrv_val_vlan_map_t *>(ptr);
  val_vlan_map_t *val_vlan_map1 = &(drv_val_vlan_map1->vm);

  EXPECT_EQ(UNC_VF_VALID, val_vlan_map1->valid[UPLL_IDX_VLAN_ID_VM]);
  free(vlanmap_val1); free(vlanmap_val2);
}

TEST(VlanMapMoMgr_FilterAttributes, create_auditstatus_valid) {
  VlanMapMoMgr obj;
  pfc_log_set_level(PFC_LOGLVL_VERBOSE);

  pfcdrv_val_vlan_map_t *vlanmap_val1 =
    reinterpret_cast<pfcdrv_val_vlan_map_t *>
      (malloc(sizeof(pfcdrv_val_vlan_map_t)));
  vlanmap_val1->vm.vlan_id = 567;     //  between 1 and 4095
  vlanmap_val1->vm.valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  pfcdrv_val_vlan_map_t *vlanmap_val2 =
    reinterpret_cast<pfcdrv_val_vlan_map_t *>
      (malloc(sizeof(pfcdrv_val_vlan_map_t)));
  vlanmap_val2->vm.vlan_id = 1;     //  between 1 and 4095
  vlanmap_val2->vm.valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;

  void *ptr =
    reinterpret_cast<void *>(vlanmap_val1);
  EXPECT_FALSE(obj.FilterAttributes((*&ptr), (void*)vlanmap_val2,
                                       false, UNC_OP_CREATE));
  pfcdrv_val_vlan_map_t *drv_val_vlan_map1 =
    reinterpret_cast<pfcdrv_val_vlan_map_t *>(ptr);
  val_vlan_map_t *val_vlan_map1 = &(drv_val_vlan_map1->vm);

  EXPECT_EQ(UNC_VF_VALID, val_vlan_map1->valid[UPLL_IDX_VLAN_ID_VM]);
  free(vlanmap_val1); free(vlanmap_val2);
}
/* =============================*CompareValidValue*========================== */

TEST_F(VlanMapTest, auditstatus_true) {
/*Check if valid flags are set to valid no vlaue when audit status is true */
  VlanMapMoMgr obj;
  val_vlan_map *vlanmap_val1 = ZALLOC_TYPE(val_vlan_map );
  vlanmap_val1->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_INVALID;
  val_vlan_map *vlanmap_val2 = ZALLOC_TYPE(val_vlan_map );
  vlanmap_val2->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  void *ptr =
    reinterpret_cast<void *>(vlanmap_val1);
  obj.CompareValidValue((*&ptr), (void*)vlanmap_val2, true);
  EXPECT_EQ(UNC_VF_INVALID, vlanmap_val1->valid[UPLL_IDX_VLAN_ID_VM]);
}
TEST_F(VlanMapTest, auditstatus_false) {
/*Check if valid flags are set to valid no vlaue when audit status is true */
  VlanMapMoMgr obj;
  val_vlan_map *vlanmap_val1 = ZALLOC_TYPE(val_vlan_map );
  vlanmap_val1->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_INVALID;
  vlanmap_val1->vlan_id = 567;     // between 1 and 4095
  val_vlan_map *vlanmap_val2 = ZALLOC_TYPE(val_vlan_map );
  vlanmap_val2->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  vlanmap_val2->vlan_id = 3454;     //  between 1 and 4095
  void *ptr =
    reinterpret_cast<void *>(vlanmap_val1);
  obj.CompareValidValue((*&ptr), (void*)vlanmap_val2, true);
  EXPECT_EQ(UNC_VF_INVALID, vlanmap_val1->valid[UPLL_IDX_VLAN_ID_VM]);
}
// ===================*ValVlanmapAttributeSupportCheck====================

TEST_F(VlanMapTest, VLANID_IS_VALID) {
  VlanMapMoMgr obj;
  unc_keytype_operation_t operation =UNC_OP_CREATE;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID;
  uint8_t attrs[1];
  attrs[unc::capa::vlan_map::kCapVlanId] = 1;
  EXPECT_EQ(UPLL_RC_SUCCESS,
            obj.ValVlanmapAttributeSupportCheck(vlanmap_val, attrs, operation));
}

TEST_F(VlanMapTest, VLANID_IS_VALID_NO_VALUE) {
  VlanMapMoMgr obj;
  unc_keytype_operation_t operation =UNC_OP_CREATE;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID_NO_VALUE;
  uint8_t attrs[1];
  attrs[unc::capa::vlan_map::kCapVlanId] = 1;
  EXPECT_EQ(UPLL_RC_SUCCESS,
            obj.ValVlanmapAttributeSupportCheck(vlanmap_val, attrs, operation));
}

TEST_F(VlanMapTest, capa_vlan_id_zero_op_create) {
  VlanMapMoMgr obj;
  unc_keytype_operation_t operation =UNC_OP_CREATE;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID_NO_VALUE;
  uint8_t attrs[1];
  attrs[unc::capa::vlan_map::kCapVlanId] = 0;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR,
            obj.ValVlanmapAttributeSupportCheck(vlanmap_val, attrs, operation));
}
TEST_F(VlanMapTest, capa_vlan_id_zero_op_update) {
  VlanMapMoMgr obj;
  unc_keytype_operation_t operation =UNC_OP_UPDATE;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID_NO_VALUE;
  uint8_t attrs[1];
  attrs[unc::capa::vlan_map::kCapVlanId] = 0;
  EXPECT_EQ(UPLL_RC_ERR_NOT_SUPPORTED_BY_CTRLR,
            obj.ValVlanmapAttributeSupportCheck(vlanmap_val, attrs, operation));
}
TEST_F(VlanMapTest, capa_vlan_id_zero_op_read) {
  VlanMapMoMgr obj;
  unc_keytype_operation_t operation =UNC_OP_READ;
  val_vlan_map *vlanmap_val = ZALLOC_TYPE(val_vlan_map );
  vlanmap_val->valid[UPLL_IDX_VLAN_ID_VM] = UNC_VF_VALID_NO_VALUE;
  uint8_t attrs[1];
  attrs[unc::capa::vlan_map::kCapVlanId] = 0;
  EXPECT_EQ(UPLL_RC_SUCCESS,
            obj.ValVlanmapAttributeSupportCheck(vlanmap_val, attrs, operation));
}
TEST_F(VlanMapTest, val_vlan_NULL) {
  VlanMapMoMgr obj;
  unc_keytype_operation_t operation =UNC_OP_READ;
  val_vlan_map *vlanmap_val = NULL;
  uint8_t attrs[1];
  attrs[unc::capa::vlan_map::kCapVlanId] = 0;
  EXPECT_EQ(UPLL_RC_ERR_GENERIC,
            obj.ValVlanmapAttributeSupportCheck(vlanmap_val, attrs, operation));
}
/* =========================*IsReferenced*============================= */
// Function not implemeted

TEST_F(VlanMapTest, return_success) {
  VlanMapMoMgr obj;
  ConfigKeyVal *ikey = NULL;
  //upll_keytype_datatype_t dt_type = UPLL_DT_CANDIDATE;
  DalDmlIntf *dmi = NULL;
  IpcReqRespHeader *req = NULL;
EXPECT_EQ(UPLL_RC_SUCCESS, obj.IsReferenced(req, ikey, dmi));
}

