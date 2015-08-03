/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef STUB_MOMGR_IMPL_HH_
#define STUB_MOMGR_IMPL_HH_

#include <limits.h>
#include <gtest/gtest.h>
#include "momgr_intf.hh"
#include "pfc/event.h"
#include "cxx/pfcxx/synch.hh"
#include "unc/keytype.h"

#include "upll_errno.h"
#include "dal/dal_dml_intf.hh"
#include "ipc_util.hh"
#include "cxx/pfcxx/module.hh"
#include "capa_module_stub.hh"
#include "ctrlr_mgr.hh"
#include "ipct_st.hh"
#include "dal/dal_dml_intf.hh"

using std::string;
using std::list;
using std::set;
using unc::upll::ipc_util::IpcReqRespHeader;
using unc::upll::ipc_util::ConfigKeyVal;
using unc::upll::ipc_util::ConfigNotification;
using unc::upll::dal::DalDmlIntf;


using ::testing::TestWithParam;
using namespace unc::upll::ipc_util;
using ::testing::Values;

using namespace unc::upll::config_momgr;

class StubMoMgr : public MoManager
{
public:
	virtual upll_rc_t TxUpdateController(unc_key_type_t keytype,
	                                    uint32_t session_id, uint32_t config_id,
	                                    UpdateCtrlrPhase phase,
	                                    set<string> *affected_ctrlr_set,
	                                    DalDmlIntf *dmi,
	                                    ConfigKeyVal **err_ckv ) {
		ConfigKeyVal* configKeyVal(new ConfigKeyVal(UNC_KT_FLOWLIST_ENTRY,IpctSt::kIpcInvalidStNum));
		*err_ckv = configKeyVal;
		if ( phase == kUpllUcpDelete )
		{
			return ( session_id == 0)? UPLL_RC_ERR_GENERIC:UPLL_RC_SUCCESS;
		}
		else if ( phase == kUpllUcpCreate )
		{
			return ( config_id == 0)? UPLL_RC_ERR_GENERIC:UPLL_RC_SUCCESS;
		}
		else if (phase == kUpllUcpUpdate )
			std::cout<<"affected_ctrlr_set->size() "<< affected_ctrlr_set->size()<<std::endl;
			return ( affected_ctrlr_set->size() == 0 )? UPLL_RC_ERR_GENERIC:UPLL_RC_SUCCESS;
	}
	virtual upll_rc_t TxVote(unc_key_type_t keytype, DalDmlIntf *dmi, ConfigKeyVal **err_ckv ) {
		 return UPLL_RC_SUCCESS;
	}

	virtual  upll_rc_t TxVoteCtrlrStatus(
			unc_key_type_t keytype,
	      list<CtrlrVoteStatus*> *ctrlr_vote_status,
	      DalDmlIntf *dmi) {
		  return UPLL_RC_SUCCESS;
	}


virtual  upll_rc_t AuditUpdateController(unc_key_type_t, const char*, uint32_t, uint32_t, unc::upll::config_momgr::UpdateCtrlrPhase, bool*,
                        bool *ctrlr_affected,
                        DalDmlIntf *dmi,
                        ConfigKeyVal **err_ckv){
 return UPLL_RC_SUCCESS;
}

	virtual upll_rc_t TxCopyCandidateToRunning(
			unc_key_type_t keytype,
			list<CtrlrCommitStatus*> *ctrlr_commit_status,
			DalDmlIntf *dmi) {
		return UPLL_RC_SUCCESS;
	}

	virtual upll_rc_t TxEnd(unc_key_type_t keytype, DalDmlIntf *dmi){
		return UPLL_RC_SUCCESS;
	}

	virtual upll_rc_t CreateMo(IpcReqRespHeader *req, ConfigKeyVal *key,
			DalDmlIntf *dmi) {
		return UPLL_RC_SUCCESS;
	}
	virtual upll_rc_t CreateImportMo(IpcReqRespHeader *req, ConfigKeyVal *key,
			DalDmlIntf *dmi, const char *ctrlr_id,const char *domain_id) {
		return UPLL_RC_SUCCESS;
	}
	virtual upll_rc_t DeleteMo(IpcReqRespHeader *req, ConfigKeyVal *key,
			DalDmlIntf *dmi) {
		return UPLL_RC_SUCCESS;
	}

	virtual upll_rc_t UpdateMo(IpcReqRespHeader *req, ConfigKeyVal *key,
			DalDmlIntf *dmi) {
		  return UPLL_RC_SUCCESS;
	}

	/*virtual upll_rc_t RenameMo(IpcReqRespHeader *req, ConfigKeyVal *key,
			DalDmlIntf *dmi, const char *ctrlr_id) {
		return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
	}*/

	virtual upll_rc_t ReadMo(IpcReqRespHeader *req, ConfigKeyVal *key,
			DalDmlIntf *dmi) {
		return UPLL_RC_SUCCESS;
	}

	virtual upll_rc_t ReadSiblingMo(IpcReqRespHeader *req, ConfigKeyVal *key,
			bool begin, DalDmlIntf *dal)
	{
		return UPLL_RC_SUCCESS;
	}
	virtual upll_rc_t ReadSiblingCount(IpcReqRespHeader *req, ConfigKeyVal *key,
			DalDmlIntf *dmi) {
		return UPLL_RC_SUCCESS;
	}

	virtual upll_rc_t ControlMo(IpcReqRespHeader *req, ConfigKeyVal *key,
			DalDmlIntf *dmi) {
		return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
	}

	virtual upll_rc_t AuditUpdateController(unc_key_type_t keytype,
			const char *ctrlr_id,
			uint32_t session_id, uint32_t config_id,
			UpdateCtrlrPhase phase,
                        bool *ctrlr_affected,
			DalDmlIntf *dmi,
                        ConfigKeyVal **err_ckv)  {
		if ( phase == kUpllUcpDelete )
			{
				return ( session_id == 0)? UPLL_RC_ERR_GENERIC:UPLL_RC_SUCCESS;
			}
			else if ( phase == kUpllUcpCreate )
			{
				return ( config_id == 10)? UPLL_RC_ERR_GENERIC:UPLL_RC_SUCCESS;
			}
			else if (phase == kUpllUcpUpdate )

				return UPLL_RC_SUCCESS;
	  }
	    // virtual upll_rc_t AuditVote(const char *ctrlr_id) = 0;
	virtual upll_rc_t AuditVoteCtrlrStatus(unc_key_type_t keytype,
			CtrlrVoteStatus *vote_satus,
			DalDmlIntf *dmi) {
	    	return UPLL_RC_SUCCESS;
	    }
	virtual upll_rc_t AuditCommitCtrlrStatus(unc_key_type_t keytype,
			CtrlrCommitStatus *commit_satus,
			DalDmlIntf *dmi) {
	    	return UPLL_RC_SUCCESS;
	    }
	virtual upll_rc_t AuditEnd(unc_key_type_t keytype, const char *ctrlr_id,
			DalDmlIntf *dmi) {
		std::string cntrl_id(ctrlr_id);
		return ( 0 == cntrl_id.compare("success"))?UPLL_RC_SUCCESS:UPLL_RC_ERR_GENERIC;
	}

	virtual upll_rc_t MergeValidate(unc_key_type_t keytype,
			const char *ctrlr_id,
			ConfigKeyVal *conflict_ckv,
			DalDmlIntf *dmi) {
		return UPLL_RC_SUCCESS;
	}

	virtual upll_rc_t MergeImportToCandidate(unc_key_type_t keytype,
			const char *ctrlr_id,
			DalDmlIntf *dmi) {
		return UPLL_RC_SUCCESS;
	}
	virtual upll_rc_t ImportClear(unc_key_type_t keytype,
			const char *ctrlr_id,
	                                DalDmlIntf *dmi) {
	    	 return UPLL_RC_SUCCESS;
	     }

    virtual upll_rc_t CopyRunningToStartup(unc_key_type_t kt,
    		DalDmlIntf *dmi) {
    	return UPLL_RC_SUCCESS;
    }
    virtual upll_rc_t ClearStartup(unc_key_type_t kt, DalDmlIntf *dmi) {
	    	   return UPLL_RC_SUCCESS;
    }
    virtual upll_rc_t LoadStartup(unc_key_type_t kt, DalDmlIntf *dmi) {
    	return UPLL_RC_SUCCESS;
    }
    virtual upll_rc_t CopyRunningToCandidate(unc_key_type_t kt,
    		DalDmlIntf *dmi) {
    	return UPLL_RC_SUCCESS;
    }
    virtual upll_rc_t IsCandidateDirty(unc_key_type_t kt, bool *dirty,
    		DalDmlIntf *dmi)   {
    	return UPLL_RC_SUCCESS;
    }
    virtual void UpplNotificationHandler(pfc_event_t event, pfc_ptr_t arg) {

    }
    virtual void DriverNotificationHandler(pfc_event_t event, pfc_ptr_t arg)  {

    }

    virtual upll_rc_t GetChildConfigKey(ConfigKeyVal *&okey,
                                        ConfigKeyVal *parent_key) {
      return UPLL_RC_ERR_GENERIC;
    }

    virtual upll_rc_t GetParentConfigKey(ConfigKeyVal *&okey,
                                         ConfigKeyVal *ikey) {
      return UPLL_RC_ERR_GENERIC;
    }
};


#endif /* STUB_MOMGR_IMPL_HH_ */
