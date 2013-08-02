/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * DalQueryBuilder.h
 *
 *  Created on: Jan 6, 2013
 *      Author: guest
 */

#ifndef QUERYBUILDER_H_
#define QUERYBUILDER_H_

#include <stdint.h>
#include <string>
#include "dal_bind_info.hh"

namespace unc {
namespace upll {
namespace dal {

/* enum corresponding to each sql statement templates */
enum DalApiNum {
  kDalGetSingleRecQT = 0,
  kDalGetMultiRecQT,
  kDalRecExistsQT,
  kDalGetSibBegQT,
  kDalGetSibRecQT,
  kDalGetSibCountQT,
  kDalGetRecCountQT,
  kDalCreateRecQT,
  kDalDelRecQT,
  kDalUpdateRecQT,
  kDalGetDelRecQT,
  kDalGetCreatedRecQT,
  kDalGetModRecQT,
  kDalCopyEntireRecQT,
  kDalCopyModRecDelQT,
  kDalCopyModRecCreateQT,
  kDalCopyModRecUpdateQT,
  kDalCopyMatchingRecQT,
  kDalCheckRecIdenticalQT
};

/* sql template tokens */
enum DalQuerytoken {
  kDalInvalidToken,

  // Enum for Input Related Tokens
  kDalMandInColNames,
  kDalMandInColValues,
  kDalMandInColUpdExpr,

  // Enum for Output Related Tokens
  kDalMandOutColNames,
  kDalOptOutColNames,
  kDalOptDstOutColUpdTempExpr,

  // Enum for Match Related Tokens
  kDalMandMatchColNames,
  kDalOptMatchColNames,
  kDalMandMatchColEqExpr,
  kDalOptMatchColEqExpr,
  kDalOptMatchColEqNotLastExpr,
  kDalMandMatchColLstGtrExpr,

  // Enum for Primary Key Related Tokens
  kDalPkeyColNames,
  kDalMatchPriColEqTempExpr,

  // Enum for Table related Tokens
  kDalCfg1TableName,
  kDalCfg2TableName,
  kDalDstTableName,
  kDalSrcTableName
};

class DalQueryBuilder {
  public:
    DalQueryBuilder();
    virtual ~DalQueryBuilder();

    /* get_sql_statement
     *   get sql query statement executed by database
     *
     * @param[in] API_NUM         - constant value of query template
     * @param[in] dbi             - user binding information for columns/table
     * @param[in/out] query_str   - sql statement string gets return
     * @param[in] table_index     - table index
     * @param[in] first_config    - first configuration table name
     * @param[in] second_config   - second configuration table name
     *                              if nothing, default taken
     * @return bool               - true on success else false
     */
    bool get_sql_statement(const DalApiNum API_NUM,
                           const upll::dal::DalBindInfo *dbi,
                           std::string &query_str,
                           const DalTableIndex table_index,
                           const UpllCfgType first_config,
                           const UpllCfgType second_config = UPLL_DT_INVALID)
                           const;

    // std::string query_str;

    /* query templates corresponding to API */
    static const char * DalGetSingleRecQT;
    static const char * DalGetMultiRecQT;
    static const char * DalRecExistsQT;
    static const char * DalGetSibBegQT;
    static const char * DalGetSibRecQT;
    static const char * DalGetSibCountQT;
    static const char * DalGetRecCountQT;
    static const char * DalCreateRecQT;
    static const char * DalDelRecQT;
    static const char * DalUpdateRecQT;
    static const char * DalGetDelRecQT;
    static const char * DalGetCreatedRecQT;
    static const char * DalGetModRecQT;
    static const char * DalCopyEntireRecQT;
    static const char * DalCopyModRecDelQT;
    static const char * DalCopyModRecCreateQT;
    static const char * DalCopyModRecUpdateQT;
    static const char * DalCopyMatchingRecQT;
    static const char * DalCheckRecIdenticalQT;

    /* replacement tokens */
    // Input Related Tokens
    // {mand_in_columns} - col1, col2, ...
    // column names of attributes bound for input
    static const char * DalMandInColNames;
    // {mand_insert_?} - ?, ?, ?, ...
    // count of attributes bound for input
    static const char * DalMandInColValues;
    // {mand_in_columns_with_?} - col1 = ?, col2 = ?, col3 = ?, ...
    // For all attributes bound for input
    static const char * DalMandInColUpdExpr;

    // Output Related Tokens
    // {mand_out_columns} - col1, col2, ...
    // column names of attributes bound for output
    static const char * DalMandOutColNames;
    // {opt_out_columns} - col1, col2, ...
    // column names of attributes bound for output
    // All column names, if no attributes bound for output
    static const char * DalOptOutColNames;
    // {opt_out_columns_with_temp} -
    // <table>.col1 = temp.col1, <table>.col2 = temp.col2, ...
    // column names of attributes bound for output
    // All column names, if no attributes bound for output
    static const char * DalOptDstOutColUpdTempExpr;

    // Match Related Tokens
    // {mand_match_columns} - col1, col2, ...
    // column names of attributes bound for match
    static const char * DalMandMatchColNames;
    // {opt_match_columns} - col1, col2, ...
    // column names of attributes bound for match
    // All column names, if no attributes bound for output
    static const char * DalOptMatchColNames;
    // {mand_WHERE_match_columns_eq} -
    // WHERE col1 = ?, col2 = ?, ...
    // column names of attributes bound for match
    static const char * DalMandMatchColEqExpr;
    // {opt_WHERE_match_columns_eq} -
    // WHERE col1 = ?, col2 = ?, ...
    // column names of attributes bound for match
    // Empty string if no attributes bound for match
    static const char * DalOptMatchColEqExpr;
    // {opt_WHERE_match_columns_eq_not_last} -
    // WHERE col1 = ?, col2 = ?, ..., col(n-1) = ?
    // n is the number of attributes bound for match
    // column names of attributes bound for match
    static const char * DalOptMatchColEqNotLastExpr;
    // {mand_WHERE_match_columns_last_greater} -
    // WHERE col1 = ?, col2 = ?, ..., coln > ?
    // n is the number of attributes bound for match
    // column names of attributes bound for match
    static const char * DalMandMatchColLstGtrExpr;

    // Primary Key Related Tokens
    // {primary_key_columns} - col1, col2, ...
    // All primary key column names of the table
    static const char * DalPkeyColNames;
    // {match_dst_primary_key_columns_eq_with_temp} - col1, col2, ...
    // <dst_table>.col1 = temp.col1 AND <dst_table>.col2 = temp.col2 ...
    // For all primary key column names of the table
    static const char * DalMatchPriColEqTempExpr;

    // Table Related Tokens
    // {config1_table_name} - <config1>_<table_name>
    static const char * DalCfg1TableName;
    // {config2_table_name} - <config2>_<table_name>
    static const char * DalCfg2TableName;
    // {src_config_table_name} - <src_config>_<table_name>
    static const char * DalDstTableName;
    // {dst_config_table_name} - <dst_config>_<table_name>
    static const char * DalSrcTableName;

    /* configuration type */
    static const char * DalStateType;
    static const char * DalCandidateType;
    static const char * DalRunningType;
    static const char * DalStartupType;
    static const char * DalImportType;
    static const char * DalAuditType;

  private:
    /* str_to_num
     *   converts token string to enum constants
     *
     * @param[in] tokenstr    - replacement token string
     * @return DalQuerytoken  - valid DalQuerytoken enum on success
     */
    DalQuerytoken str_to_num(const std::string &tokenstr) const;

    /* get_config_prefix
     *   verifies config type, and adds appropriate config type prefix
     *
     * @param[in] type              - configuration Type
     * @param[in/out] config_type   - configuration string prefix
     * @return bool                 - true on success else false
     */
    bool get_config_prefix(const UpllCfgType type,
                           std::string &config_type) const;

    /* next_token
     *   keep finding tokens in sql template until all token found
     *
     * @param[in] query_template    - query template specific to API
     * @param[in] start_position    - next start position of each token
     * @param[out] token            - actual token string which need to replace
     * @param[in/out] last_position - last end position of previous token
     * @return bool                 - true on success else false
     */
    bool next_token(const std::string &query_template,
                    const size_t start_position,
                    std::string *token,
                    size_t *last_position) const;

    /* get_bind_str
     *   generate column's or table name string according to token, and it
     *   will be use for sql statement
     *
     * @param[in] token           - replacement token
     * @param[in] dbi             - Dal Bind info pointers for list of columns
     * @param[out] replc_str      - actual columns names to table names string
     * @param[in] table_index     - table index
     * @param[in] first_config    - the first configuration type
     * @param[in] second_config   - second configuration type
     * @return bool               - true on success else false
     */
    bool get_bind_str(const DalQuerytoken token,
                      const DalBindInfo *dbi,
                      std::string &replc_str,
                      const DalTableIndex table_index,
                      const UpllCfgType first_config,
                      const UpllCfgType second_config) const;


    /* find_last_pkey_pos_for_match
     *   Finds the position of greater Primary Key index bound for match
     *   in DalBindList
     * @param[in] dbi             - Dal Bind info pointers for list of columns
     * @param[out] &last_pkey_pos - position of greater pkey index which is
     *                              bound for match
     */
    bool find_last_pkey_pos_for_match(const DalBindInfo *dbi,
                                      uint16_t *last_pkey_pos) const;

    /* fill_column_names
     *   Build comma separated column names as available in bind info based
     *   on the IoCode
     *
     * @param[in] io_code         - Input/Output/Match
     * @param[in] table_index     - table index
     * @param[in] dbi             - Dal Bind info pointers for list of columns
     * @param[out] replc_str      - actual columns names to table names string
     * @return bool               - true on success else false
     */
    bool fill_column_names(const DalIoCode io_code,
                           const DalTableIndex table_index,
                           const DalBindInfo *dbi,
                           std::string &replc_str) const;
};
}  // namespace dal
}  // namespace upll
}  // namespace unc
#endif /* QUERYBUILDER_H_ */

