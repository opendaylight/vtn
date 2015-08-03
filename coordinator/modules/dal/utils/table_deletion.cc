/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 *   table_deletion.cc
 *   Generates sql commands for deleting all tables of UPLL
 */

#include <stdio.h>
#include <stdint.h>
#include <string>
#include <iostream>
#include <fstream>
#include "dal/dal_schema.hh"
#include "table_defines.hh"

using namespace std;

ofstream upll_delete_file;

void build_delete_table_script() {
  uint16_t cfg_idx;
  uint16_t tbl_idx;
  std::string line;

  // Print Copyright
  upll_delete_file << copyrights_header.c_str();

  // Print File Header
  upll_delete_file <<
    "\n"
    "/**\n"
    " *  upll_delete_table.sql\n"
    " *  Contains SQL commands to delete all the tables created by UPLL\n"
    " */\n";

  for (cfg_idx = 0; cfg_idx < kUpllDbNumCfgId; cfg_idx++) {
    for (uint16_t tbl_iter = uudstbl::kDalNumTables; tbl_iter > 0; tbl_iter--) {
      tbl_idx = tbl_iter - 1;
      // Controller Table appears only in Candidate
      if (tbl_idx == uudstbl::kDbiCtrlrTbl && cfg_idx != kCfgIdCandidate) {
        continue;
      }

      // Delete Table
      line.clear();
      line += "DROP TABLE ";
      line += get_cfg_str(static_cast<UpllDbCfgId>(cfg_idx));
      line += uudschema::TableName(tbl_idx);
      line += ";";
      upll_delete_file << endl << line.c_str();
    }
  }
}

int main() {
  upll_delete_file.open("delete_script.sql");
  build_delete_table_script();
  upll_delete_file.close();
  return 0;
}
