/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * table_creation.cc
 *   Prints Table Creation commands
 *   ./upll_create_table.exe > upll_create_table.sql
 *   prints sql commands for creating all tables of UPLL
 */

#include <stdio.h>
#include <stdint.h>
#include <string>
#include "sqltypes.h"
#include "sql.h"
#include "dal/dal_schema.hh"

#define uudal unc::upll::dal
#define uudschema unc::upll::dal::schema
#define uudstbl unc::upll::dal::schema::table

/*
 * Default Values:
 * varchar 		' '
 * bytea 		'\000'
 * smallint		0
 * integer 		0
 * bigint  		0
 * operstatus 		DOWN (need to check with MoMgr enum for value, use 0 for now)
 * adminstatus		DOWN (need to check with MoMgr enum for value, use 0 for now)
 * cs_attr		APPLIED(1) (for im_ and au_ tables)
 * cs_rowstatus		APPLIED(1) (for im_ and au_ tables)
 * cs_attr		NOT_APPLIED(3) (for su_, ca_ and ru_ tables)
 * cs_rowstatus		NOT_APPLIED(3) (for su_, ca_ and ru_ tables)
 * valid_attr		INVALID(0)
 */
// Config Types
enum UpllDbCfgId {
  kCfgIdStartUp = 0,
  kCfgIdCandidate,
  kCfgIdRunning,
  kCfgIdImport,
  kCfgIdAudit,
  kUpllDbNumCfgId
};
const std::string cfg_str[kUpllDbNumCfgId] =
  { "su_", "ca_", "ru_", "im_", "au_" };

// Default Types
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
    default:
      return ("");
  }
}

void build_create_table_script() {
  uint16_t cfg_idx;
  uint16_t tbl_idx;
  uint16_t col_idx;
  char size_str[4];
  UpllDbDefaultType def_type;
  std::string line;
  bool first;

  // Print Copyright
  line.clear();
  line += "/*\n"
      " * Copyright (c) 2012-2014 NEC Corporation\n"
      " * All rights reserved.\n"
      " *\n"
      " * This program and the accompanying materials are made available "
      "under the\n"
      " * terms of the Eclipse Public License v1.0 which accompanies this\n"
      " * distribution, and is available at "
      "http://www.eclipse.org/legal/epl-v10.html\n"
      " */\n";
  printf("%s", line.c_str());

  // Print File Header
  line.clear();
  line += "/**\n"
          " * upll_create_table.sql\n"
          " *   Contains SQL commands to create all the tables need for UPLL\n"
          " */\n";
  printf("\n%s", line.c_str());

  for (cfg_idx = 0; cfg_idx < kUpllDbNumCfgId; cfg_idx++) {
  for (tbl_idx = 0; tbl_idx < uudstbl::kDalNumTables; tbl_idx++) {
    // Controller Table appears only in Candidate
    if (tbl_idx == uudstbl::kDbiCtrlrTbl && cfg_idx != kCfgIdCandidate) {
      break;
    }

    // Create Table
    line.clear();
    line += "CREATE TABLE ";
    line += get_cfg_str(static_cast<UpllDbCfgId>(cfg_idx));
    line += uudschema::TableName(tbl_idx);
    line += " (";
    printf("\n%s", line.c_str());

    for (col_idx = 0; col_idx < uudschema::TableNumCols(tbl_idx); col_idx++) {
      line.clear();

      // Column Name
      line += "  ";
      line += uudschema::ColumnName(tbl_idx, col_idx);
      // printf("\nCfg : %d; Tbl : %d; Col : %d", cfg_idx, tbl_idx, col_idx);
      line += " ";

      // data type
      line += get_data_type_str(uudschema::ColumnDbDataTypeId(
              tbl_idx,
              col_idx));

      // dimension
      if (uudschema::ColumnDbArraySize(tbl_idx, col_idx) > 1 &&
          uudschema::ColumnDbDataTypeId(tbl_idx, col_idx) != SQL_BINARY) {
        line += "(";
        memset(size_str, '0', 4);
        sprintf(size_str, "%zd", uudschema::ColumnDbArraySize(
                tbl_idx,
                col_idx));
        line += size_str;
        line += ")";
      }

      // default
      line += " default ";
      def_type = get_default_type(uudschema::ColumnName(tbl_idx, col_idx),
                                  uudschema::ColumnDbDataTypeId(tbl_idx,
                                                                col_idx));
      if (def_type == kDefaultTypeBinary) {
        line += "'";
        for (uint16_t i = 0; i < uudschema::ColumnDbArraySize(tbl_idx,
                                                              col_idx); i++) {
          line+= get_default_str((UpllDbCfgId)cfg_idx, def_type);
        }
        line += "'";
      } else {
        line+= get_default_str((UpllDbCfgId)cfg_idx, def_type);
      }

      // uinque constraint for rename tables
      /*
      if (strstr(uudschema::TableName(tbl_idx), "rename") > 0 &&
          col_idx >= uudschema::TableNumPkCols(tbl_idx)) {
        line += " unique";
      }*/
      line += ",";
      printf("\n  %s", line.c_str());
    }  // for all columns
    line.clear();

    // Primary Keys
    line += "PRIMARY KEY(";
    first = true;
    for (col_idx = 0; col_idx < uudschema::TableNumPkCols(tbl_idx);
         col_idx++) {
      if (first == false) {
        line += ", ";
      } else {
        first = false;
      }
      line += uudschema::ColumnName(tbl_idx, col_idx);
    }
    if ((cfg_idx == kCfgIdCandidate) &&
        uudschema::TableParentIndex(tbl_idx) < uudstbl::kDalNumTables) {
      line += "),";
    } else {
      line += "));";
    }
    printf("\n  %s", line.c_str());

    // Foreign Keys
    if ((cfg_idx == kCfgIdCandidate) &&
        uudschema::TableParentIndex(tbl_idx) < uudstbl::kDalNumTables) {
      line.clear();
      line += "FOREIGN KEY(";
      first = true;
      for (col_idx = 0; col_idx < uudschema::TableNumFkCols(tbl_idx);
           col_idx++) {
        if (first == false) {
          line += ", ";
        } else {
          first = false;
        }
        line += uudschema::ColumnName(tbl_idx, col_idx);
      }
      line += ") ";
      line += "REFERENCES ";
      line += get_cfg_str(static_cast<UpllDbCfgId>(cfg_idx));
      line += uudschema::TableName(uudschema::TableParentIndex(tbl_idx));
      line += "(";
      first = true;
      for (col_idx = 0; col_idx < uudschema::TableNumFkCols(tbl_idx);
           col_idx++) {
        if (first == false) {
          line += ", ";
        } else {
          first = false;
        }
        line += uudschema::ColumnName(uudschema::TableParentIndex(tbl_idx),
                                      col_idx);
      }
      line += "));";
      printf("\n  %s", line.c_str());
    }

    printf("\n");
  }  // for all tables
  }  // for all config types
}  // build_create_table_script

void build_create_index_script() {
  printf("\nCREATE INDEX ca_vbr_if_tbl_semindex ON ca_vbr_if_tbl USING btree (logical_port_id, controller_name, domain_id, valid_portmap, valid_logical_port_id);");
  printf("\nCREATE INDEX ca_policingprofile_entry_tbl_semindex ON ca_policingprofile_entry_tbl USING btree (flowlist, valid_flowlist);");
  printf("\nCREATE INDEX ca_vtn_policingmap_tbl_semindex ON ca_vtn_policingmap_tbl USING btree (policername, valid_policername);");
  printf("\nCREATE INDEX ca_vbr_policingmap_tbl_semindex ON ca_vbr_policingmap_tbl USING btree (policername, valid_policername);");
  printf("\nCREATE INDEX ca_vbr_if_policingmap_tbl_semindex ON ca_vbr_if_policingmap_tbl USING btree (policername, valid_policername);");

  printf("\nCREATE INDEX ru_vbr_if_tbl_showindex ON ru_vbr_if_tbl (vtn_name, vex_name, valid_vex_name);");
}

int main() {
  build_create_table_script();
  build_create_index_script();
  return 0;
}
