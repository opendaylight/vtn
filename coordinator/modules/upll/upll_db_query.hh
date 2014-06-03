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


//  This file declares the user supplied executable psql queries 
//  statements
const char* const QUERY_PP_CAND_REF_COUNT_UPDATE = "UPDATE ca_policingprofile_ctrlr_tbl \
      SET ref_count = ref_count + 1 \
      WHERE policingprofile_name = ? AND ctrlr_name = ?";

const char* const QUERY_PP_IMP_REF_COUNT_UPDATE = "UPDATE im_policingprofile_ctrlr_tbl \
      SET ref_count = ref_count + 1 \
      WHERE policingprofile_name = ? AND ctrlr_name = ? ";

const char* const QUERY_PP_AUD_REF_COUNT_UPDATE = "UPDATE au_policingprofile_ctrlr_tbl \
      SET ref_count = ref_count + 1 \
      WHERE policingprofile_name = ? AND ctrlr_name = ? ";

const char* const QUERY_FF_CAND_REF_COUNT_UPDATE = "UPDATE ca_flowlist_ctrlr_tbl \
      SET ref_count = ref_count + 1 \
      where flowlist_name = ? and ctrlr_name = ? ";

const char* const QUERY_FF_IMP_REF_COUNT_UPDATE = "UPDATE im_flowlist_ctrlr_tbl \
      SET ref_count = ref_count + 1 \
      where flowlist_name = ? and ctrlr_name = ? ";

const char* const QUERY_FF_AUD_REF_COUNT_UPDATE = "UPDATE au_flowlist_ctrlr_tbl \
      SET ref_count = ref_count + 1 \
      where flowlist_name = ? and ctrlr_name = ? ";
#endif
