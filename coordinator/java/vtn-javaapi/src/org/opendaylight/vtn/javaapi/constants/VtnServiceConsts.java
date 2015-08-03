/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.constants;

/**
 * VtnServiceConsts.
 */
public final class VtnServiceConsts {

	public static final String JAVAAPI_VERSION = "V1.4";

	public static final int IPC_RESUL_CODE_INDEX = 7;
	public static final int IPC_COUNT_INDEX = 2;
	public static final int UPPL_RC_ERR_NO_SUCH_INSTANCE = 2015;
	public static final int UPLL_RC_ERR_NO_SUCH_INSTANCE = 1010;
	public static final int UPPL_ERROR_INITIAL_INDEX = 1999;
	public static final int UPLL_ERROR_INITIAL_INDEX = 999;

	public static final String PUT = "put";
	public static final String GET = "get";
	public static final String POST = "post";
	public static final String DELETE = "delete";

	public static final String COMMON_CONF_FILEPATH = "org/opendaylight/vtn/javaapi/conf.properties";
	public static final String APP_CONF_FILEPATH = "webapp_connection.properties";
	public static final String MAPMODE_CONF_FILEPATH = "mapmode.properties";
	public static final String UPPL_ERRORS_FILEPATH = "org/opendaylight/vtn/javaapi/ipc/enums/uppl_errors.properties";
	public static final String UPLL_ERRORS_FILEPATH = "org/opendaylight/vtn/javaapi/ipc/enums/upll_errors.properties";
	public static final String SINGLETON_EXCEPTION = "Already instantiated";
	public static final String STRUCT_METHOD_POSTFIX = "Struct";
	public static final String STRUCT_METHOD_PREFIX = "get";
	public static final String RESOURCES = "org/opendaylight/vtn/javaapi/resources";
	public static final String TEST_RESOURCES = "org.opendaylight.vtn.javaapi.resources";

	public static final String CONN_POOL_SIZE = "max_ipc_conn_pool_size_";
	public static final String MAX_REP_DEFAULT = "max_repetition_default";
	public static final int MAX_REP_COUNT = 100;

	public static final String DOT_REGEX = "\\.";
	public static final String MAC_ADD_REGEX = "^([0-9a-fA-F]{4}(\\.[0-9a-fA-F]{4}){2})$";
	public static final String ETH_TYPE_REGEX = "0[xX][0-9a-fA-F]{1,4}";
	public static final String IPV4_ADD_REGEX = "^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$";
	public static final String IPV6_ADD_REGEX = "^[a-f0-9]{1,4}:([a-f0-9]{0,4}:){2,6}[a-f0-9]{1,4}$";
	public static final String INTEGER_REGEX = "\\d+";
	public static final String NETMASK_REGEX = "^(((128|192|224|240|248|252|254)\\.0\\.0\\.0)|(255\\.(0|128|192|224|240|248|252|254)\\.0\\.0)|(255\\.255\\.(0|128|192|224|240|248|252|254)\\.0)|(255\\.255\\.255\\.(0|128|192|224|240|248|252|254)))$";

	public static final String ALPHANUM_REGEX = "^[0-9A-Za-z](\\w)*$";
	public static final String ALARM_REGEX = "^(\\d{1,19})$";
	public static final String SLASH = "/";
	public static final String EMPTY_STRING = "";
	public static final String COMMA = ",";
	public static final String SPACE = " ";
	public static final String EQUAL = "=";
	public static final String DOT = ".";
	public static final String CLASS_EXT = ".class";
	public static final String OPEN_CURLY_BRACES = "{";
	public static final String CLOSE_CURLY_BRACES = "}";
	public static final String HYPHEN = "-";
	public static final String UNDERSCORE = "_";
	public static final String COLON = ":";
	public static final String NEW_LINE_CHAR = "\n";
	public static final String OPEN_SQR_BRACKET = "[";
	public static final String CLOSE_SQR_BRACKET = "]";
	public static final String VALID_DESCRIPTION = "^[a-zA-Z][A-Za-z0-9.,_ ]*";

	public static final int RESPONSE_SUCCESS = 200;
	public static final String SWITCH_REGEX = "^[A-Za-z0-9][A-Za-z0-9-/_]*$";
	public static final String LOGICAL_PORT_ID_REGEX = "^[A-Za-z0-9][A-Za-z0-9-.]*$";

	public static final long INVALID_CONFIGID = 0;
	public static final long INVALID_SESSIONID = 0;

	public static final String WHITESPACE = "\\s";
	// public static final String INVALID_FORMAT = "Invalid Format";
	public static final String RESOURCE_METHOD_INCORRECT = "Resource method is not implemented";

	public static final String UTF8 = "UTF-8";
	public static final String JAVAAPI_JARPATH = "javaapi_path";
	public static final String INDEX_ERROR_MSG = "index parameter not allowed for op : count";
	public static final String VERSION_REGEX = "^[0-9][0-9.]*[0-9]$|^[0-9]$";

	public static final String WEB_API = "WEB-API";
	public static final String UNC_GUI = "UNC-GUI";
	public static final String ZERO = "0";
	public static final String INCORRECT_METHOD_INVOCATION = "Incorrect method invocation";

	public static final int DEFAULT_NUMBER = 0;
	public static final String DEFAULT_IP = "0.0.0.0";
	public static final String DEFAULT_IPV6 = "0000:0000:0000:0000:0000:0000:0000:0000";
	public static final String DEFAULT_MAC = "0000.0000.0000";

	public static final String OPEN_SMALL_BRACES = "(";
	public static final String CLOSE_SMALL_BRACES = ")";

	public static final String QUESTION_MARK = "?";

	public static final String QUOTE_CHAR = "\"";
	
	public static final String CONF_FILE_FIELD_POLC = "polc";
	public static final String CONF_FILE_FIELD_HPVANC = "hpvanc";
	public static final String CONF_FILE_FIELD_ODC = "odc";
}
