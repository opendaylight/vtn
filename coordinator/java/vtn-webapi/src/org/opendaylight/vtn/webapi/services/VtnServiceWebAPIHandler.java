/*
 * Copyright (c) 2012-2015 NEC Corporation
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
import org.opendaylight.vtn.webapi.constants.ApplicationConstants;
import org.opendaylight.vtn.webapi.enums.ContentTypeEnum;
import org.opendaylight.vtn.webapi.enums.HttpErrorCodeEnum;
import org.opendaylight.vtn.webapi.exception.VtnServiceWebAPIException;
import org.opendaylight.vtn.webapi.pojo.SessionBean;
import org.opendaylight.vtn.webapi.utils.DataConverter;
import org.opendaylight.vtn.webapi.utils.VtnServiceCommonUtil;
import org.opendaylight.vtn.webapi.utils.VtnServiceWebUtil;
import org.opendaylight.vtn.webapi.utils.ConfigurationManager;

/**
 * The Class VtnServiceWebAPIHandler.This class is defined at the handler layer
 * where the actual request object is prepared and validated for the
 * correctness.This class will contain get, post, put , validate and delete
 */
public class VtnServiceWebAPIHandler {

	/** The Constant LOGGER. */
	private static final Logger LOG = Logger
			.getLogger(VtnServiceWebAPIHandler.class.getName());

	/** Status of commit **/
	private int commitStatus = ApplicationConstants.SUCCESS;

	/**
	 * Get Status of commit
	 */
	public int getCommitStatus() {
		return commitStatus;
	}

	/**
	 * Get. This method will convert the httprequest to JSON format and then
	 * will validate the same for json syntax. At the same time the count and
	 * detail information will be removed from resource path it will also decide
	 * whether the call is for JSON/XML by passed URI format. After getting the
	 * response it will convert the same in either format JSON/XML
	 * 
	 * @param request
	 *            the request
	 * @return the JSONObject
	 * @throws VtnServiceWebAPIException
	 *             the vtn service web api exception
	 */
	public JSONObject get(final HttpServletRequest request)
			throws VtnServiceWebAPIException {
		LOG.trace("Start VtnServiceWebAPIHandler#get()");
		JSONObject serviceResponse = null;
		JsonObject serviceRequest = new JsonObject();
		String resourcePath = VtnServiceCommonUtil.getResourceURI(request
				.getRequestURI());
		final Map<String, String[]> paramsMap = request.getParameterMap();
		serviceRequest = VtnServiceCommonUtil.getOpParameter(serviceRequest,
				resourcePath);
		resourcePath = VtnServiceCommonUtil
				.removeCountOrDetailFromURI(resourcePath);

		// The URI /configuration/diff is not supported by WebAPI since U17.
		if (ApplicationConstants.URI_DIFF.equals(resourcePath)) {
			serviceResponse = VtnServiceWebUtil.prepareErrResponseJson(
					HttpErrorCodeEnum.UNC_NOT_FOUND.getCode());
		} else {
			final VtnServiceWebAPIController vtnServiceWebAPIController = new VtnServiceWebAPIController();
			if (null != paramsMap && !paramsMap.isEmpty()) {
				serviceRequest = DataConverter.convertMapToJson(paramsMap,
						serviceRequest);
				serviceResponse = vtnServiceWebAPIController.get(
						VtnServiceWebUtil.prepareHeaderJson(request),
						serviceRequest, resourcePath);
			} else {
				if (VtnServiceCommonUtil.validateGetAPI(resourcePath)) {
					serviceResponse = vtnServiceWebAPIController.get(
							VtnServiceWebUtil.prepareHeaderJson(request),
							resourcePath);
				} else {
					serviceResponse = vtnServiceWebAPIController.get(
							VtnServiceWebUtil.prepareHeaderJson(request),
							serviceRequest, resourcePath);
				}
			}
		}
		LOG.debug("serviceResponse : " + serviceResponse);
		LOG.trace("Complete VtnServiceWebAPIHandler#get()");
		return serviceResponse;
	}

	/**
	 * Post. This method will convert the httprequest to JSON format and then
	 * will validate the same for json syntax it will also decide whether the
	 * call is for JSON/XML by passed URI format. will prepare the session json
	 * also to take session for api call later. After getting the response it
	 * will convert the same in either format JSON/XML
	 * 
	 * @param request
	 *            the request
	 * @return the JSONObject
	 * @throws VtnServiceWebAPIException
	 *             the vtn service web api exception
	 */
	public JSONObject post(final HttpServletRequest request)
			throws VtnServiceWebAPIException {
		LOG.trace("Start VtnServiceWebAPIHandler#post()");
		JSONObject serviceResponse = null;
		JsonObject serviceRequest = null;
		try {
			final String resourcePath = VtnServiceCommonUtil
					.getResourceURI(request.getRequestURI());
			final String contentType = request.getContentType();
			serviceRequest = new VtnServiceWebUtil().prepareRequestJson(
					request, contentType);
			setAttributeForAccesslog(request, serviceRequest);

			final JsonObject sessionJson = VtnServiceWebUtil
					.prepareHeaderJson(request);
			final VtnServiceWebAPIController vtnServiceWebAPIController = new VtnServiceWebAPIController();
			serviceResponse = vtnServiceWebAPIController.post(sessionJson,
					serviceRequest, resourcePath);
			commitStatus = vtnServiceWebAPIController.getCommitStatus();
			LOG.debug("serviceResponse : " + serviceResponse);
		} catch (final IOException e) {
			LOG.error(e, "Internal server error occurred : " + e.getMessage());
			throw new VtnServiceWebAPIException(
					HttpErrorCodeEnum.UNC_INTERNAL_SERVER_ERROR.getCode());
		}
		LOG.trace("Complete VtnServiceWebAPIHandler#post()");
		return serviceResponse;
	}

	/**
	 * Put. This method will convert the httprequest to JSON format and then
	 * will validate the same for json syntax it will also decide whether the
	 * call is for JSON/XML by passed URI format. will prepare the session json
	 * also to take session for api call later. After getting the response it
	 * will convert the same in either format JSON/XML
	 * 
	 * @param request
	 *            the request
	 * @return the JSONObject
	 * @throws VtnServiceWebAPIException
	 *             the vtn service web api exception
	 */
	public JSONObject put(final HttpServletRequest request)
			throws VtnServiceWebAPIException {
		LOG.trace("Start VtnServiceWebAPIHandler#put()");
		JSONObject serviceResponse = null;
		JsonObject serviceRequest = null;
		try {
			final String resourcePath = VtnServiceCommonUtil
					.getResourceURI(request.getRequestURI());
			final String contentType = request.getContentType();
			if (resourcePath.equals(ApplicationConstants.ALARMSTR)) {
				final Map<String, String[]> paramsMap = request
						.getParameterMap();
				if (null != paramsMap && !paramsMap.isEmpty()) {
					serviceRequest = new JsonObject();
					serviceRequest = DataConverter.convertMapToJson(paramsMap,
							serviceRequest);
				}
			} else {
				serviceRequest = new VtnServiceWebUtil().prepareRequestJson(
						request, contentType);
			}
			setAttributeForAccesslog(request, serviceRequest);

			final VtnServiceWebAPIController vtnServiceWebAPIController = new VtnServiceWebAPIController();
			if (!VtnServiceWebUtil.checkUriForNoConfig(resourcePath)) {
				serviceResponse = vtnServiceWebAPIController.put(
						VtnServiceWebUtil.prepareHeaderJson(request),
						serviceRequest, resourcePath);
			} else {
				serviceResponse = vtnServiceWebAPIController.putForNoConfig(
						VtnServiceWebUtil.prepareHeaderJson(request),
						serviceRequest, resourcePath);
			}
			commitStatus = vtnServiceWebAPIController.getCommitStatus();
			LOG.debug("serviceResponse : " + serviceResponse);
		} catch (final IOException e) {
			LOG.error(e, "Internal server error occurred : " + e.getMessage());
			throw new VtnServiceWebAPIException(
					HttpErrorCodeEnum.UNC_INTERNAL_SERVER_ERROR.getCode());
		}
		LOG.trace("Complete VtnServiceWebAPIHandler#put()");
		return serviceResponse;
	}

	/**
	 * Delete. This method will convert the httprequest to JSON format and then
	 * will validate the same for json syntax it will also decide whether the
	 * call is for JSON/XML by passed URI format. will prepare the session json
	 * also to take session for api call later. After getting the response it
	 * will convert the same in either format JSON/XML
	 * 
	 * @param request
	 *            the request
	 * @return the JSONObject
	 * @throws VtnServiceWebAPIException
	 *             the vtn service web api exception
	 */
	public JSONObject delete(final HttpServletRequest request)
			throws VtnServiceWebAPIException {
		LOG.trace("Start VtnServiceWebAPIHandler#delete()");
		JSONObject serviceResponse = null;
		JsonObject serviceRequest = new JsonObject();
		final String resourcePath = VtnServiceCommonUtil.getResourceURI(request
				.getRequestURI());
		final Map<String, String[]> paramsMap = request.getParameterMap();
		final VtnServiceWebAPIController vtnServiceWebAPIController = new VtnServiceWebAPIController();
		if (!VtnServiceWebUtil.checkUriForNoConfig(resourcePath)) {
			if (null != paramsMap && !paramsMap.isEmpty()) {
				serviceRequest = DataConverter.convertMapToJson(paramsMap,
						serviceRequest);
				setAttributeForAccesslog(request, serviceRequest);
				serviceResponse = vtnServiceWebAPIController.delete(
						VtnServiceWebUtil.prepareHeaderJson(request),
						serviceRequest, resourcePath);
			} else {
				serviceResponse = vtnServiceWebAPIController.delete(
						VtnServiceWebUtil.prepareHeaderJson(request), resourcePath);
			}
		} else {
			serviceResponse = vtnServiceWebAPIController.deleteForNoConfig(
					VtnServiceWebUtil.prepareHeaderJson(request), resourcePath);
		}
		commitStatus = vtnServiceWebAPIController.getCommitStatus();
		LOG.debug("serviceResponse : " + serviceResponse);
		LOG.trace("Complete VtnServiceWebAPIHandler#delete()");
		return serviceResponse;
	}

	/**
	 * Validate.This method will validate for the header part coming with the
	 * request and check for the authentication and authorization It will also
	 * check for valid URI, if URI is not valid then it will fail the call and
	 * return forbidden error.
	 * 
	 * @param request
	 *            the request
	 * @throws VtnServiceWebAPIException
	 *             the vtn service web api exception
	 */
	public void validate(final HttpServletRequest request)
			throws VtnServiceWebAPIException {
		LOG.trace("Start VtnServiceWebAPIHandler#validate()");
		if (!VtnServiceCommonUtil.validateMediaType(request)) {
			LOG.error("Invalid media type");
			throw new VtnServiceWebAPIException(
					HttpErrorCodeEnum.UNC_UNSUPPORTED_MEDIA_TYPE.getCode());
		}
		
		// in case of GET method check the validity of Accept header
		if (request.getMethod().equalsIgnoreCase(
				ApplicationConstants.GET_METHOD_NAME)) {
			final String acceptHeaderValue = request
					.getHeader(ApplicationConstants.HTTP_HEADER_ACCEPT);
			LOG.debug("Accept header's value : " + acceptHeaderValue);
			if (acceptHeaderValue != null) {
				String acceptHeader[] = acceptHeaderValue
						.split(ApplicationConstants.SEMI_COLON);
				if (acceptHeader.length > 0) {
					if (acceptHeader[0]
							.equalsIgnoreCase(ApplicationConstants.DEFAULT_ACCEPT)
							|| acceptHeader[0]
									.equalsIgnoreCase(ContentTypeEnum.APPLICATION_JSON
											.getContentType())
							|| acceptHeader[0]
									.equalsIgnoreCase(ContentTypeEnum.APPLICATION_XML
											.getContentType())) {
						LOG.info("Valid value for Accept");
					} else {
						LOG.error("Invalid accept header value");
						throw new VtnServiceWebAPIException(
								HttpErrorCodeEnum.UNC_NOT_ACCEPTABLE.getCode());
					}
				} else {
					LOG.error("Invalid accept header value");
					throw new VtnServiceWebAPIException(
							HttpErrorCodeEnum.UNC_NOT_ACCEPTABLE.getCode());
				}
			}
		}

		final JsonObject headerInfo = VtnServiceWebUtil
				.prepareHeaderJson(request);
		final SessionBean bean = VtnServiceCommonUtil
				.getSessionObject(headerInfo);

		if (!VtnServiceCommonUtil.authoriseUser(bean, request.getMethod())) {
			LOG.error("Unautorized user");
			throw new VtnServiceWebAPIException(
					HttpErrorCodeEnum.UNC_FORBIDDEN.getCode());
		}

		// perform extra checks for restriction of XML information in OpenStack
		// operations
		if (VtnServiceCommonUtil.isOpenStackResurce(request)) {
			restrictXmlforOpenStack(request);
		}

		LOG.trace("Complete VtnServiceWebAPIHandler#validate()");
	}

	/**
	 * Check for XML restrictions in Content-Type, Accept and URI extension
	 * 
	 * @param request
	 * @throws VtnServiceWebAPIException
	 */
	private void restrictXmlforOpenStack(final HttpServletRequest request)
			throws VtnServiceWebAPIException {
		LOG.trace("Start VtnServiceWebAPIHandler#restrictXmlforOpenStack()");
		if (request.getMethod().equalsIgnoreCase(
				ApplicationConstants.POST_METHOD_NAME)
				|| request.getMethod().equalsIgnoreCase(
						ApplicationConstants.PUT_METHOD_NAME)) {
			String contentType = request.getContentType().split(
					ApplicationConstants.SEMI_COLON)[0];
			if (contentType.equalsIgnoreCase(ContentTypeEnum.APPLICATION_XML
					.getContentType())) {
				LOG.error("Invalid media type for OpenStack operations");
				throw new VtnServiceWebAPIException(
						HttpErrorCodeEnum.UNC_UNSUPPORTED_MEDIA_TYPE.getCode());
			}
		}

		// in case of GET method check the validity of Accept header
		if (request.getMethod().equalsIgnoreCase(
				ApplicationConstants.GET_METHOD_NAME)) {
			final String acceptHeaderValue = request
					.getHeader(ApplicationConstants.HTTP_HEADER_ACCEPT);
			LOG.debug("Accept header's value : " + acceptHeaderValue);
			if (acceptHeaderValue != null) {
				String acceptHeader[] = acceptHeaderValue
						.split(ApplicationConstants.SEMI_COLON);
				if (acceptHeader.length > 0) {
					if (acceptHeader[0]
							.equalsIgnoreCase(ApplicationConstants.DEFAULT_ACCEPT)
							|| acceptHeader[0]
									.equalsIgnoreCase(ContentTypeEnum.APPLICATION_JSON
											.getContentType())) {
						LOG.info("Valid value for Accept");
					} else {
						LOG.error("Invalid accept hedare value");
						throw new VtnServiceWebAPIException(
								HttpErrorCodeEnum.UNC_NOT_ACCEPTABLE.getCode());
					}
				} else {
					LOG.error("Invalid accept hedare value");
					throw new VtnServiceWebAPIException(
							HttpErrorCodeEnum.UNC_NOT_ACCEPTABLE.getCode());
				}
			}
		}

		String requestURI = request.getRequestURI();
		String finalURI;
		if (null != requestURI
				&& requestURI.startsWith(ApplicationConstants.CONTEXTPATH)) {
			finalURI = requestURI.replace(ApplicationConstants.CONTEXTPATH,
					ApplicationConstants.BLANK_STR);
			final int paramaStartIndex = finalURI
					.indexOf(ApplicationConstants.QUESTION_MARK_CHAR);
			if (paramaStartIndex != -1) {
				finalURI = finalURI.substring(0, paramaStartIndex);
			}

			final int extensionStartIndex = finalURI
					.lastIndexOf(ApplicationConstants.DOT_REGEX);
			if (extensionStartIndex != -1) {
				final String extension = finalURI
						.substring(extensionStartIndex);
				if (null != extension
						&& (extension.equals(ApplicationConstants.TYPE_XML) || extension
								.equals(ApplicationConstants.TYPE_JSON))) {
					throw new VtnServiceWebAPIException(
							HttpErrorCodeEnum.UNC_CUSTOM_NOT_FOUND.getCode());
				}
			}
		}
		LOG.trace("Complete VtnServiceWebAPIHandler#restrictXmlforOpenStack()");
	}

	private void setAttributeForAccesslog(final HttpServletRequest request, JsonObject serviceRequest) {
		String requestBodyName = null;
		try {
			requestBodyName = ConfigurationManager.getInstance()
				.getConfProperty(ApplicationConstants.REQ_BODY);
		} catch (Exception e) {
			LOG.warning("Read parameter(REQ_BODY) from the config file failed, use default value.");
			requestBodyName = ApplicationConstants.REQ_BODY_DEF_NAME;
		}
		if (null == requestBodyName || requestBodyName.isEmpty()) {
			requestBodyName = ApplicationConstants.REQ_BODY_DEF_NAME;
		}

		request.setAttribute(requestBodyName, serviceRequest.toString());
	}
}
