/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.openstack.dbmanager;

/**
 * SQL Factory class for all OpenStack related operations
 */
public class VtnOpenStackSQLFactory {

	/* CRUD SQLs for os_free_counter_tbl */
	public static final String SEL_FC_SQL = "select os_res_counter from os_free_counter_tbl where os_res_id = ? and os_vtn_name = ? order by os_res_counter";
	public static final String DEL_FC_SQL = "delete from os_free_counter_tbl where os_res_id = ? and os_vtn_name = ? and os_res_counter = ?";
	public static final String INS_FC_SQL = "insert into os_free_counter_tbl(os_res_counter,os_vtn_name,os_res_id) values(?,?,?)";
	public static final String DEL_VTN_CHILD_SQL = "delete from os_free_counter_tbl where os_vtn_name = ? and os_res_id != ?";
	public static final String DEL_NODE_CHILD_SQL = "delete from os_free_counter_tbl where os_vtn_name = ?";
	public static final String FID_FC_SQL = "select count(*) from os_free_counter_tbl where os_res_id = ? and os_vtn_name = ? and os_res_counter = ?";
	public static final String CHK_FC_MAX_ID_SQL = "select os_res_counter from os_free_counter_tbl where os_res_id = ? and os_vtn_name = ? and os_res_counter > ?";
	public static final String SEL_MAX_FC_SQL = "select os_res_counter from os_free_counter_tbl where os_res_id = ? and os_vtn_name = ? order by os_res_counter DESC limit 1";

	/* CRUD SQLs for os_vtn_tbl */
	public static final String CHK_VTN_SQL = "select count(*) from os_vtn_tbl where os_vtn_name = ?";
	public static final String INS_VTN_SQL = "insert into os_vtn_tbl(os_vtn_id, os_vtn_name, os_vtn_status)  values(?, ?, ?)";
	public static final String DEL_VTN_SQL = "delete from os_vtn_tbl where os_vtn_name = ?";
	public static final String SEL_VTN_LIST_ID_CNT_SQL = "select count(os_vtn_id) from os_vtn_tbl where os_vtn_id != 0";
	public static final String SEL_VTN_LIST_ID_LIST_SQL = "select os_vtn_id from os_vtn_tbl where os_vtn_id != 0 order by os_vtn_id";
	public static final String IS_AUTO_VTN_LIST_NAME_SQL = "select os_vtn_status from os_vtn_tbl where os_vtn_name = ?";
	public static final String CHK_MAX_AUTO_VTN_ID_SQL = "select os_vtn_id from os_vtn_tbl where os_vtn_status != 0 and os_vtn_id > ?";
	public static final String CHK_VTN_EXIST_SQL = "select count(*) from os_vtn_tbl";
	
	/* CRUD SQLs for os_vbr_tbl */
	public static final String CHK_VBR_SQL = "select count(*) from os_vbr_tbl where os_vtn_name = ? and os_vbr_name = ?";
	public static final String SEL_VBR_LIST_ID_CNT_SQL = "select count(os_vbr_id) from os_vbr_tbl where os_vtn_name = ? and os_vbr_id != 0";
	public static final String INS_VBR_SQL = "insert into os_vbr_tbl(os_vbr_id, os_vtn_name, os_vbr_name, os_vbr_status) values(?, ?, ?, ?)";
	public static final String DEL_VBR_SQL = "delete from os_vbr_tbl where os_vbr_id = ? and os_vtn_name = ? and os_vbr_name = ?";
	public static final String SEL_VBR_LIST_ID_LIST_SQL = "select os_vbr_id from os_vbr_tbl where os_vtn_name = ? and os_vbr_id != 0 order by os_vbr_id";
	public static final String IS_AUTO_VBR_LIST_NAME_SQL = "select os_vbr_status from os_vbr_tbl where os_vtn_name = ? and os_vbr_name = ?";
	public static final String CHK_MAX_AUTO_VBR_ID_SQL = "select os_vbr_id from os_vbr_tbl where os_vtn_name = ? and os_vbr_status != 0 and os_vbr_id > ?";

	/* CRUD SQLs for os_vbr_if_tbl */
	public static final String CHK_VBR_IF_SQL = "select count(*) from os_vbr_if_tbl where os_vtn_name = ? and os_vbr_name = ? and os_vbr_if_name=?";
	public static final String SEL_VBR_IF_MAP_SQL = "select os_map_type from os_vbr_if_tbl where os_vtn_name = ? and os_vbr_name = ? and os_vbr_if_name=?";
	public static final String SEL_VBR_IF_LP_SQL = "select os_logical_port_id from os_vbr_if_tbl where os_vtn_name = ? and os_vbr_name = ? and os_vbr_if_name=?";
	public static final String INS_VBR_IF_SQL = "insert into os_vbr_if_tbl(os_vbr_if_id, os_vtn_name, os_vbr_name, os_vbr_if_name, os_map_type, os_logical_port_id, os_vbr_if_status) values(?, ?, ?, ?, ?, ?, ?)";
	public static final String UP_VBR_IF_SQL = "update os_vbr_if_tbl set os_map_type = ?, os_logical_port_id = ? where os_vtn_name = ? and os_vbr_name = ? and os_vbr_if_name= ?";
	public static final String DEL_VBR_IF_SQL = "delete from os_vbr_if_tbl where os_vbr_if_id = ? and os_vtn_name = ? and os_vbr_name = ? and os_vbr_if_name=?";
	public static final String SEL_VBR_IF_IDS_SQL = "select os_vbr_if_id from os_vbr_if_tbl where os_vtn_name = ? and os_vbr_name = ? and os_vbr_if_id != 0 and os_vbr_if_id < ?";
	public static final String IS_AUTO_VBR_IF_LIST_NAME_SQL = "select os_vbr_if_status from os_vbr_if_tbl where os_vtn_name = ? and os_vbr_name = ? and os_vbr_if_name = ?";
	public static final String CHK_MAX_AUTO_VBR_IF_ID_SQL = "select os_vbr_if_id from os_vbr_if_tbl where os_vtn_name = ? and os_vbr_if_status != 0 and os_vbr_if_id > ?";
	public static final String SEL_VBR_IF_LIST_ID_CNT_SQL = "select count(os_vbr_if_id) from os_vbr_if_tbl where os_vtn_name = ? and os_vbr_if_id != 0";
	public static final String SEL_VBR_IF_LIST_ID_LIST_SQL = "select os_vbr_if_id from os_vbr_if_tbl where os_vtn_name = ? and os_vbr_if_id != 0 order by os_vbr_if_id";
	public static final String SEL_VBR_IF_MAX_ID_SQL = "select os_vbr_if_id from os_vbr_if_tbl where os_vtn_name = ? and os_vbr_if_id != 0 order by os_vbr_if_id DESC limit 1";

	/* CRUD SQLs for os_vbr_if_filter_tbl */
	public static final String INS_VBR_IF_FL_SQL = "insert into os_vbr_if_filter_tbl(os_vtn_name, os_vbr_name, os_vbr_if_name, os_vbr_if_filter_name) values(?, ?, ?, ?)";
	public static final String SEL_VBR_IF_FL_SQL = "select os_vbr_if_filter_name from os_vbr_if_filter_tbl where os_vtn_name = ? and os_vbr_name = ? and os_vbr_if_name=?";
	public static final String DEL_VBR_IF_FL_SQL = "delete from os_vbr_if_filter_tbl where os_vtn_name = ? and os_vbr_name = ? and os_vbr_if_name=?";

	/* CRUD SQLs for os_vrt_tbl */
	public static final String CHK_VRT_SQL = "select count(*) from os_vrt_tbl where os_vtn_name = ? and os_vrt_name = ?";
	public static final String INS_VRT_SQL = "insert into os_vrt_tbl(os_vtn_name, os_vrt_name) values(?, ?)";
	public static final String DEL_VRT_SQL = "delete from os_vrt_tbl where os_vtn_name = ? and os_vrt_name = ?";

	/* CRUD SQLs for os_controller_tbl */
	public static final String SEL_DEST_CTRL_SQL = "select os_controller_id from os_controller_tbl";
	public static final String INS_DEST_CTRL_SQL = "insert into os_controller_tbl(os_controller_id) values(?)";
	public static final String DEL_DEST_CTRL_SQL = "delete from os_controller_tbl";

	/* CRUD SQLs for os_vrt_route_tbl */
	public static final String CHK_ROUTE_SQL = "select count(*) from os_vrt_route_tbl where os_vtn_name = ? and os_vrt_name = ? and os_vrt_route_name = ?";
	public static final String SEL_ROUTE_SQL = "select os_vrt_route_name from os_vrt_route_tbl where os_vtn_name = ? and os_vrt_name = ?";
	public static final String INS_ROUTE_SQL = "insert into os_vrt_route_tbl(os_vtn_name, os_vrt_name, os_vrt_route_name) values(?, ?, ?)";
	public static final String DEL_ROUTE_SQL = "delete from os_vrt_route_tbl where os_vtn_name = ? and os_vrt_name = ? and os_vrt_route_name = ?";

	/* CRUD SQLs for os_vrt_if_tbl */
	public static final String CHK_VRT_IF_SQL = "select count(*) from os_vrt_if_tbl where os_vtn_name = ? and os_vrt_name = ? and os_vrt_if_name=?";
	public static final String SEL_VRT_IF_ID_SQL = "select count(*) from os_vrt_if_tbl where os_vtn_name = ? and os_vrt_if_id != 0";
	public static final String SEL_VRT_IF_IDS_SQL = "select os_vrt_if_id from os_vrt_if_tbl where os_vtn_name = ? and os_vrt_name = ? and os_vrt_if_id != 0";
	public static final String SEL_VRT_IF_VBR_SQL = "select os_vbr_name from os_vrt_if_tbl where os_vtn_name = ? and os_vrt_if_id = ?";
	public static final String SEL_VRT_IF_VRT_SQL = "select os_vrt_name from os_vrt_if_tbl where os_vtn_name = ? and os_vbr_name = ? and os_vrt_if_id = ?";
	public static final String INS_VRT_IF_SQL = "insert into os_vrt_if_tbl(os_vrt_if_id, os_vtn_name, os_vrt_name, os_vrt_if_name, os_vbr_name) values(?, ?, ?, ?, ?)";
	public static final String DEL_VRT_IF_SQL = "delete from os_vrt_if_tbl where os_vtn_name = ? and os_vrt_name = ? and os_vrt_if_id = ?";
	
	/* CRUD SQLs for os_fl_tbl */
	public static final String CHK_FL_SQL = "select count(*) from os_fl_tbl where os_fl_name = ?";
	public static final String SEL_FL_NAME_SQL = "select os_fl_name from os_fl_tbl";
	public static final String INS_FLOW_LIST_SQL = "insert into os_fl_tbl(os_fl_id, os_fl_name, os_fl_status) values(?, ?, ?)";
	public static final String DEL_FLOW_LIST_SQL = "delete from os_fl_tbl where os_fl_name = ?";
	public static final String SEL_FLOW_LIST_ID_CNT_SQL = "select count(os_fl_id) from (select os_fl_id from os_fl_tbl where os_fl_id != 0 group by os_fl_id) as id_list";
	public static final String SEL_FLOW_LIST_ID_LIST_SQL = "select os_fl_id from os_fl_tbl where os_fl_id != 0 group by os_fl_id order by os_fl_id";
	public static final String IS_AUTO_FLOW_LIST_NAME_SQL = "select os_fl_status from os_fl_tbl where os_fl_name = ?";
	public static final String CHK_FLOW_LIST_ID_SQL = "select count(*) from os_fl_tbl where os_fl_id = ?";
	public static final String SEL_FLOW_LIST_ID_SQL = "select os_fl_id from os_fl_tbl where os_fl_name = ?";
	
	/* CRUD SQLs for os_ff_vbr_tbl */
	public static final String SEL_FF_VBR_SQL = "SELECT os_vtn_name, os_vbr_name, os_vbr_if_name FROM os_ff_vbr_tbl WHERE os_fl_name = ?";
	public static final String INS_FLOW_FILTER_VBR_SQL = "insert into os_ff_vbr_tbl(os_vtn_name, os_vbr_name, os_vbr_if_name, os_fl_name) values(?, ?, ?, ?)";
	public static final String DEL_FLOW_FILTER_VBR_SQL = "delete from os_ff_vbr_tbl where os_vtn_name = ? and os_vbr_name = ? and os_vbr_if_name = ? and os_fl_name = ?";
	public static final String SEL_FF_VBR_LIST_FILTER_ID_SQL = "SELECT os_fl_name FROM os_ff_vbr_tbl WHERE os_vtn_name = ? and os_vbr_name = ? and os_vbr_if_name = ?";
	
	/* CRUD SQLs for os_ff_vrt_tbl */
	public static final String SEL_FF_VRT_SQL = "SELECT os_vtn_name, os_vrt_name, os_vrt_if_name, os_vbr_name FROM os_ff_vrt_tbl WHERE os_fl_name = ?";
	public static final String INS_FLOW_FILTER_VRT_SQL = "insert into os_ff_vrt_tbl(os_vtn_name, os_vrt_name, os_vrt_if_name, os_fl_name, os_vbr_name) values(?, ?, ?, ?, ?)";
	public static final String DEL_FLOW_FILTER_VRT_SQL = "delete from os_ff_vrt_tbl where os_vtn_name = ? and os_vrt_name = ? and os_vrt_if_name = ? and os_fl_name = ?";
	public static final String GET_FF_VRT_BY_VBR_SQL = "SELECT os_vrt_name FROM os_ff_vrt_tbl WHERE os_vtn_name = ? and os_vbr_name = ? and os_vrt_if_name = ? limit 1";
	public static final String SEL_FF_VRT_LIST_FILTER_ID_SQL = "SELECT os_fl_name FROM os_ff_vrt_tbl WHERE os_vtn_name = ? and os_vbr_name  = ? and os_vrt_if_name = ?";
}
