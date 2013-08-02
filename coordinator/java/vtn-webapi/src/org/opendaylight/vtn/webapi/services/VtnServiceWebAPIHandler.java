/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.webapi.services;

import java.io.IOException;
import java.util.Map;

import javax.servlet.http.HttpServletRequest;

import org.json.JSONObject;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.webapi.enums.ApplicationConstants;
import org.opendaylight.vtn.webapi.exception.VtnServiceWebAPIException;
import org.opendaylight.vtn.webapi.utils.DataConverter;
import org.opendaylight.vtn.webapi.utils.VtnServiceCommonUtil;
import org.opendaylight.vtn.webapi.utils.VtnServiceWebUtil;


/**
 * The Class VtnServiceWebAPIHandler.This class is defined at the handler layer where the actual request 
 * object is prepared and validated for the correctness.This class will contain get, post, put , validate and delete 
 */
public class VtnServiceWebAPIHandler {
	/** The Constant LOGGER. */
	private static final Logger LOG = Logger.getLogger(VtnServiceWebAPIHandler.class.getName());
	
	/** The vtn service web api controller. */
	private final VtnServiceWebAPIController vtnServiceWebAPIController;
	
	/** The resource path. */
	private String resourcePath;
	
	/** The content type. */
	private String contentType;
	
	
	/**
	 * Instantiates a new vtn service web api controller and latter it will be used for calling the method written for java api call.
	 */
	public VtnServiceWebAPIHandler() {
		LOG.trace("Initializing web api handler #VtnServiceWebAPIHandler()");
		this.vtnServiceWebAPIController = new VtnServiceWebAPIController();
		LOG.trace("Web api handler initialized successfully #VtnServiceWebAPIHandler()");
	}

	/**
	 * Get. This method will convert the httprequest to JSON format and then will validate the same for json syntax.
	 * At the same time the count and detail information will be removed from resource path 
	 * it will also decide whether the call is for JSON/XML by passed URI format. 
	 * After getting the response it will convert the same in either format JSON/XML
	 * @param request the request
	 * @return the string
	 * @throws VtnServiceWebAPIException the vtn service web api exception
	 */
	public String get(final HttpServletRequest request) throws VtnServiceWebAPIException{
		JSONObject serviceResponse = null;
		JsonObject serviceRequest = new JsonObject();
		String responseString = null;
		LOG.trace("GET method of web api handler starts #get()");
		this.resourcePath = VtnServiceCommonUtil.getResourceURI(request.getRequestURI());
		this.contentType = VtnServiceCommonUtil.getContentType(request.getRequestURI());
		final Map<String, String[]> paramsMap = request.getParameterMap();
		serviceRequest = VtnServiceCommonUtil.getOpParameter(serviceRequest, resourcePath);
		this.resourcePath = VtnServiceCommonUtil.removeCountOrDetailFromURI(this.resourcePath);
		if (null != paramsMap && !paramsMap.isEmpty()) {
			serviceRequest = DataConverter.convertMapToJson(paramsMap, serviceRequest);
			serviceResponse = vtnServiceWebAPIController.get(VtnServiceWebUtil.prepareHeaderJson(request), serviceRequest,
					this.resourcePath);
		} else {
			if (VtnServiceCommonUtil.validateGetAPI(this.resourcePath)) {
				serviceResponse = vtnServiceWebAPIController.get(VtnServiceWebUtil.prepareHeaderJson(request), this.resourcePath);
			} else {
				serviceResponse = vtnServiceWebAPIController.get(VtnServiceWebUtil.prepareHeaderJson(request), serviceRequest,
						this.resourcePath);
			}
		}
		responseString = DataConverter.getConvertedResponse(serviceResponse, contentType);
		LOG.trace("GET method of web api handler ends #get()");

		return responseString;
	}


	
	/**
	 * Post. This method will convert the httprequest to JSON format and then will validate the same for json syntax 
	 * it will also decide whether the call is for JSON/XML by passed URI format.
	 * will prepare the session json also to take session for api call later. 
	 * After getting the response it will convert the same in either format JSON/XML
	 * @param request the request
	 * @return the string
	 * @throws VtnServiceWebAPIException the vtn service web api exception
	 */
	public String post(final HttpServletRequest request) throws VtnServiceWebAPIException{
		JSONObject serviceResponse = null;
		JsonObject serviceRequest = null;
		String responseString = null;
		try{
			LOG.trace("POST method of web api handler starts #post()");
			this.resourcePath = VtnServiceCommonUtil.getResourceURI(request.getRequestURI());
			this.contentType =  VtnServiceCommonUtil.getContentType(request.getRequestURI());
			serviceRequest = new VtnServiceWebUtil().prepareRequestJson(request, contentType);
			JsonObject sessionJson = VtnServiceWebUtil.prepareHeaderJson(request);
			serviceResponse =  vtnServiceWebAPIController.post(sessionJson, serviceRequest, this.resourcePath);
			responseString = DataConverter.getConvertedResponse(serviceResponse, contentType);
			LOG.trace("POST method of web api handler ends #post()");
		} catch (IOException e) {			
			LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
			throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
		}
		
		return responseString;
	}


	/**
	 * Put. This method will convert the httprequest to JSON format and then will validate the same for json syntax 
	 * it will also decide whether the call is for JSON/XML by passed URI format.
	 * will prepare the session json also to take session for api call later. 
	 * After getting the response it will convert the same in either format JSON/XML
	 * @param request the request
	 * @return the string
	 * @throws VtnServiceWebAPIException the vtn service web api exception
	 */
	public String put(final HttpServletRequest request) throws VtnServiceWebAPIException{
		JSONObject serviceResponse = null;
		JsonObject serviceRequest = null;
		String responseString = null;
		try{
			LOG.trace("PUT method of web api handler starts #put()");
			this.resourcePath = VtnServiceCommonUtil.getResourceURI(request.getRequestURI());
			this.contentType =  VtnServiceCommonUtil.getContentType(request.getRequestURI());
			serviceRequest = new VtnServiceWebUtil().prepareRequestJson(request, contentType);
			serviceResponse =  vtnServiceWebAPIController.put(VtnServiceWebUtil.prepareHeaderJson(request) ,serviceRequest, this.resourcePath);
			responseString = DataConverter.getConvertedResponse(serviceResponse, contentType);
			LOG.trace("PUT method of web api handler ends #put()");
		} catch (IOException e) {			
			LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
			throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR, ApplicationConstants.DEFAULT_ERROR_DESCRIPTION);
		} 
			return responseString;
		}


	/**
	 * Delete. This method will convert the httprequest to JSON format and then will validate the same for json syntax 
	 * it will also decide whether the call is for JSON/XML by passed URI format.
	 * will prepare the session json also to take session for api call later. 
	 * After getting the response it will convert the same in either format JSON/XML
	 * @param request the request
	 * @return the string
	 * @throws VtnServiceWebAPIException the vtn service web api exception
	 */
	public String delete(final HttpServletRequest request) throws VtnServiceWebAPIException{
		JSONObject serviceResponse = null;
		JsonObject serviceRequest = new JsonObject();
		String responseString = null;			
			LOG.trace("DELETE method of web api handler starts #delete()");
			this.resourcePath = VtnServiceCommonUtil.getResourceURI(request.getRequestURI());
			this.contentType =  VtnServiceCommonUtil.getContentType(request.getRequestURI());
			Map<String, String[]> paramsMap = request.getParameterMap();
			if(null != paramsMap && !paramsMap.isEmpty()){
				serviceRequest = DataConverter.convertMapToJson(paramsMap, serviceRequest);
				serviceResponse =  vtnServiceWebAPIController.delete(VtnServiceWebUtil.prepareHeaderJson(request), serviceRequest, this.resourcePath);
			}else{
				serviceResponse =  vtnServiceWebAPIController.delete(VtnServiceWebUtil.prepareHeaderJson(request), this.resourcePath);
			}
			responseString = DataConverter.getConvertedResponse(serviceResponse, contentType);
			LOG.trace("DELETE method of web api handler ends#delete()");
		return responseString;
	}	
	
	/**
	 * Validate.This method will validate for the header part coming with the request and check for the authentication and authorization
	 * It will also check for valid URI, if URI is not valid then it will fail the call and return forbidden error.
	 *
	 * @param request the request
	 * @throws VtnServiceWebAPIException the vtn service web api exception
	 */
	public void validate(final HttpServletRequest request) throws VtnServiceWebAPIException {	
			LOG.trace("validate method of web api handler starts #validate()");
			if(!VtnServiceCommonUtil.validateURI(request.getRequestURI(), request.getContentType())){
				throw new VtnServiceWebAPIException(ApplicationConstants.FORBIDDEN_ERROR, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.FORBIDDEN_ERROR));
			}
			final JsonObject headerInfo  = VtnServiceWebUtil.prepareHeaderJson(request);
			if(!VtnServiceCommonUtil.authenticateUser(headerInfo, request.getMethod())){
				throw new VtnServiceWebAPIException(ApplicationConstants.USER_UNAUTHORISED_ERROR, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.USER_UNAUTHORISED_ERROR));
			}
			LOG.trace("validate method of web api handler ends #validate()");

	}
	
	
}
