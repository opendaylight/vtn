/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 *    table_create.hh
 *    Contians initialization schema information for tables
 *
 */



#ifndef _TABLE_CREATION_HH
#define _TABLE_CREATION_HH

#include <iostream>
#include <fstream>

using namespace std;

ofstream upll_create_file;

string create_index_script =
     "\nCREATE INDEX ca_vbr_if_tbl_semindex ON ca_vbr_if_tbl USING btree "
       "(logical_port_id, ctrlr_name, domain_id, valid_portmap, "
       "valid_logical_port_id);"
     "\nCREATE INDEX ca_policingprofile_entry_tbl_semindex ON "
       "ca_policingprofile_entry_tbl USING btree (flowlist, valid_flowlist);"
     "\nCREATE INDEX ca_vtn_policingmap_tbl_semindex ON "
       "ca_vtn_policingmap_tbl USING btree (policername, valid_policername);"
     "\nCREATE INDEX ca_vbr_policingmap_tbl_semindex ON "
       "ca_vbr_policingmap_tbl USING btree (policername, valid_policername);"
     "\nCREATE INDEX ca_vbr_if_policingmap_tbl_semindex ON "
       "ca_vbr_if_policingmap_tbl USING btree "
       "(policername, valid_policername);"
     "\nCREATE INDEX ru_vbr_if_tbl_showindex ON ru_vbr_if_tbl "
       "(vtn_name, vex_name, valid_vex_name);"
     "\nCREATE INDEX ca_vbr_vlanmap_tbl_index_1 ON ca_vbr_vlanmap_tbl USING "
       "btree (logical_port_id, logical_port_id_valid, domain_id, vlanid, "
       "ctrlr_name, valid_vlanid);"
     "\nCREATE INDEX ca_vterm_if_tbl_index_1 ON ca_vterm_if_tbl USING btree "
       "(logical_port_id, ctrlr_name, domain_id, valid_portmap, "
       "valid_logical_port_id);"
     "\nCREATE INDEX ru_vbr_if_tbl_index_1 ON ru_vbr_if_tbl USING btree "
       "(logical_port_id, ctrlr_name, domain_id, valid_portmap, "
       "valid_logical_port_id);"
     "\nCREATE INDEX im_vbr_if_tbl_index_1 ON im_vbr_if_tbl USING btree "
       "(logical_port_id, ctrlr_name, domain_id, valid_portmap, "
       "valid_logical_port_id);"
     "\nCREATE INDEX im_policingprofile_entry_tbl_index_1 ON "
       "im_policingprofile_entry_tbl USING btree (flowlist, valid_flowlist);"
     "\nCREATE INDEX im_vtn_policingmap_tbl_index_1 ON im_vtn_policingmap_tbl"
       " USING btree (policername, valid_policername);"
     "\nCREATE INDEX im_vbr_policingmap_tbl_index_1 ON im_vbr_policingmap_tbl"
       " USING btree (policername, valid_policername);"
     "\nCREATE INDEX im_vbr_if_policingmap_tbl_index_1 ON "
       "im_vbr_if_policingmap_tbl USING btree (policername, valid_policername);"
     "\nCREATE INDEX im_vbr_vlanmap_tbl_index_1 ON im_vbr_vlanmap_tbl USING "
       "btree (logical_port_id, logical_port_id_valid, domain_id, vlanid, "
       "ctrlr_name, valid_vlanid);"
     "\nCREATE INDEX im_vterm_if_tbl_index_1 ON im_vterm_if_tbl USING btree "
       "(logical_port_id, ctrlr_name, domain_id, valid_portmap, "
       "valid_logical_port_id);";



#endif  //_TABLE_CREATION_HH
