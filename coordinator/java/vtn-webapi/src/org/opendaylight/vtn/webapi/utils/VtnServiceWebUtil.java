/*
 * Copyright (c) 2012-2014 NEC Corporation
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

import org.apache.commons.codec.binary.Base64;
import org.json.JSONException;
import org.json.JSONObject;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.webapi.constants.ApplicationConstants;
import org.opendaylight.vtn.webapi.enums.HttpErrorCodeEnum;
import org.opendaylight.vtn.webapi.enums.SessionEnum;
import org.opendaylight.vtn.webapi.exception.VtnServiceWebAPIException;

/**
 * The Class VtnServiceWebUtil.
 */
public class VtnServiceWebUtil {

	/** The Constant LOG. */
	private static final Logger LOG = Logger.getLogger(VtnServiceWebUtil.class
			.getName());

	/**
	 * Prepare request json.
	 * 
	 * @param request
	 *            the request
	 * @param contentType
	 *            the content type
	 * @return the json object
	 * @throws IOException
	 *             Signals that an I/O exception has occurred.
	 * @throws VtnServiceWebAPIException
	 *             the vtn service web api exception
	 */
	public JsonObject prepareRequestJson(final HttpServletRequest request,
			final String contentType) throws IOException,
			VtnServiceWebAPIException {
		LOG.trace("Start VtnServiceWebUtil#prepareRequestJson()");
		JsonObject serviceRequest = null;
		StringBuilder requestStr = null;
		BufferedReader reader = request.getReader();
			requestStr = new StringBuilder();
			String line = reader.readLine();
			while (line != null) {
				requestStr.append(line).append(ApplicationConstants.NEW_LINE);
				line = reader.readLine();
			}
		reader.close();
		/*
		 * exceptional case handle for router-creation operation. 
		 * POST is allowed with no request body
		 */
		if (VtnServiceCommonUtil.getResourceURI(request.getRequestURI())
						.endsWith(ApplicationConstants.ROUTERS)) {
			requestStr = new StringBuilder();
			requestStr.append(ApplicationConstants.EMPTY_JSON);
		}

		try {
			serviceRequest = DataConverter.getConvertedRequestObject(
					requestStr.toString(), contentType);
		} catch (ClassCastException e) {
			/*
			 * exceptional case handle for tenant-creation operation. POST is
			 * allowed with no request body
			 */
			if (VtnServiceCommonUtil.getResourceURI(request.getRequestURI())
					.endsWith(ApplicationConstants.TENANTS)) {
				requestStr = new StringBuilder();
				requestStr.append(ApplicationConstants.EMPTY_JSON);
				serviceRequest = DataConverter.getConvertedRequestObject(
						requestStr.toString(), contentType);
			} else {
				throw e;
			}
		}
		LOG.debug("Request String : " + requestStr.toString());
		
		LOG.debug("Request Json : " + serviceRequest);
		LOG.trace("Complete VtnServiceWebUtil#prepareRequestJson()");
		return serviceRequest;

	}

	/**
	 * Prepare header json.
	 * 
	 * @param request
	 *            the request
	 * @return the json object
	 * @throws VtnServiceWebAPIException
	 */
	public static JsonObject
			prepareHeaderJson(final HttpServletRequest request)
					throws VtnServiceWebAPIException {
		LOG.trace("Start VtnServiceWebUtil#prepareHeaderJson()");
		final JsonObject headerJson = new JsonObject();
		
		/*
		 * check if Authorization header is available or not
		 * If available then decode username and password, if
		 * not available then check for header fields for 
		 * username and password
		 */
		final String authrizationHeader = request
				.getHeader(ApplicationConstants.HTTP_AUTHERIZATION);
		if (authrizationHeader != null
				&& authrizationHeader
						.startsWith(ApplicationConstants.AUTHERIZATION_BASIC)) {
			// Authorization: Basic base64credentials
			String base64Credentials = authrizationHeader.substring(
					ApplicationConstants.AUTHERIZATION_BASIC.length()).trim();
			String credentials = new String(
					Base64.decodeBase64(base64Credentials));
			// credentials = username:password
			final String[] values = credentials
					.split(ApplicationConstants.COLON);
			if (values.length == 2) {
				headerJson.addProperty(
						SessionEnum.USERNAME.getSessionElement(), values[0]);
				headerJson.addProperty(
						SessionEnum.PASSWORD.getSessionElement(), values[1]);
			}
		} else {
			final Enumeration<?> headerEnum = request.getHeaderNames();
			if (null != headerEnum) {
				while (headerEnum.hasMoreElements()) {
					final String nextElement = (String) headerEnum
							.nextElement();
					if (SessionEnum.USERNAME.getSessionElement().equals(
							nextElement)
							|| SessionEnum.PASSWORD.getSessionElement().equals(
									nextElement)) {
						headerJson.addProperty(nextElement,
								request.getHeader(nextElement));
					}
				}
			}
		}

		/*
		 * Set user-name and password, if request came from NEC OpenFlow Plugin
		 */
		setOpenStackAuthentications(request, headerJson);

		headerJson.addProperty(SessionEnum.IPADDRESS.getSessionElement(),
				request.getRemoteAddr());
		headerJson.addProperty(ApplicationConstants.TYPE,
				ApplicationConstants.SESSION_TYPE);
		final JsonObject sessionJson = new JsonObject();
		sessionJson.add(ApplicationConstants.SESSION, headerJson);
		LOG.debug("Session Json : " + sessionJson);
		LOG.trace("Complete VtnServiceWebUtil#prepareHeaderJson()");
		return sessionJson;
	}

	/**
	 * Set/override the user-name and password for the request, if it came from
	 * NEC OpenFlow Plugin for OpenStack APIs
	 * 
	 * @param request
	 *            - Http Request Object
	 * @param headerJson
	 *            - Session Json object
	 * @throws VtnServiceWebAPIException
	 */
	private static void setOpenStackAuthentications(
			final HttpServletRequest request, final JsonObject headerJson)
			throws VtnServiceWebAPIException {
		LOG.trace("Start VtnServiceWebUtil#setOpenStackAuthentications");
		/*
		 * If resource is related with OpenStack API resources then add/update
		 * user-name and password
		 */
		if (VtnServiceCommonUtil.isOpenStackResurce(request)) {
			if (request.getMethod().equalsIgnoreCase(
					ApplicationConstants.GET_METHOD_NAME)) {
				LOG.debug("Set username and password for oper user");
				// Set for oper user in case of GET operations
				headerJson.addProperty(
						SessionEnum.USERNAME.getSessionElement(),
						ApplicationConstants.ROLE_OPERATOR);
				headerJson.addProperty(
						SessionEnum.PASSWORD.getSessionElement(),
						ApplicationConstants.DEFAULT_PASSWD);
			} else {
				LOG.debug("Set username and password for admin user");
				// Set for admin in case of operations other than GET
				headerJson.addProperty(
						SessionEnum.USERNAME.getSessionElement(),
						ApplicationConstants.ROLE_ADMIN);
				headerJson.addProperty(
						SessionEnum.PASSWORD.getSessionElement(),
						ApplicationConstants.DEFAULT_PASSWD);
			}
		}
		LOG.debug("Session Json : " + headerJson);
		LOG.trace("Start VtnServiceWebUtil#setOpenStackAuthentications");
	}

	/**
	 * Prepare aquire config json.
	 * 
	 * @return the json object
	 * @throws VtnServiceWebAPIException
	 *             the vtn service web api exception
	 */
	public static JsonObject prepareAquireConfigJSON()
			throws VtnServiceWebAPIException {
		LOG.trace("Start VtnServiceWebUtil#prepareAquireConfigJSON()");
		final JsonObject configJson = new JsonObject();
		final String opForce = ConfigurationManager.getInstance()
				.getConfProperty(ApplicationConstants.OP);
		if (!opForce.isEmpty()) {
			configJson.addProperty(ApplicationConstants.OP, opForce);
		}

		/* Set timeout. */
		String timeout = ConfigurationManager.getInstance()
				.getConfProperty(ApplicationConstants.CFG_MODE_TIMEOUT);
		if (timeout != null && !timeout.isEmpty()) {
			CommonValidator validator = new CommonValidator();
			long min = Integer.MIN_VALUE;
			long max = Integer.MAX_VALUE;
			try {
				if (!validator.isValidRange(timeout, min, max)) {
					timeout = ApplicationConstants.CFG_TIMEOUT_DEFAULT;
				}
			} catch (Exception e) {
				timeout = ApplicationConstants.CFG_TIMEOUT_DEFAULT;
			}
		} else {
			timeout = ApplicationConstants.CFG_TIMEOUT_DEFAULT;
		}
		configJson.addProperty(ApplicationConstants.TIMEOUT, timeout);
		LOG.trace("Complete VtnServiceWebUtil#prepareAquireConfigJSON()");
		return configJson;
	}

	/**
	 * Prepare aquire read lock json.
	 * 
	 * @return the json object
	 * @throws VtnServiceWebAPIException
	 *             the vtn service web api exception
	 */
	public static JsonObject prepareAquireReadLockJSON()
			throws VtnServiceWebAPIException {
		LOG.trace("Start VtnServiceWebUtil#prepareAquireReadLockJSON()");
		final JsonObject acquireReadLockObj = new JsonObject();
		final JsonObject readLockJson = new JsonObject();
		readLockJson.addProperty(
				ApplicationConstants.TIMEOUT,
				ConfigurationManager.getInstance().getConfProperty(
						ApplicationConstants.TIMEOUT));
		acquireReadLockObj.add(ApplicationConstants.READLOCK, readLockJson);
		LOG.trace("Complete VtnServiceWebUtil#prepareAquireReadLockJSON()");
		return acquireReadLockObj;
	}

	/**
	 * Prepare config commit json.
	 * 
	 * @param operation
	 *            the operation
	 * @return the json object
	 */
	public static JsonObject prepareConfigCommitOrSaveJSON(
			final String operation) {
		LOG.trace("Start VtnServiceWebUtil#prepareConfigCommitOrSaveJSON()");
		final JsonObject commitConfigJson = new JsonObject();
		final JsonObject commitConfigObj = new JsonObject();
		commitConfigJson.addProperty(ApplicationConstants.OPERATION, operation);
		commitConfigObj.add(ApplicationConstants.CONFIGURATION_STRING,
				commitConfigJson);
		LOG.trace("Complete VtnServiceWebUtil#prepareConfigCommitOrSaveJSON()");
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
		LOG.trace("Start VtnServiceWebUtil#prepareCandidateAbortJSON()");
		final JsonObject abortConfigJson = new JsonObject();
		final JsonObject abortConfigObj = new JsonObject();
		abortConfigJson.addProperty(ApplicationConstants.OPERATION, operation);
		abortConfigObj.add(ApplicationConstants.CANDIDATE, abortConfigJson);
		LOG.trace("Complete VtnServiceWebUtil#prepareCandidateAbortJSON()");
		return abortConfigObj;
	}

	/**
	 * Prepare err response json.
	 * 
	 * @param errCode
	 *            the err code
	 * @param description
	 *            the description
	 * @return the jSON object
	 * @throws JSONException
	 */
	public static JSONObject prepareErrResponseJson(final int errCode) {
		LOG.trace("Start VtnServiceWebUtil#prepareErrResponseJson()");
		JSONObject errJson = null;
		if (HttpErrorCodeEnum.UNC_STATUS_OK.getCode() != errCode) {
			final JSONObject temErrorJSON = new JSONObject();
			errJson = new JSONObject();
			try {
				temErrorJSON.put(ApplicationConstants.ERR_CODE, errCode);
				temErrorJSON.put(ApplicationConstants.ERR_DESCRIPTION,
						VtnServiceCommonUtil.getErrorMessage(errCode));
				errJson.put(ApplicationConstants.ERROR, temErrorJSON);
			} catch (final JSONException e) {
				LOG.error(e, "Internal server error : " + e);
			}
		}
		LOG.trace("Complete VtnServiceWebUtil#prepareErrResponseJson()");
		return errJson;
	}

	/**
	 * Check string for null or empty.
	 * 
	 * @param valueStr
	 *            the value str
	 * @return true, if successful
	 */
	public static boolean checkStringForNullOrEmpty(final String valueStr) {
		LOG.trace("Start VtnServiceWebUtil#checkStringForNullOrEmpty()");
		boolean status = false;
		if (null != valueStr && !valueStr.trim().isEmpty()) {
			status = true;
		}
		LOG.trace("Complete VtnServiceWebUtil#checkStringForNullOrEmpty()");
		return status;
	}
}
