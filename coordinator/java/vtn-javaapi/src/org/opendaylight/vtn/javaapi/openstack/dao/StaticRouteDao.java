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
import org.opendaylight.vtn.javaapi.openstack.beans.StaticRouteBean;
import org.opendaylight.vtn.javaapi.openstack.dbmanager.VtnOpenStackSQLFactory;

/**
 * Data Access Object Class for os_vrt_route_tbl table
 */
public class StaticRouteDao {

	private static final Logger LOG = Logger.getLogger(StaticRouteDao.class
			.getName());

	/**
	 * Insert Static-Route information into database
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param staticRouteBean
	 *            - Bean corresponding to os_vrt_route_tbl
	 * @return - insertion status
	 * @throws SQLException
	 */
	public int insert(Connection connection, StaticRouteBean staticRouteBean)
			throws SQLException {
		int status;
		final String sql = VtnOpenStackSQLFactory.INS_ROUTE_SQL;
		PreparedStatement statement = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, staticRouteBean.getVtnName());
			statement.setString(2, staticRouteBean.getVrtName());
			statement.setString(3, staticRouteBean.getRouteName());
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
	 * Delete Static-Route information from database
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param staticRouteBean
	 *            - Bean corresponding to os_vrt_route_tbl
	 * @return - deletion status
	 * @throws SQLException
	 */
	public int delete(Connection connection, StaticRouteBean staticRouteBean)
			throws SQLException {
		int status;
		final String sql = VtnOpenStackSQLFactory.DEL_ROUTE_SQL;
		PreparedStatement statement = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, staticRouteBean.getVtnName());
			statement.setString(2, staticRouteBean.getVrtName());
			statement.setString(3, staticRouteBean.getRouteName());
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
	 * Retrieve Static-Route list from database
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param staticRouteBean
	 *            - Bean corresponding to os_vrt_route_tbl
	 * @return - list of static-routes
	 * @throws SQLException
	 */
	public List<String> getList(Connection connection,
			StaticRouteBean staticRouteBean) throws SQLException {
		final List<String> routeList = new ArrayList<String>();
		final String sql = VtnOpenStackSQLFactory.SEL_ROUTE_SQL;
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		try {
			statement = connection.prepareStatement(sql);
			resultSet = statement.executeQuery();
			if (resultSet.next()) {
				routeList.add(resultSet.getString(1));
			}
		} finally {
			if (resultSet != null) {
				resultSet.close();
			}
			if (statement != null) {
				statement.close();
			}
		}
		return routeList;
	}

	/**
	 * Check the existence of Static Router
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param staticRouteBean
	 *            - Bean corresponding to os_vrt_route_tbl
	 * @return - true, if resource found
	 * @throws SQLException
	 */
	public boolean isStaticRouteFound(Connection connection,
			StaticRouteBean staticRouteBean) throws SQLException {
		boolean isFound = false;
		final String sql = VtnOpenStackSQLFactory.CHK_ROUTE_SQL;
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, staticRouteBean.getVtnName());
			statement.setString(2, staticRouteBean.getVrtName());
			statement.setString(3, staticRouteBean.getRouteName());
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
