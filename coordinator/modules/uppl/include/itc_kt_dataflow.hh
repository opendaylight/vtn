/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   KT Dataflow implementation
 * @file    itc_kt_dataflow.hh
 *
 */

#ifndef KT_DATAFLOW_HH
#define KT_DATAFLOW_HH

#include <uncxx/dataflow.hh>
#include <vector>
#include <map>
#include <list>
#include <string>
#include "physicallayer.hh"
#include "itc_kt_base.hh"
#include "odbcm_utils.hh"

#define KT_DATAFLOW_OPER_STATUS_REF 1

using std::vector;
using std::map;
using std::multimap;
using unc::uppl::ODBCMOperator;
using unc::dataflow::DataflowUtil;
using unc::dataflow::DataflowCmn;
using unc::dataflow::DataflowDetail;
using unc::dataflow::kidx_val_df_data_flow_cmn;

typedef enum {
  UPPL_LEFT_PART = 0,
  UPPL_RIGHT_PART
}UpplBoundaryTblSection;

#define PFC_DEF_TIMEOUT 3600

#define UPPL_COMPARE_STRUCT(struct1, struct2, status) \
  if ((!strcmp((const char*)(struct1).controller_name, \
             (const char*) (struct2).controller_name)) && \
    (!strcmp((const char*)(struct1).domain_name, \
            (const char*)(struct2).domain_name)) && \
    (!strcmp((const char*)(struct1).lp_str.switch_id, \
            (const char*)(struct2).lp_str.switch_id)) && \
    (!strcmp((const char*)(struct1).lp_str.port_id, \
            (const char*)(struct2).lp_str.port_id))) \
     status = true;

#define UPPL_COMPARE_SW_SD_TP_STRUCT(struct1, struct2, status) \
  if (((!strcmp((const char*)(struct1).controller_name, \
             (const char*) (struct2).controller_name)) && \
    (!strcmp((const char*)(struct1).domain_name, \
            (const char*)(struct2).domain_name)) && \
    (!strcmp((const char*)(struct1).lp_str.switch_id, \
            (const char*)(struct2).lp_str.switch_id)))) \
     status = true;

/* @ Controller Class definition */
class Kt_Dataflow: public Kt_Base {
  public:
    Kt_Dataflow();

    ~Kt_Dataflow();


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

    UncRespCode getDomainType(OdbcmConnectionHandler *db_conn,
                                             void* key_struct,
                                             void* val_struct,
                                             uint32_t data_type,
                                             UpplDomainType &domain_type);

    void Fill_Attr_Syntax_Map();

    multimap<string, boundary_val>* get_boundary_map() {
      return &boundary_map_;
    }

    map<string, lp_struct>* get_LP_map() {
      return &LP_map_;
    }

    multimap<string, lmp_struct>* get_LMP_map() {
      return &LMP_map_;
    }

    multimap<string, port_struct>* get_PP_map() {
      return &PP_map_;
    }

    map<string, int>* get_count_map() {
      return &count_map_;
    }

    map<string, uint32_t>* get_is_validated_map() {
      return &is_validated_map_;
    }

    vector<boundary_record>* get_boundary_tbl_vect() {
      return &boundary_tbl_vect_;
    }


    UncRespCode traversePFC(OdbcmConnectionHandler *db_conn,
                                          uint32_t session_id,
                                          uint32_t configuration_id,
                                          void* key_struct,
                                          ServerSession &sess,
                                          bool is_head_node,
                                          DataflowCmn *parentnode,
                                          DataflowCmn *lastPfcNode,
                                          string &ingress_bdry_id,
                                          uint32_t option2);

    UncRespCode traverseVNP(OdbcmConnectionHandler *db_conn,
                                          uint32_t session_id,
                                          uint32_t configuration_id,
                                          void* key_struct,
                                          ServerSession &sess,
                                          bool is_head_node,
                                          DataflowCmn *parentnode,
                                          DataflowCmn *lastPfcNode,
                                          string &ingress_bdry_id);

    UncRespCode traverseUNKNOWN(OdbcmConnectionHandler *db_conn,
                                          uint32_t session_id,
                                          uint32_t configuration_id,
                                          void* key_struct,
                                          ServerSession &sess,
                                          bool is_head_node,
                                          DataflowCmn *parentnode,
                                          DataflowCmn *lastPfcNode,
                                          string &ingress_bdry_id);

  protected:
    DataflowUtil df_util_;

    uint32_t max_dataflow_traverse_count_;

    UncRespCode checkFlowLimitAndTraverse(OdbcmConnectionHandler *db_conn,
                                         uint32_t session_id,
                                         uint32_t configuration_id,
                                         ServerSession &sess,
                                         void* key_struct,
                                         vector<DataflowCmn*>* node,
                                         bool is_head_node,
                                         string &ingress_bdry_id);

    UncRespCode fill_ctrlr_dom_count_map(OdbcmConnectionHandler *db_conn,
                                         string ctr_name);

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
        pfc_bool_t is_state) {}


    UncRespCode checkBoundaryAndTraverse(OdbcmConnectionHandler *db_conn,
                                          uint32_t session_id,
                                          uint32_t configuration_id,
                                          void* key_struct,
                                          ServerSession &sess,
                                          bool is_head_node,
                                          DataflowCmn *source_node,
                                          DataflowCmn *lastPfcNode,
                                          string &ingress_bdry_id);

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
    UncRespCode FindNeighbourCtr(OdbcmConnectionHandler *db_conn,
                                    DataflowCmn *lastPfcNode,
                                    boundary_val *neighbour_ctr_key,
                                    list<boundary_val> &found_nbrs,
                                    string &ingress_bdry_id);

    UncRespCode PrepareBoundaryMap(OdbcmConnectionHandler *db_conn);

    UncRespCode fill_boundary_map(OdbcmConnectionHandler *db_conn);

    UncRespCode getBoundaryPorts(DataflowCmn *lastPfcNode,
                                         boundary_val *neighbour_ctr_key,
                                         list<boundary_val> &found_nbrs,
                                         string &ingress_bdry_id);

    UncRespCode PrepareCollectiveLPMap(OdbcmConnectionHandler *db_conn,
                                             string ctr_name, string dom_name);

    UncRespCode PrepareLMPMap(OdbcmConnectionHandler *db_conn,
                                 key_logical_port_t* key_lp);

    UncRespCode PreparePPMap(OdbcmConnectionHandler *db_conn,
                                 key_port_t* key_pt, string pp_map_key);

    UncRespCode update_boundary_tbl_vect(string lp_map_key,
                                      uint16_t boundary_iter_pos,
                                      uint8_t part);
    UncRespCode getkeysfrom_boundary_map(string ctr_name,
                               list<boundary_val> &found_keys,
                               list<boundary_val> &found_vals,
                               string ingress_domain_name,
                               uint8_t ctr_type);

    UncDataflowReason CreateDfCmnNodeForNonPfc(OdbcmConnectionHandler *db_conn,
                                  DataflowDetail *df_segment,
                                  DataflowCmn *source_node,
                                  DataflowCmn *df_cmn,
                                  boundary_val *ingress_obj_bval,
                                  boundary_val &egress_obj_bval,
                                  bool is_egress,
                                  UncRespCode &err_code);
    multimap<string, boundary_val> boundary_map_;

    map<string, lp_struct> LP_map_;

    multimap<string, lmp_struct> LMP_map_;

    multimap<string, port_struct> PP_map_;

    map<string, int> count_map_;

    map<string, uint32_t> is_validated_map_;

    vector<boundary_record> boundary_tbl_vect_;
};
#endif
