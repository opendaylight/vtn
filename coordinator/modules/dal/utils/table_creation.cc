/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 *   table_creation.cc
 *   Generates sql commands for creating all tables of UPLL
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "sqltypes.h"
#include "dal/dal_schema.hh"
#include "table_defines.hh"
#include "table_creation.hh"
#include "table_common.cc"


//Writes Copyrights information
void copyrights() {
  upll_create_file << copyrights_header.c_str();
}

//write header information
void header() {
  upll_create_file <<
     "\n"
     "/**\n"
     " *  upll_create_table.sql\n"
     " *  Contains SQL commands to create/update all the tables "
       "need for UPLL\n"
     " */\n";
}

//Method to create tables
void build_create_table_script() {
  uint16_t cfg_idx;
  uint16_t tbl_idx;
  uint16_t col_idx;
  char size_str[4];
  UpllDbDefaultType def_type;
  string line;
  bool first;

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
      upll_create_file << endl << line.c_str();

      for (col_idx = 0; col_idx < uudschema::TableNumCols(tbl_idx) + 2; col_idx++) {
        line.clear();
        // if col_idx > NumCols and table type is Candidate, continue
        // the loop to create c_flag and u_flag

        if (col_idx >= uudschema::TableNumCols(tbl_idx)) {
          if (cfg_idx != kCfgIdCandidate || tbl_idx == uudstbl::kDbiCtrlrTbl ||
              tbl_idx == uudstbl::kDbiCfgTblDirtyTbl)
            break;
        }

        // Column Name
        line += "  ";
        if (cfg_idx == kCfgIdCandidate && tbl_idx != uudstbl::kDbiCfgTblDirtyTbl)
          line += uudschema::CandColumnName(tbl_idx, col_idx);
        else
          line += uudschema::ColumnName(tbl_idx, col_idx);
        line += " ";

        // data type
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
          // dimension
          if (uudschema::ColumnDbArraySize(tbl_idx, col_idx) > 1 &&
              uudschema::ColumnDbDataTypeId(tbl_idx, col_idx) != SQL_BINARY) {
            line += "(";
            memset(size_str, '0', 4);
            sprintf(size_str, "%zd", uudschema::ColumnDbArraySize(tbl_idx, col_idx));
            line += size_str;
            line += ")";
          }
        }

        // default
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

        line += ",";
        upll_create_file <<"\n  " << line.c_str();
      }  // for all columns
      line.clear();

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
      upll_create_file <<"\n  " << line.c_str();

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
        upll_create_file <<"\n  " << line.c_str();
      }
      upll_create_file << endl;
    }  // for all tables
  }    // for all config types
}      // build_create_table_script

//Method to create Index
void build_create_index_script() {
  upll_create_file << create_index_script.c_str() << endl << endl;
}

//Method to Insert Values
void insert_values() {
  upll_create_file << "\n/* INSERTING DEFAULT ROWS IN DIRTY TABLE */\n\n";
  upll_create_file << insert_default_rows_in_dirty_tbl().c_str() << endl;
  upll_create_file << select_and_drop_dirty_insert_values.c_str() << endl;
}

int main( int argc, char *argv[] )
{
  string filename;

  if ( argc != 2 ) {
     printf( "\n\nusage: %s version(U13/U14/...)\n\n", argv[0]);
     exit(0);
  }

  filename = create_filepath;
  filename += "upll_create_table.sql";
  upll_create_file.open(filename.c_str());

  copyrights();
  header();
  build_create_table_script();
  build_create_index_script();
  insert_values();

  upll_create_file.close();
  return 0;
}
