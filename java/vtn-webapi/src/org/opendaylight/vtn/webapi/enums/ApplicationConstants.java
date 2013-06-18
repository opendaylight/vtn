/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.webapi.enums;

/**
 * The Class ApplicationConstants.This class contains all the constants used in the web api application
 */
public final class ApplicationConstants {
	
	/**
	 * Instantiates a new application constants.
	 */
	private ApplicationConstants() {//private default constructor so that no one can make the object of this class	
	}
	
	/** The Constant ZERO. */
	public static final int ZERO = 0;
	
	/** The Constant TIMEOUT. */
	public static final String TIMEOUT = "timeout";
	
	/** The Constant READLOCK. */
	public static final String READLOCK = "readlock";
	
	/** The Constant TYPE_XML. */
	public static final String TYPE_XML = ".xml";
	
	/** The Constant TYPE_JSON. */
	public static final String TYPE_JSON = ".json";
	
	/** The Constant ACCESS_PROPERTY_PATH. */
	public static final String ACCESS_PROPERTY_PATH = "/access.properties";
	
	/** The Constant PWD_PROPERTY_PATH. */
	public static final String PWD_PROPERTY_PATH = "/pwd.properties";
	
	/** The Constant WEBAPI_CONF_PROPERTY_PATH. */
	public static final String WEBAPI_CONF_PROPERTY_PATH = "/webapiconf.properties";
	
	/** The Constant EMPTY_STRING. */
	public static final String EMPTY_STRING = "";
	
	/** The Constant DEFAULT_ERROR_DESCRIPTION. */
	public final static String DEFAULT_ERROR_DESCRIPTION = "internal server error";
	
	/** The Constant PROPERTY_NOT_FOUND_ERROR_DESC. */
	public static final String PROPERTY_NOT_FOUND_ERROR_DESC = "Property Read Error";
	
	/** The Constant OPERATION. */
	public static final String OPERATION = "operation";
	
	/** The Constant OPERATION_COMMIT. */
	public static final String OPERATION_COMMIT = "commit";
	
	/** The Constant OPERATION_SAVE. */
	public static final String OPERATION_SAVE = "save";
	
	/** The Constant CONFIGURATION_STRING. */
	public static final String CONFIGURATION_STRING = "configuration";
	
	/** The Constant COLON. */
	public static final String COLON = ":";
	
	/** The Constant ONE. */
	public static final int ONE = 1;
	/* Http Error codes and error descriptions */
	//error codes
	/** The Constant VTN_ERRORCODE_PREFIX. */
	public static final String VTN_ERRORCODE_PREFIX = "vtns_err_";
	
	/** The Constant STATUS_OK. */
	public static final String STATUS_OK = "200";
	
	/** The Constant BAD_REQUEST_ERROR. */
	public static final String BAD_REQUEST_ERROR = "400";
	
	/** The Constant USER_UNAUTHORISED_ERROR. */
	public static final String USER_UNAUTHORISED_ERROR = "401";
	
	/** The Constant FORBIDDEN_ERROR. */
	public static final String FORBIDDEN_ERROR = "403";
	
	/** The Constant RESOURCE_NOT_FOUND_ERROR. */
	public static final String RESOURCE_NOT_FOUND_ERROR = "404";
	
	/** The Constant METHOD_NOT_ALLOWED_ERROR. */
	public static final String METHOD_NOT_ALLOWED_ERROR = "405";
	
	/** The Constant MISSING_HEADER_INFO. */
	public static final String MISSING_HEADER_INFO = "410";
	
	/** The Constant INTERNAL_SERVER_ERROR. */
	public static final String INTERNAL_SERVER_ERROR = "500";
	
	/** The Constant PROPERTY_NOT_FOUND_ERROR. */
	public static final String PROPERTY_NOT_FOUND_ERROR = "501";
	
	/** The Constant PROPERTY_FILE_READ_ERROR. */
	public static final String PROPERTY_FILE_READ_ERROR = "502";
	
	/** The Constant SESSION_CREATION_FAILED. */
	public static final String SESSION_CREATION_FAILED = "510";
	
	/** The Constant AQUIRE_CONFIGURATION_FAILED. */
	public static final String AQUIRE_CONFIGURATION_FAILED = "511";
	
	/** The Constant RELEASE_SESSION_FAILED. */
	public static final String RELEASE_SESSION_FAILED = "512";
	
	/** The Constant RELEASE_CONFIGURATION_FAILED. */
	public static final String RELEASE_CONFIGURATION_FAILED = "513";
	
	/** The Constant MD5_ALGO_ERROR. */
	public static final String MD5_ALGO_ERROR = "411";
	
	/** The Constant UNSUPPORTED_PASSWORD_STRING_FORMAT_ERROR. */
	public static final String UNSUPPORTED_PASSWORD_STRING_FORMAT_ERROR = "412";
	
	
	/** The Constant PROXY_OPT. */
	public static final String PROXY_OPT = "PROXY";
	
	/** The Constant ACCESS_ALL. */
	public static final String ACCESS_ALL = "ALL";
	
	/** The Constant COMMA_STR. */
	public static final String COMMA_STR = ",";
	
	/** The Constant ROLE_ADMIN. */
	public static final String ROLE_ADMIN = "admin";
	
	/** The Constant ROLE_OPERATOR. */
	public static final String ROLE_OPERATOR = "oper";
	
	/** The Constant HTTP_GET. */
	public static final String HTTP_GET = "get";
	
	/** The Constant CONFIG_MODE. */
	public static final String CONFIG_MODE = "configmode";
	
	/** The Constant SESSION_OBJECT. */
	public static final String SESSION_OBJECT = "session";
	
	/** The Constant SESSION_ID_STR. */
	public static final String SESSION_ID_STR = "session_id";
	
	/** The Constant CONFIG_ID_STR. */
	public static final String CONFIG_ID_STR = "config_id";
	
	/** The Constant MD5. */
	public static final String MD5 = "MD5";
	
	/** The Constant ENCODE_UTF8_FORMAT. */
	public static final String ENCODE_UTF8_FORMAT = "UTF-8";
	
	/** The Constant SESSION_TYPE. */
	public static final String SESSION_TYPE = "webapi";
	
	/** The Constant TYPE. */
	public static final String TYPE = "type";
	
	/** The Constant STATUS_SUCCESS. */
	public static final String STATUS_SUCCESS = "success";
	
	/** The Constant ERR_CODE. */
	public static final String ERR_CODE = "code";
	
	/** The Constant ERR_DESCRIPTION. */
	public static final String ERR_DESCRIPTION = "msg";
	
	/** The Constant TO_REMOVE_XML_ATTR_START. */
	public static final String TO_REMOVE_XML_ATTR_START = "<o>";
	
	/** The Constant TO_REMOVE_XML_ATTR_END. */
	public static final String TO_REMOVE_XML_ATTR_END = "</o>";
	
	/** The Constant JSON_GARBAGE. */
	public static final String JSON_GARBAGE = "@";
	
	/** The Constant CONTEXTPATH. */
	public static final String CONTEXTPATH = "/vtn-webapi";
	
	/** The Constant OP. */
	public static final String OP = "op";
	
	/** The Constant COUNT. */
	public static final String COUNT = "/count";
	
	/** The Constant DETAIL. */
	public static final String DETAIL = "/detail";
	
	/** The Constant SLASH. */
	public static final char SLASH = '/';

	/** The Constant ERROR. */
	public static final String ERROR = "error";
	
	public static final String GETLISTAPI = "getListAPI";

	public static final CharSequence CONTROLLERSTR = "/controllers";
	
	public static final CharSequence DOMAINSTR = "/domains";
	
	public static final CharSequence LOGICALPORTSSTR = "/logicalports";

	public static final String SESSIONSTR = "/sessions/";
	
} 
