/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.resources.openstack;

import java.sql.Connection;
import java.sql.SQLException;
import java.util.*;
import java.lang.Integer;

import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.init.VtnServiceInitManager;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.openstack.dbmanager.ResourceIdManager;
import org.opendaylight.vtn.javaapi.openstack.beans.FreeCounterBean;

public final class AutoIdManager {

	/** The Constant LOG. */
	private static final Logger LOG = Logger.getLogger(
										AutoIdManager.class.getName());
	/** The id manager. */
	private static AutoIdManager idManager;

	/**the id map*/
	private Map<String, Integer> idMap = null;

	/**
	 * Instantiates a new id manager.
	 */
	private AutoIdManager() {
		LOG.trace("Start AutoIdManager#AutoIdManager()");
		idMap = new HashMap<String, Integer>();
		idMap.clear();
		LOG.trace("Complete AutoIdManager#AutoIdManager()");
	}

	/**
	 * Gets the single instance of AutoIdManager.
	 * 
	 * @return single instance of AutoIdManager
	 */
	public static AutoIdManager getInstance() {
		LOG.trace("Start AutoIdManager#getInstance()");
		if (null == idManager) {
			idManager = new AutoIdManager();
		}
		LOG.trace("Complete AutoIdManager#getInstance()");
		return idManager;
	}

	/**
	 * insert the key and value to idMap.
	 */
	private void insert(String autoName, int id) {
		idMap.put(autoName, id);
	}

	/**
	 * delete the key and value from idMap.
	 */
	public synchronized void delete(String autoName) {
		if (autoName != null) {
			idMap.remove(autoName);
		}
	}

	/**
	 * get the max id from idMap's values.
	 */
	public synchronized int getMaxId() {
		int id = 0;
		Iterator<Map.Entry<String, Integer>> iter = idMap.entrySet().iterator();
		while(iter.hasNext()) {
			Map.Entry<String, Integer> entry = iter.next();
			Integer value = entry.getValue();
			if (value.intValue() > id) {
				id = value.intValue();
			}
		}
		return id;
	}

	/**
	 * find the specified key in idMap.
	 */
	public boolean find(String autoName) {
		boolean isFind = false;
		if (null != idMap.get(autoName)) {
			isFind = true;
		}
		return isFind;
	}

	/**
	 *get the auto tenant name 
	 * @param none
	 * @return
	 */
	public synchronized String getAutoTenantName() 
						throws SQLException{
		String autoName = null;
		int counter = -1;
		Connection connection = null;
		LOG.trace("start AutoIdManager#getAutoTenantName()");

		try {
			connection = VtnServiceInitManager.getDbConnectionPoolMap()
					.getConnection();
			final ResourceIdManager resourceIdManager = new ResourceIdManager();
			final FreeCounterBean freeCounterBean = new FreeCounterBean();
			freeCounterBean
					.setResourceId(VtnServiceOpenStackConsts.TENANT_RES_ID);
			freeCounterBean
					.setVtnName(VtnServiceOpenStackConsts.DEFAULT_VTN);
			counter = resourceIdManager.getResourceCounter(connection,
					freeCounterBean);
		} catch (final SQLException exception) {
			LOG.error("AutoIdManager#getAutoTenantName():Internal server error:" + exception);
			throw exception;
		} finally {
			if (null != connection) {
				LOG.info("Free connection...");
				VtnServiceInitManager.getDbConnectionPoolMap()
							.freeConnection(connection);
			}

			if (counter != -1) {
				autoName = VtnServiceOpenStackConsts.VTN_PREFIX + counter;
				insert(autoName, counter);
			}
		}

		LOG.trace("Complete AutoIdManager#getAutoTenantName()");
		return autoName;
	}

	/**
	 * This method will destroy the objects which are still opened or live in
	 * the memory to flush out the same
	 */
	@Override
	public void finalize() {
		LOG.trace("Start AutoIdManager#finalize()");
		idMap.clear();
		idMap = null;
		idManager = null;
		LOG.trace("Complete AutoIdManager#finalize()");
	}
}
