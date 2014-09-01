/* Copyright (c) 2012-2013 NEC Corporation                 */
/* NEC CONFIDENTIAL AND PROPRIETARY                        */
/* All rights reserved by NEC Corporation.                 */
/* This program must be used solely for the purpose for    */
/* which it was furnished by NEC Corporation. No part      */
/* of this program may be reproduced or disclosed to       */
/* others, in any form, without the prior written          */
/* permission of NEC Corporation. Use of copyright         */
/* notice does not evidence publication of the program.    */

#include <unc/keytype.h>
#include <../../stub/tclib_module/tclib_interface_stub.hh>
namespace unc {
namespace tclib {

TcLibInterfaceStub::TcLibInterfaceStub() {
  ctr_type = UNC_CT_UNKNOWN;
  fill_driver_info_ = PFC_FALSE;
  tclib_stub_failure_ = PFC_FALSE;
}

unc_keytype_ctrtype_t TcLibInterfaceStub::HandleGetControllerType() {
  if (ctr_type == UNC_CT_PFC)
    return UNC_CT_PFC;
  else if (ctr_type == UNC_CT_VNP)
    return UNC_CT_VNP;
  else
    return UNC_CT_UNKNOWN;
}

void fill_driver_info(TcDriverInfoMap &drv_map) {
  std::string ctr[] = {"first", "second", "third", "four"};

  typedef std::vector<std::string> VEC_CTR;
  VEC_CTR vec_ctr, vec_ctr1, vec_ctr2;
  std::vector<std::string>::iterator it;

  it = vec_ctr.begin();
  vec_ctr.assign(ctr, ctr+4);

  it = vec_ctr1.begin();
  vec_ctr1.assign(ctr, ctr+4);

  it = vec_ctr2.begin();
  vec_ctr2.assign(ctr, ctr+4);

  TcDriverInfoMap map_ctr;
  std::map<unc_keytype_ctrtype_t, VEC_CTR> ::iterator it1;
  map_ctr.insert(pair<unc_keytype_ctrtype_t, VEC_CTR>(UNC_CT_PFC, vec_ctr));
  map_ctr.insert(pair<unc_keytype_ctrtype_t, VEC_CTR>(UNC_CT_VNP, vec_ctr1));

  for (it1 = map_ctr.begin(); it1 != map_ctr.end(); it1++) {
    VEC_CTR vec_print;
    vec_print = it1->second;
#if 0
    for (it = vec_print.begin(); it < vec_print.end(); it++)
      std::cout << ' ' << *it;
    std::cout << '\n';
#endif
  }
  drv_map = map_ctr;
}

TcCommonRet TcLibInterfaceStub::HandleCommitVoteRequest(
    uint32_t session_id,
    uint32_t config_id,
    TcDriverInfoMap& driver_info) {

  if (session_id == 0)
    return TC_FAILURE;

  if (fill_driver_info_ == PFC_FALSE) {
    return TC_SUCCESS;
  } else if (fill_driver_info_ == PFC_TRUE) {
    fill_driver_info(driver_info);
    return TC_SUCCESS;
  }
  return TC_SUCCESS;
}

TcCommonRet
TcLibInterfaceStub::HandleCommitGlobalAbort(uint32_t session_id,
                                            uint32_t config_id,
                                            TcCommitOpAbortPhase fail_phase) {
  if (tclib_stub_failure_ == PFC_TRUE)
    return TC_FAILURE;

  return TC_SUCCESS;
}

TcCommonRet
TcLibInterfaceStub::HandleAuditVoteRequest(uint32_t session_id,
                                           uint32_t driver_id,
                                           string controller_id,
                                           TcDriverInfoMap &driver_info) {
  if (tclib_stub_failure_ == PFC_TRUE) {
    return TC_FAILURE;
  }

  return TC_SUCCESS;
}

TcCommonRet
TcLibInterfaceStub::HandleAuditGlobalCommit(uint32_t session_id,
                                            uint32_t driver_id,
                                            string controller_id,
                                            TcDriverInfoMap &driver_info,
                                            TcAuditResult& audit_result) {
  if (fill_driver_info_ == PFC_FALSE) {
    return TC_SUCCESS;
  } else if (fill_driver_info_ == PFC_TRUE) {
    fill_driver_info(driver_info);
    audit_result = TC_AUDIT_FAILURE;
    return TC_SUCCESS;
  }
  return TC_SUCCESS;
}

TcCommonRet
TcLibInterfaceStub::HandleAuditGlobalAbort(
    uint32_t session_id,
    unc_keytype_ctrtype_t driver_id,
    string controller_id,
    TcAuditOpAbortPhase operation_phase) {
  if (tclib_stub_failure_ == PFC_TRUE)
    return TC_FAILURE;

  return TC_SUCCESS;
}

}  // namespace tclib
}  // namespace unc

