/*
 * Copyright (c) 2014-2015 NEC Corporation
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

#include "table_common.hh"

std::string insert_default_row_table_name(uudstbl::kDalTableIndex table_index, uint16_t cfg_idx) {
  std::string tbl_name = "", select_drop_dirty = "";
  if(table_index == uudstbl::kDbiCfgTblDirtyTbl)
    select_drop_dirty.assign(select_and_drop_dirty_insert_function);
  else if(table_index == uudstbl::kDbiUpllSystemTbl)
    select_drop_dirty.assign(select_and_drop_systemTbl_insert_function);

  size_t position = select_drop_dirty.find("!!");
  select_drop_dirty.replace(position, std::string("!!").length(), "");

  tbl_name += get_cfg_str(static_cast<UpllDbCfgId>(cfg_idx));
  tbl_name += uudschema::TableName(table_index);
  select_drop_dirty.insert(position, tbl_name.c_str());
  return select_drop_dirty;
}

std::string insert_default_rows_in_dirty_tbl() {
  // Insert default rows into ca_upll_cfg_dirty_tbl
  // TODO: Remove hardcoding for operations
  std::string default_rows = "";
  default_rows.assign(dirty_insert_function);
  default_rows += dirty_insert_values;
  static char operations[] = { '1', '2', '3' };  // UNC_OP_DELETE, UNC_OP_CREATE, UNC_OP_UPDATE ;
  static int num_operations = sizeof(operations)/sizeof(operations[0]);

  size_t position = default_rows.find("??");
  default_rows.replace(position, std::string("??").length(), "");

  string insert_sql = "\n";
  bool first = true;
  for (uint16_t table_index = 0; (table_index < uudstbl::kDalNumTables);
     table_index++) {
    if (table_index == uudstbl::kDbiCfgTblDirtyTbl ||
        table_index == uudstbl::kDbiUpllSystemTbl ||
        table_index == uudstbl::kDbiCtrlrTbl ||
        table_index == uudstbl::kDbiVtnCfgTblDirtyTbl) {
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
  default_rows.insert(position, insert_sql.c_str());
  return default_rows;
}

std::string insert_default_rows(uudstbl::kDalTableIndex table_index, uint16_t cfg_idx) {
  std::string default_rows = "";
  if(table_index == uudstbl::kDbiCfgTblDirtyTbl)
    default_rows.assign(dirty_insert_function);
  else if(table_index == uudstbl::kDbiUpllSystemTbl)
    default_rows.assign(systemTbl_insert_function);
  default_rows += dirty_insert_values;

  size_t position = default_rows.find("??");
  default_rows.replace(position, std::string("??").length(), "");
  default_rows.insert(position, default_rows_.c_str());
  default_rows += "\n";
  default_rows += insert_default_row_table_name(table_index, cfg_idx).c_str();
  return default_rows;
}

// Method to create tables
std::string build_create_table_script(UpllDBAddition dbAdditionMethod,
                                      uint16_t cfg_idx = 0) {
  uint16_t tbl_idx;
  uint16_t col_idx;
  char size_str[4];
  UpllDbDefaultType def_type;
  string line;
  line.clear();
  bool first;

  if(dbAdditionMethod == SINGLECONFIGONENTIRETABLE)
    goto jump;

  line += "\n";
  for (cfg_idx = 0; cfg_idx < kUpllDbNumCfgId; cfg_idx++) {
    jump:
    for (tbl_idx = 0; tbl_idx < uudstbl::kDalNumTables; tbl_idx++) {
      // Controller Table appears only in particular configuration
      if ((tbl_idx == uudstbl::kDbiCtrlrTbl && cfg_idx != kCfgIdCandidate) ||
          (tbl_idx == uudstbl::kDbiCfgTblDirtyTbl && cfg_idx == kCfgIdTempDel) ||
          (tbl_idx == uudstbl::kDbiVtnCfgTblDirtyTbl && cfg_idx != kCfgIdCandidate) ||
          (tbl_idx == uudstbl::kDbiPpScratchTbl && cfg_idx != kCfgIdCandidate) || 
          (tbl_idx == uudstbl::kDbiFlScratchTbl && cfg_idx != kCfgIdCandidate) ||
          (tbl_idx == uudstbl::kDbiSpdScratchTbl && cfg_idx != kCfgIdCandidate)) {
        break;
      }
      if ((cfg_idx == kCfgIdSysTbl && tbl_idx != uudstbl::kDbiUpllSystemTbl) ||
          (cfg_idx != kCfgIdSysTbl && tbl_idx == uudstbl::kDbiUpllSystemTbl)) {
          continue;
      }

      // Appends Create Table relevant string
      if(dbAdditionMethod != SINGLECONFIGONENTIRETABLE) {
        line += "CREATE TABLE ";
      } else {
        line += "CREATE TABLE IF NOT EXISTS ";
      }
      line += get_cfg_str(static_cast<UpllDbCfgId>(cfg_idx));
      line += uudschema::TableName(tbl_idx);
      line += " (\n  ";

      for (col_idx = 0; col_idx < uudschema::TableNumCols(tbl_idx) + 2; col_idx++) {
        // if col_idx > NumCols and table type is Candidate, continue
        // the loop to create c_flag and u_flag
        if (col_idx >= uudschema::TableNumCols(tbl_idx)) {
          // To avoid c_flag and u_flag in table generation
          if (cfg_idx != kCfgIdCandidate || tbl_idx == uudstbl::kDbiCtrlrTbl ||
              tbl_idx == uudstbl::kDbiCfgTblDirtyTbl || tbl_idx == uudstbl::kDbiVtnCfgTblDirtyTbl ||
              tbl_idx == uudstbl::kDbiUpllSystemTbl || tbl_idx == uudstbl::kDbiPpScratchTbl ||
              tbl_idx == uudstbl::kDbiFlScratchTbl || tbl_idx == uudstbl::kDbiSpdScratchTbl)
            break;
        }


        // Column Name
        line += "  ";
        if (cfg_idx == kCfgIdCandidate && tbl_idx != uudstbl::kDbiCfgTblDirtyTbl)
          line += uudschema::CandColumnName(tbl_idx, col_idx);
        else
          line += uudschema::ColumnName(tbl_idx, col_idx);
        line += " ";

        // Data type
        if (cfg_idx == kCfgIdCandidate && tbl_idx != uudstbl::kDbiCfgTblDirtyTbl)
          line += get_data_type_str(uudschema::CandColumnDbDataTypeId(tbl_idx, col_idx));
        else
          line += get_data_type_str(uudschema::ColumnDbDataTypeId(tbl_idx, col_idx));

        if (cfg_idx == kCfgIdCandidate && tbl_idx != uudstbl::kDbiCfgTblDirtyTbl) {
          if (uudschema::CandColumnDbArraySize(tbl_idx, col_idx) > 1 &&
              uudschema::CandColumnDbDataTypeId(tbl_idx, col_idx) != SQL_BINARY) {
            line += "(";
            memset(size_str, '0', 4);
            sprintf(size_str, "%zd", uudschema::CandColumnDbArraySize(tbl_idx, col_idx));
            line += size_str;
            line += ")";
          }
        } else {
          // Dimension
          if (uudschema::ColumnDbArraySize(tbl_idx, col_idx) > 1 &&
              uudschema::ColumnDbDataTypeId(tbl_idx, col_idx) != SQL_BINARY) {
            line += "(";
            memset(size_str, '0', 4);
            sprintf(size_str, "%zd", uudschema::ColumnDbArraySize(tbl_idx, col_idx));
            line += size_str;
            line += ")";
          }
        }

        // Default
        line += " default ";
        if (cfg_idx == kCfgIdCandidate && tbl_idx != uudstbl::kDbiCfgTblDirtyTbl){
          def_type = get_default_type(uudschema::CandColumnName(tbl_idx, col_idx),
                            uudschema::CandColumnDbDataTypeId(tbl_idx, col_idx));
        } else {
          def_type = get_default_type(uudschema::ColumnName(tbl_idx, col_idx),
                            uudschema::ColumnDbDataTypeId(tbl_idx, col_idx));
        }

        if (def_type == kDefaultTypeBinary) {
          line += "'";
          if (cfg_idx == kCfgIdCandidate && tbl_idx != uudstbl::kDbiCfgTblDirtyTbl) {
            for (uint16_t i = 0; i < uudschema::ColumnDbArraySize(tbl_idx, col_idx); i++) {
              line+= get_default_str((UpllDbCfgId)cfg_idx, def_type);
            }
        } else {
          for (uint16_t i = 0; i < uudschema::ColumnDbArraySize(tbl_idx, col_idx); i++) {
            line+= get_default_str((UpllDbCfgId)cfg_idx, def_type);
          }
        }
        line += "'";
        } else {
          line+= get_default_str((UpllDbCfgId)cfg_idx, def_type);
        }

        line += ",\n  ";
      }

      // Primary Keys
      line += "PRIMARY KEY(";
      first = true;
      for (col_idx = 0;
             col_idx < uudschema::TableNumPkCols(tbl_idx); col_idx++) {
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
      line += "\n";

      // Foreign Keys
      if ((cfg_idx == kCfgIdCandidate) &&
          uudschema::TableParentIndex(tbl_idx) < uudstbl::kDalNumTables) {
        line += "  FOREIGN KEY(";
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
          line += uudschema::ColumnName(
                              uudschema::TableParentIndex(tbl_idx), col_idx);
        }
        line += "));\n";
      }
      line += "\n";
    }
    if(dbAdditionMethod == SINGLECONFIGONENTIRETABLE)
      break;
  }
  return line;
}
