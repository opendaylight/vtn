/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.openstack.dao;

import java.sql.BatchUpdateException;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.SQLTimeoutException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.List;

import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.openstack.beans.FlowFilterVbrBean;
import org.opendaylight.vtn.javaapi.openstack.beans.FlowFilterVrtBean;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.openstack.dbmanager.VtnOpenStackSQLFactory;

/**
 * Data Access Object Class for os_ff_vbr_tbl and os_ff_vrt_tbl table
 */
public class FlowFilterDao {

	private static final Logger LOG = Logger.getLogger(FlowFilterDao.class
			.getName());

	/**
	 * Retrieve flow filter from database.
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param filterId
	 *            - filter id.
	 * @return - JsonArray of apply ports
	 * @throws SQLException
	 */
	public JsonArray getFlowFilters(Connection connection, String filterId)
			throws SQLException {
		JsonArray applyports = new JsonArray();
		String sql = VtnOpenStackSQLFactory.SEL_FF_VBR_SQL;
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, filterId);
			resultSet = statement.executeQuery();
			while (resultSet.next()) {
				applyports.add(makeNetWork(resultSet.getString(1),
						resultSet.getString(2), resultSet.getString(3).substring(6)));
			}

			if (resultSet != null) {
				resultSet.close();
				resultSet = null;
			}
			if (statement != null) {
				statement.close();
				statement = null;
			}

			sql = VtnOpenStackSQLFactory.SEL_FF_VRT_SQL;
			statement = connection.prepareStatement(sql);
			statement.setString(1, filterId);
			resultSet = statement.executeQuery();
			while (resultSet.next()) {
				applyports.add(makeRouter(resultSet.getString(1),
						resultSet.getString(2), resultSet.getString(3).substring(6)));
			}
		} finally {
			if (resultSet != null) {
				resultSet.close();
			}
			if (statement != null) {
				statement.close();
			}
		}
		return applyports;
	}

	/**
	 * 
	 */
	private JsonObject makeNetWork(String tenant_id, String net_id,
			String port_id) {
		JsonObject networkObject = new JsonObject();
		networkObject.addProperty(VtnServiceOpenStackConsts.TENANT, tenant_id);
		networkObject.addProperty(VtnServiceOpenStackConsts.NETWORK, net_id);
		networkObject.addProperty(VtnServiceOpenStackConsts.PORT, port_id);
		return networkObject;
	}

	/**
	 * 
	 */
	private JsonObject makeRouter(String tenant_id, String router_id,
			String if_id) {
		JsonObject routerObject = new JsonObject();
		routerObject.addProperty(VtnServiceOpenStackConsts.TENANT, tenant_id);
		routerObject.addProperty(VtnServiceOpenStackConsts.ROUTER, router_id);
		routerObject.addProperty(VtnServiceOpenStackConsts.INTERFACE, if_id);
		return routerObject;
	}

	/**
	 * Judge batch result.
	 * 
	 * @param status
	 *            - result array
	 * @return - If all success, true. Otherwise false.
	 */
	private boolean judgeBatchResult(int[] status) {
		for (int j = 0; j < status.length; j++) {
			if (!(status[j] >= 0 || status[j] == Statement.SUCCESS_NO_INFO)) {
				return false;
			}
		}
		
		return true;
	}
	
	/**
	 * Insert flow filter information into database.
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param filterBeanList
	 *            - Bean list corresponding to os_ff_vbr_tbl
	 * @return - insertion status
	 * @throws SQLException
	 */
	public int insertVbridgeFilterInfo(Connection connection,
			ArrayList<FlowFilterVbrBean> filterBeanList) throws SQLException {
		int retVal = 1;
		int[] status;
		final String sql = VtnOpenStackSQLFactory.INS_FLOW_FILTER_VBR_SQL;
		PreparedStatement statement = null;

		if (filterBeanList.isEmpty()) {
			return retVal;
		}

		try {
			statement = connection.prepareStatement(sql);
			int i = 0;
			for (FlowFilterVbrBean filterBean : filterBeanList) {
				i++;
				statement.setString(1, filterBean.getVtnName());
				statement.setString(2, filterBean.getVbrName());
				statement.setString(3, filterBean.getVbrIfName());
				statement.setString(4, filterBean.getFlName());

				statement.addBatch();

				if (i % 1000 == 0) {
					status = statement.executeBatch();
					if (!judgeBatchResult(status)) {
						retVal = 0;
						break;
					}
				}
			}
			if (0 != retVal && (i % 1000 != 0)) {
				status = statement.executeBatch();
				if (!judgeBatchResult(status)) {
					retVal = 0;
				}
			}
		} catch(BatchUpdateException e) {
			retVal = 0;
		} catch(SQLTimeoutException e) {
			retVal = 0;
		} finally {
			if (statement != null) {
				statement.close();
			}
		}
		LOG.debug("Insertion status(os_ff_vbr_tbl) : " + retVal);
		return retVal;
	}

	/**
	 * Insert flow filter information into database.
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param filterBeanList
	 *            - Bean list corresponding to os_ff_vrt_tbl
	 * @return - insertion status
	 * @throws SQLException
	 */
	public int insertVrouterFilterInfo(Connection connection,
			ArrayList<FlowFilterVrtBean> filterBeanList) throws SQLException {
		int retVal = 1;
		int[] status;
		final String sql = VtnOpenStackSQLFactory.INS_FLOW_FILTER_VRT_SQL;
		PreparedStatement statement = null;

		if (filterBeanList.isEmpty()) {
			return retVal;
		}

		try {
			statement = connection.prepareStatement(sql);
			int i = 0;
			for (FlowFilterVrtBean filterBean : filterBeanList) {
				i++;
				statement.setString(1, filterBean.getVtnName());
				statement.setString(2, filterBean.getVrtName());
				statement.setString(3, filterBean.getVrtIfName());
				statement.setString(4, filterBean.getFlName());
				statement.setString(5, filterBean.getVbrName());

				statement.addBatch();

				if (i % 1000 == 0) {
					status = statement.executeBatch();
					if (!judgeBatchResult(status)) {
						retVal = 0;
						break;
					}
				}
			}
			if (0 != retVal && (i % 1000 != 0)) {
				status = statement.executeBatch();
				if (!judgeBatchResult(status)) {
					retVal = 0;
				}
			}
		} catch(BatchUpdateException e) {
			retVal = 0;
		} catch(SQLTimeoutException e) {
			retVal = 0;
		} finally {
			if (statement != null) {
				statement.close();
			}
		}
		LOG.debug("Insertion status(os_ff_vrt_tbl) : " + retVal);
		return retVal;
	}

	/**
	 * Delete flow filter information from database.
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param filterBeanList
	 *            - Bean list corresponding to os_ff_vbr_tbl
	 * @return - deletion status
	 * @throws SQLException
	 */
	public int deleteVbridgeFilterInfo(Connection connection,
			ArrayList<FlowFilterVbrBean> filterBeanList) throws SQLException {
		int retVal = 1;
		int[] status;
		final String sql = VtnOpenStackSQLFactory.DEL_FLOW_FILTER_VBR_SQL;
		PreparedStatement statement = null;

		if (filterBeanList.isEmpty()) {
			return retVal;
		}

		try {
			statement = connection.prepareStatement(sql);
			int i = 0;
			for (FlowFilterVbrBean filterBean : filterBeanList) {
				i++;
				statement.setString(1, filterBean.getVtnName());
				statement.setString(2, filterBean.getVbrName());
				statement.setString(3, filterBean.getVbrIfName());
				statement.setString(4, filterBean.getFlName());

				statement.addBatch();

				if (i % 1000 == 0) {
					status = statement.executeBatch();
					if (!judgeBatchResult(status)) {
						retVal = 0;
						break;
					}
				}
			}
			if (0 != retVal && (i % 1000 != 0)) {
				status = statement.executeBatch();
				if (!judgeBatchResult(status)) {
					retVal = 0;
				}
			}
		} catch(BatchUpdateException e) {
			retVal = 0;
		} catch(SQLTimeoutException e) {
			retVal = 0;
		} finally {
			if (statement != null) {
				statement.close();
			}
		}
		LOG.debug("Deletion status(os_ff_vbr_tbl) : " + retVal);
		return retVal;
	}

	/**
	 * Delete flow filter information from database.
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param filterBeanList
	 *            - Bean list corresponding to os_ff_vrt_tbl
	 * @return - deletion status
	 * @throws SQLException
	 */
	public int deleteVrouterFilterInfo(Connection connection,
			ArrayList<FlowFilterVrtBean> filterBeanList) throws SQLException {
		int retVal = 1;
		int[] status;
		final String sql = VtnOpenStackSQLFactory.DEL_FLOW_FILTER_VRT_SQL;
		PreparedStatement statement = null;

		if (filterBeanList.isEmpty()) {
			return retVal;
		}

		try {
			statement = connection.prepareStatement(sql);
			int i = 0;
			for (FlowFilterVrtBean filterBean : filterBeanList) {
				i++;
				statement.setString(1, filterBean.getVtnName());
				statement.setString(2, filterBean.getVrtName());
				statement.setString(3, filterBean.getVrtIfName());
				statement.setString(4, filterBean.getFlName());

				statement.addBatch();

				if (i % 1000 == 0) {
					status = statement.executeBatch();
					if (!judgeBatchResult(status)) {
						retVal = 0;
						break;
					}
				}
			}
			if (0 != retVal && (i % 1000 != 0)) {
				status = statement.executeBatch();
				if (!judgeBatchResult(status)) {
					retVal = 0;
				}
			}
		} catch(BatchUpdateException e) {
			retVal = 0;
		} catch(SQLTimeoutException e) {
			retVal = 0;
		} finally {
			if (statement != null) {
				statement.close();
			}
		}
		LOG.debug("Deletion status(os_ff_vrt_tbl) : " + retVal);
		return retVal;
	}

	/**
	 * Retrieve list of interface from database.
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param filterName
	 *            - Filter name
	 * @return - List of interface information
	 * @throws SQLException
	 */
	public ArrayList<FlowFilterVrtBean> getInterfaceList(Connection connection,
			String filterName) throws SQLException {
		ArrayList<FlowFilterVrtBean> portList = new ArrayList<FlowFilterVrtBean>();
		String sql = VtnOpenStackSQLFactory.SEL_FF_VBR_SQL;
		PreparedStatement statement = null;
		ResultSet resultSet = null;

		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, filterName);
			resultSet = statement.executeQuery();
			while (resultSet.next()) {
				FlowFilterVrtBean filter = new FlowFilterVrtBean();

				filter.setVtnName(resultSet.getString(1));
				filter.setVbrName(resultSet.getString(2));
				filter.setVrtIfName(resultSet.getString(3));

				portList.add(filter);
			}

			if (resultSet != null) {
				resultSet.close();
				resultSet = null;
			}
			if (statement != null) {
				statement.close();
				statement = null;
			}

			sql = VtnOpenStackSQLFactory.SEL_FF_VRT_SQL;
			statement = connection.prepareStatement(sql);
			statement.setString(1, filterName);
			resultSet = statement.executeQuery();
			while (resultSet.next()) {
				FlowFilterVrtBean filter = new FlowFilterVrtBean();

				filter.setVtnName(resultSet.getString(1));
				filter.setVrtName(resultSet.getString(2));
				filter.setVrtIfName(resultSet.getString(3));
				filter.setVbrName(resultSet.getString(4));

				portList.add(filter);
			}
		} finally {
			if (resultSet != null) {
				resultSet.close();
			}
			if (statement != null) {
				statement.close();
			}
		}

		return portList.size() > 0 ? portList : null;
	}

	/**
	 * Retrieve flow filter on interface of port from database.
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param flowFilterVbrBean
	 *            - FlowFilterVbrBean.
	 * @param isLinked
	 *            - if has related vroute is true, otherwise false.
	 * @return - List of flow filter id.
	 * @throws SQLException
	 */
	public List<String> getFlowFiltersByPort(Connection connection,
			FlowFilterVbrBean flowFilterVbrBean,
			boolean isLinked) throws SQLException {
		List<String> filteIds = new ArrayList<String>();
		String sql = VtnOpenStackSQLFactory.SEL_FF_VBR_LIST_FILTER_ID_SQL;
		PreparedStatement statement = null;
		ResultSet resultSet = null;

		if (isLinked) {
			sql = VtnOpenStackSQLFactory.SEL_FF_VRT_LIST_FILTER_ID_SQL;
		}

		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, flowFilterVbrBean.getVtnName());
			statement.setString(2, flowFilterVbrBean.getVbrName());
			statement.setString(3, flowFilterVbrBean.getVbrIfName());
			resultSet = statement.executeQuery();
			while (resultSet.next()) {
				filteIds.add(resultSet.getString(1));
			}
		} finally {
			if (resultSet != null) {
				resultSet.close();
			}
			if (statement != null) {
				statement.close();
			}
		}
		return filteIds;
	}

	/**
	 * Check whether the port link with router.
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param flowFilterVrtBean
	 *            - Bean corresponding to os_ff_vrt_tbl
	 * @return - vrouter name, if resource found, otherwise false.
	 * @throws SQLException
	 */
	public String isLinkedWithRouter(Connection connection,
			FlowFilterVrtBean flowFilterVrtBean) throws SQLException {
		String sql = VtnOpenStackSQLFactory.GET_FF_VRT_BY_VBR_SQL;
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		String vrouter = null;
		try {
			statement = connection.prepareStatement(sql);
			statement.setString(1, flowFilterVrtBean.getVtnName());
			statement.setString(2, flowFilterVrtBean.getVbrName());
			statement.setString(3, flowFilterVrtBean.getVrtIfName());
			resultSet = statement.executeQuery();
			if (resultSet.next()) {
				vrouter = resultSet.getString(1);
			}
		} finally {
			if (resultSet != null) {
				resultSet.close();
			}
			if (statement != null) {
				statement.close();
			}
		}
		return vrouter;
	}
}
