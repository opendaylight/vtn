/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.init;

import java.io.IOException;
import java.util.Properties;

import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.util.VtnServiceUtil;

/**
 * The Class VtnServiceConfiguration. Initialize and provide the utility
 * interface to get the properties
 */
public final class VtnServiceConfiguration {

	private static final Logger LOG = Logger
			.getLogger(VtnServiceConfiguration.class.getName());

	private static final Properties commonConfigProperties = new Properties();
	private Properties appConfigProperties = null;
	private Properties mapModeConfigProperties = null;

	static {
		try {
			commonConfigProperties
					.load(Thread
							.currentThread()
							.getContextClassLoader()
							.getResourceAsStream(
									VtnServiceConsts.COMMON_CONF_FILEPATH));
		} catch (final IOException e) {
			LOG.error(e, "Error in loading common properties " + e);
		}
	}

	/**
	 * Instantiates a new vtn service configuration.
	 * 
	 * @throws VtnServiceException
	 */
	public VtnServiceConfiguration() throws VtnServiceException {
		LOG.trace("Start VtnServiceConfiguration#VtnServiceConfiguration()");

		appConfigProperties = new Properties();

		mapModeConfigProperties = new Properties();

		init();

		LOG.trace("Complete VtnServiceConfiguration#VtnServiceConfiguration()");
	}

	/**
	 * Initialize the configurations for JavaAPI
	 * 
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private void init() throws VtnServiceException {
		LOG.trace("Start VtnServiceConfiguration#init()");
		try {
			// read and load static properties
			synchronized (appConfigProperties) {
				appConfigProperties
						.load(Thread
								.currentThread()
								.getContextClassLoader()
								.getResourceAsStream(
										VtnServiceConsts.APP_CONF_FILEPATH));
			}

			if (Thread.currentThread().getContextClassLoader().toString()
					.contains(VtnServiceOpenStackConsts.VTN_WEB_API_ROOT)) {
				synchronized (mapModeConfigProperties) {
					mapModeConfigProperties.load(Thread
							.currentThread()
							.getContextClassLoader()
							.getResourceAsStream(
									VtnServiceConsts.MAPMODE_CONF_FILEPATH));
				}
			}			

		} catch (final IOException e) {
			VtnServiceInitManager
					.getExceptionHandler()
					.raise(Thread.currentThread().getStackTrace()[1]
							.getClassName()
							+ VtnServiceConsts.HYPHEN
							+ Thread.currentThread().getStackTrace()[1]
									.getMethodName(),
							UncJavaAPIErrorCode.APP_CONFIG_ERROR.getErrorCode(),
							UncJavaAPIErrorCode.APP_CONFIG_ERROR
									.getErrorMessage(), e);
		}
		LOG.trace("Complete VtnServiceConfiguration#init()");
	}

	/**
	 * Gets the config value for application properties
	 * 
	 * @param key
	 *            the key
	 * @return the config value
	 */
	public String getConfigValue(final String key) {
		LOG.trace("Return from VtnServiceConfiguration#getConfigValue()");
		String configValue = VtnServiceConsts.EMPTY_STRING;
		if (appConfigProperties != null) {
			configValue = VtnServiceUtil.isValidString(key) ? appConfigProperties
					.get(key).toString() : VtnServiceConsts.EMPTY_STRING;
		}
		LOG.debug(key + VtnServiceConsts.COLON + configValue);
		return configValue;
	}

	/**
	 * Gets the config value for common properties
	 * 
	 * @param key
	 *            the key
	 * @return the config value
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	public String getCommonConfigValue(final String key) {
		LOG.trace("Return from VtnServiceConfiguration#getCommonConfigValue()");
		String configValue = VtnServiceConsts.EMPTY_STRING;
		if (commonConfigProperties != null) {
			configValue = VtnServiceUtil.isValidString(key) ? commonConfigProperties
					.get(key).toString() : VtnServiceConsts.EMPTY_STRING;
		}
		LOG.debug(key + VtnServiceConsts.COLON + configValue);
		return configValue;
	}

	/**
	 * Get map-mode property value for vlan-map creation corresponding to
	 * OpenStack's Port operation
	 * 
	 * @param key
	 * @return
	 */
	public String getMapModeValue() {
		LOG.trace("Return from VtnServiceConfiguration#getMapModeValue()");
		// initialize with 0 as default
		String configValue = VtnServiceConsts.ZERO;
		if (mapModeConfigProperties != null) {
			configValue = mapModeConfigProperties.get(
					VtnServiceOpenStackConsts.VLANMAP_MODE).toString();
		}
		LOG.debug("Map Mode " + VtnServiceConsts.COLON + configValue);
		return configValue;
	}
}
