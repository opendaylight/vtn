/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 *   table_optimize.cc
 *   Generates optimization sql schema
 *
 */

#include "table_optimize.hh"
#include "table_defines.hh"
#include "table_common.cc"

// Writes Copyrights information
void copyrights() {
  upll_optimize_file << copyrights_header_2014.c_str();
}

// Writes header information
void header(char *version) {
  char upll_filename[80];
  sprintf(upll_filename,"\n/**\n *  %s0000_upll_opt.sql\n",version);
  upll_optimize_file << upll_filename;
  upll_optimize_file <<
    " *  Contains SQL commands to Optimize DB for version " << version << "\n";
  upll_optimize_file << " */\n";
}

// Indexing columns
void create_index(indexInfo indexing_tbl[], uint16_t table_size,
                  UpllDbCfgId cfgID) {
  uint16_t tbl_idx;
  uint16_t col_idx;
  bool comma_flag = false;
  string line, tbl_name;

  upll_optimize_file << "\n\n/* COLUMN INDEXING */\n\n";
  upll_optimize_file << indexing_storedprocedure.c_str();

  for (uint16_t idx = 0; idx < table_size/24; idx++) {
    tbl_idx = indexing_tbl[idx].table_id;
    tbl_name.clear();
    tbl_name = get_cfg_str(static_cast<UpllDbCfgId>(cfgID));
    tbl_name += uudschema::TableName(tbl_idx);

    line.clear();
    line += "SELECT f_create_index_if_not_exists(";
    line += "'";
    line += tbl_name.c_str();
    line += "','";
    line += tbl_name.c_str();
    line += indexing_tbl[idx].indexnumber;
    line += "','";
    for (uint16_t column_count = 0;
         column_count < indexing_tbl[idx].num_columns;
         column_count++) {
      if (comma_flag) {
        line += ",";
      }
      col_idx = indexing_tbl[idx].column_id[column_count];
      line += uudschema::ColumnName(tbl_idx, col_idx);
      comma_flag = true;
    }
    comma_flag = false;
    line += "');\n";
    upll_optimize_file << line.c_str();
  }
  upll_optimize_file << endl << endl;
  upll_optimize_file << drop_indexing_function.c_str() << endl;
}

int main(int argc, char *argv[])
{
  string filename;

  if ( argc != 2 ) {
     printf( "\n\nusage: %s version(U13/U14/...)\n\n", argv[0]);
     exit(0);
  }
  filename = optimize_filepath;
  filename += argv[1];
  filename += "0000_upll_opt.sql";

  if((strcmp(argv[1],"U14")==0) || (strcmp(argv[1],"u14")==0)) {
    upll_optimize_file.open(filename.c_str());

    copyrights();
    header(argv[1]);
    create_index(U13U14Indexing,sizeof(U13U14Indexing),kCfgIdCandidate);
  } else {
    printf("\nOptimization script not available for this version\n");
  }

  upll_optimize_file.close();
  return 0;
}
