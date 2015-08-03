/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.openstack.dao;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;

import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.openstack.beans.VBridgeInterfaceBean;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.openstack.dbmanager.VtnOpenStackSQLFactory;
import org.opendaylight.vtn.javaapi.openstack.dao.CommonDao;

/**
 * Data Access Object Class for os_vbr_if_tbl table
 */
public class VBridgeInterfaceDao {

	private static final Logger LOG = Logger
			.getLogger(VBridgeInterfaceDao.class.getName());

	/**
	 * Get resource counter for Port
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @return - next incremented counter
	 * @throws SQLException
	 */
	public int getNextId(Connection connection, String vtnName)
			throws SQLException {
		String sql = VtnOpenStackSQLFactory.SEL_VBR_IF_LIST_ID_CNT_SQL;
		int vbrIfResourceId = -1;
		PreparedStatement statement = null;
		ResultSet resultSet = null;

		CommonDao comDao = new CommonDao();
		List<Integer> idList = new ArrayList<Integer>();
		int low, high;

		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, vtnName);
			resultSet = statement.executeQuery();
			if (resultSet.next()) {
				vbrIfResourceId = resultSet.getInt(1);
				LOG.debug("Get flow list id counter : "	+ vbrIfResourceId);
			} else {
				vbrIfResourceId = 0;
			}

			if (resultSet != null) {
				resultSet.close();
				resultSet = null;
			}
			if (statement != null) {
				statement.close();
				statement = null;
			}

			if (0 == vbrIfResourceId) {
				vbrIfResourceId++;
				LOG.debug("Auto generated resource counter : " + vbrIfResourceId);
				return vbrIfResourceId;
			}

			sql = VtnOpenStackSQLFactory.SEL_VBR_IF_LIST_ID_LIST_SQL;
			statement = connection.prepareStatement(sql);
			statement.setString(1, vtnName);
			resultSet = statement.executeQuery();
			while (resultSet.next()) {
				idList.add(resultSet.getInt(1));
			}

		} finally {
			if (resultSet != null) {
				resultSet.close();
			}
			if (statement != null) {
				statement.close();
			}
		}

		if (idList.get(idList.size() - 1).intValue() == vbrIfResourceId) {
			vbrIfResourceId++;
			LOG.debug("Auto generated resource counter : " + vbrIfResourceId);
			return vbrIfResourceId;
		}

		low = 1;
		high = idList.size();
		vbrIfResourceId = comDao.getCount(idList, low, high);
		LOG.debug("Auto generated resource counter : " + vbrIfResourceId);
		return vbrIfResourceId;
	}

	/**
	 * Insert Port information into database
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param vInterfaceBean
	 *            - Bean corresponding to os_vbr_if_tbl
	 * @return - insertion status
	 * @throws SQLException
	 */
	public int
			insert(Connection connection, VBridgeInterfaceBean vInterfaceBean)
					throws SQLException {
		int status;
		final String sql = VtnOpenStackSQLFactory.INS_VBR_IF_SQL;
		PreparedStatement statement = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setInt(1, vInterfaceBean.getVbrIfId());
			statement.setString(2, vInterfaceBean.getVtnName());
			statement.setString(3, vInterfaceBean.getVbrName());
			statement.setString(4, vInterfaceBean.getVbrIfName());
			statement.setString(5, vInterfaceBean.getMapType());
			statement.setString(6, vInterfaceBean.getLogicalPortId());
			statement.setInt(7, vInterfaceBean.getVbrIfStatus());
			status = statement.executeUpdate();
		} finally {
			if (statement != null) {
				statement.close();
			}
		}
		LOG.debug("Insertion status : " + status);
		return status;
	}

	/**
	 * Insert Port information into database
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param vInterfaceBean
	 *            - Bean corresponding to os_vbr_if_tbl
	 * @return - deletion status
	 * @throws SQLException
	 */
	public int
			delete(Connection connection, VBridgeInterfaceBean vInterfaceBean)
					throws SQLException {
		int status;
		final String sql = VtnOpenStackSQLFactory.DEL_VBR_IF_SQL;
		PreparedStatement statement = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setInt(1, vInterfaceBean.getVbrIfId());
			statement.setString(2, vInterfaceBean.getVtnName());
			statement.setString(3, vInterfaceBean.getVbrName());
			statement.setString(4, vInterfaceBean.getVbrIfName());
			status = statement.executeUpdate();
		} finally {
			if (statement != null) {
				statement.close();
			}
		}
		LOG.debug("Deletion status : " + status);
		return status;
	}

	/**
	 * Retrieve map_type information into database
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param vInterfaceBean
	 *            - Bean corresponding to os_vbr_if_tbl
	 * @return - map_type
	 * @throws SQLException
	 */
	public String getMapType(Connection connection,
			VBridgeInterfaceBean vInterfaceBean) throws SQLException {
		final String sql = VtnOpenStackSQLFactory.SEL_VBR_IF_MAP_SQL;
		String mapType = null;
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, vInterfaceBean.getVtnName());
			statement.setString(2, vInterfaceBean.getVbrName());
			statement.setString(3, vInterfaceBean.getVbrIfName());
			resultSet = statement.executeQuery();
			if (resultSet.next()) {
				mapType = resultSet.getString(1);
				LOG.debug("Auto generated resource counter : " + mapType);
			}
		} finally {
			if (resultSet != null) {
				resultSet.close();
			}
			if (statement != null) {
				statement.close();
			}
		}
		return mapType;
	}

	/**
	 * Retrieve logical_port_id information into database
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param vInterfaceBean
	 *            - Bean corresponding to os_vbr_if_tbl
	 * @return - logical_port_id
	 * @throws SQLException
	 */
	public String getLogicalPortId(Connection connection,
			VBridgeInterfaceBean vInterfaceBean) throws SQLException {
		final String sql = VtnOpenStackSQLFactory.SEL_VBR_IF_LP_SQL;
		String logicalPortId = null;
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, vInterfaceBean.getVtnName());
			statement.setString(2, vInterfaceBean.getVbrName());
			statement.setString(3, vInterfaceBean.getVbrIfName());
			resultSet = statement.executeQuery();
			if (resultSet.next()) {
				logicalPortId = resultSet.getString(1);
				LOG.debug("Logical Port Id : " + logicalPortId);
			}
		} finally {
			if (resultSet != null) {
				resultSet.close();
			}
			if (statement != null) {
				statement.close();
			}
		}
		return logicalPortId;
	}

	/**
	 * Check the existence of Port
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param vInterfaceBean
	 *            - Bean corresponding to os_vbr_if_tbl
	 * @return - true, if resource found
	 * @throws SQLException
	 */
	public boolean isVbrIfFound(Connection connection,
			VBridgeInterfaceBean vInterfaceBean) throws SQLException {
		boolean isFound = false;
		final String sql = VtnOpenStackSQLFactory.CHK_VBR_IF_SQL;
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, vInterfaceBean.getVtnName());
			statement.setString(2, vInterfaceBean.getVbrName());
			statement.setString(3, vInterfaceBean.getVbrIfName());
			resultSet = statement.executeQuery();
			if (resultSet.next() && resultSet.getInt(1) > 0) {
				isFound = true;
			}
		} finally {
			if (resultSet != null) {
				resultSet.close();
			}
			if (statement != null) {
				statement.close();
			}
		}
		return isFound;
	}

	/**
	 * Update map_type and logical_port_id into database
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param vInterfaceBean
	 *            - Bean corresponding to os_vbr_if_tbl
	 * @return - update status
	 * @throws SQLException
	 */
	public int updateVlanMapInfo(Connection connection,
			VBridgeInterfaceBean vInterfaceBean) throws SQLException {
		int status;
		final String sql = VtnOpenStackSQLFactory.UP_VBR_IF_SQL;
		PreparedStatement statement = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, vInterfaceBean.getMapType());
			statement.setString(2, vInterfaceBean.getLogicalPortId());
			statement.setString(3, vInterfaceBean.getVtnName());
			statement.setString(4, vInterfaceBean.getVbrName());
			statement.setString(5, vInterfaceBean.getVbrIfName());
			status = statement.executeUpdate();
		} finally {
			if (statement != null) {
				statement.close();
			}
		}
		LOG.debug("Update status : " + status);
		return status;
	}

	/**
	 * Retrieve if_id from database for specific vbr_name and vtn_name
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param vInterfaceBean
	 *            - Bean corresponding to os_vbr_if_tbl
	 * @return - list of if_id
	 * @throws SQLException
	 */
	public List<Integer> getVbrIfIds(Connection connection,
			VBridgeInterfaceBean vInterfaceBean) throws SQLException {
		List<Integer> list = new ArrayList<Integer>();
		String sql = VtnOpenStackSQLFactory.SEL_MAX_FC_SQL;
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		int vbrIfMaxResourceId = -1;

		try {/*get the max id from free counter*/
			statement = connection.prepareStatement(sql);
			statement.setString(1, VtnServiceOpenStackConsts.PORT_RES_ID);
			statement.setString(2, vInterfaceBean.getVtnName());
			resultSet = statement.executeQuery();
			if (resultSet.next()) {
				vbrIfMaxResourceId = resultSet.getInt(1);
			}

			if (resultSet != null) {
				resultSet.close();
				resultSet = null;
			}
			if (statement != null) {
				statement.close();
				statement = null;
			}

			/*get the max interface id from the specified vtn*/
			sql = VtnOpenStackSQLFactory.SEL_VBR_IF_MAX_ID_SQL;
			statement = connection.prepareStatement(sql);
			statement.setString(1, vInterfaceBean.getVtnName());
			resultSet = statement.executeQuery();
			if (resultSet.next() && (resultSet.getInt(1) > vbrIfMaxResourceId)) {
				vbrIfMaxResourceId = resultSet.getInt(1);
			}

			if (resultSet != null) {
				resultSet.close();
				resultSet = null;
			}
			if (statement != null) {
				statement.close();
				statement = null;
			}

			/*get the id list which little than the specified id*/
			sql = VtnOpenStackSQLFactory.SEL_VBR_IF_IDS_SQL;
			statement = connection.prepareStatement(sql);
			statement.setString(1, vInterfaceBean.getVtnName());
			statement.setString(2, vInterfaceBean.getVbrName());
			statement.setInt(3, vbrIfMaxResourceId);
			resultSet = statement.executeQuery();
			while (resultSet.next()) {
				list.add(resultSet.getInt(1));
			}
		} finally {
			if (resultSet != null) {
				resultSet.close();
			}
			if (statement != null) {
				statement.close();
			}
		}
		return list;
	}
}
