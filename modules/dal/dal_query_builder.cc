/*
 * Copyright (c) 2012-2013 NEC Corporation
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
 *      Author: guest
 */

#include "upll/upll_log.hh"
#include "dal_query_builder.hh"
// #include "/home/guest/BuildEnv/src/core/include/cxx/pfcxx/synch.hh"

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

// Currently DalRecExists use DalGetRecCount Api
const char * DalQueryBuilder::DalRecExistsQT    =
  "SELECT 1 FROM {config1_table_name} {mand_WHERE_match_columns_eq}";

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

const char * DalQueryBuilder::DalDelRecQT       =
  "DELETE FROM {config1_table_name} {opt_WHERE_match_columns_eq}";

const char * DalQueryBuilder::DalUpdateRecQT    =
  "UPDATE {config1_table_name} SET {mand_in_columns_with_?}"
    " {opt_WHERE_match_columns_eq}";

const char * DalQueryBuilder::DalGetDelRecQT    =
  "SELECT {mand_out_columns} FROM {config2_table_name}"
    " WHERE ({mand_match_columns}) NOT IN"
    " ( SELECT {mand_match_columns} FROM {config1_table_name})";

const char * DalQueryBuilder::DalGetCreatedRecQT =
  "SELECT {mand_out_columns} FROM {config1_table_name}"
    " WHERE ({mand_match_columns}) NOT IN"
    " ( SELECT {mand_match_columns} FROM {config2_table_name})";

const char * DalQueryBuilder::DalGetModRecQT     =
  "SELECT {mand_out_columns} FROM {config1_table_name}"
    " WHERE ({primary_key_columns}) IN"
      " ( SELECT {primary_key_columns} FROM"
        " ( SELECT {opt_match_columns} FROM {config2_table_name} EXCEPT"
          " SELECT {opt_match_columns} FROM {config1_table_name}"
        " ) as temp"
      " )";

const char * DalQueryBuilder::DalCopyEntireRecQT =
  "DELETE FROM {dst_table_name};"
  " INSERT INTO {dst_table_name} ({opt_out_columns})"
    " SELECT {opt_out_columns} FROM {src_table_name}";

const char * DalQueryBuilder::DalCopyModRecDelQT =
  "DELETE FROM {dst_table_name} WHERE ({primary_key_columns}) NOT IN"
    " ( SELECT {primary_key_columns} FROM {src_table_name})";

const char * DalQueryBuilder::DalCopyModRecCreateQT =
  "INSERT INTO {dst_table_name} ({opt_out_columns})"
    " ( SELECT {opt_out_columns} FROM {src_table_name}"
      " WHERE ({primary_key_columns}) NOT IN"
      " ( SELECT {primary_key_columns} FROM {dst_table_name})"
    " )";

const char * DalQueryBuilder::DalCopyModRecUpdateQT =
  "UPDATE {dst_table_name} SET {opt_dst_out_columns_with_temp} FROM"
    " ( SELECT {opt_out_columns} FROM {src_table_name}"
      " WHERE ({primary_key_columns}) IN"
        " ( SELECT {primary_key_columns} FROM"
          " ( SELECT {opt_match_columns} FROM {dst_table_name} EXCEPT"
            " SELECT {opt_match_columns} FROM {src_table_name}"
          " ) AS temp1"
        " )"
    " ) AS temp WHERE {match_dst_primary_key_columns_eq_with_temp}";

const char * DalQueryBuilder::DalCopyMatchingRecQT =
  "INSERT INTO {dst_table_name} ({opt_out_columns})"
    " ( SELECT {opt_out_columns} FROM {src_table_name}"
      " {mand_WHERE_match_columns_eq} )";

const char * DalQueryBuilder::DalCheckRecIdenticalQT =
  "SELECT  COUNT(*) FROM"
    " ( SELECT {opt_match_columns} FROM"
      " ( SELECT * FROM {config1_table_name} UNION ALL"
        " SELECT * FROM {config2_table_name}"
      " ) AS temp1 GROUP BY {opt_match_columns} HAVING COUNT(*) != 2"
    " ) AS temp";

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
  { kDalUpdateRecQT, DalQueryBuilder::DalUpdateRecQT },
  { kDalGetDelRecQT, DalQueryBuilder::DalGetDelRecQT },
  { kDalGetCreatedRecQT, DalQueryBuilder::DalGetCreatedRecQT },
  { kDalGetModRecQT, DalQueryBuilder::DalGetModRecQT },
  { kDalCopyEntireRecQT, DalQueryBuilder::DalCopyEntireRecQT },
  { kDalCopyModRecDelQT, DalQueryBuilder::DalCopyModRecDelQT },
  { kDalCopyModRecCreateQT, DalQueryBuilder::DalCopyModRecCreateQT },
  { kDalCopyModRecUpdateQT, DalQueryBuilder::DalCopyModRecUpdateQT },
  { kDalCopyMatchingRecQT, DalQueryBuilder::DalCopyMatchingRecQT },
  { kDalCheckRecIdenticalQT, DalQueryBuilder::DalCheckRecIdenticalQT }
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

// Primary Key related tokens
const char * DalQueryBuilder::DalPkeyColNames = "{primary_key_columns}";
const char * DalQueryBuilder::DalMatchPriColEqTempExpr =
  "{match_dst_primary_key_columns_eq_with_temp}";

// Table related tokens
const char * DalQueryBuilder::DalCfg1TableName  = "{config1_table_name}";
const char * DalQueryBuilder::DalCfg2TableName  = "{config2_table_name}";
const char * DalQueryBuilder::DalDstTableName   = "{dst_table_name}";
const char * DalQueryBuilder::DalSrcTableName   = "{src_table_name}";

/* configuration type definitions*/
const char * DalQueryBuilder::DalStateType     = "st_";
const char * DalQueryBuilder::DalCandidateType = "ca_";
const char * DalQueryBuilder::DalRunningType   = "ru_";
const char * DalQueryBuilder::DalStartupType   = "su_";
const char * DalQueryBuilder::DalImportType    = "im_";
const char * DalQueryBuilder::DalAuditType     = "au_";

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
  std::string query_tmpl = std::string(sql_templates[dal_api_num].query_tmpl);
  size_t start_pos = 0, last_pos = 0;
  std::string token = "", replacement = "";

  //  pfc::core::Mutex ctrlr_mutex;
  //  pfc::core::ScopedMutex lock(ccc->ctrlr_mutex);

  query_str =  "";
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

  /* replace all tokens in query template to get statement */
  while (next_token(query_tmpl, start_pos, &token, &last_pos)) {
    /* get columns/table name for statement */
    if (get_bind_str(str_to_num(token), dbi, replacement,
                     table_index, first_config, second_config) != true) {
      UPLL_LOG_DEBUG("Error constructing bind string");
      query_str =  "";
      return false;
    }
    /* replace token with actual columns/table name in template */
    query_tmpl.replace((last_pos - token.length() + 1), token.length(),
                       replacement);
  }
  query_str = query_tmpl;
  return true;
}  // DalQueryBuilder::get_sql_statement

/* keep finding tokens from given query template until end of templates */
bool DalQueryBuilder::next_token(const std::string &query_template,
                                 const size_t start_position,
                                 std::string *token,
                                 size_t *last_position) const {

  if (query_template.empty()) {
    UPLL_LOG_DEBUG("Empty Query Template String");
    return false;
  }

  /* find start position of token */
  *token = "";
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

  *last_position = close_token;
  *token = query_template.substr(open_token, (close_token - open_token + 1));
  return true;
}  // DalQueryBuilder::next_token

/* converts token string to enum constants */
DalQuerytoken DalQueryBuilder::str_to_num(const std::string &tokenstr) const {
  if (tokenstr.empty()) {
    UPLL_LOG_DEBUG("Empty Token String");
    return kDalInvalidToken;
  }
  // Input Related tokens
  if (tokenstr.compare(DalMandInColNames) == 0) {
    return kDalMandInColNames;
  } else if (tokenstr.compare(DalMandInColValues) == 0) {
    return kDalMandInColValues;
  } else if (tokenstr.compare(DalMandInColUpdExpr) == 0) {
    return kDalMandInColUpdExpr;

  // Output Related tokens
  } else if (tokenstr.compare(DalMandOutColNames) == 0) {
    return kDalMandOutColNames;
  } else if (tokenstr.compare(DalOptOutColNames) == 0) {
    return kDalOptOutColNames;
  } else if (tokenstr.compare(DalOptDstOutColUpdTempExpr) == 0) {
    return kDalOptDstOutColUpdTempExpr;

  // Match Related tokens
  } else if (tokenstr.compare(DalMandMatchColNames) == 0) {
    return kDalMandMatchColNames;
  } else if (tokenstr.compare(DalOptMatchColNames) == 0) {
    return kDalOptMatchColNames;
  } else if (tokenstr.compare(DalMandMatchColEqExpr) == 0) {
    return kDalMandMatchColEqExpr;
  } else if (tokenstr.compare(DalOptMatchColEqExpr) == 0) {
    return kDalOptMatchColEqExpr;
  } else if (tokenstr.compare(DalOptMatchColEqNotLastExpr) == 0) {
    return kDalOptMatchColEqNotLastExpr;
  } else if (tokenstr.compare(DalMandMatchColLstGtrExpr) == 0) {
    return kDalMandMatchColLstGtrExpr;

  // Primary Key Related tokens
  } else if (tokenstr.compare(DalPkeyColNames) == 0) {
    return kDalPkeyColNames;
  } else if (tokenstr.compare(DalMatchPriColEqTempExpr) == 0) {
    return kDalMatchPriColEqTempExpr;

  // Table Related tokens
  } else if (tokenstr.compare(DalCfg1TableName) == 0) {
    return kDalCfg1TableName;
  } else if (tokenstr.compare(DalCfg2TableName) == 0) {
    return kDalCfg2TableName;
  } else if (tokenstr.compare(DalDstTableName) == 0) {
    return kDalDstTableName;
  } else if (tokenstr.compare(DalSrcTableName) == 0) {
    return kDalSrcTableName;
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
      config_type += std::string(DalStateType);
      return true;
    case UPLL_DT_CANDIDATE:
      config_type += std::string(DalCandidateType);
      return true;
    case UPLL_DT_RUNNING:
      config_type += std::string(DalRunningType);
      return true;
    case UPLL_DT_STARTUP:
      config_type += std::string(DalStartupType);
      return true;
    case UPLL_DT_IMPORT:
      config_type += std::string(DalImportType);
      return true;
    case UPLL_DT_AUDIT:
      config_type += std::string(DalAuditType);
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

  replc_str = "";
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
      replc_str += std::string(", ");

    if (first)
      first = false;

    replc_str += std::string(schema::ColumnName(
                             table_index,
                             col_info->get_column_index()));
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

  replc_str = "";

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
          replc_str += std::string(", ");

        if (first)
          first = false;

        replc_str += std::string("?");
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
            replc_str += std::string(", ");

          if (first)
            first = false;

          replc_str += std::string(schema::ColumnName(
                                     table_index,
                                     col_info->get_column_index()));
          replc_str += std::string("= ?");
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
            replc_str += std::string(", ");

          if (first)
            first = false;

          replc_str += std::string(schema::ColumnName(table_index, col_index));
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
            replc_str += std::string(", ");

          if (first)
            first = false;

        //  replc_str += std::string(table_name);
        //  replc_str += std::string(".");
          replc_str += std::string(schema::ColumnName(table_index, col_index));
          replc_str += std::string("= temp.");
          replc_str += std::string(schema::ColumnName(table_index, col_index));
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
              replc_str += std::string(", ");

            if (first)
              first = false;
            col_index = col_info->get_column_index();
            replc_str += std::string(table_name);
            replc_str += std::string(".");
            replc_str += std::string(schema::ColumnName(table_index,
                                                        col_index));
            replc_str += std::string("= temp.");
            replc_str += std::string(schema::ColumnName(table_index,
                                                        col_index));
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
            replc_str += std::string(", ");

          if (first)
            first = false;

          replc_str += std::string(schema::ColumnName(table_index, col_index));
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
        replc_str += std::string("WHERE ");
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
            replc_str += std::string(" AND ");

          if (first)
            first = false;

          replc_str += std::string(schema::ColumnName(
                                     table_index,
                                     col_info->get_column_index()));
          replc_str += std::string(" = ?");
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
        replc_str += std::string("WHERE ");

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
              replc_str += std::string(" AND ");

            if (first)
              first = false;

            replc_str += std::string(schema::ColumnName(
                                     table_index,
                                     col_info->get_column_index()));
            replc_str += std::string("= ?");
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
        replc_str += std::string("WHERE ");

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
            replc_str += std::string(" AND ");

          if (first)
            first = false;

          replc_str += std::string(schema::ColumnName(
                                     table_index,
                                     col_info->get_column_index()));
          if (col_count == last_pkey_pos)
            replc_str += std::string("> ?");
          else
            replc_str += std::string("= ?");
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
          replc_str += std::string(", ");

        if (first)
          first = false;

        replc_str += std::string(schema::ColumnName(table_index, col_index));
      }
      break;

    // {match_dst_primary_key_columns_eq_with_temp} - col1, col2, ...
    // <dst_table>.col1 = temp.col1 AND <dst_table>.col2 = temp.col2 ...
    // For all primary key column names of the table
    case kDalMatchPriColEqTempExpr:
      table_name = "";
      col_count = schema::TableNumPkCols(table_index);
      for (col_index = 0; col_index < col_count; col_index++) {
        if (first != true)
          replc_str += std::string(" AND ");

        if (first)
          first = false;

        if (get_bind_str(kDalDstTableName, dbi, table_name,
                         table_index, first_config, second_config) != true) {
          return false;
        }
        replc_str += std::string(table_name);
        replc_str += std::string(".");
        replc_str += std::string(schema::ColumnName(table_index, col_index));
        replc_str += std::string(" = temp.");
        replc_str += std::string(schema::ColumnName(table_index, col_index));
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

      replc_str += std::string(schema::TableName(table_index));
      break;

    // {config2_table_name} - <config2>_<table_name>
    case kDalCfg2TableName:
    // {src_table_name} - <src_config>_<table_name>
    case kDalSrcTableName:
      if (get_config_prefix(second_config, replc_str) != true) {
         UPLL_LOG_DEBUG("Second/src configuration type wrong");
         return false;
      }

      replc_str += std::string(schema::TableName(table_index));
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
