/*
 * Copyright (c) 2012-2015 NEC Corporation
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

typedef enum {
      KtSwitch = 0
}KtControllerOperStatusRef;


/* @ Controller Class definition */
class Kt_Controller: public Kt_Base {
  private:
    Kt_Base *child[KT_CONTROLLER_CHILD_COUNT];

  public:
    Kt_Controller();

    ~Kt_Controller();


    UncRespCode Create(OdbcmConnectionHandler *db_conn,
                          uint32_t session_id,
                          uint32_t configuration_id,
                          void* key_struct,
                          void* val_struct,
                          uint32_t data_type,
                          ServerSession &sess);

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
                          ServerSession &sess);

    UncRespCode UpdateKeyInstance(OdbcmConnectionHandler *db_conn,
                                     void* key_struct,
                                     void* val_struct,
                                     uint32_t data_type,
                                     uint32_t key_type,
                                    pfc_bool_t commit_ver_flag);

    UncRespCode Delete(OdbcmConnectionHandler *db_conn,
                          uint32_t session_id,
                          uint32_t configuration_id,
                          void* key_struct,
                          uint32_t data_type,
                          ServerSession &sess);
    UncRespCode DeleteKeyInstance(OdbcmConnectionHandler *db_conn,
                                     void* key_struct,
                                     uint32_t data_type,
                                     uint32_t key_type);

    UncRespCode ReadInternal(OdbcmConnectionHandler *db_conn,
                                vector<void *> &ctr_key,
                                vector<void *> &ctr_val,
                                uint32_t data_type,
                                uint32_t operation_type);

    UncRespCode ReadBulk(OdbcmConnectionHandler *db_conn,
                            void* key_struct,
                            uint32_t data_type,
                            uint32_t &max_rep_ct,
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

    UncRespCode HandleDriverEvents(OdbcmConnectionHandler *db_conn,
                                      void* key_struct,
                                      uint32_t oper_type,
                                      uint32_t data_type,
                                      void* old_val_struct,
                                      void* new_val_struct,
                                      pfc_bool_t is_events_done);

    UncRespCode HandleDriverAlarms(OdbcmConnectionHandler *db_conn,
                                      uint32_t data_type,
                                      uint32_t alarm_type,
                                      uint32_t oper_type,
                                      void* key_struct,
                                      void* val_struct);

    UncRespCode IsKeyExists(OdbcmConnectionHandler *db_conn,
                               unc_keytype_datatype_t data_type,
                               const vector<string>& key_values);

    UncRespCode GetModifiedRows(OdbcmConnectionHandler *db_conn,
                                   vector<void *> &key_struct,
                                   CsRowStatus row_status);

    UncRespCode ValidateUnknownCtrlrScalability(
        OdbcmConnectionHandler *db_conn,
        void *key_struct,
        uint8_t type,
        uint32_t data_type);

    void Fill_Attr_Syntax_Map();
    UncRespCode HandleOperStatus(OdbcmConnectionHandler *db_conn,
                                    uint32_t data_type,
                                    void *key_struct,
                                    void *value_struct);

    UncRespCode HandleOperStatus(OdbcmConnectionHandler *db_conn,
                                    uint32_t data_type,
                                    void *key_struct,
                                    void *value_struct,
                                    bool bIsInternal);

    UncRespCode NotifyOperStatus(
        OdbcmConnectionHandler *db_conn,
        uint32_t data_type,
        void *key_struct,
        void *value_struct,
        vector<OperStatusHolder> &ref_oper_status);

    UncRespCode GetOperStatus(OdbcmConnectionHandler *db_conn,
                                 uint32_t data_type,
                                 void* key_struct,
                                 uint8_t &oper_status);

    UncRespCode SetOperStatus(OdbcmConnectionHandler *db_conn,
                                 uint32_t data_type,
                                 void* key_struct,
                                 uint8_t oper_status);

    UncRespCode SendUpdatedControllerInfoToUPLL(
        uint32_t data_type,
        uint32_t operation_type,
        uint32_t key_type,
        void* key_struct,
        void* val_struct);
    UncRespCode ValidateControllerCount(OdbcmConnectionHandler *db_conn,
                               void *key_struct,
                               void *val_struct,
                               uint32_t data_type);
    UncRespCode SendOperStatusNotification(key_ctr_t ctr_key,
                                              uint8_t old_oper_st,
                                              uint8_t new_oper_st);
    UncRespCode CheckAuditFlag(OdbcmConnectionHandler *db_conn,
                               key_ctr_t key_ctr,
                               uint8_t &audit_flag);

    UncRespCode SetActualVersion(OdbcmConnectionHandler *db_conn,
                                    void* key_struct, string actual_version,
                                    uint32_t data_type,
                                    uint32_t valid_flag);
    UncRespCode SetActualControllerId(OdbcmConnectionHandler *db_conn,
                                    void* key_struct, string actual_id,
                                    uint32_t data_type, uint32_t valid_flag);
    UncRespCode CheckDuplicateControllerId(string ctr_id, string ctr_name,
                                    OdbcmConnectionHandler *db_conn);
    void FillControllerCommitVerStructure(
            OdbcmConnectionHandler *db_conn,
            DBTableSchema &kt_controller_dbtableschema,
            vector<val_ctr_commit_ver_t> &vect_obj_val_ctr,
            uint32_t &max_rep_ct,
            uint32_t operation_type,
            vector<string> &controller_id);
    void FrameCVValidValue(string attr_value,
                            val_ctr_commit_ver_t &obj_cv_ctr);
    UncRespCode ReadCtrCommitVerFromDB(
            OdbcmConnectionHandler *db_conn,
            void* key_struct,
            void* val_struct,
            uint32_t data_type,
            uint32_t operation_type,
            uint32_t &max_rep_ct,
            vector<val_ctr_commit_ver_t> &vect_val_ctr_cv,
            vector<string> &controller_id);

    UncRespCode SendSemanticRequestToUPLL(void* key_struct,
                                             uint32_t data_type);
    UncRespCode ClearImportAndStateEntries(
           OdbcmConnectionHandler *db_conn, string controller_name);

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

    void PopulateDBSchemaForCommitVersion(OdbcmConnectionHandler *db_conn,
        DBTableSchema &kt_dbtableschema,
        void* key_struct,
        void* val_struct,
        uint8_t operation_type);

    void FillControllerValueStructure(OdbcmConnectionHandler *db_conn,
        DBTableSchema &kt_controller_dbtableschema,
        vector<val_ctr_st_t> &vect_obj_val_ctr,
        uint32_t &max_rep_ct,
        uint32_t operation_type,
        vector<string> &controller_id);

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

    UncRespCode ReadCtrValFromDB(OdbcmConnectionHandler *db_conn,
                                    void* key_struct,
                                    void* val_struct,
                                    uint32_t data_type,
                                    uint32_t operation_type,
                                    uint32_t &max_rep_ct,
                                    vector<val_ctr_st_t> &vect_val_ctr_st,
                                    vector<string> &controller_id);

    UncRespCode ReadBulkInternal(OdbcmConnectionHandler *db_conn,
                                    void* key_struct,
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
    UncRespCode GetCtrValidFlag(OdbcmConnectionHandler *db_conn,
                                   void *key_struct,
                                   val_ctr_st_t &val_ctr_st,
                                   uint32_t data_type);
    void FrameValidValue(string attr_value, val_ctr_st &obj_val_ctr_st,
                         val_ctr_t &obj_val_ctr);
    void FrameCsAttrValue(string attr_value, val_ctr_t &obj_val_ctr);
    UncRespCode ValidateTypeIpAddress(OdbcmConnectionHandler *db_conn,
                                         void *key_struct,
                                         void *val_struct,
                                         uint32_t data_type,
                                         uint32_t ctrl_type);
    UncRespCode ValidateControllerType(OdbcmConnectionHandler *db_conn,
        uint32_t operation,
        uint32_t data_type,
        unc_keytype_ctrtype_t ctr_type,
        UncRespCode ctr_type_code,
        val_ctr *val_ctr);
    UncRespCode ValidateControllerVersion(OdbcmConnectionHandler *db_conn,
        uint32_t operation,
        uint32_t data_type,
        unc_keytype_ctrtype_t ctr_type,
        UncRespCode ctr_type_code,
        val_ctr *val_ctr);
    UncRespCode ValidateControllerDescription(
        OdbcmConnectionHandler *db_conn,
        uint32_t operation,
        uint32_t data_type,
        unc_keytype_ctrtype_t ctr_type,
        UncRespCode ctr_type_code,
        val_ctr *val_ctr);
    UncRespCode ValidateControllerIpAddress(
        OdbcmConnectionHandler *db_conn,
        uint32_t operation,
        uint32_t data_type,
        unc_keytype_ctrtype_t ctr_type,
        UncRespCode ctr_type_code,
        void *key_struct,
        void *val_struct);
    UncRespCode ValidateControllerUser(
        OdbcmConnectionHandler *db_conn,
        uint32_t operation,
        uint32_t data_type,
        unc_keytype_ctrtype_t ctr_type,
        UncRespCode ctr_type_code,
        val_ctr *val_ctr);
    UncRespCode ValidateControllerPassword(
        OdbcmConnectionHandler *db_conn,
        uint32_t operation,
        uint32_t data_type,
        unc_keytype_ctrtype_t ctr_type,
        UncRespCode ctr_type_code,
        val_ctr *val_ctr);
    UncRespCode ValidateControllerEnableAudit(
        OdbcmConnectionHandler *db_conn,
        uint32_t operation,
        uint32_t data_type,
        unc_keytype_ctrtype_t ctr_type,
        UncRespCode ctr_type_code,
        val_ctr *val_ctr);
    UncRespCode ValidateControllerPort(
        OdbcmConnectionHandler *db_conn,
        uint32_t operation,
        uint32_t data_type,
        unc_keytype_ctrtype_t ctr_type,
        UncRespCode ctr_type_code,
        val_ctr *val_ctr);
    UncRespCode CheckIpAndClearStateDB(OdbcmConnectionHandler *db_conn,
                                          void *key_struct);
    UncRespCode CheckSameIp(OdbcmConnectionHandler *db_conn,
                               void *key_struct,
                               void *val_struct,
                               uint32_t data_type);

  private:
    UncRespCode ValKtCtrAttributeSupportCheck(val_ctr_t *obj_val_ctr,
                            const val_ctr_t *db_ctr_val, const uint8_t* attrs,
                            unc_keytype_operation_t op_type);
};
#endif
