/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.init;

import java.util.HashMap;
import java.util.Map;

import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.connection.IpcConnPool;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.exception.VtnServiceExceptionHandler;
import org.opendaylight.vtn.javaapi.exception.VtnServiceInitFailException;
import org.opendaylight.vtn.javaapi.ipc.enums.UncIpcErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.openstack.dbmanager.ConnectionProperties;
import org.opendaylight.vtn.javaapi.openstack.dbmanager.DataBaseConnectionPool;
import org.opendaylight.vtn.javaapi.util.VtnIniParser;

/**
 * The Class VtnServiceInitManager. Initializes the JavaAPI
 */
public final class VtnServiceInitManager {

	private static final Logger LOG = Logger
			.getLogger(VtnServiceInitManager.class.getName());

	private static final VtnServiceInitManager VTNS_INITMGR = new VtnServiceInitManager();
	private static VtnServiceExceptionHandler exceptionHandler = new VtnServiceExceptionHandler();

	private static Map<ClassLoader, VtnServiceConfiguration> configurationMap = new HashMap<ClassLoader, VtnServiceConfiguration>();
	private static Map<ClassLoader, IpcConnPool> ipcConnectionPoolMap = new HashMap<ClassLoader, IpcConnPool>();
	private static Map<ClassLoader, DataBaseConnectionPool> dbConnectionPoolMap = new HashMap<ClassLoader, DataBaseConnectionPool>();

	/**
	 * Instantiates a new vtn service init manager.
	 */
	private VtnServiceInitManager() {
		LOG.trace("Start VtnServiceInitManager#VtnServiceInitManager()");
		if (VTNS_INITMGR != null) {
			throw new IllegalStateException(
					VtnServiceConsts.SINGLETON_EXCEPTION);
		}
		LOG.trace("Complete VtnServiceInitManager#VtnServiceInitManager()");
	}

	/**
	 * Gets the instance.
	 * 
	 * @return the vtn service init manager
	 */
	public static VtnServiceInitManager getInstance() {
		LOG.trace("Return from VtnServiceInitManager#getInstance()");
		return VTNS_INITMGR;
	}

	/**
	 * Initialize the JavaAPI
	 */
	public static void init() {
		LOG.trace("Start VtnServiceInitManager#init()");
		try {
			ClassLoader currentContext = Thread.currentThread()
					.getContextClassLoader();
			LOG.debug("Initialization Application Code : " + currentContext);

			// load the configurations
			configurationMap.put(currentContext, new VtnServiceConfiguration());

			// initialize the error maps with loaded properties
			UncIpcErrorCode.initializeErrorsMessages();

			// initialize the connection pooling
			final IpcConnPool ipcConnPool = new IpcConnPool();
			ipcConnPool.init(exceptionHandler);
			ipcConnectionPoolMap.put(currentContext, ipcConnPool);

			if (currentContext.toString().contains(
					VtnServiceOpenStackConsts.VTN_WEB_API_ROOT)) {
				final ConnectionProperties connectionProperties = VtnIniParser
						.getInstance(
								VtnServiceInitManager
										.getConfigurationMap()
										.getConfigValue(
												VtnServiceOpenStackConsts.INI_FILE_PATH))
						.loadConnectionProperties();
				// initialize the database connection pooling
				final DataBaseConnectionPool dbConnPool = new DataBaseConnectionPool(
						connectionProperties, exceptionHandler);
				dbConnectionPoolMap.put(currentContext, dbConnPool);
			}

			// load the resources
			PackageScan.getInstance();
		} catch (final VtnServiceException e) {
			LOG.error("VtnService Initialization Failed : " + e);
			throw new VtnServiceInitFailException(
					UncJavaAPIErrorCode.INIT_ERROR.getErrorCode()
							+ VtnServiceConsts.COLON
							+ UncJavaAPIErrorCode.INIT_ERROR.getErrorMessage(),
					e);
		}
		LOG.trace("Complete VtnServiceInitManager#init()");
	}

	/**
	 * Destroy the JavaAPI
	 */
	public void destroy() {
		dbConnectionPoolMap.get(Thread.currentThread().getContextClassLoader())
				.closeAllConnections();
	}

	/**
	 * Get exception handler
	 * 
	 * @return ExceptionHandler
	 */
	public static VtnServiceExceptionHandler getExceptionHandler() {
		LOG.trace("Return from VtnServiceInitManager#getExceptionHandler()");
		return exceptionHandler;
	}

	public static VtnServiceConfiguration getConfigurationMap() {
		LOG.debug("Request Application Code : "
				+ Thread.currentThread().getContextClassLoader());
		return configurationMap.get(Thread.currentThread()
				.getContextClassLoader());
	}

	/**
	 * Get Connection Pool Map for IPC Connections
	 * @return
	 */
	public static IpcConnPool getConnectionPoolMap() {
		LOG.debug("Request Application Code : "
				+ Thread.currentThread().getContextClassLoader());
		return ipcConnectionPoolMap.get(Thread.currentThread()
				.getContextClassLoader());
	}

	/**
	 * Get Connection Pool Map for Database Connections
	 * @return
	 */
	public static DataBaseConnectionPool getDbConnectionPoolMap() {
		LOG.debug("Request Application Code : "
				+ Thread.currentThread().getContextClassLoader());
		return dbConnectionPoolMap.get(Thread.currentThread()
				.getContextClassLoader());
	}
}
