/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * DalQueryBuilder.cc
 *
 *  Created on: Jan 6, 2013
 */

#include "uncxx/upll_log.hh"
#include "dal_query_builder.hh"

namespace unc {
namespace upll {
namespace dal {
/* sql statement templates definitions */
const char * DalQueryBuilder::DalGetSingleRecQT =
  "SELECT {mand_out_columns} FROM {config1_table_name}"
    " {mand_WHERE_match_columns_eq}";

const char * DalQueryBuilder::DalGetMultiRecQT  =
  "SELECT {mand_out_columns} FROM {config1_table_name}"
    " {opt_WHERE_match_columns_eq} ORDER BY {primary_key_columns}";

const char * DalQueryBuilder::DalRecExistsQT    =
  "SELECT 1 FROM {config1_table_name} {mand_WHERE_match_columns_eq} limit 1";

const char * DalQueryBuilder::DalGetSibBegQT    =
  "SELECT {mand_out_columns} FROM {config1_table_name}"
    " {opt_WHERE_match_columns_eq_not_last} ORDER BY {primary_key_columns}";

const char * DalQueryBuilder::DalGetSibRecQT    =
  "SELECT {mand_out_columns} FROM {config1_table_name}"
    " {mand_WHERE_match_columns_last_greater} ORDER BY {primary_key_columns}";

const char * DalQueryBuilder::DalGetSibCountQT  =
  "SELECT COUNT(*) FROM {config1_table_name}"
    " {mand_WHERE_match_columns_last_greater}";

const char * DalQueryBuilder::DalGetRecCountQT  =
  "SELECT COUNT(*) FROM {config1_table_name}"
    " {opt_WHERE_match_columns_eq}";

const char * DalQueryBuilder::DalCreateRecQT    =
  "INSERT INTO {config1_table_name} ({mand_in_columns})"
    " VALUES ({mand_insert_?})";

// Deletes a specified record in the table
const char * DalQueryBuilder::DalDelRecQT       =
  "DELETE FROM {config1_table_name} {opt_WHERE_match_columns_eq}";

// Deletes all the records in the table
const char * DalQueryBuilder::DalTruncTableQT =
  "TRUNCATE TABLE {config1_table_name} CASCADE";

const char * DalQueryBuilder::DalUpdateRecQT    =
  "UPDATE {config1_table_name} SET {mand_in_columns_with_?}"
    " {opt_WHERE_match_columns_eq}";

const char * DalQueryBuilder::DalGetDelRecQT    =
  "SELECT {mand_out_columns} FROM {config2_table_name} AS temp"
    " WHERE NOT EXISTS"
    " ( SELECT {mand_match_columns} FROM {config1_table_name}"
    "   {mand_match_columns_eq_with_temp})";

const char * DalQueryBuilder::DalGetCreatedRecQT =
  "SELECT {mand_out_columns} FROM {config1_table_name} AS temp"
    " WHERE NOT EXISTS"
    " ( SELECT {mand_match_columns} FROM {config2_table_name}"
    "   {mand_match_columns_eq_with_temp})";

const char * DalQueryBuilder::DalGetModRecQT     =
  "SELECT {mand_out_columns} FROM {config1_table_name}"
    " WHERE EXISTS"
      " ( SELECT {primary_key_columns} FROM"
        " ( SELECT {opt_match_columns} FROM {config2_table_name} EXCEPT"
          " SELECT {opt_match_columns} FROM {config1_table_name}"
        " ) AS temp WHERE {match_dst_primary_key_columns_eq_with_temp}"
      " ) ORDER BY ({primary_key_columns})";

const char * DalQueryBuilder::DalCopyEntireRecQT =
  " INSERT INTO {dst_table_name} ({opt_out_columns})"
    " SELECT {opt_out_columns} FROM {src_table_name}";

const char * DalQueryBuilder::DalCopyModRecDelQT =
  " DELETE FROM {dst_table_name} WHERE NOT EXISTS"
    " ( SELECT {primary_key_columns} FROM {src_table_name} AS temp"
      "  WHERE {match_dst_primary_key_columns_eq_with_temp})";

const char * DalQueryBuilder::DalCopyModRecCreateQT =
  " INSERT INTO {dst_table_name} ({opt_out_columns})"
    " ( SELECT {opt_out_columns} FROM {src_table_name} AS temp"
      " WHERE NOT EXISTS"
        " ( SELECT {primary_key_columns} FROM {dst_table_name}"
          " WHERE {match_dst_primary_key_columns_eq_with_temp}))";

const char * DalQueryBuilder::DalCopyModRecUpdateQT =
  "UPDATE {dst_table_name} SET {opt_dst_out_columns_with_temp} FROM"
    " ( SELECT {opt_out_columns} FROM"
      " ( SELECT {opt_match_columns} FROM {src_table_name} EXCEPT"
        " SELECT {opt_match_columns} FROM {dst_table_name}"
      " ) AS temp1"
    " ) AS temp WHERE {match_dst_primary_key_columns_eq_with_temp}";

// Copy import to candidate during merge
const char * DalQueryBuilder::DalCopyModRecCreateImportQT =
  " INSERT INTO {dst_table_name} (c_flag, {opt_out_columns})"
    " ( SELECT 1, {opt_out_columns} FROM {src_table_name} AS temp"
      " WHERE NOT EXISTS"
        " ( SELECT {primary_key_columns} FROM {dst_table_name}"
          " WHERE {match_dst_primary_key_columns_eq_with_temp}))";

// Copy import to candidate during merge
const char * DalQueryBuilder::DalCopyModRecUpdateImportQT =
  "UPDATE {dst_table_name} SET u_flag = 1, {match_columns_eq_with_temp} FROM"
    " ( SELECT {opt_match_columns} FROM"
      " ( SELECT {opt_match_columns} FROM {src_table_name} EXCEPT"
        " SELECT {opt_match_columns} FROM {dst_table_name}"
      " ) AS temp1"
    " ) AS temp WHERE {match_dst_primary_key_columns_eq_with_temp}";

// Below query should be used only for tables having ctrlr_name
// during merge of deleted records
const char * DalQueryBuilder::DalCopyModRecDelImportQT =
  " DELETE FROM {dst_table_name} WHERE NOT EXISTS"
    " ( SELECT {primary_key_columns} FROM {src_table_name} AS temp"
      "  WHERE {match_dst_primary_key_columns_eq_with_temp} AND"
      " {dst_table_name}.ctrlr_name = temp.ctrlr_name) AND ctrlr_name = ?";

// Copy Running to Candidate during abort operation
const char * DalQueryBuilder::DalCopyModRecUpdateAbortQT =
  "UPDATE {dst_table_name} SET {opt_dst_out_columns_with_temp}, c_flag = 0,"
    " u_flag = 0 FROM ( SELECT {opt_match_columns} FROM {src_table_name}"
      " ) AS temp WHERE {match_dst_primary_key_columns_eq_with_temp} AND"
        " ({dst_table_name}.c_flag = 1 OR {dst_table_name}.u_flag = 1)";

const char * DalQueryBuilder::DalCopyMatchingRecQT =
  "INSERT INTO {dst_table_name} ({opt_out_columns})"
    " ( SELECT {opt_out_columns} FROM {src_table_name}"
      " {mand_WHERE_match_columns_eq} )";

const char * DalQueryBuilder::DalCheckRecIdenticalQT =
  "SELECT  COUNT(*) FROM"
    " ( SELECT {opt_match_columns} FROM"
      " ( SELECT {opt_match_columns} FROM {config1_table_name} UNION ALL"
        " SELECT {opt_match_columns} FROM {config2_table_name}"
      " ) AS temp1 GROUP BY {opt_match_columns} HAVING COUNT(*) != 2"
    " ) AS temp";

// Set specified record in dirty table to dirty
const char * DalQueryBuilder::DalDirtyTblUpdateRecQT =
  "UPDATE {config1_table_name} SET dirty=1 {mand_WHERE_match_columns_eq}";

// Set specified record in dirty table to dirty
const char * DalQueryBuilder::DalDirtyTblResetRecQT =
  "UPDATE {config1_table_name} SET dirty=0 {mand_WHERE_match_columns_eq}";

// Clear dirty flags for all tables
const char * DalQueryBuilder::DalDirtyTblClearAllQT =
  "UPDATE {config1_table_name} SET dirty=0 WHERE dirty=1";

// Create with c_flag = 1 in CAND
const char * DalQueryBuilder::DalCreateCandRecQT    =
  "INSERT INTO {config1_table_name} ({mand_in_columns}, c_flag, u_flag)"
    " VALUES ({mand_insert_?}, 1, 0)";

// Create with u_flag = 1 in CAND if rec exists in RUNN
const char * DalQueryBuilder::DalCreateCandRecUpdateQT    =
  "INSERT INTO {config1_table_name} ({mand_in_columns}, c_flag, u_flag)"
    " VALUES ({mand_insert_?}, 0, 1)";

// Update with u_flag = 1 in CAND
const char * DalQueryBuilder::DalUpdateCandRecQT    =
  "UPDATE {config1_table_name} SET {mand_in_columns_with_?}, "
    "u_flag = CASE WHEN c_flag IS NULL OR c_flag=0 THEN 1 WHEN c_flag =1 "
      "THEN 0 ELSE c_flag END {opt_WHERE_match_columns_eq} ";

// Clear c_flag and u_flag in CAND
const char * DalQueryBuilder::DalClearCandFlagsQT =
  "UPDATE {config1_table_name} SET c_flag=0, u_flag=0 WHERE"
    " c_flag = 1 OR u_flag = 1";

const char * DalQueryBuilder::DalClearCandFlagsInVtnModeQT =
  "UPDATE {config1_table_name} SET c_flag=0, u_flag=0 WHERE"
    " ((c_flag = 1 OR u_flag = 1) AND vtn_name = ?)";

const char * DalQueryBuilder::DalClearCandCrFlagsQT =
  "UPDATE {config1_table_name} SET c_flag=0 WHERE"
    " c_flag = 1";

const char * DalQueryBuilder::DalClearCandCrFlagsInVtnModeQT =
  "UPDATE {config1_table_name} SET c_flag=0 WHERE"
    " ((c_flag = 1) AND vtn_name = ?)";

const char * DalQueryBuilder::DalClearCandUpFlagsQT =
  "UPDATE {config1_table_name} SET u_flag=0 WHERE"
    " u_flag = 1";

const char * DalQueryBuilder::DalClearCandUpFlagsInVtnModeQT =
  "UPDATE {config1_table_name} SET u_flag=0 WHERE"
    " ((u_flag = 1) AND vtn_name = ?)";
// Get created records where c_flag = 1 from CAND
const char * DalQueryBuilder::DalGetCreatedRecInCandQT =
  "SELECT {mand_out_columns} FROM {config1_table_name} WHERE"
    " c_flag = 1";

// Get CAND records where u_flag = 1
const char * DalQueryBuilder::DalGetModRecConfig1QT     =
  "SELECT {mand_out_columns} FROM {config1_table_name}"
    " WHERE u_flag = 1 ORDER BY ({primary_key_columns})";

// Get RUNN records which exists in CAND with u_flag=1
const char * DalQueryBuilder::DalGetModRecConfig2QT     =
  "SELECT {mand_out_columns} FROM {config2_table_name} as temp"
    " WHERE EXISTS"
      " ( SELECT {primary_key_columns} FROM {config1_table_name}"
        " WHERE u_flag = 1 AND {match_dst_primary_key_columns_eq_with_temp}"
      " ) ORDER BY ({primary_key_columns})";

// Get all deleted records from CAND
const char * DalQueryBuilder::DalGetCandDelRecQT =
  "SELECT {mand_out_columns} FROM {config1_table_name}";

// Get all deleted records from CAND
const char * DalQueryBuilder::DalGetCandVtnDelRecQT =
  "SELECT {mand_out_columns} FROM {config1_table_name}"
    " WHERE vtn_name = ?";

// Backup deleted records from CAND during merge
const char * DalQueryBuilder::DalBackupToCandDelQT =
  "INSERT INTO {ca_del_table_name} ({opt_out_columns})"
    " ( SELECT {opt_out_columns} FROM {config1_table_name}"
      " WHERE NOT EXISTS ( SELECT {primary_key_columns}"
        " FROM {config2_table_name} AS temp"
          " WHERE {match_dst_primary_key_columns_eq_with_temp} AND"
            " {dst_table_name}.ctrlr_name = temp.ctrlr_name) AND"
            " ctrlr_name = ?)";

const char * DalQueryBuilder::DalVtnDirtyTblInsertRecQT    =
  "INSERT INTO {config1_table_name} ({mand_in_columns})"
    " VALUES ({mand_insert_?})";

// Get Records which exists in CAND with c_flag = 1 and given vtn_name
const char * DalQueryBuilder::DalGetCreatedRecInVtnModeQT =
  "SELECT {mand_out_columns} FROM {config1_table_name} WHERE"
    " c_flag = 1 {match_column_vtn_name_eq}";

// Get Records which exists in CAND with u_flag = 1 and given vtn_name
const char * DalQueryBuilder::DalGetUpdatedRecInVtnModeQT =
  "SELECT {mand_out_columns} FROM {config1_table_name} WHERE"
    " u_flag = 1 {match_column_vtn_name_eq}"
    " ORDER BY ({primary_key_columns})";

// Get RUNN records which exists in CAND with u_flag=1 and given vtn_name
const char * DalQueryBuilder::DalGetUpdatedRecInVtnMode2QT     =
  "SELECT {mand_out_columns} FROM {config2_table_name} as temp"
    " WHERE EXISTS"
      " ( SELECT {primary_key_columns} FROM {config1_table_name}"
        " WHERE u_flag = 1 AND {match_dst_primary_key_columns_eq_with_temp}"
        " {match_column_vtn_name_eq}"
        " ) ORDER BY ({primary_key_columns})";

#if 0
const char * DalQueryBuilder::DalCopyModRecDelVtnQT =
  "DELETE FROM {config1_table_name} WHERE EXISTS"
    " SELECT {primary_key_columns} from {config1_table_name} WHERE "
    "{match_dst_primary_key_columns_eq_with_src}  AND "
    "{config1_table_name}.vtn_name = ?";

const char * DalQueryBuilder::DalCopyModRecDelVtnQT =
  "DELETE FROM {config1_table_name} WHERE EXISTS"
    " (SELECT {primary_key_columns} from {config2_table_name} WHERE"
    " {match_dst_primary_key_columns_eq_with_src}  AND"
    " {config1_table_name}.vtn_name = ?)";
#endif

const char * DalQueryBuilder::DalCopyModRecDelVtnQT =
  " DELETE FROM {dst_table_name} WHERE NOT EXISTS"
    " ( SELECT {primary_key_columns} FROM {src_table_name} AS temp"
      "  WHERE {match_dst_primary_key_columns_eq_with_temp}) AND"
        " ({dst_table_name}.vtn_name = ?)";

const char * DalQueryBuilder::DalCopyModRecCreateVtnQT =
  " INSERT INTO {dst_table_name} ({opt_out_columns})"
    " (SELECT {opt_out_columns} FROM {src_table_name} AS temp"
      " WHERE NOT EXISTS"
        " ( SELECT {primary_key_columns} FROM {dst_table_name}"
          " WHERE {match_dst_primary_key_columns_eq_with_temp})"
            " AND temp.vtn_name = ?)";

#if 0
const char * DalQueryBuilder::DalCopyModRecUpdateAbortVtnQT =
  "UPDATE {dst_table_name} SET {opt_dst_out_columns_with_temp}, c_flag = 0,"
    " u_flag = 0 FROM ( SELECT {opt_match_columns} FROM {src_table_name}"
      " ) AS temp WHERE {match_dst_primary_key_columns_eq_with_temp} AND"
        " (({dst_table_name}.c_flag = 1 OR {dst_table_name}.u_flag = 1)"
          " AND ({src_table_name}.vtn_name = ? AND"
            " {dst_table_name}.vtn_name = ?))";

const char * DalQueryBuilder::DalCopyModRecUpdateAbortVtnQT =
  "UPDATE {dst_table_name} SET {opt_dst_out_columns_with_temp}, c_flag = 0,"
    " u_flag = 0 FROM ( SELECT {opt_match_columns} FROM {src_table_name}"
      " ) AS temp WHERE {match_dst_primary_key_columns_eq_with_temp} AND"
        " ({dst_table_name}.c_flag = 1 OR {dst_table_name}.u_flag = 1) AND"
          " {dst_table_name}.vtn_name = ?)";
#endif

const char * DalQueryBuilder::DalCopyModRecUpdateAbortVtnQT =
  "UPDATE {dst_table_name} SET {opt_dst_out_columns_with_temp}, c_flag = 0,"
    " u_flag = 0 FROM ( SELECT {match_all_columns} FROM {src_table_name}"
      " ) AS temp WHERE {match_dst_primary_key_columns_eq_with_temp} AND"
        " ({dst_table_name}.c_flag = 1 OR {dst_table_name}.u_flag = 1) AND"
          " {dst_table_name}.vtn_name = ?";
/* sql templates mapping with enum constants */
static const struct SqlTemplates {
  const DalApiNum api_num;
  const char *query_tmpl;
} sql_templates[] = {
  { kDalGetSingleRecQT, DalQueryBuilder::DalGetSingleRecQT },
  { kDalGetMultiRecQT, DalQueryBuilder::DalGetMultiRecQT },
  { kDalRecExistsQT, DalQueryBuilder::DalRecExistsQT },
  { kDalGetSibBegQT, DalQueryBuilder::DalGetSibBegQT },
  { kDalGetSibRecQT, DalQueryBuilder::DalGetSibRecQT },
  { kDalGetSibCountQT, DalQueryBuilder::DalGetSibCountQT },
  { kDalGetRecCountQT, DalQueryBuilder::DalGetRecCountQT },
  { kDalCreateRecQT, DalQueryBuilder::DalCreateRecQT },
  { kDalDelRecQT, DalQueryBuilder::DalDelRecQT },
  { kDalTruncTableQT, DalQueryBuilder::DalTruncTableQT },
  { kDalUpdateRecQT, DalQueryBuilder::DalUpdateRecQT },
  { kDalGetDelRecQT, DalQueryBuilder::DalGetDelRecQT },
  { kDalGetCreatedRecQT, DalQueryBuilder::DalGetCreatedRecQT },
  { kDalGetModRecQT, DalQueryBuilder::DalGetModRecQT },
  { kDalCopyEntireRecQT, DalQueryBuilder::DalCopyEntireRecQT },
  { kDalCopyModRecDelQT, DalQueryBuilder::DalCopyModRecDelQT },
  { kDalCopyModRecCreateQT, DalQueryBuilder::DalCopyModRecCreateQT },
  { kDalCopyModRecUpdateQT, DalQueryBuilder::DalCopyModRecUpdateQT },
  { kDalCopyMatchingRecQT, DalQueryBuilder::DalCopyMatchingRecQT },
  { kDalCheckRecIdenticalQT, DalQueryBuilder::DalCheckRecIdenticalQT },
  { kDalDirtyTblUpdateRecQT, DalQueryBuilder::DalDirtyTblUpdateRecQT },
  { kDalDirtyTblResetRecQT, DalQueryBuilder::DalDirtyTblResetRecQT },
  { kDalDirtyTblClearAllQT, DalQueryBuilder::DalDirtyTblClearAllQT },
  // New queries with c_flag and u_flag
  { kDalCreateCandRecQT, DalQueryBuilder::DalCreateCandRecQT },
  { kDalCreateCandRecUpdateQT, DalQueryBuilder::DalCreateCandRecUpdateQT},
  { kDalUpdateCandRecQT, DalQueryBuilder::DalUpdateCandRecQT },
  { kDalClearCandFlagsQT, DalQueryBuilder::DalClearCandFlagsQT },
  // In vtn mode, clear c_flag and u_flag for a VTN
  { kDalClearCandFlagsInVtnModeQT,
    DalQueryBuilder::DalClearCandFlagsInVtnModeQT },
  { kDalClearCandCrFlagsQT, DalQueryBuilder::DalClearCandCrFlagsQT },
  // In vtn mode, clear c_flag and u_flag for a VTN
  { kDalClearCandCrFlagsInVtnModeQT,
    DalQueryBuilder::DalClearCandCrFlagsInVtnModeQT },
  { kDalClearCandUpFlagsQT, DalQueryBuilder::DalClearCandUpFlagsQT },
  // In vtn mode, clear c_flag and u_flag for a VTN
  { kDalClearCandUpFlagsInVtnModeQT,
    DalQueryBuilder::DalClearCandUpFlagsInVtnModeQT },
  { kDalGetCreatedRecInCandQT, DalQueryBuilder::DalGetCreatedRecInCandQT },
  { kDalGetModRecConfig1QT, DalQueryBuilder::DalGetModRecConfig1QT },
  { kDalGetModRecConfig2QT, DalQueryBuilder::DalGetModRecConfig2QT },
  { kDalCopyModRecUpdateAbortQT, DalQueryBuilder::DalCopyModRecUpdateAbortQT },
  { kDalCopyModRecCreateImportQT, DalQueryBuilder::DalCopyModRecCreateImportQT},
  { kDalCopyModRecUpdateImportQT, DalQueryBuilder::DalCopyModRecUpdateImportQT},
  { kDalCopyModRecDelImportQT, DalQueryBuilder::DalCopyModRecDelImportQT},
  // Get deleted records from CAND_DEL during commit
  { kDalGetCandDelRecQT, DalQueryBuilder::DalGetCandDelRecQT},
  // Get deleted records in vtn mode from CAND_DEL during commit
  { kDalGetCandVtnDelRecQT, DalQueryBuilder::DalGetCandVtnDelRecQT},
  // Backup deleted records from CAND during merge
  { kDalBackupToCandDelQT, DalQueryBuilder::DalBackupToCandDelQT},
  // In vtn mode, insert records vtn dirty tbl
  { kDalVtnDirtyTblInsertRecQT, DalQueryBuilder::DalVtnDirtyTblInsertRecQT },
  // In vtn mode, get created records during commit
  { kDalGetCreatedRecInVtnModeQT, DalQueryBuilder::DalGetCreatedRecInVtnModeQT},
  // In vtn mode, get updated records from CAND during commit
  { kDalGetUpdatedRecInVtnModeQT, DalQueryBuilder::DalGetUpdatedRecInVtnModeQT},
  // In vtn mode, get updated records from RUNN during commit
  { kDalGetUpdatedRecInVtnMode2QT,
    DalQueryBuilder::DalGetUpdatedRecInVtnMode2QT},
  // In vtn mode, copy deleted records during abort
  { kDalCopyModRecDelVtnModeQT, DalQueryBuilder::DalCopyModRecDelVtnQT},
  // In vtn mode, copy created records during abort
  { kDalCopyModRecCreateVtnModeQT, DalQueryBuilder::DalCopyModRecCreateVtnQT},
  // In vtn mode, copy updated records during abort
  { kDalCopyModRecUpdateAbortVtnModeQT,
    DalQueryBuilder::DalCopyModRecUpdateAbortVtnQT},
};

/* replacement token definitions*/
// Input Related tokens
const char * DalQueryBuilder::DalMandInColNames = "{mand_in_columns}";
const char * DalQueryBuilder::DalMandInColValues = "{mand_insert_?}";
const char * DalQueryBuilder::DalMandInColUpdExpr = "{mand_in_columns_with_?}";

// Output Related tokens
const char * DalQueryBuilder::DalMandOutColNames = "{mand_out_columns}";
const char * DalQueryBuilder::DalOptOutColNames = "{opt_out_columns}";
const char * DalQueryBuilder::DalOptDstOutColUpdTempExpr =
  "{opt_dst_out_columns_with_temp}";

// Match (Filter) related tokens
const char * DalQueryBuilder::DalMandMatchColNames = "{mand_match_columns}";
const char * DalQueryBuilder::DalOptMatchColNames  = "{opt_match_columns}";
const char * DalQueryBuilder::DalMandMatchColEqExpr =
  "{mand_WHERE_match_columns_eq}";
const char * DalQueryBuilder::DalOptMatchColEqExpr =
  "{opt_WHERE_match_columns_eq}";
const char * DalQueryBuilder::DalOptMatchColEqNotLastExpr =
  "{opt_WHERE_match_columns_eq_not_last}";
const char * DalQueryBuilder::DalMandMatchColLstGtrExpr =
  "{mand_WHERE_match_columns_last_greater}";
const char * DalQueryBuilder::DalMandMatchColEqTempExpr =
  "{mand_match_columns_eq_with_temp}";
const char * DalQueryBuilder::DalMatchColEqTempExpr =
  "{match_columns_eq_with_temp}";
const char * DalQueryBuilder::DalMatchColVtnNameEqExpr =
  "{match_column_vtn_name_eq}";

// Primary Key related tokens
const char * DalQueryBuilder::DalPkeyColNames = "{primary_key_columns}";
const char * DalQueryBuilder::DalMatchPriColEqTempExpr =
  "{match_dst_primary_key_columns_eq_with_temp}";
const char * DalQueryBuilder::DalMatchPriColEqExpr =
  "{match_dst_primary_key_columns_eq_with_src}";
const char * DalQueryBuilder::DalMatchAllColEqExpr =
  "{match_all_columns}";

// Table related tokens
const char * DalQueryBuilder::DalCfg1TableName  = "{config1_table_name}";
const char * DalQueryBuilder::DalCfg2TableName  = "{config2_table_name}";
const char * DalQueryBuilder::DalDstTableName   = "{dst_table_name}";
const char * DalQueryBuilder::DalSrcTableName   = "{src_table_name}";
const char * DalQueryBuilder::DalCaDelTableName = "{ca_del_table_name}";

// configuration type definitions
const char * DalQueryBuilder::DalStateType     = "st_";
const char * DalQueryBuilder::DalCandidateType = "ca_";
const char * DalQueryBuilder::DalRunningType   = "ru_";
const char * DalQueryBuilder::DalStartupType   = "su_";
const char * DalQueryBuilder::DalImportType    = "im_";
const char * DalQueryBuilder::DalAuditType     = "au_";
const char * DalQueryBuilder::DalCandidateDelType      = "ca_del_";
const char * DalQueryBuilder::DalSysType       = "sy_";
DalQueryBuilder::DalQueryBuilder() {
  // Auto-generated constructor stub
}  // DalQueryBuilder::DalQueryBuilder

DalQueryBuilder::~DalQueryBuilder() {
  // Auto-generated destructor stub
}  // DalQueryBuilder::~DalQueryBuilder

/* get sql statement which directly executed by database */
bool DalQueryBuilder::get_sql_statement(
                        const DalApiNum dal_api_num,
                        const upll::dal::DalBindInfo *dbi,
                        std::string &query_str,
                        const uint16_t table_index,
                        const UpllCfgType first_config,
                        const UpllCfgType second_config) const {
  /* get query template */
  size_t start_pos = 0, last_pos = 0;
  std::string token = "", replacement = "";

  //  pfc::core::Mutex ctrlr_mutex;
  //  pfc::core::ScopedMutex lock(ccc->ctrlr_mutex);

  query_str.erase();
  /* checking DalBindInfo */
  if (dbi == NULL) {
    UPLL_LOG_TRACE("Column bound is NULL");
  }

  /* verify table_index */
  if (table_index >= (schema::table::kDalNumTables)) {
    UPLL_LOG_DEBUG("Invalid table index - %d", table_index);
    return false;
  }

  /* verify first config type */
  if (first_config == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("First config selected as Invalid");
    return false;
  }

  /* verify when both config required to specify */
  if ((second_config != UPLL_DT_INVALID) && (first_config == second_config)) {
    UPLL_LOG_DEBUG("Both config type are same");
    return false;
  }

  query_str = sql_templates[dal_api_num].query_tmpl;

  /* replace all tokens in query template to get statement */
  while (next_token_pos(query_str, start_pos, last_pos)) {
    size_t length = last_pos - start_pos + 1;

    /* get columns/table name for statement */
    if (get_bind_str(str_to_num(query_str, start_pos, length), dbi, replacement,
                     table_index, first_config, second_config) != true) {
      UPLL_LOG_DEBUG("Error constructing bind string");
      query_str.erase();
      return false;
    }
    /* replace token with actual columns/table name in template */
    query_str.replace(start_pos, length, replacement);
    start_pos += replacement.length();
  }
  return true;
}  // DalQueryBuilder::get_sql_statement

/* keep finding tokens from given query template until end of templates */
bool DalQueryBuilder::next_token_pos(const std::string &query_template,
                                     size_t &start_position,
                                     size_t &last_position) const {
  if (query_template.empty()) {
    UPLL_LOG_DEBUG("Empty Query Template String");
    return false;
  }

  /* find start position of token */
  size_t open_token = query_template.find('{', start_position);

  /* check if no more start position */
  if (open_token == std::string::npos) {
    return false;
  }

  /* find end position of current token */
  size_t close_token = query_template.find('}', open_token);

  /* check if no more end position */
  if (close_token == std::string::npos) {
    UPLL_LOG_DEBUG("Invalid Query Template String");
    return false;
  }

  start_position = open_token;
  last_position = close_token;
  return true;
}  // DalQueryBuilder::next_token_pos

/* converts token string to enum constants */
DalQuerytoken DalQueryBuilder::str_to_num(const std::string &tokenstr,
                                          const size_t start_pos,
                                          const size_t length) const {
  if (tokenstr.empty()) {
    UPLL_LOG_DEBUG("Empty Token String");
    return kDalInvalidToken;
  }
  // Input Related tokens
  if (tokenstr.compare(start_pos, length, DalMandInColNames) == 0) {
    return kDalMandInColNames;
  } else if (tokenstr.compare(start_pos, length, DalMandInColValues) == 0) {
    return kDalMandInColValues;
  } else if (tokenstr.compare(start_pos, length, DalMandInColUpdExpr) == 0) {
    return kDalMandInColUpdExpr;

  // Output Related tokens
  } else if (tokenstr.compare(start_pos, length, DalMandOutColNames) == 0) {
    return kDalMandOutColNames;
  } else if (tokenstr.compare(start_pos, length, DalOptOutColNames) == 0) {
    return kDalOptOutColNames;
  } else if (tokenstr.compare(start_pos, length,
                              DalOptDstOutColUpdTempExpr) == 0) {
    return kDalOptDstOutColUpdTempExpr;

  // Match Related tokens
  } else if (tokenstr.compare(start_pos, length, DalMandMatchColNames) == 0) {
    return kDalMandMatchColNames;
  } else if (tokenstr.compare(start_pos, length, DalOptMatchColNames) == 0) {
    return kDalOptMatchColNames;
  } else if (tokenstr.compare(start_pos, length, DalMandMatchColEqExpr) == 0) {
    return kDalMandMatchColEqExpr;
  } else if (tokenstr.compare(start_pos, length, DalOptMatchColEqExpr) == 0) {
    return kDalOptMatchColEqExpr;
  } else if (tokenstr.compare(start_pos, length,
                              DalOptMatchColEqNotLastExpr) == 0) {
    return kDalOptMatchColEqNotLastExpr;
  } else if (tokenstr.compare(start_pos, length,
                              DalMandMatchColLstGtrExpr) == 0) {
    return kDalMandMatchColLstGtrExpr;
  } else if (tokenstr.compare(start_pos, length,
                              DalMandMatchColEqTempExpr) == 0) {
    return kDalMandMatchColEqTempExpr;
  } else if (tokenstr.compare(start_pos, length,
                              DalMatchColVtnNameEqExpr) == 0) {
    return kDalMatchColVtnNameEqExpr;

  // Primary Key Related tokens
  } else if (tokenstr.compare(start_pos, length, DalPkeyColNames) == 0) {
    return kDalPkeyColNames;
  } else if (tokenstr.compare(start_pos, length,
                              DalMatchPriColEqTempExpr) == 0) {
    return kDalMatchPriColEqTempExpr;
  } else if (tokenstr.compare(start_pos, length, DalMatchColEqTempExpr) == 0) {
    return kDalMatchColEqTempExpr;

  // Table Related tokens
  } else if (tokenstr.compare(start_pos, length, DalCfg1TableName) == 0) {
    return kDalCfg1TableName;
  } else if (tokenstr.compare(start_pos, length, DalCfg2TableName) == 0) {
    return kDalCfg2TableName;
  } else if (tokenstr.compare(start_pos, length, DalDstTableName) == 0) {
    return kDalDstTableName;
  } else if (tokenstr.compare(start_pos, length, DalSrcTableName) == 0) {
    return kDalSrcTableName;
  } else if (tokenstr.compare(start_pos, length, DalCaDelTableName) == 0) {
    return kDalCaDelTableName;
  } else if (tokenstr.compare(start_pos, length, DalMatchPriColEqExpr) == 0) {
    return kDalMatchPriColEqExpr;
  } else if (tokenstr.compare(start_pos, length, DalMatchAllColEqExpr) == 0) {
    return kDalMatchAllColEqExpr;
  } else {
    UPLL_LOG_DEBUG("Invalid token(%s) from Query template", tokenstr.c_str());
    return kDalInvalidToken;
  }
}  // DalQueryBuilder::str_to_num

/* verifies config type, and adds appropriate config type prefix */
bool DalQueryBuilder::get_config_prefix(const UpllCfgType type,
                                        std::string &config_type) const {
  switch (type) {
    case UPLL_DT_INVALID:
      UPLL_LOG_DEBUG("Invalid Config type(%d)", type);
      return false;
    case UPLL_DT_STATE:
      config_type += DalStateType;
      return true;
    case UPLL_DT_CANDIDATE:
      config_type += DalCandidateType;
      return true;
    case UPLL_DT_RUNNING:
      config_type += DalRunningType;
      return true;
    case UPLL_DT_STARTUP:
      config_type += DalStartupType;
      return true;
    case UPLL_DT_IMPORT:
      config_type += DalImportType;
      return true;
    case UPLL_DT_AUDIT:
      config_type += DalAuditType;
      return true;
    case UPLL_DT_CANDIDATE_DEL:
      config_type += DalCandidateDelType;
      return true;
    case UPLL_DT_SYSTEM:
      config_type += DalSysType;
      return true;
    default:
      UPLL_LOG_DEBUG("Unknown Config type(%d)", type);
      return false;
  }
}  // DalQueryBuilder::get_config_prefix

// Builds comma separated column names for the given io_code
bool DalQueryBuilder::fill_column_names(const DalIoCode io_code,
                                        const DalTableIndex table_index,
                                        const DalBindInfo *dbi,
                                        std::string &replc_str) const {
  bool first = true;

  DalIoType io_type;
  DalBindList bind_list;
  DalBindList::iterator bl_it;
  DalBindColumnInfo *col_info = NULL;

  replc_str.erase();
  /* verify DAL IO Code */
  if (io_code == kDalIoCodeInvalid) {
    UPLL_LOG_DEBUG("Invalid DAL IO Code - %d", io_code);
    return false;
  }

  /* verify table_index */
  if (table_index >= (schema::table::kDalNumTables)) {
    UPLL_LOG_DEBUG("Invalid table index - %d", table_index);
    return false;
  }

  /* verify DAL Bind Info */
  if (dbi == NULL) {
    UPLL_LOG_DEBUG("NULL DAL Bind Info");
    return false;
  }

  bind_list = dbi->get_bind_list();

  for (bl_it = bind_list.begin(); bl_it != bind_list.end();
       ++bl_it) {
    col_info = *bl_it;
    io_type = col_info->get_io_type();

    if (io_code == kDalIoCodeInput) {
      if ( io_type != kDalIoInputOnly &&
           io_type != kDalIoInputAndMatch ) {
        continue;
      }
    } else if (io_code == kDalIoCodeOutput) {
      if ( io_type != kDalIoOutputOnly &&
           io_type != kDalIoOutputAndMatch ) {
        continue;
      }
    } else if (io_code == kDalIoCodeMatch) {
      if ( io_type != kDalIoMatchOnly &&
           io_type != kDalIoInputAndMatch &&
           io_type != kDalIoOutputAndMatch ) {
        continue;
      }
    }

    if (first != true)
      replc_str += ", ";

    if (first)
      first = false;

    replc_str += schema::ColumnName(table_index, col_info->get_column_index());
  }  // for
  return true;
}  // DalQueryBuilder::fill_column_names

// Finding the position of greater Primary Key index bound for match
// in DalBindList
bool DalQueryBuilder::find_last_pkey_pos_for_match(
                          const DalBindInfo *dbi,
                          uint16_t *last_pkey_pos) const {
  DalBindList::iterator bl_it;
  DalBindList bind_list;
  DalBindColumnInfo *col_info = NULL;
  uint16_t last_pkey_col_index = 0;
  uint16_t index;

  if (dbi == NULL) {
    UPLL_LOG_DEBUG("Dal Bind Info is NULL - program error");
    return false;
  }
  bind_list = dbi->get_bind_list();
  if (bind_list.empty() == true) {
    UPLL_LOG_DEBUG("Dal Bind List is empty - program error");
    return false;
  }
  for (bl_it = bind_list.begin(), index = 0; bl_it != bind_list.end();
      ++bl_it, ++index) {
    col_info = *bl_it;
    if ((col_info->get_io_type() == kDalIoMatchOnly ||
        col_info->get_io_type() == kDalIoInputAndMatch ||
        col_info->get_io_type() == kDalIoOutputAndMatch) &&
        (schema::ColumnIsPKeyIndex(dbi->get_table_index(),
                            col_info->get_column_index()) &&
        col_info->get_column_index() >= last_pkey_col_index)) {
      *last_pkey_pos = index;
      last_pkey_col_index = col_info->get_column_index();
    }
  }
  UPLL_LOG_TRACE("Last pos(%d) pkey(%d)", *last_pkey_pos, last_pkey_col_index);
  return true;
}  // find_last_pkey_pos_for_match

/* generate columns/table name string corresponding to each token
 *
 * each switch case of this function returns specific format of string
 * and the format is commented out at starting of each switch cases
 */
bool DalQueryBuilder::get_bind_str(const DalQuerytoken token,
                                   const DalBindInfo *dbi,
                                   std::string &replc_str,
                                   const DalTableIndex table_index,
                                   const UpllCfgType first_config,
                                   const UpllCfgType second_config) const {
  DalIoType io_type;
  DalBindList bind_list;
  DalBindList::iterator bl_it;
  DalBindColumnInfo *col_info = NULL;
  std::string table_name;

  bool first = true;
  uint16_t col_index = 0, col_count = 0;
  uint16_t last_pkey_pos;

  replc_str.erase();

  /* verify table_index */
  if (table_index >= (schema::table::kDalNumTables)) {
    UPLL_LOG_DEBUG("Invalid table index - %d", table_index);
    return false;
  }

  /* verify first config type */
  if (first_config == UPLL_DT_INVALID) {
    UPLL_LOG_DEBUG("Invalid First config");
    return false;
  }

  /* verify when both config required to specify */
  if ((second_config != UPLL_DT_INVALID) && (first_config == second_config)) {
    UPLL_LOG_DEBUG("Both config type are same");
    return false;
  }

  switch (token) {
    // Input Related Tokens
    // {mand_in_columns} - col1, col2, ...
    // column names of attributes bound for input
    case kDalMandInColNames:
      // PFC_ASSERT(dbi);
      if (dbi == NULL) {
        UPLL_LOG_DEBUG("Null Bind Info for Mandatory token");
        return false;
      }

      if (dbi->get_input_bind_count() == 0) {
        UPLL_LOG_DEBUG("No columns bound for Mandatory Input");
        return false;
      }

      if (fill_column_names(kDalIoCodeInput,
                          table_index, dbi, replc_str) == false) {
        UPLL_LOG_DEBUG("Error Filling Column names for Input");
        return false;
      }
    break;

    // {mand_insert_?} - ?, ?, ?, ...
    // count of attributes bound for input
    case kDalMandInColValues:
      // PFC_ASSERT(dbi);
      if (dbi == NULL) {
        UPLL_LOG_DEBUG("Null Bind Info for Mandatory token");
        return false;
      }

      first = true;
      col_count = dbi->get_input_bind_count();
      if (col_count == 0) {
        UPLL_LOG_DEBUG("No columns bound for Mandatory Input");
        return false;
      }

      for (col_index = 0; col_index < col_count; col_index++) {
        if (first != true)
          replc_str += ", ";

        if (first)
          first = false;

        replc_str += "?";
      }
      break;

    // {mand_in_columns_with_?} - col1 = ?, col2 = ?, col3 = ?, ...
    // For all attributes bound for input
    case kDalMandInColUpdExpr:
      // PFC_ASSERT(dbi);
      if (dbi == NULL) {
        UPLL_LOG_DEBUG("Null Bind Info for Mandatory token");
        return false;
      }

      if (dbi->get_input_bind_count() == 0) {
        UPLL_LOG_DEBUG("No columns bound for Mandatory Input");
        return false;
      }

      bind_list = dbi->get_bind_list();
      for (bl_it = bind_list.begin(); bl_it != bind_list.end();
           ++bl_it) {
        col_info = *bl_it;
        io_type = col_info->get_io_type();
        if ( io_type == upll::dal::kDalIoInputOnly ||
            io_type == upll::dal::kDalIoInputAndMatch) {
          if (first != true)
            replc_str += ", ";

          if (first)
            first = false;

          replc_str += schema::ColumnName(table_index,
                                          col_info->get_column_index());
          replc_str += "= ?";
        }  // if
      }  // for
      break;

    // Output Related Tokens
    // {mand_out_columns} - col1, col2, ...
    // column names of attributes bound for output
    case kDalMandOutColNames:
      // PFC_ASSERT(dbi);
      if (dbi == NULL) {
        UPLL_LOG_DEBUG("Null Bind Info for Mandatory token");
        return false;
      }

      if (dbi->get_output_bind_count() == 0) {
        UPLL_LOG_DEBUG("No columns bound for Mandatory Output");
        return false;
      }

      if (fill_column_names(kDalIoCodeOutput,
                            table_index, dbi, replc_str) == false) {
        UPLL_LOG_DEBUG("Error Filling Output Column names");
        return false;
      }
      break;

    // {opt_out_columns} - col1, col2, ...
    // column names of attributes bound for output
    // All column names, if no attributes bound for output
    case kDalOptOutColNames:
      if (dbi == NULL || dbi->get_output_bind_count() == 0) {
        UPLL_LOG_TRACE("All Columns bound for Optional Output");
        col_count = schema::TableNumCols(table_index);
        for (col_index = 0; col_index < col_count; col_index++) {
          if (first != true)
            replc_str += ", ";

          if (first)
            first = false;

          replc_str += schema::ColumnName(table_index, col_index);
        }
        return true;
      }

      if (fill_column_names(kDalIoCodeOutput,
                            table_index, dbi, replc_str) == false) {
        UPLL_LOG_DEBUG("Error Filling Output Column names");
        return false;
      }
      break;

    // {opt_dst_out_columns_with_temp} -
    // <dst_table>.col1 = temp.col1, <dst_table>.col2 = temp.col2, ...
    // column names of attributes bound for output
    // All column names, if no attributes bound for output
    case kDalOptDstOutColUpdTempExpr:
      table_name = "";
      if (get_bind_str(kDalDstTableName, dbi, table_name,
                       table_index, first_config, second_config) != true) {
        return false;
      }

      if (dbi == NULL || dbi->get_output_bind_count() == 0) {
        UPLL_LOG_DEBUG("All columns bound for Optional Output");
        // Building string for all columns
        col_count = schema::TableNumCols(table_index);
        for (col_index = 0; col_index < col_count; col_index++) {
          if (first != true)
            replc_str += ", ";

          if (first)
            first = false;

        //  replc_str += std::string(table_name);
        //  replc_str += std::string(".");
          replc_str += schema::ColumnName(table_index, col_index);
          replc_str += "= temp.";
          replc_str += schema::ColumnName(table_index, col_index);
        }
      } else {
        // Building string from bind info
        bind_list = dbi->get_bind_list();
        for (bl_it = bind_list.begin(); bl_it != bind_list.end();
             ++bl_it) {
          col_info = *bl_it;
          io_type = col_info->get_io_type();
          if ( io_type == upll::dal::kDalIoOutputOnly ||
              io_type == upll::dal::kDalIoOutputAndMatch) {
            if (first != true)
              replc_str += ", ";

            if (first)
              first = false;
            col_index = col_info->get_column_index();
            replc_str += schema::ColumnName(table_index, col_index);
            replc_str += "= temp.";
            replc_str += schema::ColumnName(table_index, col_index);
          }  // if
        }  // for
      }
      break;

    // Match Related Tokens
    // {mand_match_columns} - col1, col2, ...
    // column names of attributes bound for match
    case kDalMandMatchColNames:
      if (dbi == NULL || dbi->get_match_bind_count() == 0) {
        UPLL_LOG_DEBUG("No columns bound for Mandatory match");
        return false;
      }

    // {opt_match_columns} - col1, col2, ...
    // column names of attributes bound for match
    // All column names, if no attributes bound for output
    case kDalOptMatchColNames:
      // PFC_ASSERT(dbi);
      if (dbi == NULL || dbi->get_match_bind_count() == 0) {
        UPLL_LOG_DEBUG("All columns bound for Optional match");
        col_count = schema::TableNumCols(table_index);
        for (col_index = 0; col_index < col_count; col_index++) {
          if (first != true)
            replc_str += ", ";

          if (first)
            first = false;

          replc_str += schema::ColumnName(table_index, col_index);
        }
        return true;
      }

      if (fill_column_names(kDalIoCodeMatch,
                            table_index, dbi, replc_str) == false) {
        UPLL_LOG_DEBUG("Error Filling Match Column names");
        return false;
      }
      break;

    // {mand_WHERE_match_columns_eq} -
    // WHERE col1 = ?, col2 = ?, ...
    // column names of attributes bound for match
    case kDalMandMatchColEqExpr:
      // PFC_ASSERT(dbi);
      if (dbi == NULL) {
        UPLL_LOG_DEBUG("Null Bind Info for Mandatory token");
        return false;
      }

      if (dbi->get_match_bind_count() == 0) {
        UPLL_LOG_DEBUG("No columns bound for Mandatory Match");
        return false;
      }

    // {opt_WHERE_match_columns_eq} -
    // WHERE col1 = ?, col2 = ?, ...
    // column names of attributes bound for match
    // Empty string if no attributes bound for match
    case kDalOptMatchColEqExpr:
      if (dbi == NULL || dbi->get_match_bind_count() == 0) {
        UPLL_LOG_DEBUG("No columns bound for Optional Match");
        return true;
      }

      // Building string for attributes bound for match
      if (dbi->get_match_bind_count() > 0) {
        replc_str += "WHERE ";
      }

      bind_list = dbi->get_bind_list();
      for (bl_it = bind_list.begin(); bl_it != bind_list.end();
           ++bl_it) {
        col_info = *bl_it;
        io_type = col_info->get_io_type();
        if (io_type == kDalIoMatchOnly ||
            io_type == kDalIoInputAndMatch ||
            io_type == kDalIoOutputAndMatch) {
          if (first != true)
            replc_str += " AND ";

          if (first)
            first = false;

          replc_str += schema::ColumnName(table_index,
                                          col_info->get_column_index());
          replc_str += " = ?";
        }  // if
      }  // for
      break;

    // {opt_WHERE_match_columns_eq_not_last} -
    // WHERE col1 = ?, col2 = ?, ..., col(n-1) = ?
    // n is the number of attributes bound for match
    // column names of attributes bound for match
    case kDalOptMatchColEqNotLastExpr:
      if (dbi == NULL || dbi->get_match_bind_count() < 1) {
        UPLL_LOG_DEBUG("No columns bound for Optional Match");
        return true;
      }

      // Building string for attributes bound for match
      if (dbi->get_match_bind_count() > 1)
        replc_str += "WHERE ";

      col_count = 0;
      bind_list = dbi->get_bind_list();
      for (bl_it = bind_list.begin(); bl_it != bind_list.end();
           ++bl_it) {
        col_info = *bl_it;
        io_type = col_info->get_io_type();
        if (io_type == kDalIoMatchOnly ||
            io_type == kDalIoInputAndMatch ||
            io_type == kDalIoOutputAndMatch) {
          if (col_count <= dbi->get_match_bind_count() - 2) {
            if (first != true)
              replc_str += " AND ";

            if (first)
              first = false;

            replc_str += schema::ColumnName(table_index,
                                            col_info->get_column_index());
            replc_str += "= ?";
          }
          col_count++;
        }  // if
      }  // for
      break;

    // {mand_WHERE_match_columns_last_greater} -
    // WHERE col1 = ?, col2 = ?, ..., col(n-1) > ?
    // n is the number of attributes bound for match
    // column names of attributes bound for match
    case kDalMandMatchColLstGtrExpr:
      // PFC_ASSERT(dbi);
      if (dbi == NULL) {
        UPLL_LOG_DEBUG("Null Bind Info for Mandatory token");
        return false;
      }

      if (dbi->get_match_bind_count() == 0) {
        UPLL_LOG_DEBUG("No columns bound for Mandatory Match");
        return false;
      }

      // Building string for attributes bound for match
      if (dbi->get_match_bind_count() > 0)
        replc_str += "WHERE ";

      col_count = 0;
      last_pkey_pos = UINT16_MAX;
      if (find_last_pkey_pos_for_match(dbi, &last_pkey_pos) != true) {
        return false;
      }
      bind_list = dbi->get_bind_list();
      for (bl_it = bind_list.begin(); bl_it != bind_list.end(); ++bl_it) {
        col_info = *bl_it;
        io_type = col_info->get_io_type();
        if (io_type == kDalIoMatchOnly ||
            io_type == kDalIoInputAndMatch ||
            io_type == kDalIoOutputAndMatch) {
          if (first != true)
            replc_str += " AND ";

          if (first)
            first = false;

          replc_str += schema::ColumnName(table_index,
                                          col_info->get_column_index());
          if (col_count == last_pkey_pos)
            replc_str += "> ?";
          else
            replc_str += "= ?";
        }  // if
        col_count++;
      }  // for
      break;

    // Primary Key Related Tokens
    // {primary_key_columns} - col1, col2, ...
    // All primary key column names of the table
    case kDalPkeyColNames:
      col_count = schema::TableNumPkCols(table_index);
      for (col_index = 0; col_index < col_count; col_index++) {
        if (first != true)
          replc_str += ", ";

        if (first)
          first = false;

        replc_str += schema::ColumnName(table_index, col_index);
      }
      break;

    // {mand_match_columns_eq_with_temp}
    // col1 = temp.col1 AND col2 = temp.col2 ...
    // For all match columns in bind list
    case kDalMandMatchColEqTempExpr:
      if (dbi == NULL) {
        UPLL_LOG_INFO("Null Bind Info for Mandatory token");
        return false;
      }

      if (dbi->get_match_bind_count() == 0) {
        UPLL_LOG_INFO("No columns bound for Mandatory Match");
        return false;
      }

      // Building string for attributes bound for match
      if (dbi->get_match_bind_count() > 0)
        replc_str += "WHERE ";

      bind_list = dbi->get_bind_list();
      for (bl_it = bind_list.begin(); bl_it != bind_list.end(); ++bl_it) {
        col_info = *bl_it;
        io_type = col_info->get_io_type();
        if (io_type == kDalIoMatchOnly ||
            io_type == kDalIoInputAndMatch ||
            io_type == kDalIoOutputAndMatch) {
          if (first != true)
            replc_str += " AND ";

          if (first)
            first = false;

          replc_str += schema::ColumnName(table_index,
                                          col_info->get_column_index());
          replc_str += " = temp.";
          replc_str += schema::ColumnName(table_index,
                                          col_info->get_column_index());
        }  // if
        col_count++;
      }  // for
      break;

    // {match_col_vtn_name_eq}
    // AND vtn_name = ?
    case kDalMatchColVtnNameEqExpr:
      if (dbi == NULL || dbi->get_match_bind_count() == 0) {
        UPLL_LOG_DEBUG("No columns bound for Optional Match");
        return true;
      }
      bind_list = dbi->get_bind_list();
      for (bl_it = bind_list.begin(); bl_it != bind_list.end(); ++bl_it) {
        col_info = *bl_it;
        io_type = col_info->get_io_type();
        if (io_type == kDalIoMatchOnly ||
            io_type == kDalIoInputAndMatch ||
            io_type == kDalIoOutputAndMatch) {
          if (!strncmp(schema::ColumnName(table_index,
                                          col_info->get_column_index()),
                       "vtn_name", 9)) {
            replc_str += " AND ";
            replc_str += schema::ColumnName(table_index,
                                            col_info->get_column_index());
            replc_str += " = ? ";
            break;
          }
        }  // if
      }  // for
      break;

    // {match_columns_eq_with_temp}
    // col1 = temp.col1, col2 = temp.col2 ...
    // For all match columns in bind list
    case kDalMatchColEqTempExpr:
      if (dbi == NULL) {
        UPLL_LOG_INFO("Null Bind Info for Mandatory token");
        return false;
      }

      if (dbi->get_match_bind_count() == 0) {
        UPLL_LOG_INFO("No columns bound for Mandatory Match");
        return false;
      }

      // Building string for attributes bound for match
      // if (dbi->get_match_bind_count() > 0)
      // replc_str += "WHERE ";

      bind_list = dbi->get_bind_list();
      for (bl_it = bind_list.begin(); bl_it != bind_list.end(); ++bl_it) {
        col_info = *bl_it;
        io_type = col_info->get_io_type();
        if (io_type == kDalIoMatchOnly ||
            io_type == kDalIoInputAndMatch ||
            io_type == kDalIoOutputAndMatch) {
          if (first != true)
            replc_str += " , ";

          if (first)
            first = false;

          replc_str += schema::ColumnName(table_index,
                                          col_info->get_column_index());
          replc_str += " = temp.";
          replc_str += schema::ColumnName(table_index,
                                          col_info->get_column_index());
        }  // if
        col_count++;
      }  // for
      break;

    // {match_dst_primary_key_columns_eq_with_temp} - col1, col2, ...
    // <dst_table>.col1 = temp.col1 AND <dst_table>.col2 = temp.col2 ...
    // For all primary key column names of the table
    case kDalMatchPriColEqTempExpr:
      table_name = "";
      col_count = schema::TableNumPkCols(table_index);
      for (col_index = 0; col_index < col_count; col_index++) {
        if (first != true)
          replc_str += " AND ";

        if (first)
          first = false;

        if (get_bind_str(kDalDstTableName, dbi, table_name,
                         table_index, first_config, second_config) != true) {
          return false;
        }
        replc_str += table_name;
        replc_str += ".";
        replc_str += schema::ColumnName(table_index, col_index);
        replc_str += " = temp.";
        replc_str += schema::ColumnName(table_index, col_index);
      }
      break;

    // Table Related Tokens
    // {config1_table_name} - <config1>_<table_name>
    case kDalCfg1TableName:
    // {dst_table_name} - <dst_config>_<table_name>
    case kDalDstTableName:
      if (get_config_prefix(first_config, replc_str) != true) {
         UPLL_LOG_DEBUG("First/Dest configuration type wrong");
         return false;
      }

      replc_str += schema::TableName(table_index);
      break;

    // {config2_table_name} - <config2>_<table_name>
    case kDalCfg2TableName:
    // {src_table_name} - <src_config>_<table_name>
    case kDalSrcTableName:
      if (get_config_prefix(second_config, replc_str) != true) {
         UPLL_LOG_DEBUG("Second/src configuration type wrong");
         return false;
      }

      replc_str += schema::TableName(table_index);
      break;

    case kDalCaDelTableName:
      if (get_config_prefix(UPLL_DT_CANDIDATE_DEL, replc_str) != true) {
         UPLL_LOG_DEBUG("get_config_prefix fialed.");
         return false;
      }
      replc_str += schema::TableName(table_index);
      break;

    // {match_dst_primary_key_columns_eq_with_src} - col1, col2, ...
    // col1 != <src_table>.col1 AND col2 = <src_table>.col2 ...
    // For all primary key column names of the table
    case kDalMatchPriColEqExpr:
      table_name = "";
      col_count = schema::TableNumPkCols(table_index);
      for (col_index = 0; col_index < col_count; col_index++) {
        if (first != true)
          replc_str += " AND ";

        if (first)
          first = false;

        if (get_bind_str(kDalDstTableName, dbi, table_name,
                         table_index, first_config, second_config) != true) {
          return false;
        }
        replc_str += schema::ColumnName(table_index, col_index);
        replc_str += "!=";
        replc_str += table_name;
        replc_str += ".";
        replc_str += schema::ColumnName(table_index, col_index);
      }
      break;

    // {match_all_columns}
    case kDalMatchAllColEqExpr:
      // PFC_ASSERT(dbi);
      UPLL_LOG_DEBUG("All columns bound for Optional match");
      col_count = schema::TableNumCols(table_index);
      for (col_index = 0; col_index < col_count; col_index++) {
        if (first != true)
          replc_str += ", ";

        if (first)
          first = false;

        replc_str += schema::ColumnName(table_index, col_index);
      }
      break;

    /* Invalid token */
    default:
      UPLL_LOG_DEBUG("Invalid query token");
      return false;
  }

  return true;
}  // DalQueryBuilder::get_bind_str
}  // namespace dal
}  // namespace upll
}  // namespace unc
