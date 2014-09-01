/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.util;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.init.VtnServiceInitManager;
import org.opendaylight.vtn.javaapi.openstack.constants.VtnServiceOpenStackConsts;
import org.opendaylight.vtn.javaapi.openstack.dbmanager.ConnectionProperties;

public class VtnIniParser {

	/* Logger instance */
	private static final Logger LOG = Logger.getLogger(VtnIniParser.class
			.getName());

	private static VtnIniParser parser = null;
	private Pattern iniSectionPattern = Pattern
			.compile("\\s*\\[([^]]*)\\]\\s*");
	private Pattern iniKeys = Pattern.compile("\\s*([^=]*)=(.*)");
	private Map<String, Map<String, String>> iniEntriesMap = new HashMap<String, Map<String, String>>();

	/**
	 * Default server name.
	 */
	private static final String  DEFAULT_SERVER = "localhost";

	/**
	 * Constructor that initialize with loading of ini file
	 * 
	 * @param path
	 * @throws IOException
	 */
	public VtnIniParser(String path) throws IOException {
		load(path);
	}

	/**
	 * Get instance of parser
	 * 
	 * @param path
	 * @return
	 */
	public static VtnIniParser getInstance(String path) {
		try {
			if (parser == null) {
				parser = new VtnIniParser(path);
			}
		} catch (IOException exception) {
			LOG.error(exception, "ERROR Occurecd in the UNCINIParser#getInstance:- "
					+ exception.toString());
		}
		return parser;
	}

	/**
	 * Load parser for ini with specified file path
	 * 
	 * @param path
	 *            - ini file path
	 * @throws IOException
	 */
	public void load(String path) throws IOException {
		BufferedReader br = null;
		try {
			br = new BufferedReader(new FileReader(path));
			String row;
			String iniSectionStr = null;
			while ((row = br.readLine()) != null) {
				Matcher m = iniSectionPattern.matcher(row);
				if (m.matches()) {
					iniSectionStr = m.group(1).trim();
				} else if (iniSectionStr != null) {
					m = iniKeys.matcher(row);
					if (m.matches()) {
						String key = m.group(1).trim();
						String value = m.group(2).trim();
						Map<String, String> kv = iniEntriesMap
								.get(iniSectionStr);
						if (kv == null) {
							iniEntriesMap.put(iniSectionStr,
									kv = new HashMap<String, String>());
						}
						kv.put(key, value);
					}
				}
			}
		} catch (Exception e) {
			LOG.error(e, "ERROR Occurecd in the UNCINIParser#load:- "
					+ e.toString());
		} finally {
			if (br != null) {
				br.close();
			}
		}
	}

	/**
	 * Get string values for specified key
	 * 
	 * @param section
	 * @param key
	 * @return - string value corresponding to key
	 */
	public String getString(String section, String key) {
		Map<String, String> kv = iniEntriesMap.get(section);
		return kv.get(key);
	}

	/**
	 * Load Connection properties with INI file entries
	 * 
	 * @return
	 */
	public ConnectionProperties loadConnectionProperties() {
		final ConnectionProperties connectionProperties = new ConnectionProperties();

		Map<String, String> kv = iniEntriesMap
				.get(VtnServiceOpenStackConsts.UNC_DB_DSN);

		/**
		 * set database connection properties
		 */
		connectionProperties.setDbDriver(VtnServiceOpenStackConsts.DB_DRIVER);

		// Determine server name.
		// Use default server if the server name is a UNIX domain
		// socket path or undefined.
		String server = kv.get(VtnServiceOpenStackConsts.DB_IP);
		if (server == null || server.startsWith("/")) {
			server = DEFAULT_SERVER;
		}

		connectionProperties.setDbURL(VtnServiceOpenStackConsts.DB_URL_PREFIX
				+ server
				+ VtnServiceConsts.COLON
				+ kv.get(VtnServiceOpenStackConsts.DB_PORT)
				+ VtnServiceConsts.SLASH
				+ kv.get(VtnServiceOpenStackConsts.DB_NAME));

		connectionProperties.setDbPassword(kv
				.get(VtnServiceOpenStackConsts.DB_PASSWORD));

		connectionProperties.setDbUsername(kv
				.get(VtnServiceOpenStackConsts.DB_USER));

		/**
		 * set connection pool properties
		 */
		connectionProperties.setInitialConnections(Integer
				.parseInt(VtnServiceInitManager.getConfigurationMap()
						.getConfigValue(
								VtnServiceOpenStackConsts.DB_INIT_CONN_SIZE)));

		connectionProperties.setMaxPossibleConnections(Integer
				.parseInt(VtnServiceInitManager.getConfigurationMap()
						.getConfigValue(
								VtnServiceOpenStackConsts.DB_MAX_CONN_SIZE)));

		connectionProperties.setWaitforUsedConnections(Boolean
				.parseBoolean(VtnServiceInitManager.getConfigurationMap()
						.getConfigValue(
								VtnServiceOpenStackConsts.DB_WAIT_CONDITION)));
		return connectionProperties;
	}

}
