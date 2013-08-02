/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.init;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Properties;
import java.util.Scanner;

import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.util.OrderedProperties;
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
	private OrderedProperties physicalErrorProperties = null;
	private OrderedProperties logicalErrorProperties = null;

	static {
		try {
			commonConfigProperties
					.load(Thread
							.currentThread()
							.getContextClassLoader()
							.getResourceAsStream(
									VtnServiceConsts.COMMON_CONF_FILEPATH));
		} catch (final IOException e) {
			LOG.error("Error in loading common properties " + e);
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

		physicalErrorProperties = new OrderedProperties();

		logicalErrorProperties = new OrderedProperties();

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
			loadErrorCodeProps();
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
	 * Load Physical and Logical Error Code properties in the same order as
	 * given in the properties file
	 * 
	 * @throws FileNotFoundException
	 */
	private void loadErrorCodeProps() {
		/*
		 * load logical error code and messages from properties files
		 */
		if (logicalErrorProperties != null) {
			synchronized (logicalErrorProperties) {
				final List<String> valueSet = new ArrayList<String>();
				final List<String> keySet = new ArrayList<String>();
				final Scanner scanner = new Scanner(Thread
						.currentThread()
						.getContextClassLoader()
						.getResourceAsStream(
								(VtnServiceConsts.UPLL_ERRORS_FILEPATH)));
				Scanner lineParser = null;

				/*
				 * read properties one by one in the given order from properties
				 * file
				 */
				while (scanner.hasNextLine()) {
					final String line = scanner.nextLine();
					lineParser = new Scanner(line);
					lineParser.useDelimiter(VtnServiceConsts.EQUAL);
					while (lineParser.hasNext()) {
						keySet.add(lineParser.next());
						valueSet.add(lineParser.next());
					}
				}

				logicalErrorProperties.setKeySet(keySet);
				logicalErrorProperties.setValueSet(valueSet);
			}
		}

		/*
		 * load physical error code and messages from properties files
		 */
		if (physicalErrorProperties != null) {
			synchronized (physicalErrorProperties) {
				final List<String> valueSet = new ArrayList<String>();
				final List<String> keySet = new ArrayList<String>();
				final Scanner scanner = new Scanner(Thread
						.currentThread()
						.getContextClassLoader()
						.getResourceAsStream(
								(VtnServiceConsts.UPPL_ERRORS_FILEPATH)));
				Scanner lineParser = null;

				/*
				 * read properties one by one in the given order from properties
				 * file
				 */
				while (scanner.hasNextLine()) {
					final String line = scanner.nextLine();
					lineParser = new Scanner(line);
					lineParser.useDelimiter(VtnServiceConsts.EQUAL);
					while (lineParser.hasNext()) {
						keySet.add(lineParser.next());
						valueSet.add(lineParser.next());
					}
				}

				physicalErrorProperties.setKeySet(keySet);
				physicalErrorProperties.setValueSet(valueSet);
			}
		}
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
	 * Getter for Logical Properties File
	 * 
	 * @return
	 */
	public OrderedProperties getLogicalErrorProperties() {
		return logicalErrorProperties;
	}

	/**
	 * Getter for Physical Properties File
	 * 
	 * @return
	 */
	public OrderedProperties getPhysicalErrorProperties() {
		return physicalErrorProperties;
	}
}
