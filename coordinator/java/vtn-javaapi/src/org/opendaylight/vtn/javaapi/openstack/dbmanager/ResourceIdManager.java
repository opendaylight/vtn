/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.openstack.dbmanager;

import java.sql.Connection;
import java.sql.SQLException;
import java.util.List;

import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.openstack.beans.FreeCounterBean;
import org.opendaylight.vtn.javaapi.openstack.beans.StaticRouteBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VBridgeBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VBridgeInterfaceBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VRouterBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VRouterInterfaceBean;
import org.opendaylight.vtn.javaapi.openstack.beans.VtnBean;
import org.opendaylight.vtn.javaapi.openstack.beans.FlowListBean;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.openstack.dao.FlowListDao;
import org.opendaylight.vtn.javaapi.openstack.dao.FreeCounterDao;
import org.opendaylight.vtn.javaapi.openstack.dao.StaticRouteDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VBridgeDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VBridgeInterfaceDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VRouterDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VRouterInterfaceDao;
import org.opendaylight.vtn.javaapi.openstack.dao.VtnDao;
import org.opendaylight.vtn.javaapi.openstack.dao.CommonDao;
import org.opendaylight.vtn.javaapi.resources.openstack.AutoIdManager;

/**
 * ResourceId Manager class that contains interface for management of
 * auto-generated resource counters in the system
 */
public class ResourceIdManager {

	private static final Logger LOG = Logger.getLogger(ResourceIdManager.class
			.getName());

	/**
	 * Generated resource counter for specified resource First check the
	 * available counter, otherwise increment counter and return
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param freeCounterBean
	 *            - Bean corresponding to os_free_counter_tbl
	 * @return - generated resource-id, -1 is error occurred
	 * @throws SQLException
	 */
	public int getResourceCounter(Connection connection,
			FreeCounterBean freeCounterBean) throws SQLException {
		LOG.trace("Start ResourceIdManager#getResourceId()");
		int resourceCounter = -1;
		/*
		 * Check if any resource counter is available that can be used
		 */
		final FreeCounterDao freeCounterDao = new FreeCounterDao();
		List<Integer> idList = freeCounterDao
				.getCounterList(connection, freeCounterBean);
		if (!idList.isEmpty()) {
			if (freeCounterBean.getResourceId().equals(VtnServiceOpenStackConsts.TENANT_RES_ID)) {
				boolean isFind = false;
				int maxId = AutoIdManager.getInstance().getMaxId();
				LOG.debug("getResourceCounter()max id is " + maxId);
				for (int index = 0; index < idList.size(); index++) {
					resourceCounter = idList.get(index).intValue();
					if (resourceCounter > maxId) {
						LOG.info("getResourceCounter()used free id is " + resourceCounter);
						isFind = true;
						break;
					}
				}
				if (!isFind) {
					resourceCounter = -1;
				}
			} else {
				resourceCounter = idList.get(0).intValue();
			}
		}

		freeCounterBean.setResourceCounter(resourceCounter);

		/*
		 * resource counter is available in free resource counter pool then use
		 * the same after deleting that from free resource counter pool. On the
		 * other hand increment counter and return.
		 */
		if (resourceCounter != -1) {
			if (freeCounterDao.deleteCounter(connection, freeCounterBean) != 1) {
				LOG.error("Error in deletion of resource counter from os_free_counter_tbl.");
				resourceCounter = -1;
			}
		} else {
			LOG.info("Resource counter is not available in os_free_counter_tbl.");

			LOG.debug("Resource counter required to be generated for  : "
					+ freeCounterBean.getResourceId() + " and vtn_name : "
					+ freeCounterBean.getVtnName());

			if (freeCounterBean.getResourceId().equalsIgnoreCase(
					VtnServiceOpenStackConsts.TENANT_RES_ID)) {
				LOG.debug("Resource generation for VTN.");
				final VtnDao vtnDao = new VtnDao();
				resourceCounter = vtnDao.getNextId(connection);
			} else if (freeCounterBean.getResourceId().equalsIgnoreCase(
					VtnServiceOpenStackConsts.NETWORK_RES_ID)) {
				LOG.debug("Resource generation for vBridge.");
				final VBridgeDao vBridgeDao = new VBridgeDao();
				resourceCounter = vBridgeDao.getNextId(connection,
						freeCounterBean.getVtnName());
			} else if (freeCounterBean.getResourceId().equalsIgnoreCase(
					VtnServiceOpenStackConsts.PORT_RES_ID)) {
				LOG.debug("Resource generation for port/router interface.");
				final VBridgeInterfaceDao vbrInterfaceDao = new VBridgeInterfaceDao();
				int portResourceCounter = vbrInterfaceDao.getNextId(connection,
						freeCounterBean.getVtnName());
				final VRouterInterfaceDao vrtInterfaceDao = new VRouterInterfaceDao();
				int interfaceResourceCounter = vrtInterfaceDao.getNextId(
						connection, freeCounterBean.getVtnName());
				resourceCounter = portResourceCounter
						+ interfaceResourceCounter - 1;
			} else if (freeCounterBean.getResourceId().equalsIgnoreCase(
					VtnServiceOpenStackConsts.FILTER_RES_ID)) {
				final FlowListDao flowListDao = new FlowListDao();
				int flowFilterCounter = flowListDao.getNextId(connection);
				resourceCounter = flowFilterCounter;
			}
		}
		LOG.info("Resource counter that will be used : " + resourceCounter);
		LOG.trace("Complete ResourceIdManager#getResourceId()");
		return resourceCounter;
	}

	/**
	 * Delete resource information from corresponding database table and update
	 * free resource counter pool, if resource was auto-generated
	 * 
	 * @param connection
	 *            - DB Connection instance
	 * @param freeCounterBean
	 *            - Bean corresponding to os_free_counter_tbl
	 * @param resourceBean
	 *            - Bean corresponding to resource specific table
	 * @return - true is operation executed successfully
	 * @throws SQLException
	 */
	public boolean deleteResourceId(Connection connection,
			FreeCounterBean freeCounterBean, Object resourceBean)
			throws SQLException {
		LOG.trace("Start ResourceIdManager#deleteResourceId()");

		List<Integer> ifIds = null;
		boolean result = false;
		int deletionStatus = -1;
		CommonDao comDao = new CommonDao();

		if (resourceBean instanceof VtnBean) {
			final VtnDao vtnDao = new VtnDao();
			final VtnBean vtnBean = (VtnBean) resourceBean;
			boolean isAuto = comDao.isAutoResourceName(connection, vtnBean);
			deletionStatus = vtnDao.delete(connection, (VtnBean) resourceBean);
			if ((false == isAuto) && comDao.isMaxId(connection, vtnBean)) {
				freeCounterBean.setResourceCounter(0);
			}
		} else if (resourceBean instanceof VBridgeBean) {
			final VBridgeBean vbrBean = (VBridgeBean) resourceBean;
			boolean isAuto = comDao.isAutoResourceName(connection, vbrBean);
			final VBridgeInterfaceDao vrtInterfaceDao = new VBridgeInterfaceDao();
			VBridgeInterfaceBean vInterfaceBean = new VBridgeInterfaceBean();
			vInterfaceBean
					.setVtnName(((VBridgeBean) resourceBean).getVtnName());
			vInterfaceBean
					.setVbrName(((VBridgeBean) resourceBean).getVbrName());
			ifIds = vrtInterfaceDao.getVbrIfIds(connection, vInterfaceBean);
			final VBridgeDao vBridgeDao = new VBridgeDao();
			deletionStatus = vBridgeDao.delete(connection,
					(VBridgeBean) resourceBean);
			if ((false == isAuto) && comDao.isMaxId(connection, vbrBean)) {
				freeCounterBean.setResourceCounter(0);
			}
		} else if (resourceBean instanceof VRouterBean) {
			final VRouterInterfaceDao vrtInterfaceDao = new VRouterInterfaceDao();
			VRouterInterfaceBean vInterfaceBean = new VRouterInterfaceBean();
			vInterfaceBean.setVtnName(freeCounterBean.getVtnName());
			vInterfaceBean
					.setVrtName(((VRouterBean) resourceBean).getVrtName());
			ifIds = vrtInterfaceDao.getVrtIfIds(connection, vInterfaceBean);
			final VRouterDao vRouterDao = new VRouterDao();
			deletionStatus = vRouterDao.delete(connection,
					(VRouterBean) resourceBean);
		} else if (resourceBean instanceof VRouterInterfaceBean) {
			final VRouterInterfaceDao vInterfaceDao = new VRouterInterfaceDao();
			deletionStatus = vInterfaceDao.delete(connection,
					(VRouterInterfaceBean) resourceBean);
		} else if (resourceBean instanceof VBridgeInterfaceBean) {
			final VBridgeInterfaceDao vInterfaceDao = new VBridgeInterfaceDao();
			final VBridgeInterfaceBean vbrIfBean = (VBridgeInterfaceBean) resourceBean;
			boolean isAuto = comDao.isAutoResourceName(connection, vbrIfBean);
			deletionStatus = vInterfaceDao.delete(connection,
					(VBridgeInterfaceBean) resourceBean);
			if ((false == isAuto) && comDao.isMaxId(connection, vbrIfBean)) {
				freeCounterBean.setResourceCounter(0);
			}
		} else if (resourceBean instanceof StaticRouteBean) {
			final StaticRouteDao staticRouteDao = new StaticRouteDao();
			deletionStatus = staticRouteDao.delete(connection,
					(StaticRouteBean) resourceBean);
		} else if (resourceBean instanceof FlowListBean) {
			final FlowListDao flowListDao = new FlowListDao();
			final FlowListBean flowListBean = (FlowListBean) resourceBean;
			boolean isAuto = flowListDao.isAutoFlowListName(connection,
					flowListBean.getFlName());
			deletionStatus = flowListDao.delete(connection, flowListBean);
			if (!(true == isAuto && !flowListDao.isFlowListIdFound(connection,
					freeCounterBean.getResourceCounter()))) {
				freeCounterBean.setResourceCounter(0);

			}
		}

		if (deletionStatus == VtnServiceOpenStackConsts.SUCCESS) {
			result = manageCounter(connection, freeCounterBean, resourceBean,
					ifIds);
		}

		LOG.debug("Status of operation : " + result);
		LOG.trace("Complete ResourceIdManager#deleteResourceId()");
		return result;
	}

	/**
	 * Manages the counters for all kind of resources available for OpenStack
	 * operations
	 * 
	 * @param connection
	 * @param freeCounterBean
	 * @param resourceBean
	 * @param ifIds2
	 * @return
	 * @throws SQLException
	 */
	private boolean manageCounter(Connection connection,
			FreeCounterBean freeCounterBean, Object resourceBean,
			List<Integer> ifIds) throws SQLException {
		boolean result = false;
		final FreeCounterDao freeCounterDao = new FreeCounterDao();

		/*
		 * delete child counters in case of parent entity is deleted
		 */
		if (freeCounterBean.getResourceId().equalsIgnoreCase(
				VtnServiceOpenStackConsts.TENANT_RES_ID)) {
			FreeCounterBean localBean = new FreeCounterBean();
			localBean.setVtnName(((VtnBean) resourceBean).getVtnName());
			freeCounterDao.deleteVtnChilds(connection, localBean);
		} else if (freeCounterBean.getResourceId().equalsIgnoreCase(
				VtnServiceOpenStackConsts.NETWORK_RES_ID)
				|| freeCounterBean.getResourceId().equalsIgnoreCase(
						VtnServiceOpenStackConsts.ROUTER_RES_ID)) {
			if (ifIds.size() > 0) {
				// insert unused counter into os_free_resource_counter table
				FreeCounterBean localBean = new FreeCounterBean();
				localBean.setResourceId(VtnServiceOpenStackConsts.PORT);
				localBean.setVtnName(freeCounterBean.getVtnName());
				for (int i = 0; i < ifIds.size(); i++) {
					localBean.setResourceCounter(ifIds.get(i));
					freeCounterDao.insertCounter(connection, localBean);
				}
			}
		}

		/*
		 * if resource counter is 0, then no need to insert resource counter in
		 * pool
		 */
		if (freeCounterBean.getResourceCounter() == 0) {
			LOG.debug("Resource counter is not auto-generated, insertion not required in os_res_counter table.");
			result = true;
		} else {
			LOG.debug("Resource is auto-generated, insertion required in os_res_counter table.");
			result = freeCounterDao.insertCounter(connection, freeCounterBean) == 1;
		}
		return result;
	}
}
