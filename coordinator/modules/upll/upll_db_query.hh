/*
 * Copyright (c) 2014 NEC Corporation
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
const char* const QUERY_PP_CAND_REF_COUNT_UPDATE = "UPDATE ca_policingprofile_ctrlr_tbl \
      SET ref_count = ref_count + 1, u_flag = 1 \
      WHERE policingprofile_name = ? AND ctrlr_name = ?";

const char* const QUERY_PP_IMP_REF_COUNT_UPDATE = "UPDATE im_policingprofile_ctrlr_tbl \
      SET ref_count = ref_count + 1 \
      WHERE policingprofile_name = ? AND ctrlr_name = ? ";

const char* const QUERY_PP_AUD_REF_COUNT_UPDATE = "UPDATE au_policingprofile_ctrlr_tbl \
      SET ref_count = ref_count + 1 \
      WHERE policingprofile_name = ? AND ctrlr_name = ? ";

const char* const QUERY_FF_CAND_REF_COUNT_UPDATE = "UPDATE ca_flowlist_ctrlr_tbl \
      SET ref_count = ref_count + 1, u_flag = 1 \
      where flowlist_name = ? and ctrlr_name = ? ";

const char* const QUERY_FF_IMP_REF_COUNT_UPDATE = "UPDATE im_flowlist_ctrlr_tbl \
      SET ref_count = ref_count + 1 \
      where flowlist_name = ? and ctrlr_name = ? ";

const char* const QUERY_FF_AUD_REF_COUNT_UPDATE = "UPDATE au_flowlist_ctrlr_tbl \
      SET ref_count = ref_count + 1 \
      where flowlist_name = ? and ctrlr_name = ? ";

const char* const QUERY_CA_DELETE_FL_IN_FLECTRLTBL =
    "DELETE FROM ca_flowlist_entry_ctrlr_tbl WHERE flowlist_name = ? \
             AND ctrlr_name = ? ";

const char* const QUERY_CA_UPDATE_C_U_FLAGS_IN_FLE_CTRLRTBL =
    "UPDATE ca_flowlist_entry_ctrlr_tbl  SET u_flag = 1, c_flag = 0 \
    WHERE EXISTS (SELECT 1 FROM ru_flowlist_entry_ctrlr_tbl WHERE \
     ca_flowlist_entry_ctrlr_tbl.flowlist_name = ? AND \
     ca_flowlist_entry_ctrlr_tbl.ctrlr_name = ? AND \
    ru_flowlist_entry_ctrlr_tbl.flowlist_name = ca_flowlist_entry_ctrlr_tbl.flowlist_name \
    AND \
    ru_flowlist_entry_ctrlr_tbl.sequence_num = ca_flowlist_entry_ctrlr_tbl.sequence_num \
    AND \
    ru_flowlist_entry_ctrlr_tbl.ctrlr_name = ca_flowlist_entry_ctrlr_tbl.ctrlr_name) ";
#endif
