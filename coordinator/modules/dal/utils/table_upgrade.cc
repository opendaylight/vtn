/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 *   table_updation.cc
 *   Addes new tables and updates fields
 */

#include "table_defines.hh"
#include "table_upgrade.hh"
#include "table_common.cc"


//Writes Copyrights information
void copyrights(){
  upll_upgrade_file << copyrights_header.c_str();
}

//Writes header information
void header(char *version) {
  char upll_filename[80];
  sprintf(upll_filename,"\n/**\n *  %s0000_upll_up.sql\n",version);
  upll_upgrade_file << upll_filename;
  upll_upgrade_file << 
    " *  Contains SQL commands to update UPLL DB schema for " 
    << version << " upgrade\n";
  upll_upgrade_file << " */\n";
}

//Adds newly added tables
void add_new_table(uudstbl::kDalTableIndex new_table[], uint16_t table_size) {
  uint16_t cfg_idx;
  uint16_t tbl_idx;
  uint16_t col_idx;
  char size_str[4];
  UpllDbDefaultType def_type;
  string line;
  bool first;

  upll_upgrade_file << "\n/* ADDING NEW TABLE(S) */\n";
  for (cfg_idx = 0; cfg_idx < kUpllDbNumCfgId; cfg_idx++) {
    for (uint16_t idx = 0;
         idx < table_size/sizeof(uudstbl::kDalTableIndex);
         idx++) {
      // Controller Table appears only in Candidate
      if (tbl_idx == uudstbl::kDbiCtrlrTbl && cfg_idx != kCfgIdCandidate)
        break;
      tbl_idx = new_table[idx];

      line.clear();
      line += "CREATE TABLE";
      line += " IF NOT EXISTS ";
      line += get_cfg_str(static_cast<UpllDbCfgId>(cfg_idx));
      line += uudschema::TableName(tbl_idx);
      line += " (";
      upll_upgrade_file << endl << line.c_str();

      // Loop for list if columns in each table ids
      for (col_idx = 0; col_idx < uudschema::TableNumCols(tbl_idx); col_idx++) {
        line.clear();

        // Column Name
        line += "  ";
        line += uudschema::ColumnName(tbl_idx, col_idx);
        line += " ";

        // data type
        line += get_data_type_str(
                         uudschema::ColumnDbDataTypeId(tbl_idx, col_idx));

        // dimension
        if (uudschema::ColumnDbArraySize(tbl_idx, col_idx) > 1 &&
            uudschema::ColumnDbDataTypeId(tbl_idx, col_idx) != SQL_BINARY) {
          line += "(";
          memset(size_str, '0', 4);
          sprintf(size_str, "%zd", 
                   uudschema::ColumnDbArraySize(tbl_idx, col_idx));
          line += size_str;
          line += ")";
        }

        // default
        line += " default ";
        def_type = get_default_type(
                        uudschema::ColumnName(tbl_idx, col_idx),
                        uudschema::ColumnDbDataTypeId(tbl_idx, col_idx));
        if (def_type == kDefaultTypeBinary) {
          line += "'";
          for (uint16_t i = 0; 
                  i < uudschema::ColumnDbArraySize(tbl_idx, col_idx); 
                  i++) {
            line+= get_default_str((UpllDbCfgId)cfg_idx, def_type);
          }
          line += "'";
        } else {
          line+= get_default_str((UpllDbCfgId)cfg_idx, def_type);
        }

        line += ",";
        upll_upgrade_file << "\n  " << line.c_str();

      }  // for all columns
      line.clear();

      // Primary Keys
      line += "PRIMARY KEY(";
      first = true;
      for (col_idx = 0; 
             col_idx < uudschema::TableNumPkCols(tbl_idx); 
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
      upll_upgrade_file << "\n  " << line.c_str();

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
          line += uudschema::ColumnName(
                              uudschema::TableParentIndex(tbl_idx), col_idx);
        }
        line += "));";
        upll_upgrade_file << "\n  " << line.c_str();
      }
      upll_upgrade_file << endl;
    }  // for all tables
  }  // for all config types
}

//Add new column(s) in the existing table(s)
void add_new_column(DalTableExtension extension_table[], uint16_t table_size) {
  uint16_t cfg_idx;
  uint16_t tbl_idx;
  uint16_t col_idx;
  char size_str[4];
  string line;
  line.clear();

  //upll_upgrade_file << "\n\n/* ADDING NEW COLUMN(S) */\n";
  //upll_upgrade_file << altertable_storedprocedure.c_str() << endl;

  for (cfg_idx = 0; cfg_idx < kUpllDbNumCfgId; cfg_idx++) {
    for (uint16_t idx = 0; idx < table_size/16; idx++) {
      for (uint16_t column_count = 0;
           column_count< extension_table[idx].num_columns;
           column_count++) {
        tbl_idx = extension_table[idx].table_id;

        // Alter Table
        line.clear();
        line += "\nSELECT f_add_col_if_not_exists(";
        line += "'";
        line += get_cfg_str(static_cast<UpllDbCfgId>(cfg_idx));
        line += uudschema::TableName(tbl_idx);
        line += "'";
        line += ",";
        col_idx = extension_table[idx].column_id[column_count];
        // Column Name
        line += "'";
        line += uudschema::ColumnName(tbl_idx, col_idx);
        line += "'";
        line += ",";
        line += "'";
        // data type
        line += get_data_type_str(
                 uudschema::ColumnDbDataTypeId(tbl_idx, col_idx));

        if (uudschema::ColumnDbArraySize(tbl_idx, col_idx) > 1 &&
            uudschema::ColumnDbDataTypeId(tbl_idx, col_idx) != SQL_BINARY) {
          line += "(";
          memset(size_str, '0', 4);
          sprintf(size_str, "%zd",
                     uudschema::ColumnDbArraySize(tbl_idx, col_idx));
          line += size_str;
          line += ")";
        }
        line += "'";
        upll_upgrade_file << line.c_str();
        upll_upgrade_file << ");";
      } 
      line.clear();
    } 
  }  
  upll_upgrade_file << endl;
  //upll_upgrade_file << drop_addFunction.c_str() << endl;
}

//Update default values
void update_default_values(u12u13UpdateDefaultTable update_default_table[], 
                           uint16_t table_size) {
  uint16_t cfg_idx;
  uint16_t tbl_idx;
  string updatetablename;
  string updatequery;

  upll_upgrade_file << "\n\n/* UPDATING VALUE(S) */\n";
  for (cfg_idx = 0; cfg_idx < kUpllDbNumCfgId-2; cfg_idx++) {
    for (uint16_t idx = 0; idx < table_size/16; idx++) {
      tbl_idx = update_default_table[idx].u12u13_update_table_id;
      updatetablename.clear();
      updatetablename += get_cfg_str(static_cast<UpllDbCfgId>(cfg_idx));
      updatetablename += uudschema::TableName(tbl_idx);
      updatequery = update_default_table[idx].u12u13_update_query;
      size_t start_pos = updatequery.find('$', 0);
      updatequery.erase(start_pos, 1);
      updatequery.insert(start_pos,updatetablename);
      upll_upgrade_file << endl << updatequery.c_str() << endl;
      updatequery.clear();
    }
  }
}

// Add column(s) in single congig
void addColumnsInSingleConfig(DalAllTableExtension extension_table_[],
                              UpllDbCfgId cfg_idx) {
  uint16_t col_idx, numCols;
  char size_str[4];
  //UpllDbDefaultType def_type;
  string line,cfg_id;
  line.clear();
  cfg_id.clear();

//  upll_upgrade_file << "\n\n/* ADDING NEW COLUMN(S) IN ENTIRE TABLES IN PARTICULAR CONFIGURATION */\n";
  cfg_id = get_cfg_str(static_cast<UpllDbCfgId>(cfg_idx));

    for (uint16_t tbl_idx = 0; tbl_idx < uudstbl::kDalNumTables; tbl_idx++) {
      numCols = uudschema::TableNumCols(tbl_idx);
      for (uint16_t column_count = 0;
           column_count < extension_table_[tbl_idx].num_columns;
           column_count++) {

        if (tbl_idx == uudstbl::kDbiCtrlrTbl || tbl_idx == uudstbl::kDbiCfgTblDirtyTbl) {
          break;
        }

        // Alter Table
        line.clear();
        line += "\nSELECT f_add_col_if_not_exists(";
        line += "'";
        line += cfg_id;
        line += uudschema::TableName(tbl_idx);
        line += "'";
        line += ",";
        col_idx = extension_table_[tbl_idx].column_id[column_count]-1;
        if (col_idx >= numCols) {
          if (cfg_idx != kCfgIdCandidate || tbl_idx == uudstbl::kDbiCtrlrTbl)
            break;
        }

        // Column Name
        line += "'";
        if (col_idx >= numCols && cfg_idx == kCfgIdCandidate) {
          line += uudschema::CandColumnName(tbl_idx, col_idx);
        } else {
          line += uudschema::ColumnName(tbl_idx, col_idx);
        }
        line += "'";
        line += ",";
        line += "'";

        // data type
        if (col_idx >= numCols && cfg_idx == kCfgIdCandidate) {
          line += get_data_type_str(uudschema::CandColumnDbDataTypeId(tbl_idx, col_idx));
        } else {
          line += get_data_type_str(uudschema::ColumnDbDataTypeId(tbl_idx, col_idx));
        }

        // dimension
        if (col_idx >= numCols && cfg_idx == kCfgIdCandidate) {
          if (uudschema::CandColumnDbArraySize(tbl_idx, col_idx) > 1 &&
              uudschema::CandColumnDbDataTypeId(tbl_idx, col_idx) != SQL_BINARY) {
            line += "(";
            memset(size_str, '0', 4);
            sprintf(size_str, "%zd", uudschema::CandColumnDbArraySize(tbl_idx, col_idx));
            line += size_str;
            line += ")";
          }
        } else {
          if (uudschema::ColumnDbArraySize(tbl_idx, col_idx) > 1 &&
              uudschema::ColumnDbDataTypeId(tbl_idx, col_idx) != SQL_BINARY) {
            line += "(";
            memset(size_str, '0', 4);
            sprintf(size_str, "%zd",
                        uudschema::ColumnDbArraySize(tbl_idx, col_idx));
            line += size_str;
            line += ")";
          }
        }
        line += "'";
        upll_upgrade_file << line.c_str();
        upll_upgrade_file << ");";
      }
      line.clear();
    }
  cfg_id.clear();
  upll_upgrade_file << endl << endl;
  //upll_upgrade_file << drop_addFunction.c_str() << endl;
}
        

//Rename existing column(s)
void rename_to_newColumn(renameColumnInfo rename_col[],uint16_t table_size) {
  uint16_t idx, tbl_idx;
  uint16_t cfg_idx, col_idx;
  string line;

  upll_upgrade_file << "\n\n/* RENAMING EXISTING COLUMN NAME(S) "
                       "TO NEW COLUMN NAME(S) */\n\n";
  upll_upgrade_file << renametable_storedprocedure.c_str();
//  upll_upgrade_file << "\n\n/* RENAMING EXISTING COLUMN NAME(S) "
//                       "TO NEW COLUMN NAME(S) */\n";

  for (cfg_idx = 0; cfg_idx < kUpllDbNumCfgId; cfg_idx++) {
    for (idx = 0; idx < (table_size/24); idx++) {
      line.clear();
      line += "\nSELECT f_rename_col_if_exists(";
      line += "'";
      line += get_cfg_str(static_cast<UpllDbCfgId>(cfg_idx));
      tbl_idx = rename_col[idx].table_id;
      line += uudschema::TableName(tbl_idx);
      line += "','";
      line += rename_col[idx].old_column_name;
      line += "','";
      col_idx = rename_col[idx].new_column_id;
      line += uudschema::ColumnName(tbl_idx, col_idx);
      line += "');";
      upll_upgrade_file << line.c_str();
      line.clear();
    }
  }
  upll_upgrade_file << endl << endl;
  upll_upgrade_file << drop_renameFunction.c_str() << endl << endl;
}

//Method to Insert Values in Dirty table
void insert_values() {
  upll_upgrade_file << "\n/* INSERTING DEFAULT ROWS IN DIRTY TABLE */\n\n";
  upll_upgrade_file << insert_default_rows_in_dirty_tbl().c_str() << endl;
  upll_upgrade_file << select_and_drop_dirty_insert_values.c_str() << endl;
}

//Main to call relevant upgrade functionalities based on the version input
int main ( int argc, char *argv[] )
{
  string filename;

  if ( argc != 2 ) {
     printf( "\n\nusage: %s version(U13/U14/...)\n\n", argv[0]);
     exit(0);
  }
  filename = upgrade_filepath;
  filename += argv[1];
  filename += "0000_upll_up.sql";

  if((strcmp(argv[1],"U13")==0)||(strcmp(argv[1],"u13")==0))
  {
    upll_upgrade_file.open(filename.c_str());

    copyrights();
    header(argv[1]);
    add_new_table(u12u13_new_table, sizeof(u12u13_new_table));
    add_new_column(u12u13_extension_table, sizeof(u12u13_extension_table));
    update_default_values(u12u13_update_default_table, 
                            sizeof(u12u13_update_default_table));
  }
  else if((strcmp(argv[1],"U14")==0)||(strcmp(argv[1],"u14")==0))
  {
    upll_upgrade_file.open(filename.c_str());
 
    copyrights();
    header(argv[1]);
    add_new_table(u13u14_new_table, sizeof(u13u14_new_table));

    //Adding columns
    upll_upgrade_file << "\n\n/* ADDING NEW COLUMN(S) */\n\n";
    upll_upgrade_file << altertable_storedprocedure.c_str();
      add_new_column(u13u14_extension_table, sizeof(u13u14_extension_table));
      addColumnsInSingleConfig(u13u14_extension_all_table, kCfgIdCandidate);
    upll_upgrade_file << drop_addFunction.c_str() << endl;

    rename_to_newColumn(U13U14RenameColumn,sizeof(U13U14RenameColumn));
    insert_values();
  }
  else
  {
    printf("\nUpgrade script not available for this version\n");
  }

  upll_upgrade_file.close();
  return 0;
}
