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
import java.util.ArrayList;
import java.util.List;

import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.openstack.beans.VRouterInterfaceBean;
import org.opendaylight.vtn.javaapi.openstack.dbmanager.VtnOpenStackSQLFactory;

/**
 * Data Access Object Class for os_vrt_if_tbl table
 */
public class VRouterInterfaceDao {

	/* Logger instance */
	private static final Logger LOG = Logger
			.getLogger(VRouterInterfaceDao.class.getName());

	/**
	 * Get resource counter for Router interface
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @return - next incremented counter
	 * @throws SQLException
	 */
	public int getNextId(Connection connection, String vtnName)
			throws SQLException {
		final String sql = VtnOpenStackSQLFactory.SEL_VRT_IF_ID_SQL;
		int vrtIfResourceId = -1;
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, vtnName);
			resultSet = statement.executeQuery();
			if (resultSet.next()) {
				vrtIfResourceId = resultSet.getInt(1) + 1;
				LOG.debug("Auto generated resource counter : "
						+ vrtIfResourceId);
			}
		} finally {
			if (resultSet != null) {
				resultSet.close();
			}
			if (statement != null) {
				statement.close();
			}
		}
		return vrtIfResourceId;
	}

	/**
	 * Insert Router interface information into database
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param vInterfaceBean
	 *            - Bean corresponding to os_vrt_if_tbl
	 * @return - insertion status
	 * @throws SQLException
	 */
	public int
			insert(Connection connection, VRouterInterfaceBean vInterfaceBean)
					throws SQLException {
		int status;
		final String sql = VtnOpenStackSQLFactory.INS_VRT_IF_SQL;
		PreparedStatement statement = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setInt(1, vInterfaceBean.getVrtIfId());
			statement.setString(2, vInterfaceBean.getVtnName());
			statement.setString(3, vInterfaceBean.getVrtName());
			statement.setString(4, vInterfaceBean.getVrtIfName());
			statement.setString(5, vInterfaceBean.getVbrName());
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
	 * Retrieve vbr_name from database
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param vInterfaceBean
	 *            - Bean corresponding to os_vrt_if_tbl
	 * @return - retrieved vbr_name
	 * @throws SQLException
	 */
	public String getVbridgeName(Connection connection,
			VRouterInterfaceBean vInterfaceBean) throws SQLException {
		final String sql = VtnOpenStackSQLFactory.SEL_VRT_IF_VBR_SQL;
		String vbrName = null;
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, vInterfaceBean.getVtnName());
			statement.setInt(2, vInterfaceBean.getVrtIfId());
			resultSet = statement.executeQuery();
			if (resultSet.next()) {
				vbrName = resultSet.getString(1);
				LOG.debug("Retrieved vbr_name : " + vbrName);
			}
		} finally {
			if (statement != null) {
				statement.close();
			}
		}
		return vbrName;
	}

	
	/**
	 * Retrieve vrt_name from database
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param vInterfaceBean
	 *            - Bean corresponding to os_vrt_if_tbl
	 * @return - retrieved vbr_name
	 * @throws SQLException
	 */
	public String getVrouterName(Connection connection,
			VRouterInterfaceBean vInterfaceBean) throws SQLException {
		final String sql = VtnOpenStackSQLFactory.SEL_VRT_IF_VRT_SQL;
		String vrtName = null;
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, vInterfaceBean.getVtnName());
			statement.setString(2, vInterfaceBean.getVbrName());
			statement.setInt(3, vInterfaceBean.getVrtIfId());
			resultSet = statement.executeQuery();
			if (resultSet.next()) {
				vrtName = resultSet.getString(1);
				LOG.debug("Retrieved vrt_name : " + vrtName);
			}
		} finally {
			if (statement != null) {
				statement.close();
			}
		}
		return vrtName;
	}
	/**
	 * Delete Router interface information from database
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param vInterfaceBean
	 *            - Bean corresponding to os_vrt_if_tbl
	 * @return - deletion status
	 * @throws SQLException
	 */
	public int
			delete(Connection connection, VRouterInterfaceBean vInterfaceBean)
					throws SQLException {
		int status;
		final String sql = VtnOpenStackSQLFactory.DEL_VRT_IF_SQL;
		PreparedStatement statement = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, vInterfaceBean.getVtnName());
			statement.setString(2, vInterfaceBean.getVrtName());
			statement.setInt(3, vInterfaceBean.getVrtIfId());
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
	 * Check the existence of Router interface
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param vInterfaceBean
	 *            - Bean corresponding to os_vrt_if_tbl
	 * @return - true, if resource found
	 * @throws SQLException
	 */
	public boolean isVrtIfFound(Connection connection,
			VRouterInterfaceBean vInterfaceBean) throws SQLException {
		boolean isFound = false;
		final String sql = VtnOpenStackSQLFactory.CHK_VRT_IF_SQL;
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, vInterfaceBean.getVtnName());
			statement.setString(2, vInterfaceBean.getVrtName());
			statement.setString(3, vInterfaceBean.getVrtIfName());
			resultSet = statement.executeQuery();
			if (resultSet.next() && resultSet.getInt(1) > 0) {
				isFound = true;
			}
		} finally {
			if (statement != null) {
				statement.close();
			}
		}
		return isFound;
	}

	/**
	 * Retrieve list of counters from database
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param vInterfaceBean
	 *            - Bean corresponding to os_vrt_if_tbl
	 * @return - retrieved vbr_name
	 * @throws SQLException
	 */
	public List<Integer> getVrtIfIds(Connection connection,
			VRouterInterfaceBean vInterfaceBean) throws SQLException {
		final String sql = VtnOpenStackSQLFactory.SEL_VRT_IF_IDS_SQL;
		List<Integer> ids = new ArrayList<Integer>();
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, vInterfaceBean.getVtnName());
			statement.setString(2, vInterfaceBean.getVrtName());
			resultSet = statement.executeQuery();
			while (resultSet.next()) {
				ids.add(resultSet.getInt(1));
			}
		} finally {
			if (statement != null) {
				statement.close();
			}
		}
		return ids;
	}
}
