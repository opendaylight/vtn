/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.webapi.utils;

import java.util.Arrays;
import java.util.Map;
import java.util.NoSuchElementException;
import java.util.Random;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.json.JSONException;
import org.json.JSONObject;
import org.json.XML;

import com.google.gson.JsonObject;
import com.google.gson.JsonParser;
import com.google.gson.JsonSyntaxException;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.webapi.constants.ApplicationConstants;
import org.opendaylight.vtn.webapi.enums.ContentTypeEnum;
import org.opendaylight.vtn.webapi.enums.HttpErrorCodeEnum;
import org.opendaylight.vtn.webapi.exception.VtnServiceWebAPIException;

/**
 * The Class DataConverter.This class will be used for conversion part of
 * request and response as well If the request will be in format of XML then
 * needs to convert it in JSON otherwise the same JSON will be passed as request
 * object
 * 
 */
public final class DataConverter {

	/** The Constant LOG. */
	private static final Logger LOG = Logger.getLogger(DataConverter.class
			.getName());

	/**
	 * Instantiates a new data converter.
	 */
	private DataConverter() {
	}

	/**
	 * Convert data to json.
	 * 
	 * @param rawRequestString
	 *            the raw data
	 * @param contentType
	 *            the data type
	 * @return the json object
	 * @throws VtnServiceWebAPIException
	 *             the vtn service web api exception
	 */
	public static JsonObject getConvertedRequestObject(String rawRequestString,
			final String contentType) throws VtnServiceWebAPIException {
		LOG.trace("Start DataConverter#getConvertedRequestObject()");
		JsonObject convertedObj = null;
		final JsonParser parser = new JsonParser();
		try {

			String mime = "";
			if (contentType != null) {
				String mimeArray[] = contentType.split(ApplicationConstants.SEMI_COLON);
				mime = mimeArray[0];
			}

			if (ContentTypeEnum.APPLICATION_XML.getContentType().equals(mime) ||
			    ContentTypeEnum.APPLICATION_XML_SCVMM.getContentType().equals(mime)) {
				final Random random = new Random();
				final String randomString = String.valueOf(random.nextInt());
				final String randomStringSpace = String.valueOf(random
						.nextInt());

				rawRequestString = replaceSpaceAll(rawRequestString,
						randomStringSpace);
				rawRequestString = rawRequestString.replaceAll(
						ApplicationConstants.DOT_ZERO, randomString
								+ ApplicationConstants.SESSION_TYPE);
				final org.json.JSONObject jsonObject = XML
						.toJSONObject(rawRequestString);
				if (ApplicationConstants.vtepgroup.equals(jsonObject.keys()
						.next().toString())) {
					XMLTransformationUtil.preProcessJson(jsonObject);
				}
				LOG.debug("Json before parsing : " + jsonObject);

				String jsonString = jsonObject.toString().replaceAll(
						randomString + ApplicationConstants.SESSION_TYPE,
						ApplicationConstants.DOT_ZERO);
				jsonString = jsonString.replaceAll(randomStringSpace
						+ ApplicationConstants.SESSION_TYPE,
						ApplicationConstants.SPACE_STRING);

				convertedObj = (JsonObject) parser.parse(jsonString);
			} else if (ContentTypeEnum.APPLICATION_JSON.getContentType()
					.equals(mime)) {
				convertedObj = (JsonObject) parser.parse(rawRequestString);
			} else {
				LOG.error("Content-Type is not valid");
				throw new VtnServiceWebAPIException(
						HttpErrorCodeEnum.UNC_BAD_REQUEST.getCode());
			}
			LOG.debug("Request JSON object for Java API #" + convertedObj);
		} catch (JsonSyntaxException e) {
			LOG.error(e, "Request Syntax Error : " + e);
			throw new VtnServiceWebAPIException(
					HttpErrorCodeEnum.UNC_BAD_REQUEST.getCode());
		} catch (JSONException e) {
			LOG.error(e, "Request Syntax Error : " + e);
			throw new VtnServiceWebAPIException(
					HttpErrorCodeEnum.UNC_BAD_REQUEST.getCode());
		} catch (NoSuchElementException e) {
			LOG.error(e, "Request Syntax Error : " + e);
			throw new VtnServiceWebAPIException(
					HttpErrorCodeEnum.UNC_BAD_REQUEST.getCode());
		}
		LOG.trace("Complete DataConverter#getConvertedRequestObject()");
		return convertedObj;
	}

	/**
	 * Convert map to json.
	 * 
	 * @param queryStringMap
	 *            the query string map
	 * @return the json object
	 * @throws VtnServiceWebAPIException
	 */
	public static JsonObject convertMapToJson(
			final Map<String, String[]> queryStringMap,
			final JsonObject serviceRequest) throws VtnServiceWebAPIException {
		LOG.trace("Start DataConverter#convertMapToJson()");
		JsonObject mapJson = null;
		final Set<String> queryStringKeySet = queryStringMap.keySet();
		JSONObject json = null;
		try {
			json = new JSONObject(serviceRequest.toString());
			for (final String key : queryStringKeySet) {
				final String[] valueList = queryStringMap.get(key);
				if (valueList.length == 1
						&& valueList[0]
								.equalsIgnoreCase(ApplicationConstants.BLANK_STR)) {
					LOG.error("URI parameter containing no values : " + key);
					throw new VtnServiceWebAPIException(
							HttpErrorCodeEnum.UNC_BAD_REQUEST.getCode());
				}
				if (key.equalsIgnoreCase(ApplicationConstants.OP)
						|| key.equalsIgnoreCase(ApplicationConstants.TARGETDB)) {
					LOG.debug("Parameter not required to be added for : " + key);
				} else {
					if (null != valueList
							&& valueList.length > ApplicationConstants.ONE) {
						json.put(key, Arrays.asList(valueList));
					} else {
						json.put(key, valueList[ApplicationConstants.ZERO]);
					}
				}
			}
			mapJson = (JsonObject) new JsonParser().parse(json.toString());
			LOG.debug("Coverted JSON from Map parameters : " + mapJson);
		} catch (final JSONException e) {
			LOG.error(e, "Json syntax error : " + e.getMessage());
			throw new VtnServiceWebAPIException(
					HttpErrorCodeEnum.UNC_INTERNAL_SERVER_ERROR.getCode());
		}
		LOG.trace("Complete DataConverter#convertMapToJson()");
		return mapJson;
	}

	/**
	 * Convert json to xml.
	 * 
	 * @param responseJson
	 *            the response json
	 * @param requiredContentType
	 *            the required content type
	 * @return the string
	 * @throws VtnServiceWebAPIException
	 */
	public static String getConvertedResponse(final JSONObject responseJson,
			final String requiredContentType) throws VtnServiceWebAPIException {
		LOG.trace("Start DataConverter#getConvertedResponse()");
		String responseString = null;
		LOG.debug("Json : " + responseJson + " Content-type : "
				+ requiredContentType);
		try {
			if (null != responseJson) {
				responseString = responseJson.toString();
				// conversion is required only for XML type response type
				if (ContentTypeEnum.APPLICATION_XML.getContentType().equals(
						requiredContentType) ||
						ContentTypeEnum.APPLICATION_XML_SCVMM.getContentType().equals(
						requiredContentType)) {
					// modify the json object to remove null and empty nested
					// json and arrays
					final JSONObject modifiedJson = new JSONObject(responseJson
							.toString()
							.replace(ApplicationConstants.EMPTY_JSON,
									ApplicationConstants.DUMMY_JSON)
							.replace(ApplicationConstants.EMPTY_JSON_ARRAY,
									ApplicationConstants.DUMMY_JSON));
					LOG.debug("Modified Json : " + modifiedJson.toString());
					final String xml = XMLTransformationUtil
							.convertJsonToXml(modifiedJson);
					LOG.debug("Converted XML : " + xml);
					// remove non-required dummy xml attributes
					responseString = XMLTransformationUtil
							.convertAllAttributesToElements(xml).replace(
									ApplicationConstants.DUMMY_XML,
									ApplicationConstants.BLANK_STR);
				}
			}
			LOG.debug("Response String : " + responseString);
		} catch (final Exception e) {
			LOG.error(e, "Internal server error occurred : " + e);
			throw new VtnServiceWebAPIException(
					HttpErrorCodeEnum.UNC_INTERNAL_SERVER_ERROR.getCode());
		}
		LOG.trace("Complete DataConverter#getConvertedResponse()");
		return responseString;
	}

	/**
	 * replace Space which in the end.
	 * 
	 * @param String
	 *            the xml string.
	 * @param String
	 *            the xml string.
	 * @return the string
	 * 
	 */
	private static String replaceSpaceAll(String xml, String randomString) {
		Pattern p = Pattern.compile("\"(.*?)\"");
		Matcher m = p.matcher(xml);
		while (m.find()) {
			String old = m.group();
			String new_xml = old.replaceFirst(ApplicationConstants.SPACE_STRING
					+ ApplicationConstants.QUOTATION_MARK_STRING, randomString
					+ ApplicationConstants.SESSION_TYPE
					+ ApplicationConstants.QUOTATION_MARK_STRING);
			xml = xml.replaceFirst(old, new_xml);
		}
		return xml;
	}
}
