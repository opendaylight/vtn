/*
 * Copyright (c) 2012-2013 NEC Corporation
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
  private:
    Kt_Base* parent;

  public:
    Kt_Boundary();

    ~Kt_Boundary();

    UpplReturnCode Create(uint32_t session_id,
                          uint32_t configuration_id,
                          void* key_struct,
                          void* val_struct,
                          uint32_t data_type,
                          ServerSession & sess);

    UpplReturnCode CreateKeyInstance(void* key_struct,
                                     void* val_struct,
                                     uint32_t data_type,
                                     uint32_t key_type);

    UpplReturnCode Update(uint32_t session_id,
                          uint32_t configuration_id,
                          void* key_struct,
                          void* val_struct,
                          uint32_t data_type,
                          ServerSession & sess);

    UpplReturnCode ReadInternal(vector<void *> &boundary_key,
                                vector<void *> &boundary_val,
                                uint32_t data_type,
                                uint32_t operation_type);

    UpplReturnCode Delete(uint32_t session_id,
                          uint32_t configuration_id,
                          void* key_struct,
                          uint32_t data_type,
                          ServerSession & sess);

    UpplReturnCode ReadBulk(void* key_struct,
                            uint32_t data_type,
                            uint32_t option1,
                            uint32_t option2,
                            uint32_t &max_repetition_count,
                            int child_index,
                            pfc_bool_t parent_call,
                            pfc_bool_t is_read_next);

    UpplReturnCode PerformSyntaxValidation(void* key_struct,
                                           void* val_struct,
                                           uint32_t operation,
                                           uint32_t data_type);

    UpplReturnCode PerformSemanticValidation(void* key_struct,
                                             void* val_struct,
                                             uint32_t operation,
                                             uint32_t data_type);

    UpplReturnCode GetModifiedRows(vector<void *> &key_struct,
                                   CsRowStatus row_status);

    UpplReturnCode IsKeyExists(unc_keytype_datatype_t data_type,
                               vector<string> key_values);

    UpplReturnCode HandleOperStatus(uint32_t data_type,
                                    void *key_struct,
                                    void *value_struct);

    void Fill_Attr_Syntax_Map();

    // Used by KT_CONTROLLER
    pfc_bool_t IsBoundaryReferred(unc_key_type_t keytype,
                                  void *key_struct,
                                  uint32_t data_type);
    UpplBoundaryOperStatus getBoundaryInputOperStatus(uint32_t data_type,
                                                      string controller_name,
                                                      string domain_name,
                                                      string logical_port_id);

  private:
    void PopulateDBSchemaForKtTable(
        DBTableSchema &kt_dbtableschema,
        void* key_struct,
        void* val_struct,
        uint8_t operation_type,
        uint32_t option1,
        uint32_t option2,
        vector<ODBCMOperator> &vect_key_operations,
        void* &old_value_struct,
        CsRowStatus row_status= NOTAPPLIED,
        pfc_bool_t is_filtering= false,
        pfc_bool_t is_state= PFC_FALSE);

    void FillBoundaryValueStructure(
        DBTableSchema &kt_boundary_dbtableschema,
        vector<key_boundary_t> &vect_obj_key_boundary,
        vector<val_boundary_st_t> &vect_obj_val_boundary,
        uint32_t &max_rep_ct);

    UpplReturnCode PerformRead(uint32_t session_id,
                               uint32_t configuration_id,
                               void* key_struct,
                               void* val_struct,
                               uint32_t data_type,
                               uint32_t operation_type,
                               ServerSession &sess,
                               uint32_t option1,
                               uint32_t option2,
                               uint32_t max_rep_ct);

    UpplReturnCode ReadBoundaryValFromDB(
        void* key_struct,
        void* val_struct,
        uint32_t data_type,
        uint32_t operation_type,
        uint32_t &max_repetition_count,
        vector<key_boundary_t> &vect_key_boundary,
        vector<val_boundary_t> &vect_val_boundary,
        vector<val_boundary_st_t> &vect_val_boundary_st,
        pfc_bool_t is_state = PFC_FALSE);

    UpplReturnCode SendSemanticRequestToUPLL(void* key_struct,
                                             uint32_t data_type);

    UpplReturnCode ReadBulkInternal(
        void* key_struct,
        void* val_struct,
        uint32_t data_type,
        uint32_t max_rep_ct,
        vector<key_boundary_t> &vect_key_boundary,
        vector<val_boundary_st_t> &vect_val_boundary);

    UpplReturnCode SetOperStatus(uint32_t data_type,
                                 void *key_struct,
                                 void* val_struct,
                                 UpplBoundaryOperStatus oper_status);
    UpplReturnCode GetBoundaryValidFlag(
        void *key_struct,
        val_boundary_st_t &val_boundary_valid_st);
    void FrameValidValue(string attr_value,
                         val_boundary_st &obj_val_boundary_st,
                         val_boundary_t &obj_val_boundary);
    void FrameCsAttrValue(string attr_value,
                          val_boundary_t &obj_val_boundary);
    UpplReturnCode ValidateSiblingFiltering(unsigned int ctr1_valid_val,
                                            unsigned int ctr2_valid_val,
                                            unsigned int dmn1_valid_val,
                                            unsigned int dmn2_valid_val);
    UpplReturnCode GetOperStatus(uint32_t data_type,
                                 void* key_struct,
                                 uint8_t &oper_status);
    UpplReturnCode SendOperStatusNotification(key_boundary_t bdry_key,
                                              uint8_t old_oper_st,
                                              uint8_t new_oper_st);
    UpplReturnCode GetAllBoundaryOperStatus(string controller_name,
                                            string domain_name,
                                            string logical_port_id,
                                            map<string, uint8_t> &bdry_notfn,
                                            uint32_t data_type);
};

#endif

