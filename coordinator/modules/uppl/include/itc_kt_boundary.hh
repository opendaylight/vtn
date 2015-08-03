/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   Kt Boundary implementation
 * @file    itc_kt_boundary.hh
 *
 */

#ifndef KT_BOUNDARY_HH
#define KT_BOUNDARY_HH

#include <vector>
#include <string>
#include <map>
#include "itc_kt_base.hh"
#include "physicallayer.hh"
using unc::uppl::ODBCMOperator;

/*@ Boundary Class definition */
class Kt_Boundary: public Kt_Base {
  public:
    Kt_Boundary();

    ~Kt_Boundary();

    UncRespCode Create(OdbcmConnectionHandler *db_conn,
                          uint32_t session_id,
                          uint32_t configuration_id,
                          void* key_struct,
                          void* val_struct,
                          uint32_t data_type,
                          ServerSession & sess);

    UncRespCode CreateKeyInstance(OdbcmConnectionHandler *db_conn,
                                     void* key_struct,
                                     void* val_struct,
                                     uint32_t data_type,
                                     uint32_t key_type);

    UncRespCode Update(OdbcmConnectionHandler *db_conn,
                          uint32_t session_id,
                          uint32_t configuration_id,
                          void* key_struct,
                          void* val_struct,
                          uint32_t data_type,
                          ServerSession & sess);

    UncRespCode ReadInternal(OdbcmConnectionHandler *db_conn,
                                vector<void *> &boundary_key,
                                vector<void *> &boundary_val,
                                uint32_t data_type,
                                uint32_t operation_type);

    UncRespCode Delete(OdbcmConnectionHandler *db_conn,
                          uint32_t session_id,
                          uint32_t configuration_id,
                          void* key_struct,
                          uint32_t data_type,
                          ServerSession & sess);

    UncRespCode ReadBulk(OdbcmConnectionHandler *db_conn,
                            void* key_struct,
                            uint32_t data_type,
                            uint32_t &max_repetition_count,
                            int child_index,
                            pfc_bool_t parent_call,
                            pfc_bool_t is_read_next,
                            ReadRequest *read_req);

    UncRespCode PerformSyntaxValidation(OdbcmConnectionHandler *db_conn,
                                           void* key_struct,
                                           void* val_struct,
                                           uint32_t operation,
                                           uint32_t data_type);

    UncRespCode PerformSemanticValidation(OdbcmConnectionHandler *db_conn,
                                             void* key_struct,
                                             void* val_struct,
                                             uint32_t operation,
                                             uint32_t data_type);

    UncRespCode GetModifiedRows(OdbcmConnectionHandler *db_conn,
                                   vector<void *> &key_struct,
                                   CsRowStatus row_status);

    UncRespCode IsKeyExists(OdbcmConnectionHandler *db_conn,
                               unc_keytype_datatype_t data_type,
                               const vector<string>& key_values);

    UncRespCode HandleOperStatus(
        OdbcmConnectionHandler *db_conn,
        uint32_t data_type,
        void *key_struct,
        void *value_struct,
        vector<OperStatusHolder> &ref_oper_status);

    void Fill_Attr_Syntax_Map();

    // Used by KT_CONTROLLER
    pfc_bool_t IsBoundaryReferred(OdbcmConnectionHandler *db_conn,
                                  unc_key_type_t keytype,
                                  void *key_struct,
                                  uint32_t data_type);
    UpplBoundaryOperStatus getBoundaryInputOperStatus(
        OdbcmConnectionHandler *db_conn,
        uint32_t data_type,
        string controller_name,
        string domain_name,
        string logical_port_id,
        vector<OperStatusHolder> &ref_oper_status);

    UncRespCode SendSemanticRequestToUPLL(void* key_struct,
                                             uint32_t data_type);

  private:
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
        pfc_bool_t is_state);

    void FillBoundaryValueStructure(OdbcmConnectionHandler *db_conn,
        DBTableSchema &kt_boundary_dbtableschema,
        vector<key_boundary_t> &vect_obj_key_boundary,
        vector<val_boundary_st_t> &vect_obj_val_boundary,
        uint32_t &max_rep_ct);

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
                               uint32_t max_rep_ct);

    UncRespCode ReadBoundaryValFromDB(OdbcmConnectionHandler *db_conn,
        void* key_struct,
        void* val_struct,
        uint32_t data_type,
        uint32_t operation_type,
        uint32_t &max_repetition_count,
        vector<key_boundary_t> &vect_key_boundary,
        vector<val_boundary_st_t> &vect_val_boundary_st);

    UncRespCode ReadBulkInternal(OdbcmConnectionHandler *db_conn,
        void* key_struct,
        void* val_struct,
        uint32_t data_type,
        uint32_t max_rep_ct,
        vector<key_boundary_t> &vect_key_boundary,
        vector<val_boundary_st_t> &vect_val_boundary);

    UncRespCode SetOperStatus(OdbcmConnectionHandler *db_conn,
                                 uint32_t data_type,
                                 void *key_struct,
                                 void* val_struct,
                                 UpplBoundaryOperStatus oper_status);
    UncRespCode GetBoundaryValidFlag(OdbcmConnectionHandler *db_conn,
        void *key_struct,
        val_boundary_st_t &val_boundary_valid_st,
        uint32_t data_type);
    void FrameValidValue(string attr_value,
                         val_boundary_st &obj_val_boundary_st,
                         val_boundary_t &obj_val_boundary);
    void FrameCsAttrValue(string attr_value,
                          val_boundary_t &obj_val_boundary);
    UncRespCode ValidateSiblingFiltering(unsigned int ctr1_valid_val,
                                            unsigned int ctr2_valid_val,
                                            unsigned int dmn1_valid_val,
                                            unsigned int dmn2_valid_val);
    UncRespCode GetOperStatus(OdbcmConnectionHandler *db_conn,
                                 uint32_t data_type,
                                 void* key_struct,
                                 uint8_t &oper_status);
    UncRespCode SendOperStatusNotification(key_boundary_t bdry_key,
                                              uint8_t old_oper_st,
                                              uint8_t new_oper_st);
    UncRespCode GetAllBoundaryOperStatus(OdbcmConnectionHandler *db_conn,
                                            string controller_name,
                                            string domain_name,
                                            string logical_port_id,
                                            map<string, uint8_t> &bdry_notfn,
                                            uint32_t data_type);
    UncRespCode CheckBoundaryExistence(
        OdbcmConnectionHandler *db_conn,
        void *key_struct,
        void *val_struct,
        uint32_t data_type);
};

#endif

