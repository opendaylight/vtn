/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 *   table_optimize.hh
 *   Contians optimization schema information for tables
 *
 */

#ifndef _TABLE_OPTIMIZE_HH_
#define _TABLE_OPTIMIZE_HH_

#include <iostream>
#include <fstream>
#include "table_defines.hh"


using namespace std;

ofstream upll_optimize_file;

string indexing_storedprocedure =
  "CREATE OR REPLACE function f_create_index_if_not_exists("
  "_tablename varchar(32), _indexname varchar(32), _indexingcolumns varchar) RETURNS void\n"
  "  LANGUAGE plpgsql AS\n"
  "  $func$\n"
  "  declare i_exists integer;\n"
  "  BEGIN\n"
  "    select into i_exists count(*) from pg_class where relname = _indexname;\n"
  "    if i_exists = 0 THEN\n"
  "    EXECUTE\n"
  "      'CREATE INDEX ' || _indexname || ' ON ' || _tablename || ' USING btree "
  "(' || _indexingcolumns || ');';\n"
  "    ELSE\n"
  "      RAISE NOTICE 'INDEX % already exists for table %', _indexname, _tablename;\n"
  "    END IF;\n"
  "  END\n"
  "  $func$;\n\n\n";

string drop_indexing_function =
  "DROP function f_create_index_if_not_exists(_tablename varchar(32), "
  "_indexname varchar(32), _indexingcolumns varchar);\n";

struct indexInfo {
  uudstbl::kDalTableIndex table_id;
  UpllDbCfgId cfgID;
  int num_columns;
  const char *indexnumber;
  uudal::DalColumnIndex column_id[6];
};

indexInfo U13U14Indexing[] =
  {
    { uudstbl::kDbiVbrVlanMapTbl, kCfgIdCandidate, 6, "_index_1",
      { uudstbl::vbridge_vlanmap::kDbiLogicalPortId,
        uudstbl::vbridge_vlanmap::kDbiLogicalPortIdValid,
        uudstbl::vbridge_vlanmap::kDbiDomainId,
        uudstbl::vbridge_vlanmap::kDbiVlanid,
        uudstbl::vbridge_vlanmap::kDbiCtrlrName,
        uudstbl::vbridge_vlanmap::kDbiValidVlanid } },
    { uudstbl::kDbiVtermIfTbl, kCfgIdCandidate, 5, "_index_1",
      { uudstbl::vterminal_interface::kDbiLogicalPortId,
        uudstbl::vterminal_interface::kDbiCtrlrName,
        uudstbl::vterminal_interface::kDbiDomainId,
        uudstbl::vterminal_interface::kDbiValidPortMap,
        uudstbl::vterminal_interface::kDbiValidLogicalPortId } },
    { uudstbl::kDbiVbrIfTbl, kCfgIdRunning, 5, "_index_1",
      { uudstbl::vbridge_interface::kDbiLogicalPortId,
        uudstbl::vbridge_interface::kDbiCtrlrName,
        uudstbl::vbridge_interface::kDbiDomainId,
        uudstbl::vbridge_interface::kDbiValidPortMap,
        uudstbl::vbridge_interface::kDbiValidLogicalPortId } }
  };


#endif //_TABLE_OPTIMIZE_HH_
