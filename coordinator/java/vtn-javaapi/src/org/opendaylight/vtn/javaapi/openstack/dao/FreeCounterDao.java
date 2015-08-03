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
import org.opendaylight.vtn.javaapi.openstack.beans.FreeCounterBean;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.openstack.dbmanager.VtnOpenStackSQLFactory;

/**
 * Data Access Object Class for os_free_counter_tbl table
 */
public class FreeCounterDao {

	/* Logger instance */
	private static final Logger LOG = Logger.getLogger(FreeCounterDao.class
			.getName());

	/**
	 * Retrieve the resource counter for specific VTN, if available in database.
	 * Return -1 if not found
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param freeCounterBean
	 *            - Bean corresponding to os_free_counter_tbl
	 * @return - available resource counter
	 * @throws SQLException
	 */
	public List<Integer>
			getCounterList(Connection connection, FreeCounterBean freeCounterBean)
					throws SQLException {
		final String sql = VtnOpenStackSQLFactory.SEL_FC_SQL;
		int resourceCounter = -1;
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		List<Integer> idList = new ArrayList<Integer>();
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, freeCounterBean.getResourceId());
			statement.setString(2, freeCounterBean.getVtnName());
			resultSet = statement.executeQuery();
			while (resultSet.next()) {
				resourceCounter = resultSet.getInt(1);
				LOG.debug("Resource counter is available in os_free_counter_tbl : "
						+ resourceCounter);
				idList.add(resourceCounter);
			}
		} finally {
			if (resultSet != null) {
				resultSet.close();
			}
			if (statement != null) {
				statement.close();
			}
		}
		return idList;
	}

	/**
	 * Delete resource counter for specific VTN from database
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param freeCounterBean
	 *            - Bean corresponding to os_free_counter_tbl
	 * @return - deletion status
	 * @throws SQLException
	 */
	public int deleteCounter(Connection connection,
			FreeCounterBean freeCounterBean) throws SQLException {
		final String sql = VtnOpenStackSQLFactory.DEL_FC_SQL;
		int status;
		PreparedStatement statement = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, freeCounterBean.getResourceId());
			statement.setString(2, freeCounterBean.getVtnName());
			statement.setInt(3, freeCounterBean.getResourceCounter());
			status = statement.executeUpdate();
			LOG.debug("Deletion Status of resource id from os_free_counter_tbl : "
					+ status);
		} finally {
			if (statement != null) {
				statement.close();
			}
		}
		return status;
	}

	/**
	 * Insert the resource counter for specific VTN into database
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param freeCounterBean
	 *            - Bean corresponding to os_free_counter_tbl
	 * @return - insertion status
	 * @throws SQLException
	 */
	public int insertCounter(Connection connection,
			FreeCounterBean freeCounterBean) throws SQLException {
		final String sql = VtnOpenStackSQLFactory.INS_FC_SQL;
		int status;
		PreparedStatement statement = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setInt(1, freeCounterBean.getResourceCounter());
			statement.setString(2, freeCounterBean.getVtnName());
			statement.setString(3, freeCounterBean.getResourceId());
			status = statement.executeUpdate();
			LOG.debug("Deletion Status of resource id from os_free_counter_tbl : "
					+ status);
		} finally {
			if (statement != null) {
				statement.close();
			}
		}
		return status;
	}

	/**
	 * Delete all resource counters for specific VTN from database
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param freeCounterBean
	 *            - Bean corresponding to os_free_counter_tbl
	 * @return - deletion status
	 * @throws SQLException
	 */
	public int deleteVtnChilds(Connection connection,
			FreeCounterBean freeCounterBean) throws SQLException {
		final String sql = VtnOpenStackSQLFactory.DEL_VTN_CHILD_SQL;
		int status;
		PreparedStatement statement = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, freeCounterBean.getVtnName());
			statement.setString(2, VtnServiceOpenStackConsts.TENANT_RES_ID);
			status = statement.executeUpdate();
			LOG.debug("Deletion Status of resource counters from os_free_counter_tbl : "
					+ status);
		} finally {
			if (statement != null) {
				statement.close();
			}
		}
		return status;
	}

	/**
	 * Delete all resource counters for specific VTN from database
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param freeCounterBean
	 *            - Bean corresponding to os_free_counter_tbl
	 * @return - deletion status
	 * @throws SQLException
	 */
	public int deleteNodeChilds(Connection connection,
			FreeCounterBean freeCounterBean) throws SQLException {
		final String sql = VtnOpenStackSQLFactory.DEL_NODE_CHILD_SQL;
		int status;
		PreparedStatement statement = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, freeCounterBean.getVtnName());
			status = statement.executeUpdate();
			LOG.debug("Deletion Status of resource counters from os_free_counter_tbl : "
					+ status);
		} finally {
			if (statement != null) {
				statement.close();
			}
		}
		return status;
	}
	
	
	/**
	 * Find the Counter
	 * Return -1 if not found
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param freeCounterBean
	 *            - Bean corresponding to os_free_counter_tbl
	 * @return - true, if resource found
	 * @throws SQLException
	 */
	public boolean isCounterFound(Connection connection, FreeCounterBean freeCounterBean)
					throws SQLException {
		final String sql = VtnOpenStackSQLFactory.FID_FC_SQL;
		boolean isFound = false;
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, freeCounterBean.getResourceId());
			statement.setString(2, freeCounterBean.getVtnName());
			statement.setInt(3, freeCounterBean.getResourceCounter());
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
