/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * table_defines.hh - contians schema information of all tables
 *
 */

#ifndef _TABLE_DEFINES_HH_
#define _TABLE_DEFINES_HH_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <iostream>
#include <fstream>
#include "sqltypes.h"
#include "sql.h"
#include "dal/dal_schema.hh"

namespace uudal = unc::upll::dal;
namespace uudschema = unc::upll::dal::schema;
namespace uudstbl = unc::upll::dal::schema::table;

#define create_filepath "../../../sql/"
#define upgrade_filepath "../../../sql/upgrade/"
#define downgrade_filepath "../../../sql/downgrade/"
#define optimize_filepath "../../../sql/optimize/"

std::string copyrights_header =
    "/*\n"
    " * Copyright (c) 2012-2015 NEC Corporation\n"
    " * All rights reserved.\n"
    " *\n"
    " * This program and the accompanying materials are made available under "
    "the\n"
    " * terms of the Eclipse Public License v1.0 which accompanies this\n"
    " * distribution, and is available at "
    "http://www.eclipse.org/legal/epl-v10.html\n"
    " */\n"

std::string copyrights_header_2014 =
    "/*\n"
    " * Copyright (c) 2015 NEC Corporation\n"
    " * All rights reserved.\n"
    " *\n"
    " * This program and the accompanying materials are made available under "
    "the\n"
    " * terms of the Eclipse Public License v1.0 which accompanies this\n"
    " * distribution, and is available at "
    "http://www.eclipse.org/legal/epl-v10.html\n"
    " */\n"

// Config Types
enum UpllDbCfgId {
  kCfgIdStartUp = 0,
  kCfgIdCandidate,
  kCfgIdRunning,
  kCfgIdImport,
  kCfgIdAudit,
  kCfgIdTempDel,
  kCfgIdSysTbl,
  kUpllDbNumCfgId
};

// Table/Column addition methods
enum UpllDBAddition {
  ENTIRETABLE = 0,  // = 0
  SINGLECONFIGONENTIRETABLE,
  SINGLECONFIGONPARTICULARTABLE,
  ENTIRECOLUMN,     // = 3
  SINGLECONFIGONENTIRECOLUMN,
  SINGLECONFIGONPARTICULARCOLUMN,
};

const std::string cfg_str[kUpllDbNumCfgId] =
  { "su_", "ca_", "ru_", "im_", "au_", "ca_del_", "sy_" };

// default types
enum UpllDbDefaultType {
  kDefaultTypeVarchar = 0,
  kDefaultTypeBinary,
  kDefaultTypeSmallint,
  kDefaultTypeInteger,
  kDefaultTypeBigint,
  kDefaultTypeOperStatus,
  kDefaultTypeAdminStatus,
  kDefaultTypeCsAttr,
  kDefaultTypeCsRowstatus,
  kDefaultTypeValidAttr,
  kUpllDbNumDefaultTypes
};

const uint16_t kUpllNumCfgTypes = 5;
const std::string default_str[kUpllDbNumCfgId][kUpllDbNumDefaultTypes] = {
  {"' '", "\\000", "0", "0", "0", "0", "0", "3", "3", "0"},
  {"' '", "\\000", "0", "0", "0", "0", "0", "3", "3", "0"},
  {"' '", "\\000", "0", "0", "0", "0", "0", "3", "3", "0"},
  {"' '", "\\000", "0", "0", "0", "0", "0", "1", "1", "0"},
  {"' '", "\\000", "0", "0", "0", "0", "0", "1", "1", "0"},
  {"' '", "\\000", "0", "0", "0", "0", "0", "1", "1", "0"},
  {"' '", "\\000", "0", "0", "0", "0", "0", "1", "1", "0"}
};

std::string get_cfg_str(UpllDbCfgId cfg_idx) {
  if (cfg_idx >= kUpllDbNumCfgId) {
    return "";
  }
  return (cfg_str[cfg_idx]);
}

std::string get_default_str(UpllDbCfgId cfg_idx, UpllDbDefaultType def_type) {
  if (cfg_idx >= kUpllDbNumCfgId || def_type >= kUpllDbNumDefaultTypes) {
    return "";
  }
  return (default_str[cfg_idx][def_type]);
}

UpllDbDefaultType get_default_type(const char *col_name,
                                   SQLSMALLINT db_datatype) {
  std::string col_name_str(col_name);
  if (col_name_str.compare("cs_rowstatus") == 0) {
    return kDefaultTypeCsRowstatus;
  } else if (col_name_str.find("cs_") == 0) {
    return kDefaultTypeCsAttr;
  } else if (col_name_str.find("valid_") == 0) {
    return kDefaultTypeValidAttr;
  } else if (col_name_str.compare("oper_status") == 0) {
    return kDefaultTypeOperStatus;
  } else if (col_name_str.compare("admin_status") == 0) {
    return kDefaultTypeAdminStatus;
  }
  switch (db_datatype) {
    case SQL_VARCHAR:
      return kDefaultTypeVarchar;
    case SQL_SMALLINT:
      return kDefaultTypeSmallint;
    case SQL_INTEGER:
      return kDefaultTypeInteger;
    case SQL_BIGINT:
      return kDefaultTypeBigint;
    case SQL_BINARY:
      return kDefaultTypeBinary;
    default:
      return (kUpllDbNumDefaultTypes);
  }
}

std::string get_data_type_str(SQLSMALLINT dal_data_type) {
  switch (dal_data_type) {
    case SQL_VARCHAR:
      return ("varchar");
    case SQL_SMALLINT:
      return ("smallint");
    case SQL_INTEGER:
      return ("integer");
    case SQL_BIGINT:
      return ("bigint");
    case SQL_BINARY:
      return ("bytea");
    case SQL_CHAR:
      return ("character");
    default:
      return ("");
  }
}

std::string dirty_insert_function =
  "CREATE OR REPLACE function f_insert_row_if_not_exists(_dest_tblname varchar(32)) RETURNS VOID\n";

std::string systemTbl_insert_function =
  "CREATE OR REPLACE function f_init_sy_upll_system_tbl(_dest_tblname varchar(32)) RETURNS VOID\n";

std::string dirty_insert_values =
  "LANGUAGE plpgsql AS\n"
  "$func$\n"
  "DECLARE row_count integer;\n"
  "BEGIN\n"
  "  EXECUTE 'SELECT count(*) FROM ' || _dest_tblname INTO row_count;\n"
  "  IF ( row_count = 0) THEN\n"
  "    EXECUTE 'INSERT INTO ' || _dest_tblname || ' values ' || ??"
  "  ELSE\n"
  "    RAISE NOTICE 'Table % already contains % rows', _dest_tblname, row_count;\n"
  "  END IF;\n"
  "  RETURN;\n"
  "END\n"
  "$func$;\n";

std::string select_and_drop_dirty_insert_function =
  "SELECT f_insert_row_if_not_exists('!!');\n"
  "DROP function f_insert_row_if_not_exists(_dest_tblname varchar(32));\n";

std::string select_and_drop_systemTbl_insert_function =
  "SELECT f_init_sy_upll_system_tbl('!!');\n"
  "DROP function f_init_sy_upll_system_tbl(_dest_tblname varchar(32));\n";

#endif  // DAL_TABLE_DEFINES_HH_


