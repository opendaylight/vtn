/*
 * Copyright (c) 2012-2013 NEC Corporation
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

/**
 * The Class VtnServiceInitManager. Initializes the JavaAPI
 */
public final class VtnServiceInitManager {

	private static final Logger LOG = Logger
			.getLogger(VtnServiceInitManager.class.getName());

	private static final VtnServiceInitManager VTNS_INITMGR = new VtnServiceInitManager();
	private static VtnServiceExceptionHandler exceptionHandler = new VtnServiceExceptionHandler();

	private static Map<ClassLoader, VtnServiceConfiguration> configurationMap = new HashMap<ClassLoader, VtnServiceConfiguration>();
	private static Map<ClassLoader, IpcConnPool> connectionPoolMap = new HashMap<ClassLoader, IpcConnPool>();

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
			LOG.debug("Initialization Application Code : "
					+ Thread.currentThread().getContextClassLoader());

			// load the configurations
			configurationMap.put(
					Thread.currentThread().getContextClassLoader(),
					new VtnServiceConfiguration());

			// initialize the error maps with loaded properties
			UncIpcErrorCode.initializeErrorsMessages();

			// initialize the connection pooling
			final IpcConnPool ipcConnPool = new IpcConnPool();
			ipcConnPool.init(exceptionHandler);
			connectionPoolMap.put(Thread.currentThread()
					.getContextClassLoader(), ipcConnPool);

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

	public static IpcConnPool getConnectionPoolMap() {
		LOG.debug("Request Application Code : "
				+ Thread.currentThread().getContextClassLoader());
		return connectionPoolMap.get(Thread.currentThread()
				.getContextClassLoader());
	}
}
