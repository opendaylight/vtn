/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.webapi.constants;

/**
 * The Class ApplicationConstants.This class contains all the constants used in
 * the web api application
 */
public final class ApplicationConstants {

	/**
	 * Instantiates a new application constants.
	 */
	private ApplicationConstants() {
	}

	/** The Constant RESPONSE_SUCCESS. */
	public static final int SUCCESS = 200;
	
	/** The Constant RESPONSE_ACCEPTED. */
	public static final int ACCEPTED = 202;

	/** The Constant ZERO. */
	public static final int ZERO = 0;

	/** The Constant FOUR for XML length. */
	public static final int FOUR = 4;

	/** The Constant FIVE for JSON length. */
	public static final int FIVE = 5;

	/** The Constant TIMEOUT. */
	public static final String TIMEOUT = "timeout";

	/** The Constant READLOCK. */
	public static final String READLOCK = "readlock";

	/** The Constant TYPE_XML. */
	public static final String TYPE_XML = ".xml";

	/** The Constant TYPE_JSON. */
	public static final String TYPE_JSON = ".json";

	/** The Constant ACCESS_PROPERTY_PATH. */
	public static final String ACCESS_PROPERTY_PATH = "access.properties";

	/** The Constant WEBAPI_CONF_PROPERTY_PATH. */
	public static final String WEBAPI_CONF_PROPERTY_PATH = "webapiconf.properties";

	/** The Constant BLANK_STR. */
	public static final String BLANK_STR = "";

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
	/* HTTP Error codes and error descriptions */
	// error codes
	/** The Constant VTN_ERRORCODE_PREFIX. */
	public static final String VTN_ERRORCODE_PREFIX = "vtns_err_";

	/** The Constant ACCESS_ALL. */
	public static final String ACCESS_ALL = "ALL";

	/** The Constant COMMA_STR. */
	public static final String COMMA_STR = ",";

	/** The Constant ROLE_ADMIN. */
	public static final String ROLE_ADMIN = "admin";

	/** The Constant ROLE_OPERATOR. */
	public static final String ROLE_OPERATOR = "oper";

	/** The Constant HTTP_GET. */
	public static final String GET_METHOD_NAME = "get";

	/** The Constant CONFIG_MODE. */
	public static final String CONFIG_MODE = "configmode";

	/** The Constant SESSION_OBJECT. */
	public static final String SESSION = "session";

	/** The Constant SESSION_ID_STR. */
	public static final String SESSION_ID_STR = "session_id";

	/** The Constant CONFIG_ID_STR. */
	public static final String CONFIG_ID_STR = "config_id";

	/** The Constant ENCODE_UTF8_FORMAT. */
	public static final String ENCODE_UTF8_FORMAT = "UTF-8";

	/** The Constant SESSION_TYPE. */
	public static final String SESSION_TYPE = "webapi";

	/** The Constant TYPE. */
	public static final String TYPE = "type";

	/** The Constant STATUS_SUCCESS. */
	public static final String STATUS_SUCCESS = "success";

	/** The Constant ERR_CODE. */
	public static final String ERR_CODE = "err_code";

	/** The Constant ERR_DESCRIPTION. */
	public static final String ERR_DESCRIPTION = "err_msg";

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
	
	/** The Constant CFG_MODE_TIMEOUT. */
	public static final String CFG_MODE_TIMEOUT = "cfg_mode_timeout";

	public static final String COMMIT_TIMEOUT = "commit_timeout";

	public static final String ABORT_TIMEOUT = "abort_timeout";

	public static final String CANCEL_AUDIT = "cancel_audit";

	/** The Constant COUNT. */
	public static final String COUNT = "/count";

	/** The Constant DETAIL. */
	public static final String DETAIL = "/detail";

	/** The Constant SLASH. */
	public static final char SLASH = '/';

	/** The Constant ERROR. */
	public static final String ERROR = "error";

	public static final String GETLISTAPI = "getListAPI";

	public static final String ROUTERS = "/routers";
	
	public static final String TENANTS = "/tenants";
	
	public static final CharSequence CONTROLLERSTR = "/controllers";

	public static final CharSequence DOMAINSTR = "/domains";

	public static final CharSequence LOGICALPORTSSTR = "/logicalports";

	public static final String SESSIONSTR = "/sessions/";

	public static final String ALARMSTR = "/unc/alarms";

	public static final String UNC_WEB_ADMIN = "UNC_WEB_ADMIN";

	public static final String UNC_WEB_OPER = "UNC_WEB_OPER";

	public static final String CANDIDATE = "candidate";

	public static final String OPERATION_ABORT = "abort";

	public static final String WILD_CARD_STAR = "*";

	public static final String CHAR_ENCODING = "UTF-8";

	public static final String XSLT_FILE = "org/opendaylight/vtn/webapi/utils/tranformXslt.xslt";

	public static final String NEW_LINE = "\n";

	public static final String LINE_FEED = "[\\n\\r]";

	public static final String XML_STANDALONE = "yes";

	public static final char LESS_THAN = '<';

	public static final char GREATER_THAN = '>';

	public static final String NULL_STRING = "null";

	public static final String DUMMY_JSON = "{\"dummy\" : {}}";

	public static final String DUMMY_XML = " dummy=\"\"";

	public static final String EMPTY_JSON = "{}";

	public static final String EMPTY_JSON_ARRAY = "[]";

	public static final String vtepgroup = "vtepgroup";

	public static final String member_vteps = "member_vteps";

	public static final String member_vtep = "member_vtep";

	public static final String ipaddrs = "ipaddrs";

	public static final String ipv6addr = "ipv6addrs";

	public static final String DOT_ZERO = "0";

	public static final String POST_METHOD_NAME = "POST";

	public static final String PUT_METHOD_NAME = "PUT";

	public static final String DELETE_METHOD_NAME = "delete";

	public static final String HTTP_HEADER_ACCEPT = "Accept";

	public static final String HTTP_AUTHERIZATION = "Authorization";

	public static final String AUTHERIZATION_BASIC = "Basic";

	public static final String DOT_REGEX = ".";

	public static final String QUESTION_MARK_CHAR = "?";

	public static final String DEFAULT_ACCEPT = "*/*";

	public static final String HYPHEN = "-";

	public static final String INFO = "/info";

	public static final String RETRY_AFTER = "Retry-After";

	public static final String AUTHORIZATION_RESP_HEADER = "WWW-Authenticate";

	public static final String AUTHORIZATION_RESP_VALUE = "BASIC realm=\"Web API\"";

	public static final String MD5 = "MD5";

	public static final String DEFAULT_PASSWD = "adminpass";

	public static final String TARGETDB = "targetdb";

	public static final String inport = "inport";
	public static final String macdstaddr = "macdstaddr";
	public static final String macdstaddr_mask = "macdstaddr_mask";
	public static final String macsrcaddr = "macsrcaddr";
	public static final String macsrcaddr_mask = "macsrcaddr_mask";
	public static final String macethertype = "macethertype";
	public static final String vlan_id = "vlan_id";
	public static final String vlan_priority = "vlan_priority";
	public static final String iptos = "iptos";
	public static final String ipproto = "ipproto";
	public static final String ipdstaddr = "ipdstaddr";
	public static final String ipdstaddr_mask = "ipdstaddr_mask";
	public static final String ipsrcaddr = "ipsrcaddr";
	public static final String ipsrcaddr_mask = "ipsrcaddr_mask";
	public static final String l4dstport_icmptype = "l4dstport_icmptype";
	public static final String l4dstport_icmptype_mask = "l4dstport_icmptype_mask";
	public static final String l4srcport_icmptype = "l4srcport_icmptype";
	public static final String l4srcport_icmptype_mask = "l4srcport_icmptype_mask";
	public static final String ipv6dstaddr = "ipv6dstaddr";
	public static final String ipv6dstaddr_mask = "ipv6dstaddr_mask";
	public static final String ipv6srcaddr = "ipv6srcaddr";
	public static final String ipv6srcaddr_mask = "ipv6srcaddr_mask";
	public static final String outputport = "outputport";
	public static final String enqueueport = "enqueueport";
	public static final String queue_id = "queue_id";
	public static final String setmacdstaddr = "setmacdstaddr";
	public static final String setmacsrcaddr = "setmacsrcaddr";
	public static final String setvlan_id = "setvlan_id";
	public static final String setvlan_priority = "setvlan_priority";
	public static final String setipdstaddr = "setipdstaddr";
	public static final String setipsrcaddr = "setipsrcaddr";
	public static final String setiptos = "setiptos";
	public static final String setl4dstport_icmptype = "setl4dstport_icmptype";
	public static final String setl4srcport_icmptype = "setl4srcport_icmptype";
	public static final String setipv6dstaddr = "setipv6dstaddr";
	public static final String setipv6srcaddr = "setipv6srcaddr";
	public static final String stripvlan = "stripvlan";

	public static final String OS_RESOURCE_PKG = "org.opendaylight.vtn.javaapi.resources.openstack.";
	public static final String SEMI_COLON = ";";
	
	/** The Constant CFG_TIMEOUT_DEFAULT. */
	public static final String CFG_TIMEOUT_DEFAULT = "20500";
	
	public static final String COMMIT_TIMEOUT_DEFAULT = "20500";

	public static final String ABORT_TIMEOUT_DEFAULT = "20500";

	public static final String CANCEL_AUDIT_DEFAULT = "1";

	/** The Space  */
	public static final String SPACE_STRING = " ";
	/** The quotation marks */
	public static final String QUOTATION_MARK_STRING = "\"";

	/** The Constant for Acceselog. */
	public static final String REQ_BODY = "requestBodyName";

	public static final String REQ_BODY_DEF_NAME = "org.opendaylight.vtn.unc.requestbody";

        public static final String RES_ERR_MSG = "errorMsgName";

        public static final String RES_ERR_MSG_DEF_NAME = "org.opendaylight.vtn.unc.errormessage";

    /** 
     * URI /configuration/diff
     */
    public static final String URI_DIFF = "/configuration/diff";

	public static final String MODE = "mode";

	public static final String VIRTUAL_MODE = "virtual";

	public static final String REAL_MODE = "real";

	public static final String VTN_MODE = "vtn";

	public static final String GLOBAL_MODE = "global";

	public static final String VTNS_STRING = "vtns";

	public static final String TENANT_STRING = "tenants";

	public static final String FLOWLISTS_STRING = "flowlists";

	public static final String UNIFIED_NETWORKS_STRING = "unified_networks";

	public static final String FILTERS_STRING = "filters";

	public static final String CONTROLLERS_STRING = "controllers";

	public static final String BOUNDARIES_STRING = "boundaries";

	public static final String VTN = "vtn";

	public static final String VTNNAME = "vtn_name";

	public static final String ID = "id";

	public static final String SLASH_STRING = "/";
}
