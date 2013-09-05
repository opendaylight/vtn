/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.webapi.utils;

import java.util.Arrays;
import java.util.List;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.webapi.enums.ApplicationConstants;
import org.opendaylight.vtn.webapi.enums.ContentTypeEnum;
import org.opendaylight.vtn.webapi.enums.SessionEnum;
import org.opendaylight.vtn.webapi.exception.VtnServiceWebAPIException;
import org.opendaylight.vtn.webapi.pojo.SessionBean;

/**
 * The Class VtnServiceCommonUtil.
 */
public final class VtnServiceCommonUtil {
	
	/** The Constant LOG. */
	private static final Logger LOG = Logger.getLogger(VtnServiceCommonUtil.class.getName());

	/**
	 * Instantiates a new vtn service common util.
	 */
	private VtnServiceCommonUtil() {
		
	}
	/**
	 * Method getErrorDescription - Below method is used to get error description based on error code.
	 *
	 * @param errorCode the error code
	 * @return String
	 * @throws VtnServiceWebAPIException the vtn service exception
	 */
	public static String getErrorDescription(final String errorCode) throws VtnServiceWebAPIException {
		LOG.trace(" Getting error description on the basis of error code #getErrorDescription()");
		return ConfigurationManager.getInstance().getConfProperty(ApplicationConstants.VTN_ERRORCODE_PREFIX + errorCode);
	}
  
	/**
	 * Method getErrorDescription - Below method is used to get error description based on error code.
	 *
	 * @param errorCode the error code
	 * @param description the description
	 * @return String
	 */
	public static String logErrorDetails(final String errorCode) {
		LOG.trace(" Getting error description by appending error code and description #getErrorDescription()");
		String errorDetails = null;
		try{
			errorDetails = new StringBuilder().append(errorCode).append("-").append(ConfigurationManager.getInstance().getConfProperty(ApplicationConstants.VTN_ERRORCODE_PREFIX + errorCode)).toString();
		}catch (Exception e) {
			LOG.error(logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
		}
		return errorDetails;
	}
	
	
	/**
	 * Authenticate user.
	 *
	 * @param headerInfo the header info
	 * @param httpMethod the http method
	 * @return true, if successful
	 * @throws VtnServiceWebAPIException the vtn service exception
	 */
    /*public static boolean authenticateUser(final JsonObject headerInfo, final String httpMethod) throws VtnServiceWebAPIException{
    	LOG.trace(" Authenticating user  #authenticateUser() :starts");
    	boolean authenticationStatus= false;    	
    	String role = null;
    	String password = null;
    	final SessionBean bean = getSessionObject(headerInfo);
		final ConfigurationManager configurationManager = ConfigurationManager.getInstance();
		final String roleAndPassString = configurationManager.getPWDProperty(bean.getUserName());
		if(null != roleAndPassString && !roleAndPassString.isEmpty() && roleAndPassString.indexOf(ApplicationConstants.COLON) != -1){
			String[] roleAndPassArr = roleAndPassString.split(ApplicationConstants.COLON);
			role = roleAndPassArr[ApplicationConstants.ZERO];
			password = roleAndPassArr[ApplicationConstants.ONE];
		}
		if(authoriseUser(bean, httpMethod)){
			authenticationStatus = checkRoleAccessability(role, httpMethod, password, bean);
		}
		LOG.trace(" Authenticating user  #authenticateUser() :ends");
    	return authenticationStatus;
	}*/
    
    /**
     * Authorise user.
     *
     * @param bean the bean
     * @param httpMethod the http method
     * @return true, if successful
     * @throws VtnServiceWebAPIException the vtn service web api exception
     */
    public static boolean authoriseUser(SessionBean bean,  String httpMethod) throws VtnServiceWebAPIException{
    	boolean ipStatus = false;
    	ConfigurationManager configurationManager = ConfigurationManager.getInstance();
    	LOG.trace(" Authorizinging user  #authoriseUser() : starts");
		final String ipAddresses =  configurationManager.getAccessProperty(ApplicationConstants.ACCESS_ALL);
		final String httpMethodAccessIpAddress = configurationManager.getAccessProperty(httpMethod.toUpperCase());
		if(ipAddresses != null && ApplicationConstants.WILD_CARD_STAR.equals(ipAddresses.trim())){
			ipStatus = true;
		}
		else if(ipAddresses.indexOf(bean.getIpAddress()) != -1){
			ipStatus = true;
		}
		else if(null != httpMethodAccessIpAddress && !httpMethodAccessIpAddress.isEmpty() && ApplicationConstants.WILD_CARD_STAR.equals(httpMethodAccessIpAddress.trim())){
			ipStatus = true;
		}
		else if(null != httpMethodAccessIpAddress && !httpMethodAccessIpAddress.isEmpty() && httpMethodAccessIpAddress.indexOf(bean.getIpAddress()) != -1){
			ipStatus = true;
		}
		
    	LOG.trace(" Authorizinging user  #authoriseUser() : ends");
    	return ipStatus;
	}


    /**
     * Check role accessability.
     *
     * @param role the role
     * @param httpMethod the http method
     * @param password the password
     * @param bean the bean
     * @return true, if successful
     * @throws VtnServiceWebAPIException the vtn service web api exception
     */
   /* private static boolean checkRoleAccessability(final String role, final String httpMethod, final String password, final SessionBean bean) throws VtnServiceWebAPIException {
    	LOG.trace("Validating role and accessability of the user : starts");
    	boolean authenticationStatus = false;
    	if(VtnServiceWebUtil.checkStringForNullOrEmpty(role) && ApplicationConstants.ROLE_ADMIN.equals(role) && (VtnServiceWebUtil.checkStringForNullOrEmpty(password) && password.equals(convertPasswordStrToMd5Str(bean.getPassword())))){
			authenticationStatus = true;
		}else if(VtnServiceWebUtil.checkStringForNullOrEmpty(role) && ApplicationConstants.ROLE_OPERATOR.equals(role) && httpMethod.equalsIgnoreCase(ApplicationConstants.HTTP_GET) && (VtnServiceWebUtil.checkStringForNullOrEmpty(password) && password.equals(bean.getPassword()))){
			authenticationStatus = true;
		}
    	LOG.trace("Validating role and accessability of the user : ends");
    	return authenticationStatus;
	}*/

	/**
	 * Gets the session from json.
	 *
	 * @param sessionObjRes the session obj res
	 * @return the session from json
	 */
	public static long getSessionFromJson(final JsonObject sessionObjRes) {
		LOG.trace("Getting session id from recieved session Json response : starts");
		long sessionID = -1;
		if(null != sessionObjRes){
			JsonObject sessionJson = sessionObjRes.getAsJsonObject(ApplicationConstants.SESSION_OBJECT);
			sessionID = null != sessionJson && sessionJson.has(ApplicationConstants.SESSION_ID_STR) ? sessionJson.getAsJsonPrimitive(ApplicationConstants.SESSION_ID_STR).getAsLong() : -1;
		}
		LOG.trace("Getting session id from recieved session Json response : ends");
		return sessionID;
	}

	/**
	 * Gets the config id from json.
	 *
	 * @param configObjRes the config obj res
	 * @return the config id from json
	 */
	public static long getConfigIdFromJson(final JsonObject configObjRes) {
		LOG.trace("Getting config id from recieved config Json response : starts");
		long configID = -1;
		if(null != configObjRes){
			JsonObject configModeJson = configObjRes.getAsJsonObject(ApplicationConstants.CONFIG_MODE);
			configID = null != configModeJson && configModeJson.has(ApplicationConstants.CONFIG_ID_STR) ? configModeJson.getAsJsonPrimitive(ApplicationConstants.CONFIG_ID_STR).getAsInt() : -1;
		}
		LOG.trace("Getting config id from recieved config Json response : ends");
		return configID;
	}
	
	/**
	 * Gets the content type.
	 *
	 * @param uri the uri
	 * @return the content type
	 */
	public static String getContentType(final String uri) {
		LOG.trace("Getting content type from URI : starts");
		String contentType = ContentTypeEnum.APPLICATION_JSON.getContentType();
		if(null != uri && (uri.endsWith(ApplicationConstants.TYPE_XML))){
			contentType = ContentTypeEnum.APPLICATION_XML.getContentType();
		}
		LOG.trace("Getting content type from URI : ends");
		LOG.trace("Getting content type from URI :"+contentType);
		return contentType;
	}
	
	/**
	 * Validate uri.
	 *
	 * @param uri the uri
	 * @param contentType the content type
	 * @return true, if successful
	 */
	public static boolean validateURI(final String uri, final String contentType){
		LOG.trace("validation for URI : starts");
		LOG.debug("uri : " + uri + " content-type : " + contentType);
		boolean uriStatus = false;
		if(null != uri && (uri.endsWith(ApplicationConstants.TYPE_XML) || uri.endsWith(ApplicationConstants.TYPE_JSON))){
			uriStatus =  true;
		}
		if(uriStatus && (ContentTypeEnum.APPLICATION_JSON.getContentType().equals(contentType) || ContentTypeEnum.APPLICATION_XML.getContentType().equals(contentType))){
			uriStatus =  true;
		}else{
			uriStatus =  false;
		}
		LOG.debug("uri : " + uri + " content-type : " + contentType);
		LOG.trace("validation for URI : ends");
		return uriStatus;
	}
	
	/**
	 * Gets the session object.
	 *
	 * @param sessionJson the session json
	 * @return the session object
	 * @throws VtnServiceWebAPIException the vtn service web api exception
	 */
	public static SessionBean getSessionObject(final JsonObject sessionJson) throws VtnServiceWebAPIException{
		LOG.trace("Preparing session object : starts");
		SessionBean sessionBean = new SessionBean();
		List<String> mandatoryList = Arrays.asList(SessionEnum.USERNAME.getSessionElement(), SessionEnum.PASSWORD.getSessionElement(), SessionEnum.IPADDRESS.getSessionElement());
		JsonObject sessionJsonObj = (JsonObject) sessionJson.get(ApplicationConstants.SESSION_OBJECT);
		for (String value : mandatoryList) {
			if(!sessionJsonObj.has(value) || null == sessionJsonObj.get(value)){
				throw new VtnServiceWebAPIException(ApplicationConstants.BAD_REQUEST_ERROR, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.BAD_REQUEST_ERROR));
			}
		}
		sessionBean.setUserName(sessionJsonObj.get(SessionEnum.USERNAME.getSessionElement()) != null ?sessionJsonObj.get(SessionEnum.USERNAME.getSessionElement()).getAsString() : null );
		sessionBean.setPassword(sessionJsonObj.get(SessionEnum.PASSWORD.getSessionElement()) != null ?sessionJsonObj.get(SessionEnum.PASSWORD.getSessionElement()).getAsString() : null );
		sessionBean.setIpAddress(sessionJsonObj.get(SessionEnum.IPADDRESS.getSessionElement()) != null ?sessionJsonObj.get(SessionEnum.IPADDRESS.getSessionElement()).getAsString() : null );
		sessionBean.setLoginName(sessionJsonObj.get(SessionEnum.LOGINNAME.getSessionElement()) != null ?sessionJsonObj.get(SessionEnum.LOGINNAME.getSessionElement()).getAsString() : null );
		sessionBean.setInformation(sessionJsonObj.get(SessionEnum.INFO.getSessionElement()) != null ?sessionJsonObj.get(SessionEnum.INFO.getSessionElement()).getAsString() : null );
		sessionBean.setType(ApplicationConstants.SESSION_TYPE);
		LOG.trace("Preparing session object : ends");
		return sessionBean;
	
	}
	
	/**
	 * Gets the resource uri.
	 *
	 * @param requestURI the request uri
	 * @return the resource uri
	 */
	public static String getResourceURI(final String requestURI) {
		LOG.trace("Getting requested URI: starts " + requestURI);
		String finalURI = null;
		if (null != requestURI) {
			if (requestURI.endsWith(ApplicationConstants.TYPE_XML)) {
				finalURI = requestURI.substring(ApplicationConstants.ZERO,
						requestURI.length() - ApplicationConstants.FOUR);
			} else if (requestURI.endsWith(ApplicationConstants.TYPE_JSON)) {
				finalURI = requestURI.substring(ApplicationConstants.ZERO,
						requestURI.length() - ApplicationConstants.FIVE);
			}
		}
		if (null != finalURI) {
			finalURI = finalURI.replace(ApplicationConstants.CONTEXTPATH,
					ApplicationConstants.EMPTY_STRING);
		}
		LOG.trace("Getting requested URI: ends " + finalURI);
		return finalURI;
	}
	
	/**
	 * Convert password str to md5 str.
	 *
	 * @param simplePasswordStr the simple password str
	 * @return the string
	 * @throws VtnServiceWebAPIException the vtn service web api exception
	 */
	/*private static String convertPasswordStrToMd5Str(final String simplePasswordStr) throws VtnServiceWebAPIException{
		LOG.trace("Converting password string to MD5 using MD5 algorithm: starts");
		StringBuffer sb = null;
        try {
            final java.security.MessageDigest md = java.security.MessageDigest.getInstance(ApplicationConstants.MD5);
            final byte[] array = md.digest(simplePasswordStr.getBytes(ApplicationConstants.ENCODE_UTF8_FORMAT));
            sb = new StringBuffer();
            for (int i = 0; i < array.length; ++i) {
                sb.append(Integer.toHexString((array[i] & 0xFF) | 0x100).substring(1, 3));
            }
        }catch (final java.security.NoSuchAlgorithmException e) {
        	LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.MD5_ALGO_ERROR));
        	throw new VtnServiceWebAPIException(ApplicationConstants.MD5_ALGO_ERROR, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.MD5_ALGO_ERROR));
        } catch (UnsupportedEncodingException e) {
        	LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.UNSUPPORTED_PASSWORD_STRING_FORMAT_ERROR));
        	throw new VtnServiceWebAPIException(ApplicationConstants.UNSUPPORTED_PASSWORD_STRING_FORMAT_ERROR, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.UNSUPPORTED_PASSWORD_STRING_FORMAT_ERROR));
		}
        LOG.trace("Converting password string to MD5 using MD5 algorithm: ends");
        return sb.toString();
    }*/
	
	/**
	 * Gets the op parameter.
	 *
	 * @param serviceRequest the service request
	 * @param resourcePath the resource path
	 * @return the op parameter
	 */
	public static JsonObject getOpParameter(JsonObject serviceRequest, String resourcePath) {
		LOG.trace("Getting OP parameter from request URI : starts");
		String opString = null;
		if(null !=resourcePath && (resourcePath.endsWith(ApplicationConstants.COUNT) || resourcePath.endsWith(ApplicationConstants.DETAIL))){
			opString = resourcePath.substring(resourcePath.lastIndexOf(ApplicationConstants.SLASH)+1, resourcePath.length());
		}
		if(null != opString && !opString.isEmpty()){
			serviceRequest.addProperty(ApplicationConstants.OP, opString);
		}
		LOG.trace("Getting OP parameter from request URI : ends");
		return serviceRequest;
	}
	
	/**
	 * This method will remove /count or /detail from URI and then will return actual resource path.
	 *
	 * @param resourcePath the resource path
	 * @return the string
	 */
	public static String removeCountOrDetailFromURI(final String resourcePath) {
		LOG.trace("Removing detail/count from URI and putting the same in requestJson: starts");
		String opString = resourcePath;
		if(null !=resourcePath && (resourcePath.endsWith(ApplicationConstants.COUNT) || resourcePath.endsWith(ApplicationConstants.DETAIL))){
			opString = resourcePath.substring(ApplicationConstants.ZERO, resourcePath.lastIndexOf(ApplicationConstants.SLASH));
		}
		LOG.trace("Removing detail/count from URI and putting the same in requestJson: end");
		return opString;
	}
	
	/**
	 * 
	 * @param URI
	 * @return
	 * @throws VtnServiceWebAPIException 
	 */
	public static boolean validateGetAPI(String URI) throws VtnServiceWebAPIException{
		boolean status = false;
		if(null != URI && !URI.isEmpty()){
			if(URI.trim().contains(ApplicationConstants.CONTROLLERSTR) && URI.trim().contains(ApplicationConstants.DOMAINSTR) && URI.trim().contains(ApplicationConstants.LOGICALPORTSSTR))
				status = true;
			else if(URI.trim().startsWith(ApplicationConstants.SESSIONSTR) && URI.trim().length() > ApplicationConstants.SESSIONSTR.length())
				status = true;
			else{
				final String getListAPI = ConfigurationManager.getInstance().getConfProperty(ApplicationConstants.GETLISTAPI);
				if(null != getListAPI && !getListAPI.isEmpty()){
					final String[] tempList = getListAPI.split(ApplicationConstants.COMMA_STR);
					for(int i=0; i < tempList.length; i++ ){
						final String getStr = tempList[i];
						if(URI.trim().equals(getStr)){
							status = true;
							break;
						}
					}
				}
			}
		}
		return status;
	}
}
