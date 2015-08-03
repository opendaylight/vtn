/*
 * Copyright (c) 2014-2015 NEC Corporation
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
import java.util.List;

import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.openstack.beans.VBridgeBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VBridgeInterfaceBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VtnBean;
import org.opendaylight.vtn.javaapi.openstack.dbmanager.VtnOpenStackSQLFactory;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.resources.openstack.AutoIdManager;

public class CommonDao {
	
	private static final Logger LOG = Logger.getLogger(CommonDao.class.getName());

	/**
	 * Check String is digital
	 * 
	 * @param seqnum
	 *            - String
	 * @return - true, String is digital
	 */
	public boolean isValidInteger(String str) {
		try {
			if (str.substring(0, 1).equals("0")) {
				return false;
			}
			Integer.parseInt(str);
			return true;
		} catch (NumberFormatException e) {
			return false;
		}
	}

	/**
	 * Get id counter.
	 * 
	 * @param idList
	 *            - Id list
	 * @param low
	 *            - min index
	 * @param high
	 *            - max index
	 * @return next counter
	 */
	public int getCount(List<Integer> idList, int low, int high) {
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
	 * Judge the Resource name is auto generated.
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param resourceBean
	 *            - resource bean
	 * @return - true,if it is auto generation
	 * @throws SQLException
	 */
	public boolean isAutoResourceName(Connection connection, Object resourceBean) 
			throws SQLException {
		boolean isAuto = false;
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		String sql = null;

		try {
			if (resourceBean instanceof VtnBean) {
				final VtnBean vtnBean = (VtnBean) resourceBean;
				sql = VtnOpenStackSQLFactory.IS_AUTO_VTN_LIST_NAME_SQL;
				statement = connection.prepareStatement(sql);
				statement.setString(1, vtnBean.getVtnName());
			} else if (resourceBean instanceof VBridgeBean) {
				final VBridgeBean vbrBean = (VBridgeBean) resourceBean;
				sql = VtnOpenStackSQLFactory.IS_AUTO_VBR_LIST_NAME_SQL;
				statement = connection.prepareStatement(sql);
				statement.setString(1, vbrBean.getVtnName());
				statement.setString(2, vbrBean.getVbrName());
			} else {
				final VBridgeInterfaceBean vbrIfBean = (VBridgeInterfaceBean) resourceBean;
				sql = VtnOpenStackSQLFactory.IS_AUTO_VBR_IF_LIST_NAME_SQL;
				statement = connection.prepareStatement(sql);
				statement.setString(1, vbrIfBean.getVtnName());
				statement.setString(2, vbrIfBean.getVbrName());
				statement.setString(3, vbrIfBean.getVbrIfName());
			}

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
	 * Check specified id is max.
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param FreeCounterBean
	 *            - freeCounterBean
	 * @return - true,if the id biggest
	 * @throws SQLException
	 */
	public boolean isMaxId(Connection connection, Object resourceBean) 
			throws SQLException {
		boolean isMax = false;
		boolean isVtnType = false;
		PreparedStatement statement = null;
		ResultSet resultSet = null;
		String sql = null;
		VtnBean vtnBean = null;
		VBridgeBean vbrBean = null;
		VBridgeInterfaceBean vbrIfBean = null;
		int resourceId = 0;	

		try {
			if (resourceBean instanceof VtnBean) {
				vtnBean = (VtnBean) resourceBean;
				resourceId = vtnBean.getVtnId();
				sql = VtnOpenStackSQLFactory.CHK_MAX_AUTO_VTN_ID_SQL;
				statement = connection.prepareStatement(sql);
				statement.setInt(1, resourceId);
			} else if (resourceBean instanceof VBridgeBean) {
				vbrBean = (VBridgeBean) resourceBean;
				resourceId = vbrBean.getVbrId();
				sql = VtnOpenStackSQLFactory.CHK_MAX_AUTO_VBR_ID_SQL;
				statement = connection.prepareStatement(sql);
				statement.setString(1, vbrBean.getVtnName());
				statement.setInt(2, resourceId);
			} else {
				vbrIfBean = (VBridgeInterfaceBean) resourceBean;
				resourceId = vbrIfBean.getVbrIfId();
				sql = VtnOpenStackSQLFactory.CHK_MAX_AUTO_VBR_IF_ID_SQL;
				statement = connection.prepareStatement(sql);
				statement.setString(1, vbrIfBean.getVtnName());
				statement.setInt(2, resourceId);
			}

			resultSet = statement.executeQuery();
			if (resultSet.next()) {
				//the resourceId id is not max id in auto id list
				return isMax;
			}

			if (resultSet != null) {
				resultSet.close();
				resultSet = null;
			}
			if (statement != null) {
				statement.close();
				statement = null;
			}

			sql = VtnOpenStackSQLFactory.CHK_FC_MAX_ID_SQL;
			statement = connection.prepareStatement(sql);
			if (resourceBean instanceof VtnBean) {
				isVtnType = true;
				statement.setString(1, VtnServiceOpenStackConsts.TENANT_RES_ID);
				statement.setString(2, VtnServiceOpenStackConsts.DEFAULT_VTN);
			} else if (resourceBean instanceof VBridgeBean) {				
				statement.setString(1, VtnServiceOpenStackConsts.NETWORK_RES_ID);
				statement.setString(2, vbrBean.getVtnName());
			} else {		
				statement.setString(1, VtnServiceOpenStackConsts.PORT_RES_ID);
				statement.setString(2, vbrIfBean.getVtnName());
			}

			statement.setInt(3, resourceId);
			resultSet = statement.executeQuery();
			if (resultSet.next()) {
				//the resourceId id is not max id in free id list
				return isMax;
			}
		} finally {
			if (resultSet != null) {
				resultSet.close();
			}
			if (statement != null) {
				statement.close();
			}
		}

		if (isVtnType) {
			if (resourceId > AutoIdManager.getInstance().getMaxId()) {
				isMax = true;
			}
		} else {
			isMax = true;
		}

		return isMax;
	}
}
