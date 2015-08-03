/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <stdlib.h>
#include <stdint.h>
#include <list>
#include <sstream>

#include "cxx/pfcxx/synch.hh"
#include "uncxx/upll_log.hh"
#include "config_mgr.hh"
#include "vbr_portmap_momgr.hh"
#include "unw_label_range_momgr.hh"
#include "vtunnel_momgr.hh"
#include "vbr_momgr.hh"


namespace unc {
namespace upll {
namespace kt_momgr {

std::map<std::string, std::map<std::string, uint8_t> >
                      VbrPortMapMoMgr::filled_bucket_span_;

BindInfo VbrPortMapMoMgr::vbid_bind_info[] = {
                                  { uudst::vbid_label::kDbiVtnName, CFG_KEY,
                                    offsetof(key_vbid_label, vtn_key.vtn_name),
                                    uud::kDalChar, 32 },
                                  { uudst::vbid_label::kDbiSNo, CFG_KEY,
                                    offsetof(key_vbid_label, label_row),
                                    uud::kDalUint8, 1 },
                                  { uudst::vbid_label::kDbiVBIdLabel, CFG_VAL,
                                     offsetof(val_vbid_label, label_id),
                                    uud::kDalUint32, 1 },
                                  { uudst::vbid_label::kDbiFlags, CK_VAL,
                                    offsetof(key_user_data_t, flags),
                                    uud::kDalUint8, 1 } };

BindInfo VbrPortMapMoMgr::gvtnid_bind_info[] = {
                              { uudst::gvtnid_label::kDbiCtrlrName, CFG_KEY,
                                offsetof(key_gvtnid_label, ctrlr_name),
                                uud::kDalChar, 32 },
                              { uudst::gvtnid_label::kDbiDomainId, CFG_KEY,
                                offsetof(key_gvtnid_label, domain_name),
                                uud::kDalChar, 32 },
                              { uudst::gvtnid_label::kDbiSNo, CFG_KEY,
                                offsetof(key_gvtnid_label, label_row),
                                uud::kDalUint8, 1 },
                              { uudst::gvtnid_label::kDbiGVtnIdLabel, CFG_VAL,
                                offsetof(val_gvtnid_label, label_id),
                                uud::kDalUint32, 1 } };

BindInfo VbrPortMapMoMgr::vbid_rename_bind_info[] = {
  { uudst::vbid_label::kDbiVtnName, CFG_MATCH_KEY, offsetof(
    key_vtn_t, vtn_name), uud::kDalChar, kMaxLenVtnName + 1 },
  { uudst::vbid_label::kDbiVtnName, CFG_INPUT_KEY, offsetof(
    key_rename_vnode_info_t, new_unc_vtn_name),
    uud::kDalChar, kMaxLenVtnName + 1 },
  { uudst::vbid_label::kDbiFlags, CK_VAL, offsetof(
    key_user_data_t, flags), uud::kDalUint8, 1 }
};

/* GetVbIdChildConfigKey is used to create vbid_ckv with key filled */
upll_rc_t VbrPortMapMoMgr::GetVbIdChildConfigKey(ConfigKeyVal *&okey,
                                      ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vbid_label *vbid_key = NULL;
  if (okey && okey->get_key()) {
    vbid_key = reinterpret_cast<key_vbid_label *>(
                    okey->get_key());
  } else {
    vbid_key = reinterpret_cast<key_vbid_label *>(
      ConfigKeyVal::Malloc(sizeof(key_vbid_label)));
  }
  void *pkey;
  if (parent_key == NULL) {
    if (!okey)
      okey = new ConfigKeyVal(UNC_KT_VBR_PORTMAP,
                              IpctSt::kIpcStKeyVbid, vbid_key, NULL);
    else if (okey->get_key() != vbid_key)
      okey->SetKey(IpctSt::kIpcStKeyVbid, vbid_key);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
    if (parent_key->get_key_type() == UNC_KT_VBR_PORTMAP) {
      uuu::upll_strncpy(vbid_key->vtn_key.vtn_name,
            static_cast<key_vbr_portmap_t*>
            (pkey)->vbr_key.vtn_key.vtn_name,
            (kMaxLenVtnName+1));
    }
  }
  if (!pkey) {
    if (!okey || !(okey->get_key()))
      ConfigKeyVal::Free(vbid_key);
    return UPLL_RC_ERR_GENERIC;
  }
  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_VBR_PORTMAP,
                            IpctSt::kIpcStKeyVbid, vbid_key, NULL);
  else if (okey->get_key() != vbid_key)
    okey->SetKey(IpctSt::kIpcStKeyVbid, vbid_key);
  SET_USER_DATA(okey, parent_key);
  return result_code;
}

/* GetGvtnChildConfigKey is used to create gvtnid_ckv with key filled */
upll_rc_t VbrPortMapMoMgr::GetGVtnChildConfigKey(ConfigKeyVal *&okey,
                                      ConfigKeyVal *parent_key) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_gvtnid_label *gvtnid_key = NULL;
  if (okey && okey->get_key()) {
    gvtnid_key = reinterpret_cast<key_gvtnid_label *>(
                    okey->get_key());
  } else {
    gvtnid_key = reinterpret_cast<key_gvtnid_label *>(
      ConfigKeyVal::Malloc(sizeof(key_gvtnid_label)));
  }
  void *pkey;
  if (parent_key == NULL) {
    if (!okey)
      okey = new ConfigKeyVal(UNC_KT_VBR_PORTMAP,
                 IpctSt::kIpcStKeyGVtnId, gvtnid_key, NULL);
    else if (okey->get_key() != gvtnid_key)
      okey->SetKey(IpctSt::kIpcStKeyGVtnId, gvtnid_key);
    return UPLL_RC_SUCCESS;
  } else {
    pkey = parent_key->get_key();
  }
  if (!pkey) {
    if (!okey || !(okey->get_key()))
      ConfigKeyVal::Free(gvtnid_key);
    return UPLL_RC_ERR_GENERIC;
  }
  if (!okey)
    okey = new ConfigKeyVal(UNC_KT_VBR_PORTMAP,
               IpctSt::kIpcStKeyGVtnId, gvtnid_key, NULL);
  else if (okey->get_key() != gvtnid_key)
    okey->SetKey(IpctSt::kIpcStKeyGVtnId, gvtnid_key);
  SET_USER_DATA(okey, parent_key);
  return result_code;
}

/* DupVbIdConfigKeyVal is used to create a copy of vbid_ckv */
upll_rc_t VbrPortMapMoMgr::DupVbIdConfigKeyVal(ConfigKeyVal *&okey,
                            ConfigKeyVal *&req) {
  UPLL_FUNC_TRACE;
  if (req == NULL) return UPLL_RC_ERR_GENERIC;
  if (okey != NULL) return UPLL_RC_ERR_GENERIC;
  if (req->get_key_type() != UNC_KT_VBR_PORTMAP) return UPLL_RC_ERR_GENERIC;
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();

  if (tmp) {
    val_vbid_label *ival =
                      reinterpret_cast<val_vbid_label *>(GetVal(req));
    if (ival == NULL) {
      UPLL_LOG_DEBUG("Null Val structure");
      return UPLL_RC_ERR_GENERIC;
    }
    val_vbid_label *vbid_val = reinterpret_cast<val_vbid_label *>
        (ConfigKeyVal::Malloc(sizeof(val_vbid_label)));
    memcpy(vbid_val, ival, sizeof(val_vbid_label));
    tmp1 = new ConfigVal(IpctSt::kIpcStValVbid, vbid_val);
  };
  void *tkey = (req != NULL) ? (req)->get_key() : NULL;
  if (!tkey) {
    UPLL_LOG_DEBUG("Null tkey");
    DELETE_IF_NOT_NULL(tmp1);
    return UPLL_RC_ERR_GENERIC;
  }
  key_vbid_label *ikey = reinterpret_cast<key_vbid_label *>(tkey);
  key_vbid_label *vbid_key = reinterpret_cast<key_vbid_label *>
      (ConfigKeyVal::Malloc(sizeof(key_vbid_label)));
  memcpy(vbid_key, ikey, sizeof(key_vbid_label));
  okey = new ConfigKeyVal(UNC_KT_VBR_PORTMAP, IpctSt::kIpcStKeyVbid,
                          vbid_key, tmp1);
  if (okey == NULL) {
    DELETE_IF_NOT_NULL(tmp1);
    FREE_IF_NOT_NULL(vbid_key);
  }
  return UPLL_RC_SUCCESS;
}

/* DupGvtnIdConfigKeyVal is used to create a copy of gvtnid_ckv */
upll_rc_t VbrPortMapMoMgr::DupGvtnIdConfigKeyVal(ConfigKeyVal *&okey,
                            ConfigKeyVal *&req) {
  UPLL_FUNC_TRACE;
  if (req == NULL) return UPLL_RC_ERR_GENERIC;
  if (okey != NULL) return UPLL_RC_ERR_GENERIC;
  if (req->get_key_type() != UNC_KT_VBR_PORTMAP) return UPLL_RC_ERR_GENERIC;
  ConfigVal *tmp1 = NULL, *tmp = (req)->get_cfg_val();

  if (tmp) {
    val_gvtnid_label *ival =
                      reinterpret_cast<val_gvtnid_label *>(GetVal(req));
    if (ival == NULL) {
      UPLL_LOG_DEBUG("Null Val structure");
      return UPLL_RC_ERR_GENERIC;
    }
    val_gvtnid_label *gvtn_val = reinterpret_cast<val_gvtnid_label *>
        (ConfigKeyVal::Malloc(sizeof(val_gvtnid_label)));
    memcpy(gvtn_val, ival, sizeof(val_gvtnid_label));
    tmp1 = new ConfigVal(IpctSt::kIpcStValGVtnId, gvtn_val);
  }
  void *tkey = (req != NULL) ? (req)->get_key() : NULL;
  if (!tkey) {
    UPLL_LOG_DEBUG("Null tkey");
    DELETE_IF_NOT_NULL(tmp1);
    return UPLL_RC_ERR_GENERIC;
  }
  key_gvtnid_label *ikey = reinterpret_cast<key_gvtnid_label *>(tkey);
  key_gvtnid_label *gtvn_key = reinterpret_cast<key_gvtnid_label *>
      (ConfigKeyVal::Malloc(sizeof(key_gvtnid_label)));
  memcpy(gtvn_key, ikey, sizeof(key_gvtnid_label));
  okey = new ConfigKeyVal(UNC_KT_VBR_PORTMAP, IpctSt::kIpcStKeyGVtnId,
                          gtvn_key, tmp1);
  if (okey == NULL) {
    DELETE_IF_NOT_NULL(tmp1);
    FREE_IF_NOT_NULL(gtvn_key);
  }
  return UPLL_RC_SUCCESS;
}
/* AllocVbid is used at the time of merge operation to
 * allocate the specified vbid label.
 * If label already exists INSTANCE_EXISTS error is returned,
 * else, entry is created in vbid_tbl
 * */
upll_rc_t VbrPortMapMoMgr::AllocVbid(uint8_t *vtn_name, uint32_t label,
              DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
              TcConfigMode config_mode, string config_vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ck_vbid = NULL;
  result_code = GetVbIdChildConfigKey(ck_vbid, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetVbIdChildConfigKey Failed with result_code %d",
                        result_code);
    return result_code;
  }
  unc_keytype_operation_t op = UNC_OP_UPDATE;
  key_vbid_label *vbid_key = reinterpret_cast<key_vbid_label *>
                             (ck_vbid->get_key());
  uuu::upll_strncpy(vbid_key->vtn_key.vtn_name,
                    vtn_name, (kMaxLenVtnName + 1));
  vbid_key->label_row = label/32 + 1;
  uint32_t bit_pos = label%32;
  if (bit_pos == 0)
    vbid_key->label_row--;
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(ck_vbid, dt_type, UNC_OP_READ,
                         dbop, dmi, VBIDTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      op = UNC_OP_CREATE;
    } else {
      UPLL_LOG_DEBUG("Error in ReadConfigDB: %d", result_code);
      DELETE_IF_NOT_NULL(ck_vbid);
      return result_code;
    }
  }
  val_vbid_label *vbid_val = reinterpret_cast<val_vbid_label *>
                              GetVal(ck_vbid);
  uint32_t tmp_label = 0;
  if (bit_pos)
    tmp_label = 1 << (bit_pos-1);
  else
    tmp_label = 1 << 31;
  if (tmp_label & vbid_val->label_id) {
    DELETE_IF_NOT_NULL(ck_vbid);
    return UPLL_RC_ERR_INSTANCE_EXISTS;
  }
  vbid_val->label_id |= tmp_label;
  DbSubOp dbop_update = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  result_code = UpdateConfigDB(ck_vbid, dt_type, op, dmi, &dbop_update,
                               config_mode, config_vtn_name, VBIDTBL);
  DELETE_IF_NOT_NULL(ck_vbid);
  return result_code;
}

/* AllocVbid is used to allocate a free vbid label.
 * If free label is found, SUCCESS is returned and label param is set.
 * */
upll_rc_t VbrPortMapMoMgr::AllocVbid(ConfigKeyVal *ckv_vbr_pm, uint32_t &label,  //NOLINT
          DalDmlIntf *dmi, TcConfigMode config_mode, string config_vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // READ all CANDIDATE_DEL VBID entries associated to the vtn
  ConfigKeyVal *ckv_del_vbr = NULL;
  result_code = PopulateDelVbidRecords(ckv_vbr_pm, ckv_del_vbr, dmi,
                                       config_mode, config_vtn_name);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    // there is no entry in DEL tbl, so clear ckv
    DELETE_IF_NOT_NULL(ckv_del_vbr);
  } else if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in PopulateDelVbidIdRecords: %d", result_code);
    DELETE_IF_NOT_NULL(ckv_del_vbr);
    return result_code;
  }
  key_vbr_portmap_t *vbr_pm_key =
                   reinterpret_cast<key_vbr_portmap_t*> (ckv_vbr_pm->get_key());
  ConfigKeyVal *ck_vbid = NULL;
  unc_keytype_operation_t op  = UNC_OP_CREATE;
  result_code = GetVbIdChildConfigKey(ck_vbid, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetVbIdChildConfigKey Failed with result_code %d",
                        result_code);
    DELETE_IF_NOT_NULL(ckv_del_vbr);
    return UPLL_RC_ERR_GENERIC;
  }
  key_vbid_label *vbid_key = reinterpret_cast<key_vbid_label *>
                              (ck_vbid->get_key());
  uuu::upll_strncpy(vbid_key->vtn_key.vtn_name,
                   vbr_pm_key->vbr_key.vtn_key.vtn_name,
                   (kMaxLenVtnName + 1));
  bool is_first_create = false;
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(ck_vbid, UPLL_DT_CANDIDATE, UNC_OP_READ,
                         dbop, dmi, VBIDTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      label = 1;
      result_code = UPLL_RC_SUCCESS;
      is_first_create = true;
    } else {
      UPLL_LOG_DEBUG("Error in ReadConfigDB: %d", result_code);
      DELETE_IF_NOT_NULL(ck_vbid);
      DELETE_IF_NOT_NULL(ckv_del_vbr);
      return result_code;
    }
  }
  bool label_found = false;
  uint32_t tmp_label = 1;
  uint16_t label_row = 1;
  ConfigKeyVal *ck_tmp = ck_vbid;
  if (result_code == UPLL_RC_SUCCESS) {
    while (ck_tmp) {
      key_vbid_label *key_label =
                      reinterpret_cast<key_vbid_label*>(ck_tmp->get_key());
      val_vbid_label *val_label =
                      reinterpret_cast<val_vbid_label*>(GetVal(ck_tmp));
      uint32_t max_label_row = key_label->label_row;
      if (is_first_create)
        max_label_row = 125;
      while (max_label_row >= label_row) {
        is_first_create = false;
        uint32_t tmp_label_id = val_label->label_id;
        if (max_label_row != label_row) {
          tmp_label_id = 0;
          is_first_create = true;
        }
        if (~tmp_label_id & MAX_LABEL_RANGE) {
          uint32_t bit_mask = 1;
          uint8_t bit_count = 1;
          while (bit_count <= 32) {
            while (tmp_label_id & bit_mask) {
              bit_count++;
              bit_mask <<= 1;
            }
            if (bit_count <=32)
              label_found = true;
            uint32_t tmp_id = (label_row-1)*32 + bit_count;
            uint32_t tmp_row = label_row;
            if (label_found) {
              bool is_label_used = false;
              result_code = CheckIfLabelIsInDelTable(ckv_vbr_pm, ckv_del_vbr,
                            &tmp_id, dmi, &is_label_used);
              if (result_code == UPLL_RC_SUCCESS) {
                if (is_label_used == true) {
                  UPLL_LOG_DEBUG("label used in another mode. "
                                 "Searching again...");
                  bit_count++;
                  bit_mask <<= 1;
                  label_found = false;
                } else {
                  bit_mask = 1;
                  if (tmp_id%32 == 0)
                    bit_mask <<= 31;
                  else
                    bit_mask <<= tmp_id%32-1;
                }
              } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
                UPLL_LOG_DEBUG("Error in CheckIfLabelIsInDelTable READ");
                DELETE_IF_NOT_NULL(ckv_del_vbr);
                DELETE_IF_NOT_NULL(ck_vbid);
                return result_code;
              }
            }
            if (label_found) {
              tmp_label = tmp_label_id | bit_mask;
              tmp_row = tmp_id/32+1;
              if (tmp_id%32 == 0)
                tmp_row--;
              if (label_row != tmp_row) {
                bool is_row_found = false;
                ConfigKeyVal *find_ckv_tmp = ck_vbid;
                while (find_ckv_tmp) {
                  key_vbid_label *key_tmp = reinterpret_cast<key_vbid_label*>
                                          (find_ckv_tmp->get_key());
                  if (key_tmp->label_row == tmp_row) {
                    is_row_found = true;
                    break;
                  }
                  find_ckv_tmp = find_ckv_tmp->get_next_cfg_key_val();
                }
                label_row = tmp_row;
                uint32_t tmp_bit_shift = 1;
                if ((tmp_id%32) == 0)
                  tmp_bit_shift <<= 31;
                else
                  tmp_bit_shift <<= ((tmp_id%32)-1);
                if (is_row_found) {
                  op = UNC_OP_UPDATE;
                  val_vbid_label *val_tmp = reinterpret_cast<val_vbid_label*>
                                            GetVal(find_ckv_tmp);
                  tmp_label = val_tmp->label_id | tmp_bit_shift;
                } else {
                  tmp_label = tmp_bit_shift;
                }
              } else {
                if (!is_first_create)
                  op = UNC_OP_UPDATE;
              }
              label = tmp_id;
              label_row = tmp_row;
              break;
            }
          }
        }
        if (label_found)
          break;
        label_row++;
      }
      if (label_found)
        break;
      ck_tmp = ck_tmp->get_next_cfg_key_val();
    }
  }
  DELETE_IF_NOT_NULL(ckv_del_vbr);
  if (label_row < 126) {
    if (label_found == false) {
      label = ((label_row-1)*32)+1;
    }
  } else {
    UPLL_LOG_WARN("vbid label allocation failed");
    DELETE_IF_NOT_NULL(ck_vbid);
    return UPLL_RC_ERR_EXCEEDS_RESOURCE_LIMIT;
  }
  ck_vbid->DeleteNextCfgKeyVal();
  val_vbid_label *vbid_label =
                  reinterpret_cast<val_vbid_label*>(GetVal(ck_vbid));
  vbid_label->label_id = tmp_label;
  key_vbid_label *key_label =
                  reinterpret_cast<key_vbid_label*>(ck_vbid->get_key());
  key_label->label_row = label_row;

  DbSubOp dbop_update = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  result_code = UpdateConfigDB(ck_vbid, UPLL_DT_CANDIDATE, op, dmi,
                  &dbop_update, config_mode, config_vtn_name, VBIDTBL);
  DELETE_IF_NOT_NULL(ck_vbid);
  return result_code;
}

/* DeAllocVbid is used to de-allocate all the label-ids under a given vtn.
 * SUCCESS is returned if no label-ids are found or entries are deleted
 */
upll_rc_t VbrPortMapMoMgr::DeAllocVbid(ConfigKeyVal *ckv_vbr_pm,
          DalDmlIntf *dmi, TcConfigMode config_mode, string config_vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vbr_portmap_t *vbr_pm_key = reinterpret_cast<key_vbr_portmap_t*>
                                  (ckv_vbr_pm->get_key());
  ConfigKeyVal *ck_vbid = NULL;
  result_code = GetVbIdChildConfigKey(ck_vbid, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetVbIdChildConfigKey Failed with result_code %d",
                        result_code);
    return UPLL_RC_ERR_GENERIC;
  }
  key_vbid_label *vbid_key =
                  reinterpret_cast<key_vbid_label *>(ck_vbid->get_key());
  uuu::upll_strncpy(vbid_key->vtn_key.vtn_name,
                    vbr_pm_key->vbr_key.vtn_key.vtn_name,
                   (kMaxLenVtnName + 1));
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  result_code = UpdateConfigDB(ck_vbid, UPLL_DT_CANDIDATE, UNC_OP_DELETE, dmi,
                        &dbop, config_mode, config_vtn_name, VBIDTBL);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    result_code = UPLL_RC_SUCCESS;
  }
  DELETE_IF_NOT_NULL(ck_vbid);
  return result_code;
}

/* DeAllocVbid is used to de-allocate a given label-id under a given vtn.
 * SUCCESS is returned if label is deleted
 * ERROR is returned is label-id to be deleted is not found */
upll_rc_t VbrPortMapMoMgr::DeAllocVbid(ConfigKeyVal *ckv_vbr_pm, uint32_t label,
           DalDmlIntf *dmi, TcConfigMode config_mode, string config_vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  key_vbr_portmap_t *vbr_pm_key = reinterpret_cast<key_vbr_portmap_t*>
                                  (ckv_vbr_pm->get_key());
  ConfigKeyVal *ck_vbid = NULL;
  result_code = GetVbIdChildConfigKey(ck_vbid, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetVbIdChildConfigKey failed with result_code %d",
                        result_code);
    return UPLL_RC_ERR_GENERIC;
  }
  key_vbid_label *vbid_key =
                  reinterpret_cast<key_vbid_label *>(ck_vbid->get_key());
  uuu::upll_strncpy(vbid_key->vtn_key.vtn_name,
                    vbr_pm_key->vbr_key.vtn_key.vtn_name,
                    (kMaxLenVtnName + 1));
  vbid_key->label_row = (label/32) + 1;
  if (label%32 == 0)
    vbid_key->label_row--;
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(ck_vbid, UPLL_DT_CANDIDATE, UNC_OP_READ,
                         dbop, dmi, VBIDTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_WARN("Label ID doesn't exist");
    } else {
      UPLL_LOG_DEBUG("Error in ReadConfigDB: %d", result_code);
    }
    DELETE_IF_NOT_NULL(ck_vbid);
    return result_code;
  }
  val_vbid_label *vbid_label =
                  reinterpret_cast<val_vbid_label*>(GetVal(ck_vbid));
  uint32_t bit_shift = 1;
  vbid_label->label_id = vbid_label->label_id & ~(bit_shift << ((label%32)-1));
  unc_keytype_operation_t op  = UNC_OP_UPDATE;
  result_code = UpdateConfigDB(ck_vbid, UPLL_DT_CANDIDATE, op, dmi, &dbop,
                               config_mode, config_vtn_name, VBIDTBL);
  DELETE_IF_NOT_NULL(ck_vbid);
  return result_code;
}

/* AllocGvtnId is used to allocate a free gvtn label.
 * If free label is found, SUCCESS is returned and label param is set.
 */
upll_rc_t VbrPortMapMoMgr::AllocGvtnId(ConfigKeyVal *&ckv_spd, uint8_t* vtn_id,
                      uint32_t &label, DalDmlIntf *dmi,
                      TcConfigMode config_mode, std::string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ckv_spd) {
    UPLL_LOG_ERROR("Empty ckv_spd");
    return UPLL_RC_ERR_GENERIC;
  }
  // READ all the GVTNID associated to the spine ctrlr-domain
  ConfigKeyVal *ckv_gvtn = NULL;
  result_code = PopulateGvtnId(ckv_spd, ckv_gvtn, dmi, config_mode, vtn_name);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in PopulateGvtnId: %d", result_code);
    DELETE_IF_NOT_NULL(ckv_gvtn);
    return result_code;
  }

  // READ all CANDIDATE_DEL GVTNID entries associated to the spine ctrlr-domain
  ConfigKeyVal *ck_del_gvtn = NULL;
  result_code = PopulateDelGvtnIdRecords(ckv_spd, ck_del_gvtn, dmi,
                                       config_mode, vtn_name);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    // there is no entry in DEL tbl, so clear ckv
    DELETE_IF_NOT_NULL(ck_del_gvtn);
  } else if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in PopulateDelGvtnIdRecords: %d", result_code);
    DELETE_IF_NOT_NULL(ckv_gvtn);
    DELETE_IF_NOT_NULL(ck_del_gvtn);
    return result_code;
  }
  while (ckv_spd) {
    ConfigKeyVal *tmp_ckv_spd = ckv_spd->get_next_cfg_key_val();
    ckv_spd->set_next_cfg_key_val(NULL);

    // based on the user specified range for a given spine domain,
    // set the available bits as 0 and unavailable bits as 1
    uint32_t user_label_range[126]= {0};
    memset(user_label_range, MAX_LABEL_RANGE, sizeof(user_label_range));
    result_code = CalculateRange(ckv_spd, user_label_range,
                  dmi, config_mode, vtn_name);

    // find the free GTVNID based on range,
    // CANDIDATE_DEL and sequential bucket distribution
    result_code = GetFreeGvtnId(ckv_gvtn, ck_del_gvtn, vtn_id, label,
                  user_label_range, dmi, config_mode, vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Error in GetFreeGvtnId: %d", result_code);
      DELETE_IF_NOT_NULL(ckv_gvtn);
      DELETE_IF_NOT_NULL(ck_del_gvtn);
      return result_code;
    }
    if (label != 0) {
      // if label is not found, traverse to next spine domain
      // and continue to search
      DELETE_IF_NOT_NULL(tmp_ckv_spd);
      break;
    }
    DELETE_IF_NOT_NULL(ckv_spd);
    ckv_spd = tmp_ckv_spd;
  }
  DELETE_IF_NOT_NULL(ckv_gvtn);
  DELETE_IF_NOT_NULL(ck_del_gvtn);
  if (label == 0) {
    // Free GVTNID not found in any of the spine domains
    UPLL_LOG_WARN("GVtnId could not be allocated");
    return UPLL_RC_ERR_EXCEEDS_RESOURCE_LIMIT;
  }
  return result_code;
}

/* DeAllocGvtnId is used to de-allocate all rows under a given spine-domain
 * SUCCESS is returned if rows are deleted
 * ERROR is returned if deletion fails */
upll_rc_t VbrPortMapMoMgr::DeAllocGvtnId(ConfigKeyVal *ckv_spd, DalDmlIntf *dmi,
                           TcConfigMode config_mode, string config_vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv_gvtn = NULL;
  result_code = GetGVtnChildConfigKey(ckv_gvtn, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetGVtnChildConfigKey Failed with result_code %d",
                        result_code);
    return result_code;
  }
  val_unw_spdom_ext *spine_val = reinterpret_cast<val_unw_spdom_ext*>
                                                  (GetVal(ckv_spd));
  if (spine_val == NULL) {
    UPLL_LOG_ERROR("Empty spine val recieved");
    DELETE_IF_NOT_NULL(ckv_gvtn);
    return UPLL_RC_ERR_GENERIC;
  }
  if (spine_val->val_unw_spine_dom.valid[UPLL_IDX_SPINE_CONTROLLER_ID_UNWS] !=
       UNC_VF_VALID ||
      spine_val->val_unw_spine_dom.valid[UPLL_IDX_SPINE_DOMAIN_ID_UNWS] !=
       UNC_VF_VALID) {
    UPLL_LOG_ERROR("invalid ctrlr/domain recieved");
    DELETE_IF_NOT_NULL(ckv_gvtn);
    return UPLL_RC_ERR_GENERIC;
  }
  key_gvtnid_label *gvtnid_key =
                    reinterpret_cast<key_gvtnid_label *>(ckv_gvtn->get_key());
  uuu::upll_strncpy(gvtnid_key->ctrlr_name,
                    spine_val->val_unw_spine_dom.spine_controller_id,
                    (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(gvtnid_key->domain_name,
                    spine_val->val_unw_spine_dom.spine_domain_id,
                    (kMaxLenDomainId + 1));
  DbSubOp dbop = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  result_code = UpdateConfigDB(ckv_gvtn, UPLL_DT_CANDIDATE, UNC_OP_DELETE, dmi,
                        &dbop, config_mode, config_vtn_name, GVTNIDTBL);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    result_code = UPLL_RC_SUCCESS;
  }
  DELETE_IF_NOT_NULL(ckv_gvtn);
  return result_code;
}
/* DeAllocGvtnId is used to de-allocate a gvtn-id under a given spine-domain
 * SUCCESS is returned if label is deleted
 * ERROR is returned is label-id to be deleted is not found */
upll_rc_t VbrPortMapMoMgr::DeAllocGvtnId(ConfigKeyVal *vtun_ckv,
                            DalDmlIntf *dmi,
                            upll_keytype_datatype_t dt_type,
                            TcConfigMode config_mode, std::string vtn_name) {
  UPLL_FUNC_TRACE;
  if (!vtun_ckv) {
    UPLL_LOG_DEBUG("Empty vtun_ckv");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv_gvtn = NULL;
  result_code = GetGVtnChildConfigKey(ckv_gvtn, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetGVtnChildConfigKey Failed with result_code %d",
                        result_code);
    return UPLL_RC_ERR_GENERIC;
  }
  controller_domain ctrlr_dom = {NULL, NULL};
  GET_USER_DATA_CTRLR_DOMAIN(vtun_ckv, ctrlr_dom);
  key_gvtnid_label *gvtnid_key =
                    reinterpret_cast<key_gvtnid_label *>(ckv_gvtn->get_key());
  val_convert_vtunnel* vtun_val = reinterpret_cast<val_convert_vtunnel*>
                                   (GetVal(vtun_ckv));
  uuu::upll_strncpy(gvtnid_key->ctrlr_name,
                    ctrlr_dom.ctrlr, (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(gvtnid_key->domain_name,
                    ctrlr_dom.domain, (kMaxLenDomainId + 1));
  gvtnid_key->label_row = (vtun_val->label/32) + 1;

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(ckv_gvtn, dt_type, UNC_OP_READ,
                         dbop, dmi, GVTNIDTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_WARN("vtun_val label ID doesn't exist");
    } else {
      UPLL_LOG_DEBUG("Error in ReadConfigDB: %d", result_code);
    }
    DELETE_IF_NOT_NULL(ckv_gvtn);
    return result_code;
  }
  val_gvtnid_label *gvtnid_label =
                    reinterpret_cast<val_gvtnid_label*>(GetVal(ckv_gvtn));
  gvtnid_label->label_id &= ~(1 << (vtun_val->label % 32));
  result_code = UpdateConfigDB(ckv_gvtn, dt_type, UNC_OP_UPDATE, dmi,
                               &dbop, config_mode, vtn_name, GVTNIDTBL);
  DELETE_IF_NOT_NULL(ckv_gvtn);
  return result_code;
}

/* CalculateRange sets the user_label_range bits as available
 * based on the user specified label range for a given label
 */
upll_rc_t VbrPortMapMoMgr::CalculateRange(ConfigKeyVal *spd_ckv,
                            uint32_t *user_label_range, DalDmlIntf *dmi,
                            TcConfigMode config_mode, std::string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  std::multimap<uint32_t, uint32_t> label_range;
  val_unw_spdom_ext *val_spine = reinterpret_cast<val_unw_spdom_ext*>
                                 (GetVal(spd_ckv));
  if (val_spine == NULL)
    return UPLL_RC_ERR_GENERIC;
  if (val_spine->val_unw_spine_dom.valid[2] != UNC_VF_VALID) {
    // default range is from 1 to 4000;
    label_range.insert(std::make_pair(1, 4000));
  } else {
    ConfigKeyVal *ckv_label_range = NULL;
    UNWLabelRangeMoMgr* unw_label_mgr  =  reinterpret_cast<
        UNWLabelRangeMoMgr*>(const_cast<MoManager *>(
                GetMoManager(UNC_KT_UNW_LABEL_RANGE)));
    result_code = unw_label_mgr->GetChildConfigKey(ckv_label_range, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Error in GetChildCOnfigKey: %d", result_code);
      return result_code;
    }
    key_unw_label *key_label =
                   reinterpret_cast<key_unw_label*>(ckv_label_range->get_key());
    uuu::upll_strncpy(key_label->unw_label_id,
                      val_spine->val_unw_spine_dom.unw_label_id,
                     (kMaxLenUnwLabelName + 1));
    key_unw_spine_domain *key_spine =
            reinterpret_cast<key_unw_spine_domain*>(spd_ckv->get_key());
    uuu::upll_strncpy(key_label->unified_nw_key.unified_nw_id,
                      key_spine->unw_key.unified_nw_id,
                      (kMaxLenUnwSpineID + 1));
    upll_keytype_datatype_t dt_type = (config_mode == TC_CONFIG_VTN) ?
                                       UPLL_DT_RUNNING :
                                       UPLL_DT_CANDIDATE;
    DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
    result_code = unw_label_mgr->ReadConfigDB(ckv_label_range, dt_type,
                         UNC_OP_READ, dbop, dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(ckv_label_range);
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        // assume default label range of 1 - 4000
        label_range.insert(
                  std::make_pair(1, 4000));
        result_code = UPLL_RC_SUCCESS;
      } else {
        UPLL_LOG_DEBUG("Error in ReadConfigDB: %d", result_code);
        return result_code;
      }
    }
    ConfigKeyVal *tmp_ckv_label_range = ckv_label_range;
    while (ckv_label_range) {
      key_unw_label_range *key_range= reinterpret_cast<key_unw_label_range*>
                                      (ckv_label_range->get_key());
      label_range.insert(
                  std::make_pair(key_range->range_min, key_range->range_max));
      ckv_label_range = ckv_label_range->get_next_cfg_key_val();
    }
    DELETE_IF_NOT_NULL(tmp_ckv_label_range);
    // remove repeating and overlapping ranges from user specified range
    FilterLabelRange(&label_range);
  }
  std::multimap<uint32_t, uint32_t>::iterator label_iter = label_range.begin(),
                                              it_end = label_range.end();
  for (; label_iter != it_end; ++label_iter) {
    uint16_t min_range = (*label_iter).first, max_range = (*label_iter).second;
    uint8_t min_index = min_range/32, max_index = max_range/32;

    // set label bits as available for user specified range
    if (min_index != max_index) {
      for (int i = min_index+1; i < max_index+1; i++)
        user_label_range[i] = 0;
    }
    uint32_t bit_pos = 1;
    while (min_range%32) {
      bit_pos *= 2;
      min_range--;
    }
    user_label_range[min_index] &= (bit_pos - 1);
    bit_pos = 2;
    while (max_range%32) {
      bit_pos *= 2;
      max_range--;
    }
    user_label_range[max_index] |= ~(bit_pos - 1);
  }
  return result_code;
}

/* FilterLabelRange removes duplicate and overlapping entries
 * from user specified range
 */
void VbrPortMapMoMgr::FilterLabelRange(
                            std::multimap<uint32_t, uint32_t> *label_range) {
  UPLL_FUNC_TRACE;
  if (label_range->empty()) {
    return;
  }
  std::multimap<uint32_t, uint32_t>::iterator it_cur = label_range->begin(),
                                              it_tmp = label_range->begin(),
                                              it_next = ++it_tmp;
  while (it_next != label_range->end()) {
    if (it_cur->first == it_next->first) {
      if (it_cur->second < it_next->second) {
        it_cur->second = it_next->second;
      }
      it_tmp = it_next++;
      label_range->erase(it_tmp);
    } else {
      if (it_next->first <= it_cur->second) {
        if (it_next->second > it_cur->second) {
          it_cur->second = it_next->second;
        }
        it_tmp = it_next++;
        label_range->erase(it_tmp);
      } else {
        it_cur = it_next++;
      }
    }
  }
}

/* PopulateDelVbidRecords: populates all the deleted records
 * from ca_del_convert_vbr_tbl. This info will be used to check
 * if vbid is already allocated during vbid allocation
 */
upll_rc_t VbrPortMapMoMgr::PopulateDelVbidRecords(ConfigKeyVal *ckv_vbr_pm,
                      ConfigKeyVal *&ckv_del_vbr, DalDmlIntf *dmi,
                      TcConfigMode config_mode, std::string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  VbrMoMgr* vbr_mgr  =  reinterpret_cast<
        VbrMoMgr*>(const_cast<MoManager *>(
                GetMoManager(UNC_KT_VBRIDGE)));
  result_code = vbr_mgr->GetChildConvertConfigKey(ckv_del_vbr, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in GetChildConvertConfigKey: %d", result_code);
    return result_code;
  }
  if (!ckv_vbr_pm || !ckv_del_vbr)
    return UPLL_RC_ERR_GENERIC;
  key_vbr_portmap_t *vbr_pm_key = reinterpret_cast<key_vbr_portmap_t*>
                                   (ckv_vbr_pm->get_key());
  key_convert_vbr_t *cov_vbr_key = reinterpret_cast<key_convert_vbr_t*>
                                   (ckv_del_vbr->get_key());
  uuu::upll_strncpy(cov_vbr_key->vbr_key.vtn_key.vtn_name,
            vbr_pm_key->vbr_key.vtn_key.vtn_name, (kMaxLenVtnName + 1));

  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone, kOpInOutNone};
  return (vbr_mgr->ReadConfigDB(ckv_del_vbr, UPLL_DT_CANDIDATE_DEL,
                         UNC_OP_READ, dbop, dmi, CONVERTTBL));
}

/* PopulateGvtnId : populates all the gtvn records from ca_gvtn_tbl
 * It will be used to select a free gvtn-id
 */
upll_rc_t VbrPortMapMoMgr::PopulateGvtnId(ConfigKeyVal *ckv_spd,
                            ConfigKeyVal *&ckv_gvtn, DalDmlIntf *dmi,
                            TcConfigMode config_mode, std::string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  result_code = GetGVtnChildConfigKey(ckv_gvtn, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG(" GetGVtnChildConfigKey Failed with result_code %d",
                        result_code);
    return UPLL_RC_ERR_GENERIC;
  }
  val_unw_spdom_ext* spine_val = reinterpret_cast<val_unw_spdom_ext*>
                                 (GetVal(ckv_spd));
  if (spine_val == NULL)
    return UPLL_RC_ERR_GENERIC;

  uint8_t *ctrlr_name = spine_val->val_unw_spine_dom.spine_controller_id;
  uint8_t *dom_name = spine_val->val_unw_spine_dom.spine_domain_id;

  key_gvtnid_label *gvtnid_key = reinterpret_cast<key_gvtnid_label *>
                                  (ckv_gvtn->get_key());
  uuu::upll_strncpy(gvtnid_key->ctrlr_name, ctrlr_name, (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(gvtnid_key->domain_name, dom_name, (kMaxLenDomainId + 1));
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(ckv_gvtn, UPLL_DT_CANDIDATE, UNC_OP_READ,
                         dbop, dmi, GVTNIDTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      ConfigKeyVal *ckv_new = NULL, *tmp_ckv = NULL;
      result_code = UPLL_RC_SUCCESS;
      uint32_t default_bucket_span[126] = {0};
      default_bucket_span[0] = 0x00000001;
      default_bucket_span[125] = 0xFFFFFFFE;
      for (int i = 1; i < 127; i++) {
        key_gvtnid_label *update_gvtnid_key =
                    reinterpret_cast<key_gvtnid_label*>(ckv_gvtn->get_key());
        update_gvtnid_key->label_row = i;
        val_gvtnid_label *gvtnid_label =
                    reinterpret_cast<val_gvtnid_label*>(GetVal(ckv_gvtn));
        gvtnid_label->label_id = default_bucket_span[i-1];

        DbSubOp dbop_update = { kOpNotRead, kOpMatchNone, kOpInOutNone };
        result_code = UpdateConfigDB(ckv_gvtn, UPLL_DT_CANDIDATE, UNC_OP_CREATE,
                      dmi, &dbop_update, config_mode, vtn_name, GVTNIDTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("Error in UpdateConfigDB: %d", result_code);
          return result_code;
        }
        ConfigKeyVal *prev_ckv = tmp_ckv;
        tmp_ckv = NULL;
        result_code = DupConfigKeyVal(tmp_ckv, ckv_gvtn, GVTNIDTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Error in DupConfigKeyVal: %d", result_code);
          DELETE_IF_NOT_NULL(ckv_new);
        }
        if (!ckv_new) {
          ckv_new = tmp_ckv;
        } else {
          prev_ckv->AppendCfgKeyVal(tmp_ckv);
        }
      }
      ckv_gvtn->ResetWith(ckv_new);
      DELETE_IF_NOT_NULL(ckv_new);
    }
  }
  return result_code;
}

/* PopulateDelGvtnIdRecords: populates all the deleted records
 * from ca_del_convert_vtunnel_tbl. This info will be used to check
 * if gvtnid is already allocated during gvtnid allocation
 */
upll_rc_t VbrPortMapMoMgr::PopulateDelGvtnIdRecords(ConfigKeyVal *ckv_spd,
                            ConfigKeyVal *&ck_del_gvtn, DalDmlIntf *dmi,
                            TcConfigMode config_mode, std::string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  VtunnelMoMgr* vtunnel_mgr  =  reinterpret_cast<
        VtunnelMoMgr*>(const_cast<MoManager *>(
                GetMoManager(UNC_KT_VTUNNEL)));
  result_code = vtunnel_mgr->GetChildConvertConfigKey(ck_del_gvtn, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Error in GetChildConvertConfigKey: %d", result_code);
    return result_code;
  }
  if (!ckv_spd || !ck_del_gvtn)
    return UPLL_RC_ERR_GENERIC;

  val_unw_spdom_ext* spine_val = reinterpret_cast<val_unw_spdom_ext*>
                                 (GetVal(ckv_spd));
  uint8_t *ctrlr_name = spine_val->val_unw_spine_dom.spine_controller_id;
  uint8_t *dom_name = spine_val->val_unw_spine_dom.spine_domain_id;

//  uint8_t *ctrlr_name = (uint8_t*)"vtn1", *dom_name = (uint8_t*)"vtn1";
  SET_USER_DATA_CTRLR(ck_del_gvtn, ctrlr_name);
  SET_USER_DATA_DOMAIN(ck_del_gvtn, dom_name);

  DbSubOp dbop = {kOpReadMultiple, kOpMatchCtrlr|kOpMatchDomain, kOpInOutNone};
  return (vtunnel_mgr->ReadConfigDB(ck_del_gvtn, UPLL_DT_CANDIDATE_DEL,
                         UNC_OP_READ, dbop, dmi, CONVERTTBL));
}

/* GetFreeGvtnId is used to find free gvtnid based on user label range,
 * already deleted entries in DEL_TBL and current bucket
 */
upll_rc_t VbrPortMapMoMgr::GetFreeGvtnId(ConfigKeyVal *ckv_gvtn,
          ConfigKeyVal *ck_del_gvtn, uint8_t *vtn_id, uint32_t &label,
          uint32_t *default_range, DalDmlIntf *dmi,
          TcConfigMode config_mode, std::string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // next bucket to be used
  key_gvtnid_label *gvtnid_key =
                    reinterpret_cast<key_gvtnid_label *>(ckv_gvtn->get_key());
  std::string ctrlr_id(reinterpret_cast<char*>(gvtnid_key->ctrlr_name));
  std::string dom_id(reinterpret_cast<char*>(gvtnid_key->domain_name));

  // if available_bucket happens to be full, then next_free_bucket will be used
  uint8_t available_bucket = filled_bucket_span_[ctrlr_id][dom_id];

  uint8_t label_row = 0, next_bucket = 0;
  uint32_t tmp_label = 1;
  std::map<uint8_t, ConfigKeyVal*> buck_key_label;
  ConfigKeyVal *ck_tmp = ckv_gvtn;
  while (ck_tmp) {
    key_gvtnid_label *key_label =
                      reinterpret_cast<key_gvtnid_label*>(ck_tmp->get_key());
    label_row = key_label->label_row - 1;
    if (next_bucket <= label_row) {
      val_gvtnid_label *val_label =
                        reinterpret_cast<val_gvtnid_label*>(GetVal(ck_tmp));
      if (~val_label->label_id & ~default_range[label_row]) {
        uint8_t bucket = label_row / 8;
        buck_key_label[bucket] = ck_tmp;
        next_bucket = label_row + 8;
      }
    }
    ck_tmp = ck_tmp->get_next_cfg_key_val();
  }

  bool label_found = false;
  std::map<uint8_t, ConfigKeyVal*>::iterator
                              buck_key_label_end = buck_key_label.end();
  for (int i =0; i < 16; i++) {
    if (buck_key_label.find(available_bucket) != buck_key_label_end) {
      ConfigKeyVal *ckv_tmp_gvtn = buck_key_label[available_bucket];
      key_gvtnid_label *gvtnid_key = reinterpret_cast<key_gvtnid_label*>
                                     (ckv_tmp_gvtn->get_key());
      uint8_t max_rep_count = (available_bucket*8) + 8;
      uint8_t present_row = gvtnid_key->label_row;

      while (ckv_tmp_gvtn && present_row <= max_rep_count) {
        val_gvtnid_label *gvtnid_label =
                    reinterpret_cast<val_gvtnid_label*>(GetVal(ckv_tmp_gvtn));
        uint32_t free_bit = 1, bit_count = 0;
        if (gvtnid_label->label_id == MAX_LABEL_RANGE) {
          present_row++;
          ckv_tmp_gvtn = ckv_tmp_gvtn->get_next_cfg_key_val();
          continue;
        }
        while (bit_count < 32) {
          while ((default_range[present_row-1] & free_bit) ||
               (gvtnid_label->label_id) & free_bit) {
            free_bit <<= 1;
            bit_count++;
          }
          if (bit_count < 32)
            label_found = true;
          uint32_t tmp_id = (present_row-1)*32 + bit_count;
          uint32_t tmp_row = present_row;

          if (label_found) {
            bool is_label_used = false;
            result_code = CheckIfLabelIsInDelTable(ck_del_gvtn, vtn_id, dmi,
                                                   &tmp_id, &is_label_used);
            if (result_code == UPLL_RC_SUCCESS) {
              if (is_label_used == true) {
                UPLL_LOG_DEBUG("label used in another mode.Searching again...");
                bit_count++;
                free_bit <<= 1;
                label_found = false;
              }
            } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
              UPLL_LOG_DEBUG("Error in CheckIfLabelIsInDelTable READ");
              return result_code;
            }
          }
          if (label_found == true) {
            tmp_label = gvtnid_label->label_id | free_bit;
            tmp_row = tmp_id/32+1;
            if (tmp_row != present_row) {
              bool is_row_found = false;
              ConfigKeyVal *find_ckv_tmp = ckv_gvtn;
              while (find_ckv_tmp) {
                key_gvtnid_label *key_tmp = reinterpret_cast<key_gvtnid_label*>
                                        (find_ckv_tmp->get_key());
                if (key_tmp->label_row == tmp_row) {
                  is_row_found = true;
                  break;
                }
                find_ckv_tmp = find_ckv_tmp->get_next_cfg_key_val();
              }
              present_row = tmp_row;
              uint32_t tmp_bit_shift = 1;
              tmp_bit_shift <<= (tmp_id%32);
              if (is_row_found) {
                val_vbid_label *val_tmp = reinterpret_cast<val_vbid_label*>
                                          GetVal(find_ckv_tmp);
                tmp_label = val_tmp->label_id | tmp_bit_shift;
              }
            }
            label = tmp_id;
            label_row = tmp_row;
            break;
          }
        }
        if (label_found == true)
          break;
        present_row++;
        ckv_tmp_gvtn = ckv_tmp_gvtn->get_next_cfg_key_val();
      }
      if (label_found == true)
        break;
    }
    available_bucket++;
    if (available_bucket == 16)
      available_bucket = 0;
  }
  if (label_found == true) {
      ckv_gvtn->DeleteNextCfgKeyVal();
      key_gvtnid_label *gvtnid_key = reinterpret_cast<key_gvtnid_label*>
                                     (ckv_gvtn->get_key());
      val_gvtnid_label *gvtnid_val = reinterpret_cast<val_gvtnid_label*>
                                     (GetVal(ckv_gvtn));
      gvtnid_key->label_row = label_row;
      gvtnid_val->label_id = tmp_label;
      if (++available_bucket == 16)
        available_bucket = 0;
      filled_bucket_span_[ctrlr_id][dom_id] = available_bucket;
  } else {
    UPLL_LOG_DEBUG("gvtnid allocation failed in current range.");
    UPLL_LOG_DEBUG("Trying with next range");
    return result_code;
  }
  DbSubOp dbop_update = { kOpNotRead, kOpMatchNone, kOpInOutNone };
  result_code = UpdateConfigDB(ckv_gvtn, UPLL_DT_CANDIDATE, UNC_OP_UPDATE, dmi,
                          &dbop_update, config_mode, vtn_name, GVTNIDTBL);
  return result_code;
}

/* CheckIfLabelIsInDelTable:  checks if gvtn_label is in DEL_TBL.
 * If no entries are presnt then label can be used.
 * Else if convet vtunnel vtn_name matches with vtn_name param, then return the
 * associated label for label allocatoion.
 * Else new label has to be found
 */
upll_rc_t VbrPortMapMoMgr::CheckIfLabelIsInDelTable(ConfigKeyVal *ck_del_gvtn,
                            uint8_t *vtn_name, DalDmlIntf *dmi,
                            uint32_t *gvtn_label, bool *is_label_used) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  while (ck_del_gvtn) {
    key_convert_vtunnel *del_vtun_key = reinterpret_cast<key_convert_vtunnel*>
                                    (ck_del_gvtn->get_key());
    val_convert_vtunnel *conv_vtun_val = reinterpret_cast<val_convert_vtunnel*>
                                  (GetVal(ck_del_gvtn));
    if (!strcmp(reinterpret_cast<char*>(del_vtun_key->vtn_key.vtn_name),
                reinterpret_cast<char*>(vtn_name))) {
      UPLL_LOG_DEBUG("Entry with VTN match exists. So reassign same GVTN");
      *gvtn_label = conv_vtun_val->label;
      break;
    } else if (conv_vtun_val->label == *gvtn_label) {
      UPLL_LOG_DEBUG("Entry with ctrlr_dom match exists. Allocate other GVTN");
      *is_label_used = true;
      break;
    }
    ck_del_gvtn = ck_del_gvtn->get_next_cfg_key_val();
  }
  return result_code;
}

/* CheckIfLabelIsInDelTable:  checks if vbid_label is in DEL_TBL.
 * If no entries are present then label can be used.
 * Else if convet vbridge vbr_key matches with vbr_port_map key param,
 * then return the associated label for label allocatoion.
 * Else new label has to be found
 */
upll_rc_t VbrPortMapMoMgr::CheckIfLabelIsInDelTable(ConfigKeyVal *ck_vbr_pm,
                           ConfigKeyVal *ckv_del_vbr, uint32_t *vbid_label,
                           DalDmlIntf *dmi, bool *is_label_used) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ck_vbr_pm) {
    return UPLL_RC_ERR_GENERIC;
  }
  while (ckv_del_vbr) {
    key_convert_vbr_t *del_vbr_key = reinterpret_cast<key_convert_vbr_t*>
                                    (ckv_del_vbr->get_key());
    val_convert_vbr_t *del_vbr_val = reinterpret_cast<val_convert_vbr_t*>
                                    (GetVal(ckv_del_vbr));
    key_vbr_portmap_t *vbr_pm_key =
                     reinterpret_cast<key_vbr_portmap_t*>(ck_vbr_pm->get_key());
    if (!strcmp(reinterpret_cast<char*>(del_vbr_key->vbr_key.vtn_key.vtn_name),
               reinterpret_cast<char*>(vbr_pm_key->vbr_key.vtn_key.vtn_name)) &&
       (!strcmp(reinterpret_cast<char*>(del_vbr_key->vbr_key.vbridge_name),
               reinterpret_cast<char*>(vbr_pm_key->vbr_key.vbridge_name)))) {
      UPLL_LOG_DEBUG("Entry with VBIDGE match exists. So reassign same VBID");
      *vbid_label = del_vbr_val->label;
      break;
    } else if (del_vbr_val->label == *vbid_label) {
      UPLL_LOG_DEBUG("Entry with vtn match exists. Allocate other VBID");
      *is_label_used = true;
      break;
    }
    ckv_del_vbr = ckv_del_vbr->get_next_cfg_key_val();
  }
  return result_code;
}

upll_rc_t VbrPortMapMoMgr::TxCopyLabelTblFromCandidateToRunning(
                          unc_keytype_operation_t op,
                          DalDmlIntf* dmi,
                          TcConfigMode config_mode,
                          std::string vtn_name) {
  UPLL_FUNC_TRACE;
  uint8_t *vtnname = NULL;
  if (!vtn_name.empty()) {
    vtnname = reinterpret_cast<uint8_t *>(
        const_cast<char *>(vtn_name.c_str()));
  }

  DalResultCode db_result = uud::kDalRcSuccess;
  upll_rc_t     result_code = UPLL_RC_SUCCESS;
  MoMgrTables tbl[1] = {GVTNIDTBL};  //  VBID is not handled here
  for (int i = 0; i < 1; i++) {
    const uudst::kDalTableIndex tbl_index = GetTable(
      tbl[i], UPLL_DT_CANDIDATE);
    if (tbl_index >= uudst::kDalNumTables)
      continue;

    if (!dmi->IsTableDirtyShallow(tbl_index, config_mode, vtnname)) {
      UPLL_LOG_DEBUG("No modified records in VBID/GVTNID tbl");
      continue;
    }

    db_result = dmi->CopyModifiedRecords(UPLL_DT_RUNNING, UPLL_DT_CANDIDATE,
                           tbl_index, NULL, op, config_mode, vtnname);
    if (db_result != uud::kDalRcSuccess) {
      result_code = DalToUpllResCode(db_result);
      if (result_code != UPLL_RC_SUCCESS) {
        if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          result_code = UPLL_RC_SUCCESS;
          continue;
        }
        return result_code;
      }
    }
  }
  return result_code;
}

upll_rc_t VbrPortMapMoMgr::AllocGvtnId(ConfigKeyVal *vtun_ckv, DalDmlIntf *dmi,
                            upll_keytype_datatype_t dt_type,
                            TcConfigMode config_mode, std::string vtn_name) {
  UPLL_FUNC_TRACE;
  if (!vtun_ckv) {
    UPLL_LOG_INFO("NULL vtun_ckv");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv_gvtn = NULL;
  result_code = GetGVtnChildConfigKey(ckv_gvtn, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("GetGVtnChildConfigKey Failed %d", result_code);
    return UPLL_RC_ERR_GENERIC;
  }
  controller_domain ctrlr_dom = {NULL, NULL};
  GET_USER_DATA_CTRLR_DOMAIN(vtun_ckv, ctrlr_dom);
  key_gvtnid_label *gvtnid_key =
                    reinterpret_cast<key_gvtnid_label *>(ckv_gvtn->get_key());
  val_convert_vtunnel* vtun_val = reinterpret_cast<val_convert_vtunnel*>
                                   (GetVal(vtun_ckv));
  if (vtun_val == NULL) {
    UPLL_LOG_ERROR("NULL vtun_val");
    DELETE_IF_NOT_NULL(ckv_gvtn);
    return UPLL_RC_ERR_GENERIC;
  }
  uuu::upll_strncpy(gvtnid_key->ctrlr_name,
                    ctrlr_dom.ctrlr, (kMaxLenCtrlrId + 1));
  uuu::upll_strncpy(gvtnid_key->domain_name,
                    ctrlr_dom.domain, (kMaxLenDomainId + 1));
  gvtnid_key->label_row = (vtun_val->label/32) + 1;

  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  result_code = ReadConfigDB(ckv_gvtn, dt_type, UNC_OP_READ,
                             dbop, dmi, GVTNIDTBL);
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    result_code = PopulateGvtnId(ckv_gvtn, vtun_val, dmi, dt_type,
                                 config_mode, vtn_name);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("Error in PopulateGvtnId: %d", result_code);
    }
  } else if (result_code == UPLL_RC_SUCCESS) {
    val_gvtnid_label *gvtnid_label =
                    reinterpret_cast<val_gvtnid_label*>(GetVal(ckv_gvtn));
    gvtnid_label->label_id |= (1 << (vtun_val->label % 32));
    result_code = UpdateConfigDB(ckv_gvtn, dt_type, UNC_OP_UPDATE, dmi,
                                 &dbop, config_mode, vtn_name, GVTNIDTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Error in UpdateConfigDB: %d", result_code);
    }
  } else {
    UPLL_LOG_ERROR("Error in ReadConfigDB: %d", result_code);
  }

  DELETE_IF_NOT_NULL(ckv_gvtn);
  return result_code;
}

upll_rc_t VbrPortMapMoMgr::PopulateGvtnId(ConfigKeyVal *ckv_gvtn,
                           val_convert_vtunnel* vtun_val, DalDmlIntf *dmi,
                           upll_keytype_datatype_t dt_type,
                           TcConfigMode config_mode, std::string vtn_name) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint8_t label_row = (vtun_val->label/32) + 1;

  for (int i = 1; i < 127; i++) {
    key_gvtnid_label *update_gvtnid_key =
                reinterpret_cast<key_gvtnid_label*>(ckv_gvtn->get_key());
    update_gvtnid_key->label_row = i;
    val_gvtnid_label *gvtnid_label =
                reinterpret_cast<val_gvtnid_label*>(GetVal(ckv_gvtn));
    // gvtnid_label->label_id = default_bucket_span[i-1];
    gvtnid_label->label_id = ((i == 1) ? 0x00000001 :
                              ((i == 126) ? 0xFFFFFFFE : 0));
    if (i == label_row) {
      gvtnid_label->label_id |= (1 << (vtun_val->label % 32));
    }
    UPLL_LOG_TRACE("gvtn_id label %d %d", i, gvtnid_label->label_id);
    DbSubOp dbop_update = { kOpNotRead, kOpMatchNone, kOpInOutNone };
    result_code = UpdateConfigDB(ckv_gvtn, dt_type, UNC_OP_CREATE,
                  dmi, &dbop_update, config_mode, vtn_name, GVTNIDTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Error in UpdateConfigDB: %d", result_code);
      return result_code;
    }
  }
  return result_code;
}
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc
