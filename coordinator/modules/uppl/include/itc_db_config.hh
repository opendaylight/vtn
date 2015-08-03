/*
 * Copyright (c) 2012-2015 NEC Corporation
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
#include "odbcm_connection.hh"
#include <uncxx/tclib/tclib_defs.hh>

using unc::uppl::OdbcmConnectionHandler;

namespace unc {
namespace uppl {

class DBConfigurationRequest : public ITCReq {
 public:
  DBConfigurationRequest();
  ~DBConfigurationRequest();
  UncRespCode LoadAndCommitStartup(OdbcmConnectionHandler *db_conn);
  UncRespCode CopyRunningtoCandidate(OdbcmConnectionHandler *db_conn);
  UncRespCode ClearStartUpDb(OdbcmConnectionHandler *db_conn);
  UncRespCode AbortCandidateDb(OdbcmConnectionHandler *db_conn,
                               TcConfigMode config_mode);
  UncRespCode  SaveRunningToStartUp(OdbcmConnectionHandler *db_conn);
  UncRespCode SendDeletedControllerToLogical(
      OdbcmConnectionHandler *db_conn);
  UncRespCode SendCreatedControllerToLogical(
      OdbcmConnectionHandler *db_conn);
  UncRespCode SendUpdatedControllerToLogical(
      OdbcmConnectionHandler *db_conn);
  UncRespCode SendAppliedControllerToLogical(
      OdbcmConnectionHandler *db_conn);
};
}  // namespace uppl
}  // namespace unc
#endif
