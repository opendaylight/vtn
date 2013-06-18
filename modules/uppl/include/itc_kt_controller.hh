/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   KT Controller implementation
 * @file    itc_kt_controller.hh
 *
 */

#ifndef KT_CONTROLLER_HH
#define KT_CONTROLLER_HH

#include <vector>
#include <string>
#include "physicallayer.hh"
#include "itc_kt_base.hh"
#include "odbcm_utils.hh"

#define KT_CONTROLLER_CHILD_COUNT 3
#define KT_CONTROLLER_OPER_STATUS_REF 1

using std::vector;
using unc::uppl::ODBCMOperator;

typedef enum {
  KIdxDomain = 0,
      KIdxSwitch,
      KIdxLink
}KtControllerChildClass;

/* typedef enum {
  KtCtrDomain = 0,
      KtLogicalPort,
      KtSwitch,
      KtPort,
      KtLink,
      KtBoundary
}KtControllerOperStatusRef;*/

typedef enum {
      KtSwitch = 0
}KtControllerOperStatusRef;


/* @ Controller Class definition */
class Kt_Controller: public Kt_Base {
  private:
    Kt_Base *parent;
    Kt_Base *child[KT_CONTROLLER_CHILD_COUNT];

  public:
    Kt_Controller();

    ~Kt_Controller();


    UpplReturnCode Create(uint32_t session_id,
                          uint32_t configuration_id,
                          void* key_struct,
                          void* val_struct,
                          uint32_t data_type,
                          ServerSession &sess);

    UpplReturnCode CreateKeyInstance(void* key_struct,
                                     void* val_struct,
                                     uint32_t data_type,
                                     uint32_t key_type);

    UpplReturnCode Update(uint32_t session_id,
                          uint32_t configuration_id,
                          void* key_struct,
                          void* val_struct,
                          uint32_t data_type,
                          ServerSession &sess);

    UpplReturnCode UpdateKeyInstance(void* key_struct,
                                     void* val_struct,
                                     uint32_t data_type,
                                     uint32_t key_type);

    UpplReturnCode Delete(uint32_t session_id,
                          uint32_t configuration_id,
                          void* key_struct,
                          uint32_t data_type,
                          ServerSession &sess);
    UpplReturnCode DeleteKeyInstance(void* key_struct,
                                     uint32_t data_type,
                                     uint32_t key_type);

    UpplReturnCode ReadInternal(vector<void *> &ctr_key,
                                vector<void *> &ctr_val,
                                uint32_t data_type,
                                uint32_t operation_type);

    UpplReturnCode ReadBulk(void* key_struct,
                            uint32_t data_type,
                            uint32_t option1,
                            uint32_t option2,
                            uint32_t &max_rep_ct,
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

    UpplReturnCode HandleDriverEvents(void* key_struct,
                                      uint32_t oper_type,
                                      uint32_t data_type,
                                      void* old_val_struct,
                                      void* new_val_struct,
                                      pfc_bool_t is_events_done);

    UpplReturnCode HandleDriverAlarms(uint32_t data_type,
                                      uint32_t alarm_type,
                                      uint32_t oper_type,
                                      void* key_struct,
                                      void* val_struct);

    UpplReturnCode IsKeyExists(unc_keytype_datatype_t data_type,
                               vector<string> key_values);

    UpplReturnCode GetModifiedRows(vector<void *> &key_struct,
                                   CsRowStatus row_status);

    UpplReturnCode ValidateCtrlrValueCapability(string version,
                                                uint32_t key_type);

    UpplReturnCode ValidateCtrlrScalability(string version,
                                            uint32_t key_type,
                                            uint32_t data_type);
    UpplReturnCode ValidateUnknownCtrlrScalability(void *key_struct,
                                                   uint8_t type,
                                                   uint32_t data_type);

    void Fill_Attr_Syntax_Map();
    UpplReturnCode HandleOperStatus(uint32_t data_type,
                                    void *key_struct,
                                    void *value_struct);

    UpplReturnCode HandleOperStatus(uint32_t data_type,
                                    void *key_struct,
                                    void *value_struct,
                                    bool bIsInternal);

    UpplReturnCode NotifyOperStatus(uint32_t data_type,
                                    void *key_struct,
                                    void *value_struct);

    UpplReturnCode GetOperStatus(uint32_t data_type,
                                 void* key_struct,
                                 uint8_t &oper_status);

    UpplReturnCode SetOperStatus(uint32_t data_type,
                                 void* key_struct,
                                 uint8_t oper_status);

    UpplReturnCode SendUpdatedControllerInfoToUPLL(
        uint32_t data_type,
        uint32_t operation_type,
        uint32_t key_type,
        void* key_struct,
        void* val_struct);

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

    UpplReturnCode SendSemanticRequestToUPLL(void* key_struct,
                                             uint32_t data_type);

    void FillControllerValueStructure(
        DBTableSchema &kt_controller_dbtableschema,
        vector<val_ctr_st_t> &vect_obj_val_ctr,
        uint32_t &max_rep_ct,
        uint32_t operation_type,
        vector<string> &controller_id);

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

    UpplReturnCode ReadCtrValFromDB(void* key_struct,
                                    void* val_struct,
                                    uint32_t data_type,
                                    uint32_t operation_type,
                                    uint32_t &max_rep_ct,
                                    vector<val_ctr_st_t> &vect_val_ctr_st,
                                    vector<string> &controller_id);

    UpplReturnCode ReadBulkInternal(void* key_struct,
                                    void* value_struct,
                                    uint32_t data_type,
                                    uint32_t max_rep_ct,
                                    vector<val_ctr_st_t> &vect_val_ctr,
                                    vector<string> &vect_ctr_id);

    void* getChildKeyStruct(unsigned int child_class,
                            string controller_name);

    Kt_Base* GetChildClassPointer(KtControllerChildClass KIndex);

    Kt_Base* GetClassPointerAndKey(KtControllerOperStatusRef key_type,
                                   string controller_name, void* &);
    void FreeChildKeyStruct(void* key_struct, unsigned int child_class);
    void FreeKeyStruct(void* key_struct,
                       unsigned int key_type);
    UpplReturnCode GetCtrValidFlag(void *key_struct,
                                   val_ctr_st_t &val_ctr_st);
    UpplReturnCode SetActualVersion(void* key_struct, string actual_version);
    void FrameValidValue(string attr_value, val_ctr_st &obj_val_ctr_st,
                         val_ctr_t &obj_val_ctr);
    void FrameCsAttrValue(string attr_value, val_ctr_t &obj_val_ctr);
    UpplReturnCode ValidateTypeIpAddress(void *key_struct,
                                         void *val_struct,
                                         uint32_t data_type,
                                         uint32_t ctrl_type = UNC_CT_UNKNOWN);
    UpplReturnCode ValidateControllerType(
        uint32_t operation,
        uint32_t data_type,
        unc_keytype_ctrtype_t ctr_type,
        UpplReturnCode ctr_type_code,
        val_ctr *val_ctr);
    UpplReturnCode ValidateControllerVersion(
        uint32_t operation,
        uint32_t data_type,
        unc_keytype_ctrtype_t ctr_type,
        UpplReturnCode ctr_type_code,
        val_ctr *val_ctr);
    UpplReturnCode ValidateControllerDescription(
        uint32_t operation,
        uint32_t data_type,
        unc_keytype_ctrtype_t ctr_type,
        UpplReturnCode ctr_type_code,
        val_ctr *val_ctr);
    UpplReturnCode ValidateControllerIpAddress(
        uint32_t operation,
        uint32_t data_type,
        unc_keytype_ctrtype_t ctr_type,
        UpplReturnCode ctr_type_code,
        void *key_struct,
        void *val_struct);
    UpplReturnCode ValidateControllerUser(
        uint32_t operation,
        uint32_t data_type,
        unc_keytype_ctrtype_t ctr_type,
        UpplReturnCode ctr_type_code,
        val_ctr *val_ctr);
    UpplReturnCode ValidateControllerPassword(
        uint32_t operation,
        uint32_t data_type,
        unc_keytype_ctrtype_t ctr_type,
        UpplReturnCode ctr_type_code,
        val_ctr *val_ctr);
    UpplReturnCode ValidateControllerEnableAudit(
        uint32_t operation,
        uint32_t data_type,
        unc_keytype_ctrtype_t ctr_type,
        UpplReturnCode ctr_type_code,
        val_ctr *val_ctr);
    UpplReturnCode SendOperStatusNotification(key_ctr_t ctr_key,
                                              uint8_t old_oper_st,
                                              uint8_t new_oper_st);
};
#endif
