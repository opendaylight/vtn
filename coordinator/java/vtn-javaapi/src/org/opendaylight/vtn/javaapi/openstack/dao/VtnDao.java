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
import org.opendaylight.vtn.javaapi.openstack.beans.VtnBean;
import org.opendaylight.vtn.javaapi.openstack.dbmanager.VtnOpenStackSQLFactory;
import org.opendaylight.vtn.javaapi.resources.openstack.AutoIdManager;

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
		String sql = VtnOpenStackSQLFactory.SEL_VTN_LIST_ID_CNT_SQL;
		int vtnResourceId = -1;
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		List<Integer> idList = new ArrayList<Integer>();
		int id = AutoIdManager.getInstance().getMaxId();

		try {
			statement = connection.prepareStatement(sql);
			resultSet = statement.executeQuery();
			if (resultSet.next()) {
				vtnResourceId = resultSet.getInt(1);
				LOG.debug("Get flow list id counter : "	+ vtnResourceId);
			} else {
				vtnResourceId = 0;
			}

			if (resultSet != null) {
				resultSet.close();
				resultSet = null;
			}
			if (statement != null) {
				statement.close();
				statement = null;
			}

			if (0 == vtnResourceId) {
				vtnResourceId = ++id;
				LOG.debug("Auto generated resource counter : " + vtnResourceId);
				return vtnResourceId;
			}

			sql = VtnOpenStackSQLFactory.SEL_VTN_LIST_ID_LIST_SQL;
			statement = connection.prepareStatement(sql);
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

		if (idList.get(idList.size() - 1).intValue() <= id) {
			vtnResourceId = ++id;
			LOG.debug("Auto generated resource counter : " + vtnResourceId);
			return vtnResourceId;
		}

		int index = 0; 
		for (; index < idList.size(); index++) {
			if(id < idList.get(index).intValue()) {
				break;
			}
		}

		if (id + 1 < idList.get(index).intValue()) {
			vtnResourceId = id + 1;
		} else {
			vtnResourceId = getCount(idList.subList(index, idList.size()));
		}
		LOG.debug("Auto generated resource counter : " + vtnResourceId);
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
			statement.setInt(3, vtnBean.getVtnStatus());
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

	/**
	 * Check the existence of Tenant
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @return - true, if resource found
	 * @throws SQLException
	 */
	public boolean isVtnExist(Connection connection)
			throws SQLException {
		boolean isFound = false;
		final String sql = VtnOpenStackSQLFactory.CHK_VTN_EXIST_SQL;
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		try {
			statement = connection.prepareStatement(sql);
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


	private int getCount(List<Integer> idList) {
		int id = 0;

		if (idList.size() == 1) {
			id = idList.get(0).intValue() + 1;
			return id;
		}

		for (int index = 0; index < idList.size() - 1; index++) {
			if (idList.get(index).intValue() + 1 < idList.get(index + 1).intValue()) {
				id = idList.get(index).intValue() + 1;
				break;
			}
		}

		if (id == 0) {
			id = idList.get(idList.size() - 1).intValue() + 1;
		}
		return id;
	}
}
