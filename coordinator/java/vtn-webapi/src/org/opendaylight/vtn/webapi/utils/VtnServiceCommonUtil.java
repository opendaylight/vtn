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
import java.util.List;

import javax.servlet.http.HttpServletRequest;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.RestResource;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.webapi.constants.ApplicationConstants;
import org.opendaylight.vtn.webapi.enums.ContentTypeEnum;
import org.opendaylight.vtn.webapi.enums.HttpErrorCodeEnum;
import org.opendaylight.vtn.webapi.enums.SessionEnum;
import org.opendaylight.vtn.webapi.exception.VtnServiceWebAPIException;
import org.opendaylight.vtn.webapi.pojo.SessionBean;

/**
 * The Class VtnServiceCommonUtil.
 */
public final class VtnServiceCommonUtil {

	/** The Constant LOG. */
	private static final Logger LOG = Logger
			.getLogger(VtnServiceCommonUtil.class.getName());

	/**
	 * Instantiates a new vtn service common util.
	 */
	private VtnServiceCommonUtil() {
	}

	/**
	 * Method getErrorDescription - Below method is used to get error
	 * description based on error code.
	 * 
	 * @param errorCode
	 *            the error code
	 * @param description
	 *            the description
	 * @return String
	 */
	public static String logErrorDetails(final int errorCode) {
		LOG.trace("Start VtnServiceCommonUtil#logErrorDetails()");
		final String errorDetails = new StringBuilder().append(errorCode)
				.append(ApplicationConstants.HYPHEN)
				.append(getErrorMessage(errorCode)).toString();
		LOG.error(errorDetails);
		LOG.trace("Complete VtnServiceCommonUtil#logErrorDetails()");
		return errorDetails;
	}

	/**
	 * @param errorCode
	 * @return
	 */
	public static String getErrorMessage(final int errorCode) {
		String errorMessage;
		if (errorCode == HttpErrorCodeEnum.UNC_BAD_REQUEST.getCode()) {
			errorMessage = HttpErrorCodeEnum.UNC_BAD_REQUEST.getMessage();
		} else if (errorCode == HttpErrorCodeEnum.UNC_FORBIDDEN.getCode()) {
			errorMessage = HttpErrorCodeEnum.UNC_FORBIDDEN.getMessage();
		} else if (errorCode == HttpErrorCodeEnum.UNC_INTERNAL_SERVER_ERROR
				.getCode()) {
			errorMessage = HttpErrorCodeEnum.UNC_INTERNAL_SERVER_ERROR
					.getMessage();
		} else if (errorCode == HttpErrorCodeEnum.UNC_METHOD_NOT_ALLOWED
				.getCode()) {
			errorMessage = HttpErrorCodeEnum.UNC_METHOD_NOT_ALLOWED
					.getMessage();
		} else if (errorCode == HttpErrorCodeEnum.UNC_NOT_ACCEPTABLE.getCode()) {
			errorMessage = HttpErrorCodeEnum.UNC_NOT_ACCEPTABLE.getMessage();
		} else if (errorCode == HttpErrorCodeEnum.UNC_NOT_FOUND.getCode()) {
			errorMessage = HttpErrorCodeEnum.UNC_NOT_FOUND.getMessage();
		} else if (errorCode == HttpErrorCodeEnum.UNC_SERVICE_UNAVAILABLE
				.getCode()) {
			errorMessage = HttpErrorCodeEnum.UNC_SERVICE_UNAVAILABLE
					.getMessage();
		} else if (errorCode == HttpErrorCodeEnum.UNC_STATUS_OK.getCode()) {
			errorMessage = HttpErrorCodeEnum.UNC_STATUS_OK.getMessage();
		} else if (errorCode == HttpErrorCodeEnum.UNC_UNAUTHORIZED.getCode()) {
			errorMessage = HttpErrorCodeEnum.UNC_UNAUTHORIZED.getMessage();
		} else if (errorCode == HttpErrorCodeEnum.UNC_UNSUPPORTED_MEDIA_TYPE
				.getCode()) {
			errorMessage = HttpErrorCodeEnum.UNC_UNSUPPORTED_MEDIA_TYPE
					.getMessage();
		} else {
			errorMessage = HttpErrorCodeEnum.UNC_INTERNAL_SERVER_ERROR
					.getMessage();
		}
		return errorMessage;
	}

	/**
	 * Authorize user.
	 * 
	 * @param bean
	 *            the bean
	 * @param httpMethod
	 *            the http method
	 * @return true, if successful
	 * @throws VtnServiceWebAPIException
	 *             the vtn service web api exception
	 */
	public static boolean authoriseUser(SessionBean bean, String httpMethod)
			throws VtnServiceWebAPIException {
		LOG.trace("Start VtnServiceCommonUtil#authoriseUser()");
		boolean ipStatus = false;
		final ConfigurationManager configurationManager = ConfigurationManager
				.getInstance();
		final String ipAddresses = configurationManager
				.getAccessProperty(ApplicationConstants.ACCESS_ALL);
		LOG.debug("Authorized IP Address from Configuration : " + ipAddresses);
		final String httpMethodAccessIpAddress = configurationManager
				.getAccessProperty(httpMethod.toUpperCase());
		LOG.debug("Authorized methods from Configuration : " + ipAddresses);

		if (ipAddresses != null
				&& ApplicationConstants.WILD_CARD_STAR.equals(ipAddresses
						.trim())) {
			ipStatus = true;
		} else if (ipAddresses.indexOf(bean.getIpAddress()) != -1) {
			ipStatus = true;
		} else if (null != httpMethodAccessIpAddress
				&& !httpMethodAccessIpAddress.isEmpty()
				&& ApplicationConstants.WILD_CARD_STAR
						.equals(httpMethodAccessIpAddress.trim())) {
			ipStatus = true;
		} else if (null != httpMethodAccessIpAddress
				&& !httpMethodAccessIpAddress.isEmpty()
				&& httpMethodAccessIpAddress.indexOf(bean.getIpAddress()) != -1) {
			ipStatus = true;
		}

		LOG.debug("Authorize User Result : " + ipStatus);
		LOG.trace("Complete VtnServiceCommonUtil#authoriseUser()");
		return ipStatus;
	}

	/**
	 * Gets the session from json.
	 * 
	 * @param sessionObjRes
	 *            the session obj res
	 * @return the session from json
	 */
	public static long getSessionFromJson(final JsonObject sessionObjRes) {
		LOG.trace("Start VtnServiceCommonUtil#getSessionFromJson()");
		long sessionID = -1;
		if (null != sessionObjRes) {
			final JsonObject sessionJson = sessionObjRes
					.getAsJsonObject(ApplicationConstants.SESSION);

			if (null != sessionJson
					&& sessionJson.has(ApplicationConstants.SESSION_ID_STR)) {
				sessionID = sessionJson.getAsJsonPrimitive(
						ApplicationConstants.SESSION_ID_STR).getAsLong();
			}
		}
		LOG.debug("Session Id : " + sessionID);
		LOG.trace("Complete VtnServiceCommonUtil#getSessionFromJson()");
		return sessionID;
	}

	/**
	 * Gets the config id from json.
	 * 
	 * @param configObjRes
	 *            the config obj res
	 * @return the config id from json
	 */
	public static long getConfigIdFromJson(final JsonObject configObjRes) {
		LOG.trace("Start VtnServiceCommonUtil#getConfigIdFromJson()");
		long configID = -1;
		if (null != configObjRes) {
			final JsonObject configModeJson = configObjRes
					.getAsJsonObject(ApplicationConstants.CONFIG_MODE);

			if (null != configModeJson
					&& configModeJson.has(ApplicationConstants.CONFIG_ID_STR)) {
				configID = configModeJson.getAsJsonPrimitive(
						ApplicationConstants.CONFIG_ID_STR).getAsInt();
			}
		}
		LOG.debug("Config Id : " + configID);
		LOG.trace("Complete VtnServiceCommonUtil#getConfigIdFromJson()");
		return configID;
	}


	/**
	 * Gets the content type.
	 * 
	 * @param uri
	 *            the uri
	 * @return the content type
	 */
	public static String getResponseBodyContentType(final HttpServletRequest request) {
		LOG.trace("Start VtnServiceCommonUtil#getContentType()");

		String contentType = null;
		final String requestURI = request.getRequestURI();
		if (requestURI.lastIndexOf(ApplicationConstants.DOT_REGEX) != -1) {
			String extension = requestURI.substring(requestURI
					.lastIndexOf(ApplicationConstants.DOT_REGEX));
			if (extension.indexOf(ApplicationConstants.QUESTION_MARK_CHAR) != -1) {
				extension = extension.substring(0, extension.indexOf("?"));
			}
			if (null != extension
					&& (extension
							.equalsIgnoreCase(ApplicationConstants.TYPE_XML) || extension
							.equalsIgnoreCase(ApplicationConstants.TYPE_JSON))) {
				if (extension
						.equalsIgnoreCase(ApplicationConstants.TYPE_XML)) {
					contentType = ContentTypeEnum.APPLICATION_XML
							.getContentType();
				} else if (extension
						.equalsIgnoreCase(ApplicationConstants.TYPE_JSON)) {
					contentType = ContentTypeEnum.APPLICATION_JSON
							.getContentType();
				}
			}
		}

		if (null == contentType) {
			final String acceptHeaderValue = request
					.getHeader(ApplicationConstants.HTTP_HEADER_ACCEPT);
			if (acceptHeaderValue != null) {
				String acceptHeader[] = acceptHeaderValue
						.split(ApplicationConstants.SEMI_COLON);
				if (acceptHeader.length > 0) {
					if (acceptHeader[0]
							.equalsIgnoreCase(ContentTypeEnum.APPLICATION_JSON
									.getContentType())
							|| acceptHeader[0]
									.equalsIgnoreCase(ContentTypeEnum.APPLICATION_XML
											.getContentType())) {
						contentType = acceptHeader[0];
					}
				}
			}
		} 

		if (null == contentType) {
			contentType = ContentTypeEnum.APPLICATION_JSON.getContentType();
		}

		LOG.trace("Complete VtnServiceCommonUtil#getContentType()");
		return contentType;
	}

	/**
	 * Validate uri.
	 * 
	 * @param uri
	 *            the uri
	 * @param contentType
	 *            the content type
	 * @return true, if successful
	 */
	public static boolean validateMediaType(final HttpServletRequest request) {
		LOG.trace("Start VtnServiceCommonUtil#validateMediaType()");
		boolean isMediaTypeCorrect = true;
		if (request.getMethod().equalsIgnoreCase(
				ApplicationConstants.POST_METHOD_NAME)
				|| request.getMethod().equalsIgnoreCase(
						ApplicationConstants.PUT_METHOD_NAME)) {
			isMediaTypeCorrect = false;
			String contentType = request.getContentType();
			LOG.debug("Media Type : " + contentType);
			if (contentType != null) {
				String content[] = contentType
						.split(ApplicationConstants.SEMI_COLON);
				if (content.length > 0
						&& (content[0]
								.equalsIgnoreCase(ContentTypeEnum.APPLICATION_JSON
										.getContentType()) || content[0]
								.equalsIgnoreCase(ContentTypeEnum.APPLICATION_XML
										.getContentType()) || content[0]
								.equalsIgnoreCase(ContentTypeEnum.APPLICATION_XML_SCVMM
										.getContentType()))) {
					isMediaTypeCorrect = true;
				}
			}
		}
		LOG.debug("Valid Media Type : " + isMediaTypeCorrect);
		LOG.trace("Complete VtnServiceCommonUtil#validateMediaType()");
		return isMediaTypeCorrect;
	}

	/**
	 * Gets the session object.
	 * 
	 * @param sessionJson
	 *            the session json
	 * @return the session object
	 * @throws VtnServiceWebAPIException
	 *             the vtn service web api exception
	 */
	public static SessionBean getSessionObject(final JsonObject sessionJson)
			throws VtnServiceWebAPIException {
		LOG.trace("Start VtnServiceCommonUtil#getSessionObject()");
		final SessionBean sessionBean = new SessionBean();
		final List<String> mandatoryList = Arrays.asList(
				SessionEnum.USERNAME.getSessionElement(),
				SessionEnum.PASSWORD.getSessionElement(),
				SessionEnum.IPADDRESS.getSessionElement());
		final JsonObject sessionJsonObj = (JsonObject) sessionJson
				.get(ApplicationConstants.SESSION);

		for (final String value : mandatoryList) {
			if (!sessionJsonObj.has(value) || null == sessionJsonObj.get(value)) {
				throw new VtnServiceWebAPIException(
						HttpErrorCodeEnum.UNC_UNAUTHORIZED.getCode());
			}
		}
		sessionBean.setUserName(sessionJsonObj.get(SessionEnum.USERNAME
				.getSessionElement()) != null ? sessionJsonObj.get(
				SessionEnum.USERNAME.getSessionElement()).getAsString() : null);
		sessionBean.setPassword(sessionJsonObj.get(SessionEnum.PASSWORD
				.getSessionElement()) != null ? sessionJsonObj.get(
				SessionEnum.PASSWORD.getSessionElement()).getAsString() : null);
		sessionBean
				.setIpAddress(sessionJsonObj.get(SessionEnum.IPADDRESS
						.getSessionElement()) != null ? sessionJsonObj.get(
						SessionEnum.IPADDRESS.getSessionElement())
						.getAsString() : null);

		sessionBean.setType(ApplicationConstants.SESSION_TYPE);

		LOG.debug("Username : " + sessionBean.getUserName());
		LOG.debug("Password : " + sessionBean.getPassword());
		LOG.debug("Type : " + sessionBean.getType());

		LOG.trace("Complete VtnServiceCommonUtil#getSessionObject()");
		return sessionBean;

	}

	/**
	 * Gets the resource uri.
	 * 
	 * @param requestURI
	 *            the request uri
	 * @return the resource uri
	 */
	public static String getResourceURI(final String requestURI) {
		LOG.trace("Start VtnServiceCommonUtil#getResourceURI()");
		String finalURI = null;

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
					finalURI = finalURI.substring(0, extensionStartIndex);
				}
			}
		}
		LOG.debug("Final URI : " + finalURI);
		LOG.trace("Complete VtnServiceCommonUtil#getResourceURI()");
		return finalURI;
	}

	/**
	 * Gets the op parameter.
	 * 
	 * @param serviceRequest
	 *            the service request
	 * @param resourcePath
	 *            the resource path
	 * @return the op parameter
	 */
	public static JsonObject getOpParameter(JsonObject serviceRequest,
			String resourcePath) {
		LOG.trace("Start VtnServiceCommonUtil#getOpParameter()");
		String opString = null;
		if (null != resourcePath
				&& (resourcePath.endsWith(ApplicationConstants.COUNT)
						|| resourcePath.endsWith(ApplicationConstants.DETAIL) || resourcePath
							.endsWith(ApplicationConstants.INFO))) {
			opString = resourcePath.substring(
					resourcePath.lastIndexOf(ApplicationConstants.SLASH) + 1,
					resourcePath.length());
		}
		if (null != opString && !opString.isEmpty()) {
			serviceRequest.addProperty(ApplicationConstants.OP, opString);
		}
		LOG.trace("Complete VtnServiceCommonUtil#getOpParameter()");
		return serviceRequest;
	}

	/**
	 * This method will remove /count or /detail or /info from URI and then will
	 * return actual resource path.
	 * 
	 * @param resourcePath
	 *            the resource path
	 * @return the string
	 */
	public static String removeCountOrDetailFromURI(final String resourcePath) {
		LOG.trace("Start VtnServiceCommonUtil#removeCountOrDetailFromURI()");
		String opString = resourcePath;
		if (null != resourcePath
				&& (resourcePath.endsWith(ApplicationConstants.COUNT)
						|| resourcePath.endsWith(ApplicationConstants.DETAIL) || resourcePath
							.endsWith(ApplicationConstants.INFO))) {
			opString = resourcePath.substring(ApplicationConstants.ZERO,
					resourcePath.lastIndexOf(ApplicationConstants.SLASH));
		}
		LOG.trace("Complete VtnServiceCommonUtil#removeCountOrDetailFromURI()");
		return opString;
	}

	/**
	 * 
	 * @param URI
	 * @return
	 * @throws VtnServiceWebAPIException
	 */
	public static boolean validateGetAPI(String URI)
			throws VtnServiceWebAPIException {
		LOG.trace("Start VtnServiceCommonUtil#validateGetAPI()");
		boolean status = false;
		if (null != URI && !URI.isEmpty()) {
			if (URI.trim().contains(ApplicationConstants.CONTROLLERSTR)
					&& URI.trim().contains(ApplicationConstants.DOMAINSTR)
					&& URI.trim()
							.contains(ApplicationConstants.LOGICALPORTSSTR)) {
				status = true;
			} else if (URI.trim().startsWith(ApplicationConstants.SESSIONSTR)
					&& URI.trim().length() > ApplicationConstants.SESSIONSTR
							.length()) {
				status = true;
			} else {
				final String getListAPI = ConfigurationManager.getInstance()
						.getConfProperty(ApplicationConstants.GETLISTAPI);
				if (null != getListAPI && !getListAPI.isEmpty()) {
					final String[] tempList = getListAPI
							.split(ApplicationConstants.COMMA_STR);
					for (int i = 0; i < tempList.length; i++) {
						final String getStr = tempList[i];
						if (URI.trim().equals(getStr.trim())) {
							status = true;
							break;
						}
					}
				}
			}
		}
		LOG.trace("Complete VtnServiceCommonUtil#validateGetAPI()");
		return status;
	}

	/**
	 * 
	 * @param request
	 * @return
	 * @throws VtnServiceWebAPIException
	 */
	public static boolean isOpenStackResurce(final HttpServletRequest request)
			throws VtnServiceWebAPIException {
		LOG.trace("Start VtnServiceUtil#isOpenStackResurce()");
		boolean status = false;
		/*
		 * get resource path and then resolve the resource corresponding to this
		 * resource path
		 */
		final String resourcePath = VtnServiceCommonUtil.removeCountOrDetailFromURI(
				VtnServiceCommonUtil.getResourceURI(request.getRequestURI()));
		final RestResource restResource = new RestResource();
		restResource.setPath(resourcePath);
		final AbstractResource resource = restResource.getResource();

		if (resource == null) {
			LOG.debug("Decision cannot be taken that request came for UNC core APIs or UNC OpenStack APIs");
			throw new VtnServiceWebAPIException(
					HttpErrorCodeEnum.UNC_CUSTOM_NOT_FOUND.getCode());
		} else if (resource.getClass().getCanonicalName()
				.startsWith(ApplicationConstants.OS_RESOURCE_PKG)) {
			LOG.debug("Request came for UNC OpenStack APIs");
			status = true;
		}
		LOG.trace("Complete VtnServiceUtil#isOpenStackResurce()");
		return status;
	}
}
