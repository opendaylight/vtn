/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef MODULES_UPLL_UPLL_DB_QUERY_HH_
#define MODULES_UPLL_UPLL_DB_QUERY_HH_

const char *const create_time_c_flag = "1";
const char *const create_time_u_flag = "0";
//  This file declares the user supplied executable psql queries
//  statements
const char* const QUERY_PP_CAND_REF_COUNT_UPDATE =
      "UPDATE ca_policingprofile_ctrlr_tbl "
      "SET ref_count = ref_count + 1, u_flag = 1 "
      "WHERE policingprofile_name = ? AND ctrlr_name = ?";

const char* const QUERY_PP_IMP_REF_COUNT_UPDATE =
      "UPDATE im_policingprofile_ctrlr_tbl "
      "SET ref_count = ref_count + 1 "
      "WHERE policingprofile_name = ? AND ctrlr_name = ? ";

const char* const QUERY_PP_AUD_REF_COUNT_UPDATE =
      "UPDATE au_policingprofile_ctrlr_tbl "
      "SET ref_count = ref_count + 1 "
      "WHERE policingprofile_name = ? AND ctrlr_name = ? ";

const char* const QUERY_FF_CAND_REF_COUNT_UPDATE =
      "UPDATE ca_flowlist_ctrlr_tbl "
      "SET ref_count = ref_count + 1, u_flag = 1 "
      "where flowlist_name = ? and ctrlr_name = ? ";

const char* const QUERY_FF_IMP_REF_COUNT_UPDATE =
      "UPDATE im_flowlist_ctrlr_tbl "
      "SET ref_count = ref_count + 1 "
      "where flowlist_name = ? and ctrlr_name = ? ";

const char* const QUERY_FF_AUD_REF_COUNT_UPDATE =
      "UPDATE au_flowlist_ctrlr_tbl "
      "SET ref_count = ref_count + 1 "
      "where flowlist_name = ? and ctrlr_name = ? ";

const char* const QUERY_CA_DELETE_FL_IN_FLECTRLTBL =
      "DELETE FROM ca_flowlist_entry_ctrlr_tbl WHERE flowlist_name = ? "
      "AND ctrlr_name = ? ";

const char* const QUERY_CA_UPDATE_C_U_FLAGS_IN_FLE_CTRLRTBL =
      "UPDATE ca_flowlist_entry_ctrlr_tbl  SET u_flag = 1, c_flag = 0 "
      "WHERE EXISTS (SELECT 1 FROM ru_flowlist_entry_ctrlr_tbl WHERE "
       "ca_flowlist_entry_ctrlr_tbl.flowlist_name = ? AND "
       "ca_flowlist_entry_ctrlr_tbl.ctrlr_name = ? AND "
      "ru_flowlist_entry_ctrlr_tbl.flowlist_name = "
       "ca_flowlist_entry_ctrlr_tbl.flowlist_name AND "
      "ru_flowlist_entry_ctrlr_tbl.sequence_num = "
       "ca_flowlist_entry_ctrlr_tbl.sequence_num AND "
      "ru_flowlist_entry_ctrlr_tbl.ctrlr_name = "
       "ca_flowlist_entry_ctrlr_tbl.ctrlr_name) ";

const char* const QUERY_PP_SCRATCH_REF_COUNT_INC =
      "UPDATE ca_pp_scratch_tbl "
      "SET ref_count = ref_count + 1, u_flag = 1 "
      "WHERE policingprofile_name = ? AND ctrlr_name = ?"
      " AND vtn_name = ?";

const char* const QUERY_PP_SCRATCH_REF_COUNT_DEC =
      "UPDATE ca_pp_scratch_tbl "
      "SET ref_count = ref_count - 1, u_flag = 1 "
      "WHERE policingprofile_name = ? AND ctrlr_name = ?"
      " AND vtn_name = ?";

const char* const QUERY_FL_SCRATCH_REF_COUNT_INC =
      "UPDATE ca_fl_scratch_tbl "
      "SET ref_count = ref_count + 1 "
      "WHERE flowlist_name = ? AND ctrlr_name = ?"
      " AND vtn_name = ?";

const char* const QUERY_FL_SCRATCH_REF_COUNT_DEC =
      "UPDATE ca_fl_scratch_tbl "
      "SET ref_count = ref_count - 1 "
      "WHERE flowlist_name = ? AND ctrlr_name = ?"
      " AND vtn_name = ?";

const char* const QUERY_INSERT_REC_PP_SCRATCH_TBL =
      "INSERT INTO ca_pp_scratch_tbl "
      "(policingprofile_name, ctrlr_name, vtn_name, ref_count) "
      "VALUES (?, ?, ?, 1)";

const char* const QUERY_INSERT_REC_DELETE_PP_SCRATCH_TBL =
      "INSERT INTO ca_pp_scratch_tbl "
      "(policingprofile_name, ctrlr_name, vtn_name, ref_count) "
      "VALUES (?, ?, ?, -1)";

const char* const QUERY_INSERT_REC_FL_SCRATCH_TBL =
      "INSERT INTO ca_fl_scratch_tbl "
      "(flowlist_name, ctrlr_name, vtn_name, ref_count) "
      "VALUES (?, ?, ?, 1)";

const char* const QUERY_INSERT_REC_DELETE_FL_SCRATCH_TBL =
      "INSERT INTO ca_fl_scratch_tbl "
      "(flowlist_name, ctrlr_name, vtn_name, ref_count) "
      "VALUES (?, ?, ?, -1)";

const char* const QUERY_READ_NO_VTN_REF_COUNT_PP_SCRATCH_TBL =
      "SELECT ref_count FROM ca_pp_scratch_tbl"
      " WHERE policingprofile_name = ? AND ctrlr_name = ?";

const char* const QUERY_READ_NO_VTN_REF_COUNT_FL_SCRATCH_TBL =
      "SELECT ref_count FROM ca_fl_scratch_tbl"
      " WHERE flowlist_name = ? AND ctrlr_name = ?";

const char* const QUERY_READ_REF_COUNT_PP_SCRATCH_TBL =
      "SELECT ref_count FROM ca_pp_scratch_tbl"
      " WHERE policingprofile_name = ? AND ctrlr_name = ?"
      " AND vtn_name = ?";

const char* const QUERY_READ_REF_COUNT_FL_SCRATCH_TBL =
      "SELECT ref_count FROM ca_fl_scratch_tbl"
      " WHERE flowlist_name = ? AND ctrlr_name = ?"
      " AND vtn_name = ?";

const char* const QUERY_DELETE_ALL_PP_SCRATCH_TBL =
      "DELETE FROM ca_pp_scratch_tbl";

const char* const QUERY_DELETE_VTN_PP_SCRATCH_TBL =
      "DELETE FROM ca_pp_scratch_tbl WHERE "
      "vtn_name = ?";

const char* const QUERY_DELETE_ALL_FL_SCRATCH_TBL =
      "DELETE FROM ca_fl_scratch_tbl";

const char* const QUERY_DELETE_VTN_FL_SCRATCH_TBL =
      "DELETE FROM ca_fl_scratch_tbl WHERE "
      "vtn_name = ?";

const char* const QUERY_READ_DISTINCT_PPE_MAIN_TBL =
      "SELECT DISTINCT policingprofile_name FROM "
      "ca_policingprofile_entry_tbl WHERE "
      "flowlist = ?";

const char* const QUERY_SUM_PP_CTRLR_REF_COUNT =
      "SELECT SUM(ref_count) from ca_policingprofile_ctrlr_tbl "
      "WHERE policingprofile_name = ?";

const char* const QUERY_SUM_PP_SCRATCH_REF_COUNT =
      "SELECT SUM(ref_count) from  ca_pp_scratch_tbl "
      "WHERE policingprofile_name = ?";

const char* const QUERY_SUM_FL_CTRLR_REF_COUNT =
      "SELECT SUM(ref_count) from ca_flowlist_ctrlr_tbl "
      "WHERE flowlist_name = ?";

const char* const QUERY_SUM_FL_SCRATCH_REF_COUNT =
      "SELECT SUM(ref_count) from  ca_fl_scratch_tbl "
      "WHERE flowlist_name = ?";

const char* const QUERY_READ_ALL_PP_SCRATCH_TBL =
      "SELECT policingprofile_name, ctrlr_name, ref_count from "
      " ca_pp_scratch_tbl";

const char* const QUERY_READ_ALL_PP_SCRATCH_TBL_VTN =
      "SELECT policingprofile_name, ctrlr_name, ref_count from "
      " ca_pp_scratch_tbl WHERE vtn_name = ?";


const char* const QUERY_READ_ALL_FL_SCRATCH_TBL =
      "SELECT flowlist_name, ctrlr_name, ref_count from "
      " ca_fl_scratch_tbl";

const char* const QUERY_READ_ALL_FL_SCRATCH_TBL_VTN =
      "SELECT flowlist_name, ctrlr_name, ref_count from "
      " ca_fl_scratch_tbl WHERE vtn_name = ?";

const char* const QUERY_SUM_PP_SCRATCH_REF_COUNT_WITH_CTRLR =
      "SELECT SUM(ref_count) from  ca_pp_scratch_tbl "
      "WHERE policingprofile_name = ? AND "
      "ctrlr_name = ?";

const char* const QUERY_SUM_FL_SCRATCH_REF_COUNT_WITH_CTRLR =
      "SELECT SUM(ref_count) from  ca_fl_scratch_tbl "
      "WHERE flowlist_name = ? AND "
      "ctrlr_name = ?";
const char* const QUERY_IMPORT_READ_NOTEQUAL_VBRIDGE_NAME =
      "SELECT vbridge_name from im_vbr_tbl WHERE vtn_name = ? AND "
      "vbridge_name != ? AND ctrlr_name = ? AND domain_id = ?";
const char* const QUERY_CANDIDATE_READ_NOTEQUAL_VBRIDGE_NAME =
      "SELECT vbridge_name from ca_vbr_tbl WHERE vtn_name = ? AND "
      "vbridge_name != ? AND ctrlr_name = ? AND domain_id = ?";

const char* const QUERY_READ_MULTIPLE_VBR_PORTMAP =
              "SELECT DISTINCT ctrlr_name, domain_id from ca_vbr_portmap_tbl "
                       "WHERE vtn_name = ? AND vbridge_name = ? AND"
                       " valid_logical_port_id = ?";

const char* const QUERY_DELETE_ALL_SPD_SCRATCH_TBL =
      "DELETE FROM ca_spd_scratch_tbl";

const char* const QUERY_DELETE_VTN_SPD_SCRATCH_TBL =
      "DELETE FROM ca_spd_scratch_tbl WHERE "
      "vtn_name = ?";

const char* const QUERY_DISTINCT_SPD_SCRATCH_TBL_ENTRY =
      "SELECT DISTINCT unw_name, unw_spine_domain_name from ca_spd_scratch_tbl";

const char* const QUERY_SPD_SCRATCH_TBL_DELETE_INSERT =
      "INSERT INTO ca_spd_scratch_tbl"
      " (unw_name, unw_spine_domain_name, vtn_name, used_count)"
      " VALUES (?, ?, ?, -1)";

const char* const QUERY_SPD_SCRATCH_TBL_CREATE_INSERT =
      "INSERT INTO ca_spd_scratch_tbl"
      " (unw_name, unw_spine_domain_name, vtn_name, used_count)"
      " VALUES (?, ?, ?, 1)";

const char* const QUERY_DELETE_SPD_SCRATCH_TBL_FOR_MULTI_MATCH =
      "DELETE FROM ca_spd_scratch_tbl WHERE"
      " unw_name = ? AND unw_spine_domain_name = ?"
      " AND vtn_name = ?";

const char* const QUERY_READ_VBR_PORTMAP_LABEL =
      "SELECT label from ca_vbr_portmap_tbl WHERE "
      "vtn_name = ? AND valid_logical_port_id = ?";

const char* const QUERY_READ_VBR_PORTMAP_LABEL_IMPORT =
      "SELECT label from im_vbr_portmap_tbl WHERE "
      "vtn_name = ? AND valid_logical_port_id = ?";

const char* const QUERY_READ_AND_SUM_REF_COUNT_FL_SCRATCH_TBL =
      "SELECT flowlist_name, ctrlr_name, SUM(ref_count) from "
      "ca_fl_scratch_tbl GROUP by flowlist_name, ctrlr_name";

const char* const QUERY_READ_FL_SCRATCH_TBL_VTN_MODE =
      "SELECT flowlist_name, ctrlr_name, ref_count from "
      "ca_fl_scratch_tbl WHERE vtn_name = ?";

const char* const QUERY_READ_AND_SUM_REF_COUNT_PP_SCRATCH_TBL =
      "SELECT policingprofile_name, ctrlr_name, SUM(ref_count) from "
      "ca_pp_scratch_tbl GROUP by policingprofile_name, ctrlr_name";

const char* const QUERY_READ_PP_SCRATCH_TBL_VTN_MODE =
      "SELECT policingprofile_name, ctrlr_name, ref_count from "
      "ca_pp_scratch_tbl WHERE vtn_name = ?";

const char* const QUERY_SUM_PP_SCRATCH_REF_COUNT_WITH_PP =
      "SELECT SUM(ref_count) from  ca_pp_scratch_tbl "
      "WHERE policingprofile_name = ?";

const char* const QUERY_DELETE_PP_SCRATCH_TBL_MATCH_PP =
      "DELETE FROM ca_pp_scratch_tbl WHERE "
      "policingprofile_name = ?";

const char* const QUERY_DELETE_FL_SCRATCH_TBL_MATCH_FL =
      "DELETE FROM ca_fl_scratch_tbl WHERE "
      "flowlist_name = ?";
#endif
