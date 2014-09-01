/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 *   table_common.cc
 *   Contians common accessible functions to generate schema
 * 
 */


std::string insert_default_rows_in_dirty_tbl() {
  // Insert default rows into ca_upll_cfg_dirty_tbl
  // TODO: Remove hardcoding for operations
  static char operations[] = { '1', '2', '3' };  // UNC_OP_DELETE, UNC_OP_CREATE, UNC_OP_UPDATE };
  static int num_operations = sizeof(operations)/sizeof(operations[0]);

  size_t position = dirty_insert_values.find("??");
  dirty_insert_values.replace(position, std::string("??").length(), "");

  string insert_sql = "\n";
  bool first = true;
  for (uint16_t table_index = 0; (table_index < uudstbl::kDalNumTables);
     table_index++) {
    if (table_index == uudstbl::kDbiCtrlrTbl ||
        table_index == uudstbl::kDbiCfgTblDirtyTbl) {
      continue;
    }
    for (int op = 0; op < num_operations; op++) {
      if (!first) {
        insert_sql += ",'\n";
      }
      insert_sql += "      '(' || quote_literal('";
      insert_sql += uudstbl::table_schema[table_index].table_name;
      insert_sql += "') || ', ";
      insert_sql += operations[op];
      insert_sql += ", 0)";
      first = false;
     }
  }
  insert_sql += ";';\n";
  dirty_insert_values.insert(position, insert_sql.c_str());
  return dirty_insert_values;
}


