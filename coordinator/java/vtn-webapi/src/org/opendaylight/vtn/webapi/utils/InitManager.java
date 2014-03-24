/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.webapi.utils;

import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.init.VtnServiceInitManager;
import org.opendaylight.vtn.webapi.exception.VtnServiceWebAPIException;

/**
 * The Class InitManager.
 */
public final class InitManager {

	/** The Constant LOG. */
	private static final Logger LOG = Logger.getLogger(InitManager.class
			.getName());
	/** The init manager. */
	private static InitManager initMngr;

	/**
	 * Instantiates a new inits the manager.
	 * 
	 * @throws VtnServiceWebAPIException
	 *             the vtn service exception
	 */
	private InitManager() throws VtnServiceWebAPIException {
		LOG.trace("Start InitManager#InitManager()");
		initializeConfiguration();
		initializeJavaAPI();
		LOG.trace("Complete InitManager#InitManager()");
	}

	/**
	 * Initialize.
	 * 
	 * @return the inits the manager
	 * @throws VtnServiceWebAPIException
	 *             the vtn service exception
	 */
	public synchronized static InitManager initialize()
			throws VtnServiceWebAPIException {
		LOG.trace("Start InitManager#initialize()");
		if (initMngr == null) {
			initMngr = new InitManager();
		}
		LOG.trace("Complete InitManager#initialize()");
		return initMngr;
	}

	/**
	 * Initialize java api.
	 */
	private void initializeJavaAPI() {
		LOG.trace("Start InitManager#initializeJavaAPI()");
		VtnServiceInitManager.init();
		LOG.trace("Complete InitManager#initializeJavaAPI()");
	}

	/**
	 * Initialize configuration.
	 * 
	 * @throws VtnServiceWebAPIException
	 *             the vtn service web api exception
	 */
	private void initializeConfiguration() throws VtnServiceWebAPIException {
		LOG.trace("Start InitManager#initializeConfiguration()");
		ConfigurationManager.initConfiguration();
		LOG.trace("Complete InitManager#initializeConfiguration()");
	}

	/**
	 * This method will free the memory which is used by init manager instance
	 */
	@Override
	public void finalize() {
		LOG.trace("Start InitManager#finalize()");
		initMngr = null;
		LOG.trace("Complete InitManager#finalize()");
	}
}
