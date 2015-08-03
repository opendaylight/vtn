/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 *   table_downgrade.cc
 *   Drop newly added tables and updates fields to the older version
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "dal/dal_schema.hh"
#include "table_defines.hh"
#include "table_downgrade.hh"
#include "table_upgrade.hh"


// Writes Copyrights information
void copyrights(){
  upll_downgrade_file << copyrights_header_2014.c_str();
}

// Writes header information
void header(char *version) {
  char upll_filename[80];
  sprintf(upll_filename,"\n/**\n *  %s0000_upll_down.sql\n",version);
  upll_downgrade_file << upll_filename;
  upll_downgrade_file <<
    " *  Contains SQL commands to update UPLL DB schema for "
    << version << " downgrade\n";
  upll_downgrade_file << " */\n";
}

// Method to validate table(s)
void table_validation(char *version) {
  upll_downgrade_file << "\n\n/* VALIDATION FOR DOWNGRADE FROM "
    << version << " */\n";
  upll_downgrade_file << endl << u12u13_error_check.c_str() << endl;
}

// Method to remove table(s)
void remove_table(uudstbl::kDalTableIndex new_drop_table[], uint16_t table_size,
                  uint16_t cfg_idx = kUpllDbNumCfgId) {
  bool break_flag = false;
  uint16_t tbl_idx;
  string line;

  if (cfg_idx != kUpllDbNumCfgId) {
    break_flag = true;
    goto jump;
  }

  for (cfg_idx = 0; cfg_idx < kUpllDbNumCfgId; cfg_idx++) {
    jump:
    for (int idx = (table_size/sizeof(uudstbl::kDalTableIndex)-1);
         idx >= 0;
         idx--) {
      tbl_idx = new_drop_table[idx];
      line.clear();
      line += " DROP TABLE ";
      line += "IF EXISTS ";
      line += get_cfg_str(static_cast<UpllDbCfgId>(cfg_idx));
      line += uudschema::TableName(tbl_idx);
      if (cfg_idx == kCfgIdCandidate) {
        line += " CASCADE";
      }
      line += ";";
      upll_downgrade_file << endl << line.c_str();
    }
    if (break_flag == true)
      break;
  }
  upll_downgrade_file << endl;
}

void removeEntireTableOnParticularConfig(UpllDbCfgId cfg_idx) {
  string line;

  for (int tbl_idx = (uudstbl::kDalNumTables-1);
       tbl_idx >= 0;
       tbl_idx--) {
    if(tbl_idx == uudstbl::kDbiVtnCfgTblDirtyTbl ||
       tbl_idx == uudstbl::kDbiCtrlrTbl ||
       tbl_idx == uudstbl::kDbiUpllSystemTbl ||
       tbl_idx == uudstbl::kDbiCfgTblDirtyTbl)
      continue;
    line.clear();
    line += " DROP TABLE ";
    line += "IF EXISTS ";
    line += get_cfg_str(static_cast<UpllDbCfgId>(cfg_idx));
    line += uudschema::TableName(tbl_idx);
    if (cfg_idx == kCfgIdCandidate) {
      line += " cascade";
    }
    line += ";";
    upll_downgrade_file << endl << line.c_str();
  }
  upll_downgrade_file << endl << endl;
}

// Method to update already existing tables.
void remove_column(DalTableExtension extension_table[],
                   uint16_t table_size) {
  uint16_t cfg_idx;
  uint16_t tbl_idx;
  uint16_t col_idx;
  string line;

  upll_downgrade_file <<
    "\n\n/* REMOVING EXISTING COLUMN(S) */\n\n";

  for (cfg_idx = 0; cfg_idx < kUpllDbNumCfgId; cfg_idx++) {
    for (int idx = (table_size/16); idx >= 0; idx--) {
      for (uint16_t column_count = 0;
           column_count < extension_table[idx].num_columns;
           column_count++) {
        tbl_idx = extension_table[idx].table_id;

        // Alter Table
        line.clear();
        if(column_count == 0) {
          line += "ALTER TABLE ";
          line += get_cfg_str(static_cast<UpllDbCfgId>(cfg_idx));
          line += uudschema::TableName(tbl_idx);
          upll_downgrade_file << line.c_str() << endl;
          line.clear();
        }
        line += " DROP COLUMN ";
        line += "IF EXISTS ";
        upll_downgrade_file << line.c_str();
        col_idx = extension_table[idx].column_id[column_count];
        line.clear();

        // Column Name
        line += uudschema::ColumnName(tbl_idx, col_idx);
        if (column_count == (extension_table[idx].num_columns-1)) {
          line += ";\n";
        } else {
          line += ",";
        }
        upll_downgrade_file << line.c_str() << endl;
      }
      line.clear();
    }
  }
}

// Removing columns in particular configs
void removeColumnsInParticularTable(DalAllTableExtension extension_table[],
                                    UpllDbCfgId cfg_idx) {
  uint16_t col_idx, numCols;
  string line,cfg_id;
  line.clear();cfg_id.clear();

  cfg_id = get_cfg_str(static_cast<UpllDbCfgId>(cfg_idx));

  upll_downgrade_file <<
    "\n\n/* REMOVING EXISTING COLUMN(S) IN PARTICULAR CONFIGURATION TABLES */\n\n";

  for (uint16_t tbl_idx = 0; tbl_idx < uudstbl::kDalNumTables-2; tbl_idx++) {
    numCols = uudschema::TableNumCols(tbl_idx);
    for (uint16_t column_count = 0;
         column_count< extension_table[tbl_idx].num_columns;
         column_count++) {

      // Alter Table
      line.clear();
      if(column_count == 0) {
        line += "ALTER TABLE ";
        line += cfg_id;
        line += uudschema::TableName(tbl_idx);
        upll_downgrade_file << line.c_str() << endl;
        line.clear();
      }
      line += " DROP COLUMN ";
      line += "IF EXISTS ";
      upll_downgrade_file << line.c_str();
      line.clear();

      if (cfg_idx == kCfgIdCandidate) {
        col_idx = extension_table[tbl_idx].column_id[column_count]-1;
        if (col_idx >= numCols)
          line += uudschema::CandColumnName(tbl_idx, col_idx);

      } else {
        col_idx = extension_table[tbl_idx].column_id[column_count];
        line += uudschema::ColumnName(tbl_idx, col_idx);
      }

      if (column_count == (extension_table[tbl_idx].num_columns)-1) {
        line += ";\n";
      } else {
        line += ",";
      }
      upll_downgrade_file << line.c_str() << endl;
    }
    line.clear();
  }
}

// Rename existing column(s)
void rename_to_oldColumn(renameColumnInfo rename_col[], uint16_t table_size) {
  uint16_t tbl_idx;
  uint16_t cfg_idx, col_idx;
  int idx;
  string line;

  upll_downgrade_file << "\n/* RENAMING EXISTING COLUMN NAMES(S) "
                       "TO OLD COLUMN NAME(S) */\n\n";
  upll_downgrade_file << renametable_storedprocedure.c_str() << endl;

  for (cfg_idx = 0; cfg_idx < kUpllDbNumCfgId; cfg_idx++) {
    for (idx = (table_size/24)-1; idx >= 0; idx--) {
      line.clear();
      line += "\nSELECT f_rename_col_if_exists(";
      line += "'";
      line += get_cfg_str(static_cast<UpllDbCfgId>(cfg_idx));
      tbl_idx = rename_col[idx].table_id;
      line += uudschema::TableName(tbl_idx);
      line += "','";
      col_idx = rename_col[idx].new_column_id;
      line += uudschema::ColumnName(tbl_idx, col_idx);
      line += "','";
      line += rename_col[idx].old_column_name;
      line += "');";
      upll_downgrade_file << line.c_str();
      line.clear();
    }
  }
  upll_downgrade_file << endl << endl;
  upll_downgrade_file << drop_renameFunction.c_str() << endl;
}

void removeTableComments() {
  upll_downgrade_file << "\n/* REMOVING EXISTING TABLE(S) */\n";
}

// Main to call relevant downgrade functionalities based on the version input
int main(int argc, char *argv[])
{
  string filename;

  if ( argc != 2 ) {
     printf( "\n\nusage: %s version(U13/U14/...)\n\n", argv[0]);
     exit(0);
  }
  filename = downgrade_filepath;
  filename += argv[1];
  filename += "0000_upll_down.sql";

  if((strcmp(argv[1],"U13")==0)||(strcmp(argv[1],"u13")==0))
  {
    upll_downgrade_file.open(filename.c_str());

    copyrights();
    header(argv[1]);
    table_validation(argv[1]);
    removeTableComments();
    remove_table(u12u13_new_table,sizeof(u12u13_new_table));
    remove_column(u12u13_extension_table,sizeof(u12u13_extension_table));
  }
  else if((strcmp(argv[1],"U14")==0)||(strcmp(argv[1],"u14")==0))
  {
    upll_downgrade_file.open(filename.c_str());

    copyrights();
    header(argv[1]);
    removeTableComments();
    remove_table(u13u14_new_table,sizeof(u13u14_new_table));
    remove_column(u13u14_extension_table,sizeof(u13u14_extension_table));
    removeColumnsInParticularTable(u13u14_extension_all_table, kCfgIdCandidate);
    rename_to_oldColumn(U13U14RenameColumn,sizeof(U13U14RenameColumn));
  }
  else if((strcmp(argv[1],"U16")==0)||(strcmp(argv[1],"u16")==0))
  {
    upll_downgrade_file.open(filename.c_str());

    copyrights();
    header(argv[1]);
    removeTableComments();
    remove_table(u14u16_new_table,sizeof(u14u16_new_table), kCfgIdSysTbl);
    removeEntireTableOnParticularConfig(kCfgIdTempDel);
  }

  else if((strcmp(argv[1],"U17")==0)||(strcmp(argv[1],"u17")==0))
  {
    upll_downgrade_file.open(filename.c_str());

    copyrights();
    header(argv[1]);
    removeTableComments();
    remove_table(u16u17_new_table,sizeof(u16u17_new_table), kCfgIdCandidate);
    remove_table(u16u17_new_table_1,sizeof(u16u17_new_table_1));
  }

  else
  {
    printf("\nDowngrade script not available for this version\n");
  }

  upll_downgrade_file.close();
  return 0;
}
