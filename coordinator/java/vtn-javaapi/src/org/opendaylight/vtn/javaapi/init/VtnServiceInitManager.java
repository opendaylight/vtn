/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.init;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.connection.IpcConnPool;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.exception.VtnServiceExceptionHandler;
import org.opendaylight.vtn.javaapi.exception.VtnServiceInitFailException;
import org.opendaylight.vtn.javaapi.ipc.enums.IpcRequestPacketEnum;
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

	private static List<String> readAsList = new ArrayList<String>();
	private static List<String> multiCallList = new ArrayList<String>();
	
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
			// add api names with READ as List support
			setReadAsList();
		} catch (final VtnServiceException e) {
			LOG.error(e, "VtnService Initialization Failed : " + e);
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
	
	/**
	 * Get list of API names where List is created for corresponding IPC READ
	 * operations
	 */
	private static void setReadAsList() {
		readAsList
				.add(IpcRequestPacketEnum.KT_VTNSTATION_CONTROLLER_GET.name());
		readAsList
		.add(IpcRequestPacketEnum.KT_VTNSTATION_CONTROLLER_GET_COUNT.name());
		readAsList.add(IpcRequestPacketEnum.KT_VTN_DATAFLOW_GET.name());
		readAsList.add(IpcRequestPacketEnum.KT_DATAFLOW_GET.name());
		readAsList.add(IpcRequestPacketEnum.KT_CTR_DATAFLOW_GET.name());
		readAsList.add(IpcRequestPacketEnum.KT_VBRIDGE_L2Domain.name());
		readAsList.add(IpcRequestPacketEnum.KT_VBRIDGE_MAC_ENTRY_GET.name());
		readAsList.add(IpcRequestPacketEnum.KT_VROUTER_ARP_ENTRY.name());
		readAsList.add(IpcRequestPacketEnum.KT_VROUTER_ARP_ENTRY_COUNT.name());
		readAsList.add(IpcRequestPacketEnum.KT_VROUTER_IPROUTE_GET.name());
		readAsList.add(IpcRequestPacketEnum.KT_VTEP_GRP_MEMBER_GET.name());
		readAsList.add(IpcRequestPacketEnum.KT_PORT_GET_MEMBER.name());
		readAsList.add(IpcRequestPacketEnum.KT_LOGICAL_PORT_MEMBER_GET.name());
				
		// set multi-call list where NO_SUCH_INSTANCE should be treated as
		// SIUCESS for second call READ
		multiCallList.add(IpcRequestPacketEnum.KT_VRT_IF_GET.name());
		multiCallList.add(IpcRequestPacketEnum.KT_VBR_IF_GET.name());
		multiCallList.add(IpcRequestPacketEnum.KT_VUNK_IF_GET.name());
		multiCallList.add(IpcRequestPacketEnum.KT_VTEP_IF_GET.name());
		multiCallList.add(IpcRequestPacketEnum.KT_VTUNNEL_IF_GET.name());
	}

	/**
	 * Getter for readAsList
	 * 
	 * @return
	 */
	public static List<String> getReadAsList() {
		return readAsList;
	}
	
	/**
	 * Getter for multiCallList
	 * 
	 * @return
	 */
	public static List<String> getMultiCallList() {
		return multiCallList;
	}

}
