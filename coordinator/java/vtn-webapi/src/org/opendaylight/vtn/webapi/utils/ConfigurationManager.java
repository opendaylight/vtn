/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.webapi.utils;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Properties;

import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.webapi.enums.ApplicationConstants;
import org.opendaylight.vtn.webapi.exception.VtnServiceWebAPIException;


/**
 * The Class ConfigurationManager.
 *
 * This class is used to initialize and Read/Write properties in the config file.
 * Values in config file are stored as key-value pairs.
 */
public final class ConfigurationManager {
	
	/** The Constant LOG. */
	private static final Logger LOG = Logger.getLogger(ConfigurationManager.class.getName());
	/** The conf manager. */
	private static ConfigurationManager confManager;
	
	/** The access configuration. */
	private static Properties accessConfiguration;
	
	/** The pwd configuration. */
	private static Properties pwdConfiguration;

	/** The web api configuration. */
	private static Properties webAPIConfiguration;
	/**
	 * Instantiates a new configuration manager.
	 *
	 * @throws VtnServiceWebAPIException the vtn service exception
	 */
	private ConfigurationManager() throws VtnServiceWebAPIException {
		LOG.trace("Initialising Configuration Manager");
		initConfiguration();
		LOG.trace("Configuration Manager initialised");
	}
	
	/**
	 * Gets the single instance of ConfigurationManager.
	 *
	 * @return single instance of ConfigurationManager
	 * @throws VtnServiceWebAPIException the vtn service exception
	 */
	public synchronized static ConfigurationManager getInstance() throws VtnServiceWebAPIException {
		LOG.trace("Getting instance of configuration manager");
		if(null == confManager){
			confManager = new ConfigurationManager();
		}	
		LOG.trace("Returning instance of configuration manager");
		return confManager;
	}
	
	/**
	 * Initialize the configuration part and will store the same in configuration objects
	 * @throws VtnServiceWebAPIException the vtn service exception
	 */
	public static void initConfiguration() throws VtnServiceWebAPIException{
		LOG.trace("Initilising configuration properties..");
		try{
			accessConfiguration = new Properties();
			accessConfiguration.load(Thread.currentThread().getContextClassLoader().getResourceAsStream(ApplicationConstants.ACCESS_PROPERTY_PATH));
			
			pwdConfiguration = new Properties();
			pwdConfiguration.load(Thread.currentThread().getContextClassLoader().getResourceAsStream(ApplicationConstants.PWD_PROPERTY_PATH));
			
			webAPIConfiguration = new Properties();
			webAPIConfiguration.load(Thread.currentThread().getContextClassLoader().getResourceAsStream(ApplicationConstants.WEBAPI_CONF_PROPERTY_PATH));
		LOG.trace("configuration properties initialised successfully.");
		}catch (FileNotFoundException exception) {
			LOG.error(exception.getMessage());
			throw new VtnServiceWebAPIException(ApplicationConstants.PROPERTY_FILE_READ_ERROR, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.PROPERTY_FILE_READ_ERROR)); 
		}catch (IOException exception) {
			LOG.error(exception.getMessage());
			throw new VtnServiceWebAPIException(ApplicationConstants.PROPERTY_FILE_READ_ERROR, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.PROPERTY_FILE_READ_ERROR)); 
		}
	}
	
    /**
     * This method is used to read properties file and returns value based on key.
     *
     * @param key - Represents the key for which value to be retrieved.
     * @return value -  Represents corresponding value fetched from config file.
     * @throws VtnServiceWebAPIException the vtn service exception
     */
	public String getAccessProperty(final String key) throws VtnServiceWebAPIException {
		LOG.trace("getAccessProperty start # getAccessProperty()");
		final String value  = accessConfiguration.getProperty(key);
		//Check if value retrieved is null
		if (null == value || value.isEmpty()) {
			throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.INTERNAL_SERVER_ERROR));
		}
		LOG.trace("getAccessProperty end # getAccessProperty()");
		return value;
	}
	
	/**
	 * Gets the pWD resource file property.
	 *
	 * @param key the key
	 * @return the pWD property
	 * @throws VtnServiceWebAPIException the vtn service exception
	 */
	public String getPWDProperty(final String key) throws VtnServiceWebAPIException {
		LOG.trace("getPWDProperty start # getPWDProperty()");
		final String value  = pwdConfiguration.getProperty(key);
		//Check if value retrieved is null
		if (null == value || value.isEmpty()) {
			LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.PROPERTY_NOT_FOUND_ERROR));
			throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.INTERNAL_SERVER_ERROR));
		}
		LOG.trace("getPWDProperty end # getPWDProperty()");
		return value;
	}
	
	/**
	 * Gets the conf property from webAPIConfiguration object , This object will store the configuration
	 * related to Web API flow only.
	 *
	 * @param key the key
	 * @return the conf property
	 * @throws VtnServiceWebAPIException the vtn service web api exception
	 */
	public String getConfProperty(final String key) throws VtnServiceWebAPIException {
		LOG.trace("getConfProperty start # getConfProperty()");
		final String value  = webAPIConfiguration.getProperty(key);
		//Check if value retrieved is null
		if (null == value || value.isEmpty()) {
			LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.PROPERTY_NOT_FOUND_ERROR));
			throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
		}
		LOG.trace("getConfProperty end # getConfProperty()");
		return value;
	}

	/**
	 * This method will destroy the objects which are still opened or live in the memory to flush out the same
	 */
	@Override
	public void finalize() {
		 confManager = null;
		 accessConfiguration =null;		
		 pwdConfiguration = null;
		 webAPIConfiguration = null;
	}
}
