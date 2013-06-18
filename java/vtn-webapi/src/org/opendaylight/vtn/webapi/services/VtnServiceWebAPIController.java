/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.webapi.services;

import org.json.JSONObject;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.RestResource;
import org.opendaylight.vtn.webapi.enums.ApplicationConstants;
import org.opendaylight.vtn.webapi.enums.ResourcePathEnum;
import org.opendaylight.vtn.webapi.exception.VtnServiceWebAPIException;
import org.opendaylight.vtn.webapi.utils.VtnServiceCommonUtil;
import org.opendaylight.vtn.webapi.utils.VtnServiceWebUtil;

/**
 * The Class VtnServiceWebAPIController.This class will be the interface between WebAPI and Java API
 * The actual flow will be covered in this class only. 
 */
public class VtnServiceWebAPIController {

	/** The Constant LOG. */
	private static final Logger LOG = Logger.getLogger(VtnServiceWebAPIController.class.getName());

	/** The response success. */
	private final int RESPONSE_SUCCESS = 200;
	
	/** The exception status. */
	private boolean exceptionStatus = false;
	
	/**
	 * This method will get the data from java api in json format and will do not pass any request to java api.
	 * First of all it will create the session and then acquire readlock from TC through JAva API. 
	 * Then it will call the requested API and if we get success from lower layer then the response jaon will be recived and
	 * readlock will be released and same for session . 
	 *
	 * @param sessionJson the session json
	 * @param resourcePath the resource path
	 * @return the jSON object
	 * @throws VtnServiceWebAPIException the vtn service web api exception
	 */
	public JSONObject get(final JsonObject sessionJson, final String resourcePath) throws VtnServiceWebAPIException{
		LOG.trace("Java API calling for GET method initialized.");
		JSONObject responseJSON = null;
		long sessionId = 0;
		boolean isReadlock = true;
		String responseStr = null;
		final RestResource resource = new RestResource();
		try{
			LOG.debug("acquiring session id and readlock from java API");
			//acquire session id
			resource.setPath(ResourcePathEnum.SESSION_PATH.getPath());
			if(resource.post(sessionJson) != RESPONSE_SUCCESS){
				LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
				throw new VtnServiceWebAPIException(ApplicationConstants.SESSION_CREATION_FAILED,VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.SESSION_CREATION_FAILED));
			}
			sessionId = VtnServiceCommonUtil.getSessionFromJson(resource.getInfo());
			//acquire monitoring mode
			resource.setPath(ResourcePathEnum.ACQUIRE_RELEASE_MONITORING_PATH.getPath());
			resource.setSessionID(sessionId);
			if(resource.put(VtnServiceWebUtil.prepareAquireReadLockJSON()) != RESPONSE_SUCCESS){
				isReadlock=false;
				LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
				throw new VtnServiceWebAPIException(ApplicationConstants.AQUIRE_CONFIGURATION_FAILED, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.AQUIRE_CONFIGURATION_FAILED));
			}
			//send the api request json
			resource.setPath(resourcePath);
			resource.setSessionID(sessionId);
			final int status = resource.get();
			LOG.debug("JAVA API returned error code #"+status);
			if(status != RESPONSE_SUCCESS){
				responseStr = resource.getInfo() != null? resource.getInfo().toString():null;
				LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
				throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
			}
			responseStr = resource.getInfo() != null? resource.getInfo().toString():null;
			LOG.debug("JAVA API returned response # "+responseStr);
		}catch (VtnServiceWebAPIException vtnException) {	
			exceptionStatus = true;
			LOG.error(VtnServiceCommonUtil.logErrorDetails(vtnException.getErrorCode()));
		}catch (RuntimeException exception) {
			exceptionStatus = true;
			LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));	
			throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
		}
		finally{
			try{
				//release monitoring mode
			if(isReadlock){
					resource.setPath(ResourcePathEnum.ACQUIRE_RELEASE_MONITORING_PATH.getPath());
					resource.setSessionID(sessionId);
					if(resource.delete() != RESPONSE_SUCCESS){
						LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
						throw new VtnServiceWebAPIException(ApplicationConstants.RELEASE_CONFIGURATION_FAILED, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.RELEASE_CONFIGURATION_FAILED));
					}
			}
			if(sessionId != ApplicationConstants.ZERO){
					//release session
					resource.setPath(ResourcePathEnum.RELEASE_SESSION.getPath()+ sessionId);
					resource.setSessionID(sessionId);
					if(resource.delete() != RESPONSE_SUCCESS){
						LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
						throw new VtnServiceWebAPIException(ApplicationConstants.RELEASE_SESSION_FAILED, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.RELEASE_SESSION_FAILED));
					}
			}
			responseJSON = new JSONObject(responseStr);
			}catch (Exception exception) {
				exceptionStatus = true;
				LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));	
				throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
			}
			
		}
		return responseJSON;
	}
	
	/**
	 *This method will get the data from java api in json format and will pass the required request to java api.
	 * First of all it will create the session and then acquire readlock from TC through JAva API. 
	 * Then it will call the requested API and if we get success from lower layer then the response jaon will be recived and
	 * readlock will be released and same for session .
	 * @param sessionJson the session json
	 * @param reqObject the req object
	 * @param resourcePath the resource path
	 * @return the jSON object
	 * @throws VtnServiceWebAPIException the vtn service web api exception
	 */
	public JSONObject get(final JsonObject sessionJson, final JsonObject reqObject, final String resourcePath) throws VtnServiceWebAPIException {
		JSONObject responseJSON = null;
		long sessionId = 0;
		boolean isReadlock = true;
		String responseStr = null;
		final RestResource resource = new RestResource();
		try{
			LOG.debug("acquiring session id and readlock from java API");
			//acquire session id
			resource.setPath(ResourcePathEnum.SESSION_PATH.getPath());
			if(resource.post(sessionJson) != RESPONSE_SUCCESS){
				LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
				throw new VtnServiceWebAPIException(ApplicationConstants.SESSION_CREATION_FAILED, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.SESSION_CREATION_FAILED));
			}
			sessionId = VtnServiceCommonUtil.getSessionFromJson(resource.getInfo());
			//acquire monitoring mode
		
			resource.setPath(ResourcePathEnum.ACQUIRE_RELEASE_MONITORING_PATH.getPath());
			resource.setSessionID(sessionId);
			if(resource.put(VtnServiceWebUtil.prepareAquireReadLockJSON()) != RESPONSE_SUCCESS){
				isReadlock = false;
				LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
				throw new VtnServiceWebAPIException(ApplicationConstants.AQUIRE_CONFIGURATION_FAILED, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.AQUIRE_CONFIGURATION_FAILED));
			}
			//send the api request json
			resource.setPath(resourcePath);
			resource.setSessionID(sessionId);
			LOG.debug("Request object passing to JAVA API  #"+reqObject);
			final int status = resource.get(reqObject);
			LOG.debug("JAVA API returned error code #"+status);
			if(status != RESPONSE_SUCCESS){
				responseStr = resource.getInfo() != null? resource.getInfo().toString():null;
				LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
				throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
			}
			responseStr = resource.getInfo() != null? resource.getInfo().toString():null;
			LOG.debug("JAVA API returned response # "+responseStr);
		}catch (VtnServiceWebAPIException vtnException) {
			exceptionStatus = true;
			LOG.error(VtnServiceCommonUtil.logErrorDetails(vtnException.getErrorCode()));
		}catch (RuntimeException exception) {
			exceptionStatus = true;
			LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));	
			throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
		}
		finally{
			try{
				//release monitoring mode
				if(isReadlock){
					resource.setPath(ResourcePathEnum.ACQUIRE_RELEASE_MONITORING_PATH.getPath());
					resource.setSessionID(sessionId);
					if(resource.delete() != RESPONSE_SUCCESS){
						LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
						throw new VtnServiceWebAPIException(ApplicationConstants.RELEASE_CONFIGURATION_FAILED, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.RELEASE_CONFIGURATION_FAILED));
					}
				}
				
				if(sessionId != ApplicationConstants.ZERO){
					//release session
					resource.setPath(ResourcePathEnum.RELEASE_SESSION.getPath()+ sessionId);
					resource.setSessionID(sessionId);
					if(resource.delete() != RESPONSE_SUCCESS){
						LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
						throw new VtnServiceWebAPIException(ApplicationConstants.RELEASE_SESSION_FAILED, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.RELEASE_SESSION_FAILED));
					}
				}
				responseJSON = new JSONObject(responseStr);
			}catch (Exception exception) {
				exceptionStatus = true;
				LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));	
				throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
			}
		
		}
		return responseJSON;
	}

	
	/**
	 * Post.This method will be required when we have to create any component using Java API. 
	 * This method will work in the following sequence-
	 * Create session 
	 * acquire config mode
	 * calling actual Java API which is requested
	 * release config lock
	 * release session
	 *
	 * @param sessionJson the session json
	 * @param reqObject the req object
	 * @param resourcePath the resource path
	 * @return the jSON object
	 * @throws VtnServiceWebAPIException the vtn service web api exception
	 */
	public JSONObject post(final JsonObject sessionJson, final JsonObject reqObject, final String resourcePath) throws VtnServiceWebAPIException{
		JSONObject responseJSON = null; 
		String responseStr = null;
		long sessionId = 0;
		long configId = 0;
		final RestResource resource = new RestResource();
		try{
		LOG.debug("acquiring session id and config mode from java API");
		//acquire session id
		resource.setPath(ResourcePathEnum.SESSION_PATH.getPath());
		if(resource.post(sessionJson) != RESPONSE_SUCCESS){
			LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
			throw new VtnServiceWebAPIException(ApplicationConstants.SESSION_CREATION_FAILED, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.SESSION_CREATION_FAILED));
		}
		sessionId = VtnServiceCommonUtil.getSessionFromJson(resource.getInfo());
		//acquire configure mode
		resource.setPath(ResourcePathEnum.ACQUIRE_CONFIG_PATH.getPath());
		resource.setSessionID(sessionId);
		if(resource.post(VtnServiceWebUtil.prepareAquireConfigJSON()) != RESPONSE_SUCCESS){
			LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
			throw new VtnServiceWebAPIException(ApplicationConstants.AQUIRE_CONFIGURATION_FAILED, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.AQUIRE_CONFIGURATION_FAILED));
		}
		configId = VtnServiceCommonUtil.getConfigIdFromJson(resource.getInfo());
		//send the api request json
		resource.setPath(resourcePath);
		resource.setSessionID(sessionId);
		resource.setConfigID(configId);
		LOG.debug("Request object passing to JAVA API  #"+reqObject);
		final int status = resource.post(reqObject);
		LOG.debug("JAVA API returned error code #"+status);
		if(status != RESPONSE_SUCCESS){
			LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
			throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
		}
		responseStr = resource.getInfo() != null? resource.getInfo().toString():null;
		LOG.debug("JAVA API returned response # "+responseStr);
		resource.setPath(ResourcePathEnum.COMMIT_CONFIGURATION.getPath());
		resource.setSessionID(sessionId);
		resource.setConfigID(configId);
		final int commitStatus = resource.put(VtnServiceWebUtil.prepareConfigCommitOrSaveJSON(ApplicationConstants.OPERATION_COMMIT));
		if (commitStatus != RESPONSE_SUCCESS) {
			LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
				throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR,
						ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
			}
		}catch (VtnServiceWebAPIException vtnException) {	
			exceptionStatus = true;
			LOG.error(VtnServiceCommonUtil.logErrorDetails(vtnException.getErrorCode()));
		}catch (RuntimeException exception) {
			exceptionStatus = true;
			LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));	
			throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
		}
	finally{
		try{
			if(configId != ApplicationConstants.ZERO){
				//release monitoring mode
				resource.setPath(ResourcePathEnum.RELEASE_CONFIGURATION.getPath()+configId);
				resource.setSessionID(sessionId);
				resource.setConfigID(configId);
				if(resource.delete() != RESPONSE_SUCCESS){
					LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
					throw new VtnServiceWebAPIException(ApplicationConstants.RELEASE_CONFIGURATION_FAILED, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.RELEASE_CONFIGURATION_FAILED));
				}
			}
			if(sessionId != ApplicationConstants.ZERO){
				//release session
				resource.setPath(ResourcePathEnum.RELEASE_SESSION.getPath()+ sessionId);
				resource.setSessionID(sessionId);
				if(resource.delete() != RESPONSE_SUCCESS){
					LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
					throw new VtnServiceWebAPIException(ApplicationConstants.RELEASE_SESSION_FAILED, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.RELEASE_SESSION_FAILED));
				}
			}
			
			if(null == responseStr && !exceptionStatus){
				responseJSON = VtnServiceWebUtil.prepareErrResponseJson(ApplicationConstants.STATUS_OK, ApplicationConstants.STATUS_SUCCESS);
			}else{
				responseJSON = VtnServiceWebUtil.prepareErrResponseJson(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
			}
		}catch (Exception exception) {
			exceptionStatus = true;
			LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));	
			throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
		}
		
		
	}
	return responseJSON;
	}
	/**
	 * Put.This method will be required when we have to update any component using Java API. 
	 * This method will work in the following sequence-
	 * Create session 
	 * acquire config mode
	 * calling actual Java API which is requested
	 * release config lock
	 * release session
	 * @param sessionJson the session json
	 * @param reqObject the req object
	 * @param resourcePath the resource path
	 * @return the jSON object
	 * @throws VtnServiceWebAPIException the vtn service web api exception
	 */
	public JSONObject put(final JsonObject sessionJson, final JsonObject reqObject, final String resourcePath) throws VtnServiceWebAPIException {
		JSONObject responseJSON = null;
		String responseStr = null;
		long sessionId = 0;
		long configId = 0;
		final RestResource resource = new RestResource();
		try{
			LOG.debug("acquiring session id and config mode from java API");
			//acquire session id
			resource.setPath(ResourcePathEnum.SESSION_PATH.getPath());
			if(resource.post(sessionJson) != RESPONSE_SUCCESS){
				LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
				throw new VtnServiceWebAPIException(ApplicationConstants.SESSION_CREATION_FAILED, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.SESSION_CREATION_FAILED));
			}
			sessionId = VtnServiceCommonUtil.getSessionFromJson(resource.getInfo());
			//acquire configure mode
			resource.setPath(ResourcePathEnum.ACQUIRE_CONFIG_PATH.getPath());
			resource.setSessionID(sessionId);
			if(resource.post(VtnServiceWebUtil.prepareAquireConfigJSON()) != RESPONSE_SUCCESS){
				LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
				throw new VtnServiceWebAPIException(ApplicationConstants.AQUIRE_CONFIGURATION_FAILED, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.AQUIRE_CONFIGURATION_FAILED));
		}
		configId = VtnServiceCommonUtil.getConfigIdFromJson(resource.getInfo());
		//send the api request json
		resource.setPath(resourcePath);
		resource.setConfigID(configId);
		resource.setSessionID(sessionId);
		LOG.debug("Request object passing to JAVA API  #"+reqObject);
		final int status = resource.put(reqObject);
		LOG.debug("JAVA API returned error code #"+status);
		if(status != RESPONSE_SUCCESS){
			LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
			throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
		}
		responseStr = resource.getInfo() != null? resource.getInfo().toString():null;
		LOG.debug("JAVA API returned response # "+responseStr);
		resource.setPath(ResourcePathEnum.COMMIT_CONFIGURATION.getPath());
		resource.setSessionID(sessionId);
		resource.setConfigID(configId);
		final int commitStatus = resource.put(VtnServiceWebUtil.prepareConfigCommitOrSaveJSON(ApplicationConstants.OPERATION_COMMIT));
		if (commitStatus != RESPONSE_SUCCESS) {
			LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
			throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR,
						ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
		}
		}catch (VtnServiceWebAPIException vtnException) {	
			exceptionStatus = true;
			LOG.error(VtnServiceCommonUtil.logErrorDetails(vtnException.getErrorCode()));
		}catch (RuntimeException exception) {
			exceptionStatus = true;
			LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));	
			throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
		}
		finally{
			try{
				if(configId != ApplicationConstants.ZERO){
					//release monitoring mode
					resource.setPath(ResourcePathEnum.RELEASE_CONFIGURATION.getPath()+configId);
					resource.setSessionID(sessionId);
					resource.setConfigID(configId);
					if(resource.delete() != RESPONSE_SUCCESS){
						LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
						throw new VtnServiceWebAPIException(ApplicationConstants.RELEASE_CONFIGURATION_FAILED, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.RELEASE_CONFIGURATION_FAILED));
					}
				}
				if(sessionId != ApplicationConstants.ZERO){
					//release session
					resource.setPath(ResourcePathEnum.RELEASE_SESSION.getPath()+ sessionId);
					resource.setSessionID(sessionId);
					if(resource.delete() != RESPONSE_SUCCESS){
						LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
						throw new VtnServiceWebAPIException(ApplicationConstants.RELEASE_SESSION_FAILED, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.RELEASE_SESSION_FAILED));
					}
				}
				
				if(null == responseStr && !exceptionStatus){
					responseJSON = VtnServiceWebUtil.prepareErrResponseJson(ApplicationConstants.STATUS_OK, ApplicationConstants.STATUS_SUCCESS);
				}else{
					responseJSON = VtnServiceWebUtil.prepareErrResponseJson(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
				}
			}catch (Exception exception) {
				exceptionStatus = true;
				LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));	
				throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
			}
			
		}
		return responseJSON;
	}
	
	/**
	 * Delete.This method will be required when we have to delete any component using Java API. 
	 * This method will work in the following sequence-
	 * Create session 
	 * acquire config mode
	 * calling actual Java API which is requested
	 * release config lock
	 * release session
	 * @param sessionJson the session json
	 * @param reqObject the req object
	 * @param resourcePath the resource path
	 * @return the jSON object
	 * @throws VtnServiceWebAPIException the vtn service web api exception
	 */
	public JSONObject delete(final JsonObject sessionJson, final JsonObject reqObject, final String resourcePath) throws VtnServiceWebAPIException{
		JSONObject responseJSON = null;
		String responseStr = null;
		long sessionId = 0;
		long configId = 0;
		final RestResource resource = new RestResource();
		try{
			LOG.debug("acquiring session id and config mode from java API");
			//acquire session id
			resource.setPath(ResourcePathEnum.SESSION_PATH.getPath());
			if(resource.post(sessionJson) != RESPONSE_SUCCESS){
				LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
				throw new VtnServiceWebAPIException(ApplicationConstants.SESSION_CREATION_FAILED, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.SESSION_CREATION_FAILED));
			}
			sessionId = VtnServiceCommonUtil.getSessionFromJson(resource.getInfo());
			//acquire configure mode
			resource.setPath(ResourcePathEnum.ACQUIRE_CONFIG_PATH.getPath());
			resource.setSessionID(sessionId);
			if(resource.post(VtnServiceWebUtil.prepareAquireConfigJSON()) != RESPONSE_SUCCESS){
				throw new VtnServiceWebAPIException(ApplicationConstants.AQUIRE_CONFIGURATION_FAILED, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.AQUIRE_CONFIGURATION_FAILED));
		}
		configId = VtnServiceCommonUtil.getConfigIdFromJson(resource.getInfo());
		//send the api request json
		resource.setPath(resourcePath);
		resource.setConfigID(configId);
		resource.setSessionID(sessionId);
		LOG.debug("Request object passing to JAVA API  #"+reqObject);
		final int status = resource.delete(reqObject);
		LOG.debug("JAVA API returned error code #"+status);
		if(status != RESPONSE_SUCCESS){
			LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
			throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
		}
		responseStr = resource.getInfo() != null? resource.getInfo().toString():null;
		LOG.debug("JAVA API returned response # "+responseStr);
		resource.setPath(ResourcePathEnum.COMMIT_CONFIGURATION.getPath());
		resource.setSessionID(sessionId);
		resource.setConfigID(configId);
		final int commitStatus = resource.put(VtnServiceWebUtil.prepareConfigCommitOrSaveJSON(ApplicationConstants.OPERATION_COMMIT));
		if (commitStatus != RESPONSE_SUCCESS) {
			LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
			throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR,
						ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
		}
		}catch (VtnServiceWebAPIException vtnException) {	
			exceptionStatus = true;
			LOG.error(VtnServiceCommonUtil.logErrorDetails(vtnException.getErrorCode()));
		}catch (Exception exception) {
			exceptionStatus = true;
			LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));	
			throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
		}
		finally{
			try{
				if(configId != ApplicationConstants.ZERO){
					//release monitoring mode
					resource.setPath(ResourcePathEnum.RELEASE_CONFIGURATION.getPath()+configId);
					resource.setSessionID(sessionId);
					resource.setConfigID(configId);
					if(resource.delete() != RESPONSE_SUCCESS){
						LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
						throw new VtnServiceWebAPIException(ApplicationConstants.RELEASE_CONFIGURATION_FAILED, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.RELEASE_CONFIGURATION_FAILED));
					}
				}
				if(sessionId != ApplicationConstants.ZERO){
					//release session
					resource.setPath(ResourcePathEnum.RELEASE_SESSION.getPath()+ sessionId);
					resource.setSessionID(sessionId);
					if(resource.delete() != RESPONSE_SUCCESS){
						LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
						throw new VtnServiceWebAPIException(ApplicationConstants.RELEASE_SESSION_FAILED, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.RELEASE_SESSION_FAILED));
					}
				}
				if(null == responseStr && !exceptionStatus){
					responseJSON = VtnServiceWebUtil.prepareErrResponseJson(ApplicationConstants.STATUS_OK, ApplicationConstants.STATUS_SUCCESS);
				}else{
					responseJSON = VtnServiceWebUtil.prepareErrResponseJson(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
				}
			}catch (Exception exception) {
				exceptionStatus = true;
				LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));	
				throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
			}
			
		}
		return responseJSON;
	}
	
	/**
	 * Delete.This method will be required when we have to delete any component using Java API. 
	 * This method will work in the following sequence-
	 * Create session 
	 * acquire config mode
	 * calling actual Java API which is requested
	 * release config lock
	 * release session
	 * @param sessionJson the session json
	 * @param resourcePath the resource path
	 * @return the jSON object
	 * @throws VtnServiceWebAPIException the vtn service web api exception
	 */
	public JSONObject delete(final JsonObject sessionJson,  final String resourcePath) throws VtnServiceWebAPIException {
		JSONObject responseJSON = null;
		String responseStr = null;
		long sessionId = 0;
		long configId = 0;
		final RestResource resource = new RestResource();
		try{
			LOG.debug("acquiring session id and config mode from java API");
			//acquire session id
			resource.setPath(ResourcePathEnum.SESSION_PATH.getPath());
			if(resource.post(sessionJson) != RESPONSE_SUCCESS){
				LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
				throw new VtnServiceWebAPIException(ApplicationConstants.SESSION_CREATION_FAILED, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.SESSION_CREATION_FAILED));
			}
			sessionId = VtnServiceCommonUtil.getSessionFromJson(resource.getInfo());
			//acquire configure mode
			resource.setPath(ResourcePathEnum.ACQUIRE_CONFIG_PATH.getPath());
			resource.setSessionID(sessionId);
			if(resource.post(VtnServiceWebUtil.prepareAquireConfigJSON()) != RESPONSE_SUCCESS){
				LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
				throw new VtnServiceWebAPIException(ApplicationConstants.AQUIRE_CONFIGURATION_FAILED, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.AQUIRE_CONFIGURATION_FAILED));
		}
		configId = VtnServiceCommonUtil.getConfigIdFromJson(resource.getInfo());
		//send the api request json
		resource.setPath(resourcePath);
		resource.setConfigID(configId);
		resource.setSessionID(sessionId);
		final int status = resource.delete();
		LOG.debug("JAVA API returned error code #"+status);
		if(status != RESPONSE_SUCCESS){
			LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
			throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
		}
		responseStr = resource.getInfo() != null? resource.getInfo().toString():null;
		LOG.debug("JAVA API returned response # "+responseStr);
		resource.setPath(ResourcePathEnum.COMMIT_CONFIGURATION.getPath());
		resource.setSessionID(sessionId);
		resource.setConfigID(configId);
		final int commitStatus = resource.put(VtnServiceWebUtil.prepareConfigCommitOrSaveJSON(ApplicationConstants.OPERATION_COMMIT));
		if (commitStatus != RESPONSE_SUCCESS) {
			LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
			throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR,
						ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
		}
		}catch (VtnServiceWebAPIException vtnException) {
			exceptionStatus = true;
			LOG.error(VtnServiceCommonUtil.logErrorDetails(vtnException.getErrorCode()));
		}catch (RuntimeException exception) {
			exceptionStatus = true;
			LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));	
			throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
		}
		finally{
			try{
				if(configId != ApplicationConstants.ZERO){
					//release monitoring mode
					resource.setPath(ResourcePathEnum.RELEASE_CONFIGURATION.getPath()+configId);
					resource.setSessionID(sessionId);
					resource.setConfigID(configId);
					if(resource.delete() != RESPONSE_SUCCESS){
						LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
						throw new VtnServiceWebAPIException(ApplicationConstants.RELEASE_CONFIGURATION_FAILED, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.RELEASE_CONFIGURATION_FAILED));
					}
				}
				if(sessionId != ApplicationConstants.ZERO){
					//release session
					resource.setPath(ResourcePathEnum.RELEASE_SESSION.getPath()+ sessionId);
					resource.setSessionID(sessionId);
					if(resource.delete() != RESPONSE_SUCCESS){
						LOG.error("JAVA API returns error # "+resource.getInfo().getAsString());
						throw new VtnServiceWebAPIException(ApplicationConstants.RELEASE_SESSION_FAILED, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.RELEASE_SESSION_FAILED));
					}
				}
				if(null == responseStr && !exceptionStatus){
					responseJSON = VtnServiceWebUtil.prepareErrResponseJson(ApplicationConstants.STATUS_OK, ApplicationConstants.STATUS_SUCCESS);
				}else{
					responseJSON = VtnServiceWebUtil.prepareErrResponseJson(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
				}
			}catch (Exception exception) {
				exceptionStatus = true;
				LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));	
				throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
			}
			
		}
		return responseJSON;
	}
	
	
}
