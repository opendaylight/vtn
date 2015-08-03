/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   KT Root implementation
 * @file    itc_kt_root.hh
 *
 */

#ifndef KT_ROOT_HH
#define KT_ROOT_HH

#include <string>
#include <vector>
#include "physicallayer.hh"
#include "itc_kt_base.hh"

#define KT_ROOT_CHILD_COUNT 2
using unc::uppl::ODBCMOperator;

typedef enum {
  KIdxController = 0,
      KIdxBoundary
}KtRootChildClass;

/* @ Controller Class definition */

class Kt_Root: public Kt_Base {
  private:
    Kt_Base *child[KT_ROOT_CHILD_COUNT];

  public:
    Kt_Root();
    ~Kt_Root();
    UncRespCode Create(OdbcmConnectionHandler *db_conn,
                          uint32_t session_id,
                          uint32_t configuration_id,
                          void* key_struct,
                          void* val_struct,
                          uint32_t data_type,
                          ServerSession &sess);
    UncRespCode Update(OdbcmConnectionHandler *db_conn,
                          uint32_t session_id,
                          uint32_t configuration_id,
                          void* key_struct,
                          void* val_struct,
                          uint32_t data_type,
                          ServerSession &sess);

    UncRespCode Delete(OdbcmConnectionHandler *db_conn,
                          uint32_t session_id,
                          uint32_t configuration_id,
                          void* key_struct,
                          uint32_t data_type,
                          ServerSession &sess);

    UncRespCode ReadBulk(OdbcmConnectionHandler *db_conn,
                            void* key_struct,
                            uint32_t data_type,
                            uint32_t &max_rep_ct,
                            int child_index,
                            pfc_bool_t parent_call,
                            pfc_bool_t is_read_next,
                            ReadRequest *read_req);

    UncRespCode PerformRead(OdbcmConnectionHandler *db_conn,
                               uint32_t session_id,
                               uint32_t configuration_id,
                               void* key_struct,
                               void* val_struct,
                               uint32_t data_type,
                               uint32_t operation_type,
                               ServerSession &sess,
                               uint32_t option1,
                               uint32_t option2,
                               uint32_t max_rep_ct) { return UNC_RC_SUCCESS; }

    UncRespCode PerformSyntaxValidation(OdbcmConnectionHandler *db_conn,
                                           void* key_struct,
                                           void* val_struct,
                                           uint32_t operation,
                                           uint32_t data_type) {
      return UNC_RC_SUCCESS;
    }

    UncRespCode PerformSemanticValidation(OdbcmConnectionHandler *db_conn,
                                             void* key_struct,
                                             void* val_struct,
                                             uint32_t operation,
                                             uint32_t data_type) {
      return UNC_RC_SUCCESS;
    }

    void PopulateDBSchemaForKtTable(OdbcmConnectionHandler *db_conn,
        DBTableSchema &kt_dbtableschema,
        void* key_struct,
        void* val_struct,
        uint8_t operation_type,
        uint32_t data_type,
        uint32_t option1,
        uint32_t option2,
        vector<ODBCMOperator> &vect_key_operations,
        void* &old_value_struct,
        CsRowStatus row_status,
        pfc_bool_t is_filtering,
        pfc_bool_t is_state) {return;}

  private:
    void* getChildKeyStruct(uint32_t child_class);
    Kt_Base* GetChildClassPointer(KtRootChildClass KIndex);
    void FreeKeyStruct(void *key_struct,
                       uint32_t child_class);
};
#endif
