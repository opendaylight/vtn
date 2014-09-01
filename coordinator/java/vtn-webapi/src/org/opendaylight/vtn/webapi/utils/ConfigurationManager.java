/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.webapi.utils;

import java.io.IOException;
import java.util.Properties;

import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.webapi.constants.ApplicationConstants;
import org.opendaylight.vtn.webapi.enums.HttpErrorCodeEnum;
import org.opendaylight.vtn.webapi.exception.VtnServiceWebAPIException;

/**
 * The Class ConfigurationManager.
 * 
 * This class is used to initialize and Read/Write properties in the config
 * file. Values in config file are stored as key-value pairs.
 */
public final class ConfigurationManager {

	/** The Constant LOG. */
	private static final Logger LOG = Logger
			.getLogger(ConfigurationManager.class.getName());
	/** The conf manager. */
	private static ConfigurationManager confManager;

	/** The access configuration. */
	private static Properties accessConfiguration;

	/** The web api configuration. */
	private static Properties webAPIConfiguration;

	/**
	 * Instantiates a new configuration manager.
	 * 
	 * @throws VtnServiceWebAPIException
	 *             the vtn service exception
	 */
	private ConfigurationManager() throws VtnServiceWebAPIException {
		LOG.trace("Start ConfigurationManager#ConfigurationManager()");
		initConfiguration();
		LOG.trace("Complete ConfigurationManager#ConfigurationManager()");
	}

	/**
	 * Gets the single instance of ConfigurationManager.
	 * 
	 * @return single instance of ConfigurationManager
	 * @throws VtnServiceWebAPIException
	 *             the vtn service exception
	 */
	public synchronized static ConfigurationManager getInstance()
			throws VtnServiceWebAPIException {
		LOG.trace("Start ConfigurationManager#getInstance()");
		if (null == confManager) {
			confManager = new ConfigurationManager();
		}
		LOG.trace("Complete ConfigurationManager#getInstance()");
		return confManager;
	}

	/**
	 * Initialize the configuration part and will store the same in
	 * configuration objects
	 * 
	 * @throws VtnServiceWebAPIException
	 *             the vtn service exception
	 */
	public static void initConfiguration() throws VtnServiceWebAPIException {
		LOG.trace("Start ConfigurationManager#initConfiguration()");
		try {
			accessConfiguration = new Properties();
			accessConfiguration.load(Thread
					.currentThread()
					.getContextClassLoader()
					.getResourceAsStream(
							ApplicationConstants.ACCESS_PROPERTY_PATH));

			webAPIConfiguration = new Properties();
			webAPIConfiguration.load(Thread
					.currentThread()
					.getContextClassLoader()
					.getResourceAsStream(
							ApplicationConstants.WEBAPI_CONF_PROPERTY_PATH));
		} catch (final IOException exception) {
			LOG.error(exception, "VTN Service configuration initialization error : "
					+ exception.getMessage());
			throw new VtnServiceWebAPIException(
					HttpErrorCodeEnum.UNC_INTERNAL_SERVER_ERROR.getCode());
		}
		LOG.trace("Complete ConfigurationManager#initConfiguration()");
	}

	/**
	 * This method is used to read properties file and returns value based on
	 * key.
	 * 
	 * @param key
	 *            - Represents the key for which value to be retrieved.
	 * @return value - Represents corresponding value fetched from config file.
	 * @throws VtnServiceWebAPIException
	 *             the vtn service exception
	 */
	public String getAccessProperty(final String key)
			throws VtnServiceWebAPIException {
		LOG.trace("Start ConfigurationManager#getAccessProperty()");
		final String value = accessConfiguration.getProperty(key);
		// Check if value retrieved is null
		if (null == value || value.isEmpty()) {
			LOG.error("Configuration value not found for Key : " + key);
			throw new VtnServiceWebAPIException(
					HttpErrorCodeEnum.UNC_INTERNAL_SERVER_ERROR.getCode());
		}
		LOG.debug("Key : " + key + " Value : " + value);
		LOG.trace("Complete ConfigurationManager#getAccessProperty()");
		return value;
	}

	/**
	 * Gets the conf property from webAPIConfiguration object , This object will
	 * store the configuration related to Web API flow only.
	 * 
	 * @param key
	 *            the key
	 * @return the conf property
	 * @throws VtnServiceWebAPIException
	 *             the vtn service web api exception
	 */
	public String getConfProperty(final String key)
			throws VtnServiceWebAPIException {
		LOG.trace("Start ConfigurationManager#getConfProperty()");
		final String value = webAPIConfiguration.getProperty(key);
		// Check if value retrieved is null
		if (null == value) {
			throw new VtnServiceWebAPIException(
					HttpErrorCodeEnum.UNC_INTERNAL_SERVER_ERROR.getCode());
		}
		LOG.trace("Complete ConfigurationManager#getConfProperty()");
		return value;
	}

	/**
	 * This method will destroy the objects which are still opened or live in
	 * the memory to flush out the same
	 */
	@Override
	public void finalize() {
		LOG.trace("Start ConfigurationManager#finalize()");
		confManager = null;
		accessConfiguration = null;
		webAPIConfiguration = null;
		LOG.trace("Complete ConfigurationManager#finalize()");
	}
}
