/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 *    table_upgrade.hh
 *    Contians upgrade schema information for tables
 *
 */

#ifndef TABLE_UPGRADE_HH_
#define TABLE_UPGRADE_HH_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <iostream>
#include <fstream>
#include "sqltypes.h"
#include "sql.h"
#include "dal/dal_schema.hh"

using namespace std;

namespace uudal = unc::upll::dal;

ofstream upll_upgrade_file;

string drop_addFunction =
  "DROP FUNCTION f_add_col_if_not_exists(_tbl varchar(32), "
  "_col varchar(32), _type varchar(32));";

string altertable_storedprocedure =
  "CREATE OR REPLACE function f_add_col_if_not_exists(_tbl varchar(32), "
  "_col varchar(32), _type varchar(32)) "
  "RETURNS boolean\n"
  "  LANGUAGE plpgsql AS\n"
  "  $func$\n"
  "  BEGIN\n"
  "    IF EXISTS (\n"
  "    SELECT 1 FROM pg_attribute\n"
  "    WHERE  attrelid = _tbl::regclass\n"
  "    AND    attname = _col\n"
  "    AND    NOT attisdropped) THEN\n"
  "    RAISE NOTICE 'Column % already exists in table %', _col, _tbl;\n"
  "    RETURN FALSE;\n"
  "    ELSE\n"
  "    EXECUTE\n"
  "    'ALTER TABLE ' || _tbl || ' ADD COLUMN ' || quote_ident(_col) "
  "|| ' ' || _type;\n"
  "    RETURN TRUE;\n"
  "    END IF;\n"
  "  END\n"
  "  $func$;\n";

string drop_renameFunction =
  "DROP FUNCTION f_rename_col_if_exists(_tbl varchar(32), "
  "_col varchar(32), _newCol varchar(32));";

string renametable_storedprocedure =
  "CREATE OR REPLACE function f_rename_col_if_exists(_tbl varchar(32), "
  "_col varchar(32), _newCol varchar(32)) RETURNS boolean\n"
  "  LANGUAGE plpgsql AS\n"
  "  $func$\n"
  "  BEGIN\n"
  "    IF EXISTS (\n"
  "    SELECT 1 FROM pg_attribute\n"
  "    WHERE  attrelid = _tbl::regclass\n"
  "    AND    attname = _col\n"
  "    AND NOT attisdropped) THEN\n"
  "    EXECUTE\n"
  "    'ALTER TABLE ' || _tbl || ' RENAME COLUMN ' || quote_ident(_col) "
  "|| ' ' || ' TO ' || quote_ident(_newCol);\n"
  "    RETURN TRUE;\n"
  "    ELSE\n"
  "    RAISE NOTICE 'Column % does not exists in table %', _col, _tbl;\n"
  "    RETURN FALSE;\n"
  "    END IF;\n"
  "  END\n"
  "  $func$;\n";

// Names of newly tables
uudstbl::kDalTableIndex u12u13_new_table[] =
  {
    uudstbl::kDbiVterminalTbl,
    uudstbl::kDbiVtermIfTbl,
    uudstbl::kDbiVtermIfFlowFilterTbl,
    uudstbl::kDbiVtermIfFlowFilterEntryTbl,
    uudstbl::kDbiVtermIfPolicingMapTbl
  };

uudstbl::kDalTableIndex u13u14_new_table[] =  { uudstbl::kDbiCfgTblDirtyTbl };

uudstbl::kDalTableIndex u14u16_new_table[] =  { uudstbl::kDbiUpllSystemTbl };

uudstbl::kDalTableIndex u16u17_new_table[] =
  {
   uudstbl::kDbiVtnCfgTblDirtyTbl,
   uudstbl::kDbiPpScratchTbl,
   uudstbl::kDbiFlScratchTbl,
   uudstbl::kDbiSpdScratchTbl
  };
uudstbl::kDalTableIndex u16u17_new_table_1[] =
  {
   uudstbl::kDbiConvertVbrTbl,
   uudstbl::kDbiConvertVbrIfTbl,
   uudstbl::kDbiConvertVlinkTbl,
   uudstbl::kDbiVbrPortMapTbl,
   uudstbl::kDbiUnifiedNwTbl,
   uudstbl::kDbiUnwLabelTbl,
   uudstbl::kDbiUnwLabelRangeTbl,
   uudstbl::kDbiUnwSpineDomainTbl,
   uudstbl::kDbiVtnUnifiedTbl,
   uudstbl::kDbiConvertVtunnelTbl,
   uudstbl::kDbiConvertVtunnelIfTbl,
   uudstbl::kDbiVBIdTbl,
   uudstbl::kDbiGVtnIdTbl,	
   uudstbl::kDbiVtnGatewayPortTbl	
 };
// Tables to be updated with the default values
struct u12u13UpdateDefaultTable {
  uudstbl::kDalTableIndex u12u13_update_table_id;
  std::string u12u13_update_query;
};


std::string u12u13_update_default_query =
  "UPDATE $ SET \n"
  " redirect_direction=0,\n"
  " valid_redirect_direction=1,\n"
  " cs_redirect_direction=cs_redirect_node \n"
  " WHERE valid_redirect_node=1;";

// Tables to be modified
u12u13UpdateDefaultTable u12u13_update_default_table[] =
  {
    { uudstbl::kDbiVbrFlowFilterEntryTbl,u12u13_update_default_query },
    { uudstbl::kDbiVbrIfFlowFilterEntryTbl,u12u13_update_default_query },
    { uudstbl::kDbiVrtIfFlowFilterEntryTbl,u12u13_update_default_query },
    { uudstbl::kDbiVtermIfFlowFilterEntryTbl,u12u13_update_default_query }
  };


struct DalTableExtension {
  uudstbl::kDalTableIndex table_id;
  int num_columns;
  uudal::DalColumnIndex column_id[3];
};


// Attributes to be modified
DalTableExtension  u12u13_extension_table[] =
  {
    { uudstbl::kDbiVbrFlowFilterEntryTbl,
      3,
      { uudstbl::vbr_flowfilter_entry::kDbiRedirectDirection,
        uudstbl::vbr_flowfilter_entry::kDbiValidRedirectDirection,
        uudstbl::vbr_flowfilter_entry::kDbiCsRedirectDirection
      }
    },
    { uudstbl::kDbiVbrIfFlowFilterEntryTbl,
      3,
      { uudstbl::vbr_if_flowfilter_entry::kDbiRedirectDirection,
        uudstbl::vbr_if_flowfilter_entry::kDbiValidRedirectDirection,
        uudstbl::vbr_if_flowfilter_entry::kDbiCsRedirectDirection
      }
    },
    { uudstbl::kDbiVrtIfFlowFilterEntryTbl,
      3,
      { uudstbl::vrt_if_flowfilter_entry::kDbiRedirectDirection,
        uudstbl::vrt_if_flowfilter_entry::kDbiValidRedirectDirection,
        uudstbl::vrt_if_flowfilter_entry::kDbiCsRedirectDirection
      }
    }
  };


DalTableExtension  u13u14_extension_table[] =
  {
    { uudstbl::kDbiVtnTbl,1,{ uudstbl::vtn::kDbiUnknownCount,0,0 } },
    { uudstbl::kDbiVtnCtrlrTbl,1,{ uudstbl::vtn_controller::kDbiUnknownCount,0,0 } },
  };


struct renameColumnInfo {
  uudstbl::kDalTableIndex table_id;
  const char *old_column_name;
  uudal::DalColumnIndex new_column_id;
};

renameColumnInfo  U13U14RenameColumn[] =
  {
    { uudstbl::kDbiVbrTbl, "fault_count", uudstbl::vbridge::kDbiUnknownCount },
    { uudstbl::kDbiVterminalTbl, "fault_count", uudstbl::vterminal::kDbiUnknownCount },

    { uudstbl::kDbiVtnCtrlrTbl,"controller_name", uudstbl::vtn_controller::kDbiControllerName },
    { uudstbl::kDbiVtnRenameTbl,"controller_name",uudstbl::vtn_rename::kDbiControllerName },
    { uudstbl::kDbiVbrTbl,"controller_name",uudstbl::vbridge::kDbiCtrlrName },
    { uudstbl::kDbiVbrTbl,"valid_controller_name",uudstbl::vbridge::kDbiValidCtrlrName },
    { uudstbl::kDbiVbrTbl,"cs_controller_name",uudstbl::vbridge::kDbiCsCtrlrName },
    { uudstbl::kDbiVbrVlanMapTbl,"controller_name",uudstbl::vbridge_vlanmap::kDbiCtrlrName },
    { uudstbl::kDbiVbrIfTbl,"controller_name",uudstbl::vbridge_interface::kDbiCtrlrName },
    { uudstbl::kDbiVrtTbl,"controller_name",uudstbl::vrouter::kDbiCtrlrName },
    { uudstbl::kDbiVrtTbl,"valid_controller_name",uudstbl::vrouter::kDbiValidCtrlrName },
    { uudstbl::kDbiVrtTbl,"cs_controller_name",uudstbl::vrouter::kDbiCsCtrlrName },
    { uudstbl::kDbiVrtIfTbl,"controller_name",uudstbl::vrouter_interface::kDbiCtrlrName },
    { uudstbl::kDbiVterminalTbl,"controller_name",uudstbl::vterminal::kDbiCtrlrName },
    { uudstbl::kDbiVterminalTbl,"valid_controller_name",uudstbl::vterminal::kDbiValidCtrlrName },
    { uudstbl::kDbiVterminalTbl,"cs_controller_name",uudstbl::vterminal::kDbiCsCtrlrName },
    { uudstbl::kDbiVtermIfTbl,"controller_name",uudstbl::vterminal_interface::kDbiCtrlrName },
    { uudstbl::kDbiVNodeRenameTbl,"controller_name",uudstbl::vnode_rename::kDbiCtrlrName },
    { uudstbl::kDbiVlinkRenameTbl,"controller_name",uudstbl::vlink_rename::kDbiCtrlrName },
    { uudstbl::kDbiStaticIpRouteTbl,"controller_name",uudstbl::static_ip_route::kDbiCtrlrName },
    { uudstbl::kDbiDhcpRelayServerTbl,"controller_name",uudstbl::dhcprelay_server::kDbiCtrlrName },
    { uudstbl::kDbiDhcpRelayIfTbl,"controller_name",uudstbl::dhcprelay_interface::kDbiCtrlrName },
    { uudstbl::kDbiVbrNwMonTbl,"controller_name",uudstbl::vbridge_networkmonitor_group::kDbiCtrlrName },
    { uudstbl::kDbiVbrNwMonHostTbl,"controller_name",uudstbl::vbridge_networkmonitor_host::kDbiCtrlrName },
    { uudstbl::kDbiVtepTbl,"controller_name",uudstbl::vtep::kDbiCtrlrName },
    { uudstbl::kDbiVtepTbl,"valid_controller_name",uudstbl::vtep::kDbiValidCtrlrName },
    { uudstbl::kDbiVtepTbl,"cs_controller_name",uudstbl::vtep::kDbiCsCtrlrName },
    { uudstbl::kDbiVtepIfTbl,"controller_name",uudstbl::vtep_interface::kDbiCtrlrName },
    { uudstbl::kDbiVtepGrpTbl,"controller_name",uudstbl::vtep_group::kDbiCtrlrName },
    { uudstbl::kDbiVtepGrpTbl,"valid_controller_name",uudstbl::vtep_group::kDbiValidCtrlrName },
    { uudstbl::kDbiVtepGrpTbl,"cs_controller_name",uudstbl::vtep_group::kDbiCsCtrlrName },
    { uudstbl::kDbiVtepGrpMemTbl,"controller_name",uudstbl::vtep_groupmember::kDbiCtrlrName },
    { uudstbl::kDbiVtunnelTbl,"controller_name",uudstbl::vtunnel::kDbiCtrlrName },
    { uudstbl::kDbiVtunnelTbl,"valid_controller_name",uudstbl::vtunnel::kDbiValidCtrlrName },
    { uudstbl::kDbiVtunnelTbl,"cs_controller_name",uudstbl::vtunnel::kDbiCsCtrlrName },
    { uudstbl::kDbiVtunnelIfTbl,"controller_name",uudstbl::vtunnel_interface::kDbiCtrlrName }
  };

struct DalAllTableExtension {
  int num_columns;
  uudal::DalColumnIndex column_id[3];
};

DalAllTableExtension u13u14_extension_all_table[] =
  {
    { 2,{ uudstbl::vtn::kDbiCreateFlag, uudstbl::vtn::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vtn_controller::kDbiCreateFlag,uudstbl::vtn_controller::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vtn_rename::kDbiCreateFlag,uudstbl::vtn_rename::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vbridge::kDbiCreateFlag,uudstbl::vbridge::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vbridge_vlanmap::kDbiCreateFlag,uudstbl::vbridge_vlanmap::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vbridge_interface::kDbiCreateFlag,uudstbl::vbridge_interface::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vrouter::kDbiCreateFlag,uudstbl::vrouter::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vrouter_interface::kDbiCreateFlag,uudstbl::vrouter_interface::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vterminal::kDbiCreateFlag,uudstbl::vterminal::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vterminal_interface::kDbiCreateFlag,uudstbl::vterminal_interface::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vnode_rename::kDbiCreateFlag,uudstbl::vnode_rename::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vlink::kDbiCreateFlag,uudstbl::vlink::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vlink_rename::kDbiCreateFlag,uudstbl::vlink_rename::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::static_ip_route::kDbiCreateFlag,uudstbl::static_ip_route::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::dhcprelay_server::kDbiCreateFlag,uudstbl::dhcprelay_server::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::dhcprelay_interface::kDbiCreateFlag,uudstbl::dhcprelay_interface::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vbridge_networkmonitor_group::kDbiCreateFlag,uudstbl::vbridge_networkmonitor_group::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vbridge_networkmonitor_host::kDbiCreateFlag,uudstbl::vbridge_networkmonitor_host::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vunknown::kDbiCreateFlag,uudstbl::vunknown::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vunknown_interface::kDbiCreateFlag,uudstbl::vunknown_interface::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vtep::kDbiCreateFlag,uudstbl::vtep::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vtep_interface::kDbiCreateFlag,uudstbl::vtep_interface::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vtep_group::kDbiCreateFlag,uudstbl::vtep_group::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vtep_groupmember::kDbiCreateFlag,uudstbl::vtep_groupmember::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vtunnel::kDbiCreateFlag,uudstbl::vtunnel::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vtunnel_interface::kDbiCreateFlag,uudstbl::vtunnel_interface::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::flowlist::kDbiCreateFlag,uudstbl::flowlist::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::flowlist_ctrlr::kDbiCreateFlag,uudstbl::flowlist_ctrlr::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::flowlist_rename::kDbiCreateFlag,uudstbl::flowlist_rename::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::flowlist_entry::kDbiCreateFlag,uudstbl::flowlist_entry::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::flowlist_entry_ctrlr::kDbiCreateFlag,uudstbl::flowlist_entry_ctrlr::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::policingprofile::kDbiCreateFlag,uudstbl::policingprofile::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::policingprofile_ctrlr::kDbiCreateFlag,uudstbl::policingprofile_ctrlr::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::policingprofile_rename::kDbiCreateFlag,uudstbl::policingprofile_rename::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::policingprofile_entry::kDbiCreateFlag,uudstbl::policingprofile_entry::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::policingprofile_entry_ctrlr::kDbiCreateFlag,uudstbl::policingprofile_entry_ctrlr::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vtn_flowfilter::kDbiCreateFlag,uudstbl::vtn_flowfilter::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vtn_flowfilter_ctrlr::kDbiCreateFlag,uudstbl::vtn_flowfilter_ctrlr::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vtn_flowfilter_entry::kDbiCreateFlag,uudstbl::vtn_flowfilter_entry::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vtn_flowfilter_entry_ctrlr::kDbiCreateFlag,uudstbl::vtn_flowfilter_entry_ctrlr::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vbr_flowfilter::kDbiCreateFlag,uudstbl::vbr_flowfilter::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vbr_flowfilter_entry::kDbiCreateFlag,uudstbl::vbr_flowfilter_entry::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vbr_if_flowfilter::kDbiCreateFlag,uudstbl::vbr_if_flowfilter::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vbr_if_flowfilter_entry::kDbiCreateFlag,uudstbl::vbr_if_flowfilter_entry::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vrt_if_flowfilter::kDbiCreateFlag,uudstbl::vrt_if_flowfilter::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vrt_if_flowfilter_entry::kDbiCreateFlag,uudstbl::vrt_if_flowfilter_entry::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vterm_if_flowfilter::kDbiCreateFlag,uudstbl::vterm_if_flowfilter::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vterm_if_flowfilter_entry::kDbiCreateFlag,uudstbl::vterm_if_flowfilter_entry::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vtn_policingmap::kDbiCreateFlag,uudstbl::vtn_policingmap::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vtn_policingmap_ctrlr::kDbiCreateFlag,uudstbl::vtn_policingmap_ctrlr::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vbr_policingmap::kDbiCreateFlag,uudstbl::vbr_policingmap::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vbr_if_policingmap::kDbiCreateFlag,uudstbl::vbr_if_policingmap::kDbiUpdateFlag,0 } },
    { 2,{ uudstbl::vterm_if_policingmap::kDbiCreateFlag,uudstbl::vterm_if_policingmap::kDbiUpdateFlag,0 } }
  };

#endif  //TABLE_UPGRADE_HH_

