/*
 * Copyright (c) 2015 NEC Corporation
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
import org.opendaylight.vtn.javaapi.openstack.beans.FlowListBean;
import org.opendaylight.vtn.javaapi.openstack.dbmanager.VtnOpenStackSQLFactory;

/**
 * Data Access Object Class for os_fl_tbl table
 */
public class FlowListDao {

	private static final Logger LOG = Logger
			.getLogger(FlowListDao.class.getName());


	/**
	 * Retrieve filter_id from database.
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @return - list of filter_id
	 * @throws SQLException
	 */
	public List<String> getFlowList(Connection connection) throws SQLException {
		List<String> list = new ArrayList<String>();
		final String sql = VtnOpenStackSQLFactory.SEL_FL_NAME_SQL;
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		try {
			statement = connection.prepareStatement(sql);
			resultSet = statement.executeQuery();
			while (resultSet.next()) {
				list.add(resultSet.getString(1));
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
	
	
	/**
	 * Check the existence of filter
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param vtnBean
	 *            - Bean corresponding to os_fl_tbl
	 * @return - true, if resource found
	 * @throws SQLException
	 */
	public boolean isFlowListFound(Connection connection, FlowListBean filterBean)
			throws SQLException {
		boolean isFound = false;
		final String sql = VtnOpenStackSQLFactory.CHK_FL_SQL;
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, filterBean.getFlName());
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
	 * Insert flow list information into database.
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param flowListBean
	 *            - Bean corresponding to os_fl_tbl
	 * @return - insertion status
	 * @throws SQLException
	 */
	public int insert(Connection connection, FlowListBean flowListBean)
			throws SQLException {
		int status;
		final String sql = VtnOpenStackSQLFactory.INS_FLOW_LIST_SQL;
		PreparedStatement statement = null;
		
		try {
			statement = connection.prepareStatement(sql);
			statement.setInt(1, flowListBean.getFlId());
			statement.setString(2, flowListBean.getFlName());
			statement.setInt(3, flowListBean.getFlStatus());
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
	 * Delete flow list information from database.
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param flowListBean
	 *            - Bean corresponding to os_fl_tbl
	 * @return - deletion status
	 * @throws SQLException
	 */
	public int delete(Connection connection, FlowListBean flowListBean)
			throws SQLException {
		int status;
		final String sql = VtnOpenStackSQLFactory.DEL_FLOW_LIST_SQL;
		PreparedStatement statement = null;
		
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, flowListBean.getFlName());
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
	 * Get resource counter for flow list.
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @return - next incremented counter
	 * @throws SQLException
	 */
	public int getNextId(Connection connection) throws SQLException {
		int count = -1;
		List<Integer> idList = new ArrayList<Integer>();
		String sql = VtnOpenStackSQLFactory.SEL_FLOW_LIST_ID_CNT_SQL;
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		int low, high;

		try {
			statement = connection.prepareStatement(sql);
			resultSet = statement.executeQuery();
			if (resultSet.next()) {
				count = resultSet.getInt(1);
				LOG.debug("Get flow list id counter : "	+ count);
			} else {
				count = 0;
			}

			if (resultSet != null) {
				resultSet.close();
				resultSet = null;
			}
			if (statement != null) {
				statement.close();
				statement = null;
			}

			if (0 == count) {
				count++;
				LOG.debug("Auto generated resource counter : " + count);
				return count;
			}

			sql = VtnOpenStackSQLFactory.SEL_FLOW_LIST_ID_LIST_SQL;
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

		if (idList.get(idList.size() - 1).intValue() == count) {
			count++;
			LOG.debug("Auto generated resource counter : " + count);
			return count;
		}

		low = 1;
		high = idList.size();
		count = getCount(idList, low, high);
		LOG.debug("Auto generated resource counter : " + count);
		return count;
	}

	/**
	 * Get id counter for flow list.
	 * 
	 * @param idList
	 *            - Id list
	 * @param low
	 *            - min index
	 * @param high
	 *            - max index
	 * @return next counter
	 */
	private int getCount(List<Integer> idList, int low, int high) {
		int mid = low + (high - low) / 2;

		if (low == high){
			if (idList.get(mid - 1).intValue() == mid) {
				mid = mid + 1;
				return mid;
			} else {
				return mid;
			}
		} else {
			if (idList.get(mid - 1).intValue() == mid) {
				return getCount(idList, mid + 1, high);
			} else {
				if (low > (mid - 1)) {
					return mid;
				} else {
					return getCount(idList, low, mid - 1);
				}
			}
		}
	}

	/**
	 * Judge the flow list name is auto generated.
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param flowListName
	 *            - Flow list name
	 * @return - true,if it is auto generation
	 * @throws SQLException
	 */
	public boolean isAutoFlowListName(Connection connection, String flowListName) 
			throws SQLException {
		boolean isAuto = false;
		final String sql = VtnOpenStackSQLFactory.IS_AUTO_FLOW_LIST_NAME_SQL;
		PreparedStatement statement = null;
		ResultSet resultSet = null;

		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, flowListName);
			resultSet = statement.executeQuery();
			if (resultSet.next() && resultSet.getInt(1) != 0) {
				isAuto = true;
			}
		} finally {
			if (resultSet != null) {
				resultSet.close();
			}
			if (statement != null) {
				statement.close();
			}
		}
		LOG.debug("Auto generation status : " + isAuto);
		return isAuto;
	}

	/**
	 * Check existence of specified id.
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param flowListId
	 *            - Flow list id
	 * @return - true,if the id found
	 * @throws SQLException
	 */
	public boolean isFlowListIdFound(Connection connection, int flowListId) 
			throws SQLException {
		boolean isFound = false;
		final String sql = VtnOpenStackSQLFactory.CHK_FLOW_LIST_ID_SQL;
		PreparedStatement statement = null;
		ResultSet resultSet = null;

		try {
			statement = connection.prepareStatement(sql);
			statement.setInt(1, flowListId);
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
	 * Get specified flow list id.
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param flowListName
	 *            - Flow list name
	 * @return - flow list id
	 * @throws SQLException
	 */
	public int getSpecifiedFlowListId(Connection connection, String flowListName) 
			throws SQLException {
		int id = -1;
		final String sql = VtnOpenStackSQLFactory.SEL_FLOW_LIST_ID_SQL;
		PreparedStatement statement = null;
		ResultSet resultSet = null;

		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, flowListName);
			resultSet = statement.executeQuery();
			if (resultSet.next()) {
				id = resultSet.getInt(1);
			}
		} finally {
			if (resultSet != null) {
				resultSet.close();
			}
			if (statement != null) {
				statement.close();
			}
		}
		LOG.debug("Specified flow list id : " + id);
		return id;
	}
}
