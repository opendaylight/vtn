/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * table_deletion.cc
 *   Prints Table Deletion commands
 *   ./upll_delete_table.exe > upll_delete_table.sql
 *   prints sql commands for deleting all tables of UPLL
 */

#include <stdio.h>
#include <stdint.h>
#include <string>
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

std::string get_cfg_str(UpllDbCfgId cfg_idx) {
  if (cfg_idx >= kUpllDbNumCfgId) {
    return "";
  }
  return (cfg_str[cfg_idx]);
}

void build_delete_table_script() {
  uint16_t cfg_idx;
  uint16_t tbl_idx;
  std::string line;

  // Print Copyright
  line.clear();
/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
  printf("%s", line.c_str());

  // Print File Header
  line.clear();
  line += "/**\n"
          " * upll_delete_table.sql\n"
          " *   Contains SQL commands to delete all the tables created by UPLL\n"
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
    line += "DROP TABLE ";
    line += get_cfg_str(static_cast<UpllDbCfgId>(cfg_idx));
    line += uudschema::TableName(tbl_idx);
    line += ";";
    printf("\n%s", line.c_str());
  }
  }
}


int main() {
  build_delete_table_script();
  return 0;
}
