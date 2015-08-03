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
import org.opendaylight.vtn.javaapi.openstack.beans.VBridgeBean;
import org.opendaylight.vtn.javaapi.openstack.dbmanager.VtnOpenStackSQLFactory;
import org.opendaylight.vtn.javaapi.openstack.dao.CommonDao;

/**
 * Data Access Object Class for os_vbr_tbl table
 */
public class VBridgeDao {

	private static final Logger LOG = Logger.getLogger(VBridgeDao.class
			.getName());

	/**
	 * Get resource counter for Network
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @return - next incremented counter
	 * @throws SQLException
	 */
	public int getNextId(Connection connection, String vtnName)
			throws SQLException {
		String sql = VtnOpenStackSQLFactory.SEL_VBR_LIST_ID_CNT_SQL;
		int vbrResourceId = -1;
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
				vbrResourceId = resultSet.getInt(1);
				LOG.debug("Get flow list id counter : "	+ vbrResourceId);
			} else {
				vbrResourceId = 0;
			}

			if (resultSet != null) {
				resultSet.close();
				resultSet = null;
			}
			if (statement != null) {
				statement.close();
				statement = null;
			}

			if (0 == vbrResourceId) {
				vbrResourceId++;
				LOG.debug("Auto generated resource counter : " + vbrResourceId);
				return vbrResourceId;
			}

			sql = VtnOpenStackSQLFactory.SEL_VBR_LIST_ID_LIST_SQL;
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

		if (idList.get(idList.size() - 1).intValue() == vbrResourceId) {
			vbrResourceId++;
			LOG.debug("Auto generated resource counter : " + vbrResourceId);
			return vbrResourceId;
		}

		low = 1;
		high = idList.size();
		vbrResourceId = comDao.getCount(idList, low, high);
		LOG.debug("Auto generated resource counter : " + vbrResourceId);
		return vbrResourceId;
	}

	/**
	 * Insert Network information into database
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param vBridgeBean
	 *            - Bean corresponding to os_vbr_tbl
	 * @return - insertion status
	 * @throws SQLException
	 */
	public int insert(Connection connection, VBridgeBean vBridgeBean)
			throws SQLException {
		int status;
		final String sql = VtnOpenStackSQLFactory.INS_VBR_SQL;
		PreparedStatement statement = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setInt(1, vBridgeBean.getVbrId());
			statement.setString(2, vBridgeBean.getVtnName());
			statement.setString(3, vBridgeBean.getVbrName());
			statement.setInt(4, vBridgeBean.getVbrStatus());
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
	 * Delete Network information from database
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param vBridgeBean
	 *            - Bean corresponding to os_vbr_tbl
	 * @return - deletion status
	 * @throws SQLException
	 */
	public int delete(Connection connection, VBridgeBean vBridgeBean)
			throws SQLException {
		int status;
		final String sql = VtnOpenStackSQLFactory.DEL_VBR_SQL;
		PreparedStatement statement = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setInt(1, vBridgeBean.getVbrId());
			statement.setString(2, vBridgeBean.getVtnName());
			statement.setString(3, vBridgeBean.getVbrName());
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
	 * Check the existence of Network
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param vBridgeBean
	 *            - Bean corresponding to os_vbr_tbl
	 * @return - true, if resource found
	 * @throws SQLException
	 */
	public boolean isVbrFound(Connection connection, VBridgeBean vBridgeBean)
			throws SQLException {
		boolean isFound = false;
		final String sql = VtnOpenStackSQLFactory.CHK_VBR_SQL;
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, vBridgeBean.getVtnName());
			statement.setString(2, vBridgeBean.getVbrName());
			resultSet = statement.executeQuery();
			if (resultSet.next() && (resultSet.getInt(1) > 0)) {
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
}
