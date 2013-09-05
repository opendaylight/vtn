/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   KT State Base implementation
 * @file    itc_kt_state_base.hh
 *
 */

#ifndef KT_STATE_BASE_HH
#define KT_STATE_BASE_HH

#include <string>
#include <vector>
#include <map>

#include "physical_common_def.hh"
#include "unc/uppl_common.h"
#include "ipc_connection_manager.hh"
#include "odbcm_utils.hh"
#include "itc_kt_base.hh"

using std::vector;
using std::map;
using pfc::core::ipc::ServerSession;
using unc::uppl::ODBCMOperator;
using unc::uppl::DBTableSchema;

class Kt_State_Base: public Kt_Base {
 public:
  Kt_State_Base();

  virtual ~Kt_State_Base() {
  };

  UpplReturnCode Create(OdbcmConnectionHandler *db_conn,
                        uint32_t session_id,
                        uint32_t configuration_id,
                        void* key_struct,
                        void* val_struct,
                        uint32_t data_type,
                        uint32_t key_type,
                        ServerSession &sess);

  UpplReturnCode CreateKeyInstance(OdbcmConnectionHandler *db_conn,
                                   void* key_struct,
                                   void* val_struct,
                                   uint32_t data_type,
                                   uint32_t key_type);

  UpplReturnCode UpdateKeyInstance(OdbcmConnectionHandler *db_conn,
                                   void* key_struct,
                                   void* val_struct,
                                   uint32_t data_type,
                                   uint32_t key_type,
                                   void* &old_val_struct);

  UpplReturnCode Update(OdbcmConnectionHandler *db_conn,
                        uint32_t session_id,
                        uint32_t configuration_id,
                        void* key_struct,
                        void* val_struct,
                        uint32_t data_type,
                        uint32_t key_type,
                        ServerSession &sess);

  UpplReturnCode Delete(OdbcmConnectionHandler *db_conn,
                        uint32_t session_id,
                        uint32_t configuration_id,
                        void* key_struct,
                        uint32_t data_type,
                        uint32_t key_type,
                        ServerSession &sess);

  UpplReturnCode HandleDriverEvents(OdbcmConnectionHandler *db_conn,
                                    void* key_struct,
                                    uint32_t oper_type,
                                    uint32_t data_type,
                                    uint32_t key_type,
                                    void* old_val_struct,
                                    void* new_val_struct);

 private:
  UpplReturnCode HandleOperStatus(OdbcmConnectionHandler *db_conn,
                                  void* key_struct,
                                  uint32_t oper_type,
                                  uint32_t data_type,
                                  uint32_t key_type,
                                  void* new_val_struct,
                                  string controller_name);
};

#endif
