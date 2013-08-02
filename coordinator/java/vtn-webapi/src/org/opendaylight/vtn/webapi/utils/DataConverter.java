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
import java.util.Map;
import java.util.Set;

import org.json.JSONException;
import org.json.JSONObject;
import org.json.XML;

import com.google.gson.JsonObject;
import com.google.gson.JsonParser;
import com.google.gson.JsonSyntaxException;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.webapi.enums.ApplicationConstants;
import org.opendaylight.vtn.webapi.enums.ContentTypeEnum;
import org.opendaylight.vtn.webapi.exception.VtnServiceWebAPIException;

/**
 * The Class DataConverter.This class will be used for conversion part of request and response as well
 * If the request will be in format of XML then needs to convert it in JSON otherwise the same JSON will be
 * passed as request object
 * 
 */
public final class DataConverter {

	/** The Constant LOG. */
	private static final Logger LOG = Logger.getLogger(DataConverter.class.getName());
	
	/**
	 * Instantiates a new data converter.
	 */
	private DataConverter() {
	}
	
	/**
	 * Convert data to json.
	 *
	 * @param rawRequestString the raw data
	 * @param contentType the data type
	 * @return the json object
	 * @throws VtnServiceWebAPIException the vtn service web api exception
	 */
	
	
	public static JsonObject getConvertedRequestObject(final String rawRequestString, final String contentType) throws VtnServiceWebAPIException{
		JsonObject convertedObj=null;
		final JsonParser parser = new JsonParser(); 
		try{
		LOG.trace("converting request string to JSON Object #getConvertedRequestObject()");
		if(ContentTypeEnum.APPLICATION_XML.getContentType().equals(contentType)){
			org.json.JSONObject jsonObject =  XML.toJSONObject(rawRequestString);
			convertedObj =  (JsonObject) parser.parse(jsonObject.toString());
		}else{
			convertedObj =  (JsonObject) parser.parse(rawRequestString);
		}
		}catch (JsonSyntaxException jsonSyntaxException) {
			LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.BAD_REQUEST_ERROR));
			throw new VtnServiceWebAPIException(ApplicationConstants.BAD_REQUEST_ERROR, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.BAD_REQUEST_ERROR));
		} catch (JSONException e) {
			LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.BAD_REQUEST_ERROR));
			throw new VtnServiceWebAPIException(ApplicationConstants.BAD_REQUEST_ERROR, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.BAD_REQUEST_ERROR));
		}
		LOG.trace("converting request string to JSON Object completed #getConvertedRequestObject()");
		LOG.trace("Request JSON object for Java API #" +convertedObj);
		return convertedObj;
	}
	
	/**
	 * Convert map to json.
	 *
	 * @param queryStringMap the query string map
	 * @return the json object
	 * @throws VtnServiceWebAPIException 
	 */
	public static JsonObject convertMapToJson(final Map<String, String[]> queryStringMap, final JsonObject serviceRequest) throws VtnServiceWebAPIException{
		LOG.trace("converting query String Map to JSON Object #convertMapToJson()");
		final Set<String> queryStringKeySet = queryStringMap.keySet(); 
		JSONObject json = null;	
		try{
			json =  new JSONObject(serviceRequest.toString());
		for (String key : queryStringKeySet) {
			final String[] valueList = queryStringMap.get(key);
			if(null != valueList && valueList.length > ApplicationConstants.ONE){
				json.put(key, Arrays.asList(valueList));
			}else{
				json.put(key, valueList[ApplicationConstants.ZERO]);
			}
		}
		}catch (JSONException je) {
			LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
			throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.INTERNAL_SERVER_ERROR));
		}
		LOG.trace("converting query String Map to JSON Object completed#convertMapToJson()");
		return (JsonObject) new JsonParser().parse(json.toString());
		
	}
	
	/**
	 * Convert json to xml.
	 *
	 * @param responseJson the response json
	 * @param requiredContentType the required content type
	 * @return the string
	 * @throws VtnServiceWebAPIException 
	 */
	public static String getConvertedResponse(final JSONObject responseJson, final String requiredContentType) throws VtnServiceWebAPIException {
		String responseString = null;
		LOG.trace("converting response JSON to String #getConvertedResponse()");
		try{
		if(null != responseJson){
			responseString = responseJson.toString();
			if(ContentTypeEnum.APPLICATION_XML.getContentType().equals(requiredContentType)){
				responseString = XML.toString(responseJson);
			}
		}
		}catch (JSONException e) {
			LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
			throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.INTERNAL_SERVER_ERROR));
		}catch (Exception e) {
			LOG.error(VtnServiceCommonUtil.logErrorDetails(ApplicationConstants.INTERNAL_SERVER_ERROR));
			throw new VtnServiceWebAPIException(ApplicationConstants.INTERNAL_SERVER_ERROR, VtnServiceCommonUtil.getErrorDescription(ApplicationConstants.INTERNAL_SERVER_ERROR));
		}
		LOG.trace("converting response JSON to String completed #getConvertedResponse()");
		return responseString;
	}
}
