/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   KT Domain implementation
 * @file    itc_kt_ctr_domain.hh
 *
 */

#ifndef KT_CTR_DOMAIN_H
#define KT_CTR_DOMAIN_H

#include <string>
#include <vector>
#include "physicallayer.hh"
#include "itc_kt_state_base.hh"

#define KT_CTR_DOMAIN_CHILD_COUNT 1

using unc::uppl::ODBCMOperator;

typedef enum {
  KIdxLogicalPort = 0
}KtDomainChildClass;

class Kt_Ctr_Domain : public Kt_State_Base {
  private:
    Kt_Base *child[KT_CTR_DOMAIN_CHILD_COUNT];

  public:
    Kt_Ctr_Domain();
    ~Kt_Ctr_Domain();

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
                                     uint32_t key_type);

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
                                vector<void *> &key_struct,
                                vector<void *> &val_struct,
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

    UncRespCode HandleDriverAlarms(OdbcmConnectionHandler *db_conn,
                                      uint32_t data_type,
                                      uint32_t alarm_type,
                                      uint32_t oper_type,
                                      void* key_struct,
                                      void* val_struct);

    UncRespCode InvokeBoundaryNotifyOperStatus(
        OdbcmConnectionHandler *db_conn,
        uint32_t data_type,
        void *key_struct);

    UncRespCode IsKeyExists(OdbcmConnectionHandler *db_conn,
                               unc_keytype_datatype_t data_type,
                               const vector<string>& key_values);

    UncRespCode GetModifiedRows(OdbcmConnectionHandler *db_conn,
                                   vector<void *> &key_struct,
                                   CsRowStatus row_status);

    void Fill_Attr_Syntax_Map();
    UncRespCode HandleOperStatus(OdbcmConnectionHandler *db_conn,
                                    uint32_t data_type,
                                    void *key_struct,
                                    void *value_struct);

    UncRespCode GetOperStatus(OdbcmConnectionHandler *db_conn,
                                 uint32_t data_type,
                                 void* key_struct,
                                 uint8_t &oper_status);

    pfc_bool_t CompareValueStruct(void* value_struct1,
                                  void* value_struct2) {
      val_ctr_domain_st_t domain_val1 =
          *(reinterpret_cast<val_ctr_domain_st_t*> (value_struct1));
      val_ctr_domain_st_t domain_val2 =
          *(reinterpret_cast<val_ctr_domain_st_t*> (value_struct2));
      if (domain_val1.domain.type == domain_val2.domain.type &&
          memcmp(domain_val1.domain.description,
                 domain_val2.domain.description,
                 sizeof(domain_val1.domain.description)) == 0 &&
                 domain_val1.domain.cs_row_status ==
                     domain_val2.domain.cs_row_status &&
                     domain_val1.oper_status == domain_val2.oper_status) {
        return PFC_TRUE;
      }
      return PFC_FALSE;
    }

    pfc_bool_t CompareKeyStruct(void* key_struct1,
                                void* key_struct2) {
      key_ctr_domain_t domain_key1 =
          *(reinterpret_cast<key_ctr_domain_t*> (key_struct1));
      key_ctr_domain_t domain_key2 =
          *(reinterpret_cast<key_ctr_domain_t*> (key_struct2));
      if (memcmp(domain_key1.ctr_key.controller_name,
                 domain_key2.ctr_key.controller_name,
                 sizeof(domain_key1.ctr_key.controller_name)) == 0 &&
                 memcmp(domain_key1.domain_name,
                        domain_key2.domain_name,
                        sizeof(domain_key1.domain_name)) == 0) {
        return PFC_TRUE;
      }
      return PFC_FALSE;
    }


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

    void FillDomainValueStructure(OdbcmConnectionHandler *db_conn,
        DBTableSchema &kt_ctr_domain_dbtableschema,
        vector<val_ctr_domain_st> &vect_obj_val_ctr_domain_st,
        uint32_t &max_rep_ct,
        uint32_t operation_type,
        vector<key_ctr_domain> &domain_id);

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

    UncRespCode ReadDomainValFromDB(OdbcmConnectionHandler *db_conn,
        void* key_struct,
        void* val_struct,
        uint32_t data_type,
        uint32_t operation_type,
        uint32_t &max_rep_ct,
        vector<val_ctr_domain_st> &vect_val_ctr_domain_st,
        vector<key_ctr_domain> &domain_id);

    UncRespCode ReadBulkInternal(OdbcmConnectionHandler *db_conn,
        void* key_struct,
        uint32_t data_type,
        uint32_t max_rep_ct,
        vector<val_ctr_domain_st> &vect_val_ctr_domain,
        vector<key_ctr_domain> &vect_domain_id);

    void* getChildKeyStruct(int child_class,
                            string domain_name,
                            string controller_name);

    Kt_Base* GetChildClassPointer(KtDomainChildClass KIndex);
    UncRespCode SetOperStatus(OdbcmConnectionHandler *db_conn,
                                 uint32_t data_type,
                                 void* key_struct,
                                 UpplDomainOperStatus oper_status);
    void FreeChildKeyStruct(int child_class,
                            void *key_struct);
    UncRespCode GetCtrDomainValidFlag(OdbcmConnectionHandler *db_conn,
        void *key_struct,
        val_ctr_domain_st_t &val_ctr_domain_valid_st,
        uint32_t data_type);
    void FrameValidValue(string attr_value,
                         val_ctr_domain_st &obj_val_ctr_domain_st,
                         val_ctr_domain_t &obj_val_ctr_domain);
    void FrameCsAttrValue(string attr_value,
                          val_ctr_domain_t &obj_val_ctr_domain);
};
#endif
