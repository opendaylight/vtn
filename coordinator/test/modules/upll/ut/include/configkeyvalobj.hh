/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <iostream>
#include <string>
#include "vunk_if_momgr.hh"
#include "momgr_impl.hh"
namespace unc {
namespace upll {
namespace kt_momgr {
using namespace std;

class ConfigKeyValObj : public VunkIfMoMgr {
public:
void NewConfigKeyVal(ConfigKeyVal *&ck);
void OnlyKey(ConfigKeyVal *&ck);
void OnlyVal(ConfigKeyVal *&ck);
void CKNULL(ConfigKeyVal *&ck);
void EmptyConfigKeyVal(ConfigKeyVal *&ck);
};

void ConfigKeyValObj::NewConfigKeyVal(ConfigKeyVal *&ck) {
  key_vunk_if *if_key = static_cast<key_vunk_if *>
                                    (malloc(sizeof(key_vunk_if)));
  if (!if_key) {
    SCOPED_TRACE("no memory allocated for vunk_if_key");
    return;
  }
  memset(if_key,0,sizeof(key_vunk_if));
  strncpy(reinterpret_cast<char *>(if_key->vunk_key.vtn_key.vtn_name), reinterpret_cast<const char *>("VTN1"), kMaxLenVtnName+1);
  uuu::upll_strncpy(if_key->vunk_key.vunknown_name, reinterpret_cast<const char *>("VUNK1"), kMaxLenVnodeName+1);
  uuu::upll_strncpy(if_key->if_name, reinterpret_cast<const char *>("VUNKIF1"),kMaxLenInterfaceName+1);

  val_vunk_if *if_val = static_cast<val_vunk_if *>
                                    (malloc(sizeof(val_vunk_if)));
  if (!if_val) {
    SCOPED_TRACE("no memory allocated for vunk_if_val");
    return;
  }
  memset(if_val->description, 0,kMaxLenDescription);
  memcpy(&(if_val->description), "VUNKNOWN INTF", kMaxLenDescription);
  ConfigVal *cval = new ConfigVal(IpctSt::kIpcStValVunkIf, if_val);
  ck = new ConfigKeyVal(UNC_KT_VUNK_IF, IpctSt::kIpcStKeyVunkIf,if_key,cval);
}

void ConfigKeyValObj::EmptyConfigKeyVal(ConfigKeyVal *&ck) {
  key_vunk_if *if_key = static_cast<key_vunk_if *>
                                    (malloc(sizeof(key_vunk_if)));
  if (!if_key) {
    SCOPED_TRACE("no memory allocated for vunk_if_key");
    return;
  }

  val_vunk_if *if_val = static_cast<val_vunk_if *>
                                    (malloc(sizeof(val_vunk_if)));
  if (!if_val) {
    SCOPED_TRACE("no memory allocated for vunk_if_val");
    return;
  }
  ConfigVal *cval = new ConfigVal(IpctSt::kIpcStValVunkIf, if_val);
  ck = new ConfigKeyVal(UNC_KT_VUNK_IF, IpctSt::kIpcStKeyVunkIf,if_key,cval);
}

void ConfigKeyValObj::OnlyKey(ConfigKeyVal *&ck) {
  key_vunk_if *if_key = static_cast<key_vunk_if *>
                                    (malloc(sizeof(key_vunk_if)));
  if (!if_key) {
    SCOPED_TRACE("no memory allocated for vunk_if_key");
    return;
  }
  ck = new ConfigKeyVal(UNC_KT_VUNK_IF, IpctSt::kIpcStKeyVunkIf,if_key,NULL);
}

void ConfigKeyValObj::OnlyVal(ConfigKeyVal *&ck) {
  val_vunk_if *if_val = static_cast<val_vunk_if *>
                                    (malloc(sizeof(val_vunk_if)));
  if (!if_val) {
    SCOPED_TRACE("no memory allocated for vunk_if_val");
    return;
  }
  void *tmp;
  ConfigVal *cval = new ConfigVal(IpctSt::kIpcStValVunkIf, if_val);
  ck = new ConfigKeyVal(UNC_KT_VUNK_IF, IpctSt::kIpcStKeyVunkIf, tmp, NULL);
  ck->AppendCfgVal(IpctSt::kIpcStValVunkIf, cval);
}

void ConfigKeyValObj::CKNULL(ConfigKeyVal *&ck) {
  void *tmp;
  ck = new ConfigKeyVal(UNC_KT_VUNK_IF, IpctSt::kIpcStKeyVunkIf, tmp,NULL);
}




}
}
}
