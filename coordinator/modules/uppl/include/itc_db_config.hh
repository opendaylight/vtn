/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief    DB Configuration Request
 * @file     itc_db_config.hh
 *
 */

#ifndef _ITC_DB_CONFIG_HH_
#define _ITC_DB_CONFIG_HH_

#include <unc/uppl_common.h>
#include "physical_common_def.hh"
#include "physical_itc_req.hh"

namespace unc {
namespace uppl {

class DBConfigurationRequest : public ITCReq {
 public:
  DBConfigurationRequest();
  ~DBConfigurationRequest();
  UpplReturnCode LoadAndCommitStartup();
  UpplReturnCode  ClearStartUpDb();
  UpplReturnCode  ClearAllDb();
  UpplReturnCode  AbortCandidateDb();
  UpplReturnCode  SaveCandidateToRunning();
  UpplReturnCode  SaveRunningToStartUp();
  UpplReturnCode  AuditStartUp();
  UpplReturnCode  AuditRunningDb();
  UpplReturnCode SendDeletedControllerToLogical();
  UpplReturnCode SendCreatedControllerToLogical();
  UpplReturnCode SendUpdatedControllerToLogical();
};
}  // namespace uppl
}  // namespace unc
#endif

