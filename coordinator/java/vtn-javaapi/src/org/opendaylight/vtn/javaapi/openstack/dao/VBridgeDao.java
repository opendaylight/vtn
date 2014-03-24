/*
 * Copyright (c) 2013-2014 NEC Corporation
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

import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.openstack.beans.VBridgeBean;
import org.opendaylight.vtn.javaapi.openstack.dbmanager.VtnOpenStackSQLFactory;

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
		final String sql = VtnOpenStackSQLFactory.SEL_VBR_ID_SQL;
		int vbrResourceId = -1;
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, vtnName);
			resultSet = statement.executeQuery();
			if (resultSet.next()) {
				vbrResourceId = resultSet.getInt(1) + 1;
				LOG.debug("Auto generated resource counter : " + vbrResourceId);
			}
		} finally {
			if (resultSet != null) {
				resultSet.close();
			}
			if (statement != null) {
				statement.close();
			}
		}
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
}
