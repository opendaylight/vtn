/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <iostream>
#include <string>
#include "vrt_if_momgr.hh"
#include "momgr_impl.hh"
namespace unc {
namespace upll {
namespace kt_momgr {
using namespace std;

class vrtKeyValObj : public VrtIfMoMgr {
public:
void NewConfigKeyVal(ConfigKeyVal *&ck);
void OnlyKey(ConfigKeyVal *&ck);
void OnlyVal(ConfigKeyVal *&ck);
void CKNULL(ConfigKeyVal *&ck);
void EmptyConfigKeyVal(ConfigKeyVal *&ck);
void OnlyValStructure(ConfigKeyVal *&ck);
};

void vrtKeyValObj::NewConfigKeyVal(ConfigKeyVal *&ck) {
  key_vrt_if *if_key = static_cast<key_vrt_if *>
                                    (malloc(sizeof(key_vrt_if)));
  if (!if_key) {
    SCOPED_TRACE("no memory allocated for vunk_if_key");
    return;
  }
  memset(if_key,0,sizeof(key_vrt_if));
  strncpy(reinterpret_cast<char *>(if_key->vrt_key.vtn_key.vtn_name), reinterpret_cast<const char *>("VTN1"), kMaxLenVtnName+1);
  uuu::upll_strncpy(if_key->vrt_key.vrouter_name, reinterpret_cast<const char *>("VUNK1"), kMaxLenVnodeName+1);
  uuu::upll_strncpy(if_key->if_name, reinterpret_cast<const char *>("VUNKIF1"),kMaxLenInterfaceName+1);

  val_vrt_if *if_val = static_cast<val_vrt_if *>
                                    (malloc(sizeof(val_vrt_if)));
  if (!if_val) {
    SCOPED_TRACE("no memory allocated for vunk_if_val");
    return;
  }
  memset(if_val->description, 0,kMaxLenDescription);
  memcpy(&(if_val->description), "VRTINTF", kMaxLenDescription);
/*
  if_val->admin_status = UPLL_ADMIN_ENABLE;
  for (unsigned int loop = 0;
        loop < sizeof(if_val->valid) / sizeof(uint8_t); ++loop) {
    if_val->cs_attr[loop] = UNC_CS_NOT_APPLIED;
  }
  if_val->cs_row_status = UNC_CS_NOT_APPLIED;
*/
  ConfigVal *cval = new ConfigVal(IpctSt::kIpcStValVrtIf, if_val);
  ck = new ConfigKeyVal(UNC_KT_VRT_IF, IpctSt::kIpcStKeyVrtIf,if_key,cval);
}

void vrtKeyValObj::EmptyConfigKeyVal(ConfigKeyVal *&ck) {
  key_vrt_if *if_key = static_cast<key_vrt_if *>
                                    (malloc(sizeof(key_vrt_if)));
  if (!if_key) {
    SCOPED_TRACE("no memory allocated for vunk_if_key");
    return;
  }

  val_vrt_if *if_val = static_cast<val_vrt_if *>
                                    (malloc(sizeof(val_vrt_if)));
  if (!if_val) {
    SCOPED_TRACE("no memory allocated for vunk_if_val");
    return;
  }
  ConfigVal *cval = new ConfigVal(IpctSt::kIpcStValVrtIf, if_val);
  ck = new ConfigKeyVal(UNC_KT_VRT_IF, IpctSt::kIpcStKeyVrtIf,if_key,cval);
}

void vrtKeyValObj::OnlyKey(ConfigKeyVal *&ck) {
  key_vrt_if *if_key = static_cast<key_vrt_if *>
                                    (malloc(sizeof(key_vrt_if)));
  if (!if_key) {
    SCOPED_TRACE("no memory allocated for vunk_if_key");
    return;
  }
  memset(if_key,0,sizeof(key_vrt_if));
  strncpy(reinterpret_cast<char *>(if_key->vrt_key.vtn_key.vtn_name), reinterpret_cast<const char *>("VTN1"), kMaxLenVtnName+1);
  uuu::upll_strncpy(if_key->vrt_key.vrouter_name, reinterpret_cast<const char *>("VUNK1"), kMaxLenVnodeName+1);
  uuu::upll_strncpy(if_key->if_name, reinterpret_cast<const char *>("VUNKIF1"),kMaxLenInterfaceName+1);
  ck = new ConfigKeyVal(UNC_KT_VRT_IF, IpctSt::kIpcStKeyVrtIf,if_key,NULL);
}

void vrtKeyValObj::OnlyVal(ConfigKeyVal *&ck) {
  val_vrt_if *if_val = static_cast<val_vrt_if *>
                                    (malloc(sizeof(val_vrt_if)));
  if (!if_val) {
    SCOPED_TRACE("no memory allocated for vunk_if_val");
    return;
  }
  void *tmp;
  ConfigVal *cval = new ConfigVal(IpctSt::kIpcStValVrtIf, if_val);
  ck = new ConfigKeyVal(UNC_KT_VRT_IF, IpctSt::kIpcStKeyVrtIf, tmp, NULL);
  ck->AppendCfgVal(IpctSt::kIpcStValVrtIf, cval);
}

void vrtKeyValObj::CKNULL(ConfigKeyVal *&ck) {
  void *tmp;
  ck = new ConfigKeyVal(UNC_KT_VRT_IF, IpctSt::kIpcStKeyVrtIf, tmp,NULL);
}

void vrtKeyValObj::OnlyValStructure(ConfigKeyVal *&ck) {
  val_vrt_if *if_val = static_cast<val_vrt_if *>
                                    (malloc(sizeof(val_vrt_if)));
  if (!if_val) {
    SCOPED_TRACE("no memory allocated for vunk_if_val");
    return;
  }
  uuu::upll_strncpy(if_val->description, "VRTIF Desc", 12);
  if_val->valid[UPLL_IDX_DESC_VI] = UNC_VF_VALID;
  if_val->prefixlen = 12;
  if_val->valid[UPLL_IDX_PREFIXLEN_VI] = UNC_VF_VALID;
  if_val->macaddr[0] = 11;
  if_val->macaddr[1] = 11;
  if_val->macaddr[2] = 12;
  if_val->macaddr[3] = 11;
  if_val->macaddr[4] = 12;
  if_val->macaddr[5] = 11;
  if_val->valid[UPLL_IDX_MAC_ADDR_VI] = UNC_VF_VALID;
  if_val->admin_status = UPLL_ADMIN_ENABLE;
  if_val->valid[UPLL_IDX_MAC_ADDR_VI] = UNC_VF_VALID;
  void *tmp;
  ConfigVal *cval = new ConfigVal(IpctSt::kIpcStValVrtIf, if_val);
  ck = new ConfigKeyVal(UNC_KT_VRT_IF, IpctSt::kIpcStKeyVrtIf, tmp, NULL);
  ck->AppendCfgVal(IpctSt::kIpcStValVrtIf, cval);  
}

}
}
}
