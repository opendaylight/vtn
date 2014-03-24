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
import org.opendaylight.vtn.javaapi.openstack.beans.VRouterBean;
import org.opendaylight.vtn.javaapi.openstack.dbmanager.VtnOpenStackSQLFactory;

/**
 * Data Access Object Class for os_vrt_tbl table
 */
public class VRouterDao {

	private static final Logger LOG = Logger.getLogger(VRouterDao.class
			.getName());

	/**
	 * Insert Router information into database
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param vRouterBean
	 *            - Bean corresponding to os_vrt_tbl
	 * @return - insertion status
	 * @throws SQLException
	 */
	public int insert(Connection connection, VRouterBean vRouterBean)
			throws SQLException {
		int status;
		final String sql = VtnOpenStackSQLFactory.INS_VRT_SQL;
		PreparedStatement statement = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, vRouterBean.getVtnName());
			statement.setString(2, vRouterBean.getVrtName());
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
	 * Delete Router information into database
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param vRouterBean
	 *            - Bean corresponding to os_vrt_tbl
	 * @return - deletion status
	 * @throws SQLException
	 */
	public int delete(Connection connection, VRouterBean vRouterBean)
			throws SQLException {
		int status;
		final String sql = VtnOpenStackSQLFactory.DEL_VRT_SQL;
		PreparedStatement statement = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, vRouterBean.getVtnName());
			statement.setString(2, vRouterBean.getVrtName());
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
	 * Check the existence of Router
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param vRouterBean
	 *            - Bean corresponding to os_vrt_tbl
	 * @return - true, if resource found
	 * @throws SQLException
	 */
	public boolean isVrtFound(Connection connection, VRouterBean vRouterBean)
			throws SQLException {
		boolean isFound = false;
		final String sql = VtnOpenStackSQLFactory.CHK_VRT_SQL;
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, vRouterBean.getVtnName());
			statement.setString(2, vRouterBean.getVrtName());
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
