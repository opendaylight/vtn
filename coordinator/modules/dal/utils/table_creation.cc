/*
 * Copyright (c) 2012-2015 NEC Corporation
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


// Writes Copyrights information
void copyrights() {
  upll_create_file << copyrights_header.c_str();
}

// Writes header information
void header() {
  upll_create_file <<
     "\n"
     "/**\n"
     " *  upll_create_table.sql\n"
     " *  Contains SQL commands to create/update all the tables "
       "need for UPLL\n"
     " */\n";
}

void create_tables() {
  upll_create_file << build_create_table_script(ENTIRETABLE).c_str();
}

// Method to create Index
void build_create_index_script() {
  upll_create_file << create_index_script.c_str() << endl << endl;
}

// Method to Insert Values
void insert_values() {
  upll_create_file << "\n/* INSERTING DEFAULT ROWS IN DIRTY TABLE */\n\n";
  upll_create_file << insert_default_rows_in_dirty_tbl().c_str() << endl;
  upll_create_file << insert_default_row_table_name(uudstbl::kDbiCfgTblDirtyTbl, kCfgIdCandidate).c_str() << endl ;
  upll_create_file << "\n/* INSERTING DEFAULT ROWS IN UPLL SYSTEM TABLE */\n\n";
  upll_create_file << insert_default_rows(uudstbl::kDbiUpllSystemTbl, kCfgIdSysTbl).c_str() << endl;
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
  create_tables();
  build_create_index_script();
  insert_values();

  upll_create_file.close();
  return 0;
}
