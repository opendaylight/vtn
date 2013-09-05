/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.webapi.utils;

import java.io.BufferedReader;
import java.io.IOException;
import java.util.Enumeration;

import javax.servlet.http.HttpServletRequest;

import org.json.JSONException;
import org.json.JSONObject;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.webapi.enums.ApplicationConstants;
import org.opendaylight.vtn.webapi.enums.SessionEnum;
import org.opendaylight.vtn.webapi.exception.VtnServiceWebAPIException;

/**
 * The Class VtnServiceWebUtil.
 */
public class VtnServiceWebUtil {
	
	/** The Constant LOG. */
	private static final Logger LOG = Logger.getLogger(VtnServiceWebUtil.class.getName());
	/** The Constant SESSION. */
	private static final String SESSION = "session";
	
	/** The Constant NEW_LINE. */
	private static final String NEW_LINE="\n";
	
	/** The Constant OP. */
	private static final String OP = "op";
	
	/**
	 * Prepare request json.
	 *
	 * @param request the request
	 * @param contentType the content type
	 * @return the json object
	 * @throws IOException Signals that an I/O exception has occurred.
	 * @throws VtnServiceWebAPIException the vtn service web api exception
	 */
	public  JsonObject prepareRequestJson(final HttpServletRequest request, final String contentType) throws IOException, VtnServiceWebAPIException{
		LOG.trace("Preparing request JSON # prepareRequestJson()");
		final BufferedReader reader = request.getReader();
		JsonObject serviceRequest = null;
	    final StringBuilder requestStr = new StringBuilder();
	    String line = reader.readLine();
	    while (line != null) {
	    	requestStr.append(line).append(NEW_LINE);	
	        line = reader.readLine();
	    }
	    reader.close();
	    LOG.debug("Request String : " + requestStr.toString());
	    serviceRequest = DataConverter.getConvertedRequestObject(requestStr.toString(), contentType);
	    LOG.debug("Request object json # serviceRequest #");
	    return serviceRequest;
	    
	}
	
	/**
	 * Prepare header json.
	 *
	 * @param request the request
	 * @return the json object
	 */
	public static JsonObject prepareHeaderJson(final HttpServletRequest request){
		LOG.trace("Preaparing request header json # prepareHeaderJson()");
		final JsonObject headerJson = new JsonObject();
		final Enumeration<?> headerEnum = request.getHeaderNames();
		if(null !=headerEnum ){
			while (headerEnum.hasMoreElements()) {
				final String nextElement =  (String)headerEnum.nextElement();
				if(SessionEnum.USERNAME.getSessionElement().equals(nextElement) || 
					SessionEnum.PASSWORD.getSessionElement().equals(nextElement) || 
					SessionEnum.LOGINNAME.getSessionElement().equals(nextElement) || 
					SessionEnum.INFO.getSessionElement().equals(nextElement))
					{
						headerJson.addProperty(nextElement, request.getHeader(nextElement));
					}
			}
			
		}
		headerJson.addProperty(SessionEnum.IPADDRESS.getSessionElement(), request.getRemoteAddr());
		headerJson.addProperty(ApplicationConstants.TYPE, ApplicationConstants.SESSION_TYPE);
		final JsonObject sessionJson = new JsonObject();
		sessionJson.add(SESSION, headerJson);
		LOG.debug("Preaparing request header json # prepareHeaderJson() ");
		return sessionJson;
	}
	
	/**
	 * Prepare aquire config json.
	 *
	 * @return the json object
	 * @throws VtnServiceWebAPIException the vtn service web api exception
	 */
	public static JsonObject prepareAquireConfigJSON() throws VtnServiceWebAPIException {
		LOG.trace("Preaparing aquire config json # prepareAquireConfigJSON() ");
		JsonObject configJson= new JsonObject();
		configJson.addProperty(OP, ConfigurationManager.getInstance().getConfProperty(OP));
		LOG.debug("Aquire config json prepared # prepareAquireConfigJSON() ");
		return configJson;
	}
	

	/**
	 * Prepare aquire read lock json.
	 *
	 * @return the json object
	 * @throws VtnServiceWebAPIException the vtn service web api exception
	 */
	public static JsonObject prepareAquireReadLockJSON() throws VtnServiceWebAPIException {
		LOG.trace("Preaparing aquire read lock json # prepareAquireReadLockJSON() ");
		JsonObject acquireReadLockObj = new JsonObject();
		JsonObject readLockJson = new JsonObject();
		readLockJson.addProperty(ApplicationConstants.TIMEOUT, ConfigurationManager.getInstance().getConfProperty(ApplicationConstants.TIMEOUT));
		acquireReadLockObj.add(ApplicationConstants.READLOCK, readLockJson);
		LOG.trace("Aquire read lock json # prepareAquireReadLockJSON() ");
		return acquireReadLockObj;
	}

	/**
	 * Prepare config commit json.
	 *
	 * @param operation the operation
	 * @return the json object
	 */
	public static JsonObject prepareConfigCommitOrSaveJSON(final String operation) {
		JsonObject commitConfigJson = new JsonObject();
		JsonObject commitConfigObj = new JsonObject();
		commitConfigJson.addProperty(ApplicationConstants.OPERATION, operation);
		commitConfigObj.add(ApplicationConstants.CONFIGURATION_STRING, commitConfigJson);
		return commitConfigObj;
	}
	/**
	 * Prepare abort commit json.
	 * 
	 * @param operation
	 *            the operation
	 * @return the json object
	 */
	public static JsonObject prepareCandidateAbortJSON(final String operation) {
		JsonObject abortConfigJson = new JsonObject();
		JsonObject abortConfigObj = new JsonObject();
		abortConfigJson.addProperty(ApplicationConstants.OPERATION, operation);
		abortConfigObj.add(ApplicationConstants.CANDIDATE, abortConfigJson);
		return abortConfigObj;
	}
	
	
	/**
	 * Prepare err response json.
	 *
	 * @param errCode the err code
	 * @param description the description
	 * @return the jSON object
	 * @throws JSONException 
	 */
	public static JSONObject prepareErrResponseJson(final String errCode, final String description) {
		JSONObject errJson = null;
		if(!ApplicationConstants.STATUS_OK.equals(errCode)){
			JSONObject temErrorJSON = new JSONObject();
			errJson = new JSONObject();
			try{
				temErrorJSON.put(ApplicationConstants.ERR_CODE, errCode);
				temErrorJSON.put(ApplicationConstants.ERR_DESCRIPTION, description);
				errJson.put(ApplicationConstants.ERROR, temErrorJSON);
			}catch(JSONException e){
				LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
			}			
		}
		return errJson;
	}

	
	/**
	 * Check string for null or empty.
	 *
	 * @param valueStr the value str
	 * @return true, if successful
	 */
	public static boolean checkStringForNullOrEmpty(final String valueStr) {
		boolean status = false;
		if(null != valueStr && !valueStr.trim().isEmpty()){
			status = true;
		}
		return status;
	}
}
