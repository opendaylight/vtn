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
import org.opendaylight.vtn.javaapi.openstack.beans.VtnBean;
import org.opendaylight.vtn.javaapi.openstack.dbmanager.VtnOpenStackSQLFactory;

/**
 * Data Access Object Class for os_vtn_tbl table
 */
public class VtnDao {

	private static final Logger LOG = Logger.getLogger(VtnDao.class.getName());

	/**
	 * Get resource counter for Tenant
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @return - next incremented counter
	 * @throws SQLException
	 */
	public int getNextId(Connection connection) throws SQLException {
		final String sql = VtnOpenStackSQLFactory.SEL_VTN_ID_SQL;
		int vtnResourceId = -1;
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		try {
			statement = connection.prepareStatement(sql);
			resultSet = statement.executeQuery();
			if (resultSet.next()) {
				vtnResourceId = resultSet.getInt(1) + 1;
				LOG.debug("Auto generated resource counter : " + vtnResourceId);
			}
		} finally {
			if (resultSet != null) {
				resultSet.close();
			}
			if (statement != null) {
				statement.close();
			}
		}
		return vtnResourceId;
	}

	/**
	 * Insert Tenant information into database
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param vtnBean
	 *            - Bean corresponding to os_vtn_tbl
	 * @return - insertion status
	 * @throws SQLException
	 */
	public int insert(Connection connection, VtnBean vtnBean)
			throws SQLException {
		int status;
		final String sql = VtnOpenStackSQLFactory.INS_VTN_SQL;
		PreparedStatement statement = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setInt(1, vtnBean.getVtnId());
			statement.setString(2, vtnBean.getVtnName());
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
	 * Delete Tenant information into database
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param vtnBean
	 *            - Bean corresponding to os_vtn_tbl
	 * @return - deletion status
	 * @throws SQLException
	 */
	public int delete(Connection connection, VtnBean vtnBean)
			throws SQLException {
		int status;
		final String sql = VtnOpenStackSQLFactory.DEL_VTN_SQL;
		PreparedStatement statement = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, vtnBean.getVtnName());
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
	 * Check the existence of Tenant
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param vtnBean
	 *            - Bean corresponding to os_vtn_tbl
	 * @return - true, if resource found
	 * @throws SQLException
	 */
	public boolean isVtnFound(Connection connection, VtnBean vtnBean)
			throws SQLException {
		boolean isFound = false;
		final String sql = VtnOpenStackSQLFactory.CHK_VTN_SQL;
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, vtnBean.getVtnName());
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
