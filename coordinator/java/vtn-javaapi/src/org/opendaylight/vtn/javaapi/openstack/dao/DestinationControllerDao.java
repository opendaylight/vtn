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
import org.opendaylight.vtn.javaapi.openstack.dbmanager.VtnOpenStackSQLFactory;

/**
 * Data Access Object Class for os_controller_tbl table
 */
public class DestinationControllerDao {

	private static final Logger LOG = Logger
			.getLogger(DestinationControllerDao.class.getName());

	/**
	 * Insert controller_id into database table os_controller_tbl
	 * 
	 * @param connection
	 *            - DB Connection
	 * @param controllerId
	 *            - controller_id to be set
	 * @return - insertion status
	 * @throws SQLException
	 */
	public int setDestinationController(Connection connection,
			String controllerId) throws SQLException {
		LOG.trace("Start ResourceIdManager#setDestinationController()");
		LOG.debug("Controller id : " + controllerId);
		int status;
		final String sql = VtnOpenStackSQLFactory.INS_DEST_CTRL_SQL;
		PreparedStatement statement = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, controllerId);
			status = statement.executeUpdate();
		} finally {
			if (statement != null) {
				statement.close();
			}
		}
		LOG.debug("Insertion status : " + status);
		LOG.trace("Complete ResourceIdManager#setDestinationController()");
		return status;
	}

	/**
	 * Delete controller_id from database table os_controller_tbl
	 * 
	 * @param connection
	 *            - DB Connection
	 * @return - deletion status
	 * @throws SQLException
	 */
	public int deleteDestinationController(Connection connection)
			throws SQLException {
		LOG.trace("Start ResourceIdManager#deleteDestinationController()");
		int status;
		final String sql = VtnOpenStackSQLFactory.DEL_DEST_CTRL_SQL;
		PreparedStatement statement = null;
		try {
			statement = connection.prepareStatement(sql);
			status = statement.executeUpdate();
		} finally {
			if (statement != null) {
				statement.close();
			}
		}
		LOG.debug("Deletion status : " + status);
		LOG.trace("Complete ResourceIdManager#deleteDestinationController()");
		return status;
	}

	/**
	 * Retrieve controller_id from database table os_controller_tbl
	 * 
	 * @param connection
	 *            - DB Connection
	 * @return - controller_id
	 * @throws SQLException
	 */
	public String getDestinationController(Connection connection)
			throws SQLException {
		LOG.trace("Start ResourceIdManager#deleteDestinationController()");
		String controllerId = null;
		final String sql = VtnOpenStackSQLFactory.SEL_DEST_CTRL_SQL;
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		try {
			statement = connection.prepareStatement(sql);
			resultSet = statement.executeQuery();
			if (resultSet.next()) {
				controllerId = resultSet.getString(1);
				LOG.debug("Extracted controller id : " + controllerId);
			}
		} finally {
			if (resultSet != null) {
				resultSet.close();
			}
			if (statement != null) {
				statement.close();
			}
		}
		LOG.trace("Complete ResourceIdManager#deleteDestinationController()");
		return controllerId;
	}
}
