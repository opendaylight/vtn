/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.webapi.services;

import javax.servlet.http.HttpServletResponse;

import org.json.JSONException;
import org.json.JSONObject;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.RestResource;
import org.opendaylight.vtn.webapi.constants.ApplicationConstants;
import org.opendaylight.vtn.webapi.enums.HttpErrorCodeEnum;
import org.opendaylight.vtn.webapi.enums.ResourcePathEnum;
import org.opendaylight.vtn.webapi.exception.VtnServiceWebAPIException;
import org.opendaylight.vtn.webapi.utils.VtnServiceCommonUtil;
import org.opendaylight.vtn.webapi.utils.VtnServiceWebUtil;

/**
 * The Class VtnServiceWebAPIController.This class will be the interface between
 * WebAPI and Java API The actual flow will be covered in this class only.
 */
public class VtnServiceWebAPIController {

	/** The Constant LOG. */
	private static final Logger LOG = Logger
			.getLogger(VtnServiceWebAPIController.class.getName());

	/**
	 * This method will get the data from java api in json format and will do
	 * not pass any request to java api. First of all it will create the session
	 * and then acquire readlock from TC through JAva API. Then it will call the
	 * requested API and if we get success from lower layer then the response
	 * jaon will be recived and readlock will be released and same for session .
	 * 
	 * @param sessionJson
	 *            the session json
	 * @param resourcePath
	 *            the resource path
	 * @return the jSON object
	 * @throws VtnServiceWebAPIException
	 *             the vtn service web api exception
	 */
	public JSONObject get(final JsonObject sessionJson,
			final String resourcePath) throws VtnServiceWebAPIException {
		LOG.trace("Start VtnServiceWebAPIController#get()");
		JSONObject responseJSON = null;
		long sessionId = 0;
		boolean isReadlock = true;
		String responseStr = null;
		boolean exceptionStatus = false;
		final RestResource resource = new RestResource();
		try {
			LOG.debug("acquiring session id and readlock from java API");
			// acquire session id
			resource.setPath(ResourcePathEnum.SESSION_PATH.getPath());
			if (resource.post(sessionJson) != ApplicationConstants.SUCCESS) {
				isReadlock = false;
				LOG.error("JAVA API returns error # " + resource.getInfo());
				responseStr = resource.getInfo() != null ? resource.getInfo()
						.toString() : null;
				throw new VtnServiceWebAPIException();
			}
			sessionId = VtnServiceCommonUtil.getSessionFromJson(resource
					.getInfo());
			// acquire monitoring mode
			resource.setPath(ResourcePathEnum.ACQUIRE_RELEASE_MONITORING_PATH
					.getPath());
			resource.setSessionID(sessionId);
			if (resource.put(VtnServiceWebUtil.prepareAquireReadLockJSON()) != ApplicationConstants.SUCCESS) {
				isReadlock = false;
				LOG.error("JAVA API returns error # " + resource.getInfo());
				responseStr = resource.getInfo() != null ? resource.getInfo()
						.toString() : null;
				throw new VtnServiceWebAPIException();
			}
			// send the api request json
			resource.setPath(resourcePath);
			resource.setSessionID(sessionId);
			final int status = resource.get();
			LOG.debug("JAVA API returned error code #" + status);
			if (status != ApplicationConstants.SUCCESS) {
				responseStr = resource.getInfo() != null ? resource.getInfo()
						.toString() : null;
				LOG.error("JAVA API returns error # " + resource.getInfo());
				throw new VtnServiceWebAPIException();
			}
			responseStr = resource.getInfo() != null ? resource.getInfo()
					.toString() : null;
			LOG.debug("JAVA API returned response # " + responseStr);
		} catch (final Exception e) {
			LOG.error(e, "VTN Service error occurred : " + e.getMessage());
			exceptionStatus = true;
			if (!(e instanceof VtnServiceWebAPIException)) {
				throw new VtnServiceWebAPIException(
						HttpErrorCodeEnum.UNC_INTERNAL_SERVER_ERROR.getCode());
			}
		} finally {
			try {
				// release monitoring mode
				if (isReadlock) {
					resource.setPath(ResourcePathEnum.ACQUIRE_RELEASE_MONITORING_PATH
							.getPath());
					resource.setSessionID(sessionId);
					if (resource.delete() != ApplicationConstants.SUCCESS) {
						LOG.error("JAVA API returns error # "
								+ resource.getInfo());
						responseStr = resource.getInfo() != null ? resource
								.getInfo().toString() : null;
						throw new VtnServiceWebAPIException();
					}
				}
				if (sessionId != ApplicationConstants.ZERO) {
					// release session
					resource.setPath(ResourcePathEnum.RELEASE_SESSION.getPath()
							+ sessionId);
					resource.setSessionID(sessionId);
					if (resource.delete() != ApplicationConstants.SUCCESS) {
						LOG.error("JAVA API returns error # "
								+ resource.getInfo());
						responseStr = resource.getInfo() != null ? resource
								.getInfo().toString() : null;
						throw new VtnServiceWebAPIException();
					}
				}
				if (null == responseStr && !exceptionStatus) {
					responseJSON = VtnServiceWebUtil
							.prepareErrResponseJson(HttpErrorCodeEnum.UNC_STATUS_OK
									.getCode());
				} else {
					responseJSON = new JSONObject(responseStr);
				}
			} catch (final JSONException exception) {
				LOG.error(exception, "VTN Service error occurred : "
						+ exception.getMessage());
				throw new VtnServiceWebAPIException(
						HttpErrorCodeEnum.UNC_INTERNAL_SERVER_ERROR.getCode());
			}
		}
		LOG.trace("Complete VtnServiceWebAPIController#get()");
		return responseJSON;
	}

	/**
	 * This method will get the data from java api in json format and will pass
	 * the required request to java api. First of all it will create the session
	 * and then acquire readlock from TC through JAva API. Then it will call the
	 * requested API and if we get success from lower layer then the response
	 * jaon will be recived and readlock will be released and same for session .
	 * 
	 * @param sessionJson
	 *            the session json
	 * @param reqObject
	 *            the req object
	 * @param resourcePath
	 *            the resource path
	 * @return the jSON object
	 * @throws VtnServiceWebAPIException
	 *             the vtn service web api exception
	 */
	public JSONObject get(final JsonObject sessionJson,
			final JsonObject reqObject, final String resourcePath)
			throws VtnServiceWebAPIException {
		LOG.trace("Start VtnServiceWebAPIController#get()");
		JSONObject responseJSON = null;
		long sessionId = 0;
		boolean isReadlock = true;
		String responseStr = null;
		boolean exceptionStatus = false;
		final RestResource resource = new RestResource();
		try {
			LOG.debug("acquiring session id and readlock from java API");
			// acquire session id
			resource.setPath(ResourcePathEnum.SESSION_PATH.getPath());
			if (resource.post(sessionJson) != ApplicationConstants.SUCCESS) {
				isReadlock = false;
				LOG.error("JAVA API returns error # " + resource.getInfo());
				responseStr = resource.getInfo() != null ? resource.getInfo()
						.toString() : null;
				throw new VtnServiceWebAPIException();
			}
			sessionId = VtnServiceCommonUtil.getSessionFromJson(resource
					.getInfo());
			// acquire monitoring mode

			resource.setPath(ResourcePathEnum.ACQUIRE_RELEASE_MONITORING_PATH
					.getPath());
			resource.setSessionID(sessionId);
			if (resource.put(VtnServiceWebUtil.prepareAquireReadLockJSON()) != ApplicationConstants.SUCCESS) {
				isReadlock = false;
				LOG.error("JAVA API returns error # " + resource.getInfo());
				responseStr = resource.getInfo() != null ? resource.getInfo()
						.toString() : null;
				throw new VtnServiceWebAPIException();
			}
			// send the api request json
			resource.setPath(resourcePath);
			resource.setSessionID(sessionId);
			LOG.debug("Request object passing to JAVA API  #" + reqObject);
			final int status = resource.get(reqObject);
			LOG.debug("JAVA API returned error code #" + status);
			if (status != ApplicationConstants.SUCCESS) {
				responseStr = resource.getInfo() != null ? resource.getInfo()
						.toString() : null;
				LOG.error("JAVA API returns error # " + resource.getInfo());
				throw new VtnServiceWebAPIException();
			}
			responseStr = resource.getInfo() != null ? resource.getInfo()
					.toString() : null;
			LOG.debug("JAVA API returned response # " + responseStr);
		} catch (final Exception e) {
			LOG.error(e, "VTN Service error occurred : " + e.getMessage());
			exceptionStatus = true;
			if (!(e instanceof VtnServiceWebAPIException)) {
				throw new VtnServiceWebAPIException(
						HttpErrorCodeEnum.UNC_INTERNAL_SERVER_ERROR.getCode());
			}
		} finally {
			try {
				// release monitoring mode
				if (isReadlock) {
					resource.setPath(ResourcePathEnum.ACQUIRE_RELEASE_MONITORING_PATH
							.getPath());
					resource.setSessionID(sessionId);
					if (resource.delete() != ApplicationConstants.SUCCESS) {
						LOG.error("JAVA API returns error # "
								+ resource.getInfo());
						responseStr = resource.getInfo() != null ? resource
								.getInfo().toString() : null;
						throw new VtnServiceWebAPIException();
					}
				}

				if (sessionId != ApplicationConstants.ZERO) {
					// release session
					resource.setPath(ResourcePathEnum.RELEASE_SESSION.getPath()
							+ sessionId);
					resource.setSessionID(sessionId);
					if (resource.delete() != ApplicationConstants.SUCCESS) {
						LOG.error("JAVA API returns error # "
								+ resource.getInfo());
						responseStr = resource.getInfo() != null ? resource
								.getInfo().toString() : null;
						throw new VtnServiceWebAPIException();
					}
				}
				if (null == responseStr && !exceptionStatus) {
					responseJSON = VtnServiceWebUtil
							.prepareErrResponseJson(HttpErrorCodeEnum.UNC_STATUS_OK
									.getCode());
				} else {
					responseJSON = new JSONObject(responseStr);
				}
			} catch (final JSONException exception) {
				LOG.error(exception, "VTN Service error occurred : "
						+ exception.getMessage());
				throw new VtnServiceWebAPIException(
						HttpServletResponse.SC_INTERNAL_SERVER_ERROR);
			}
		}
		LOG.trace("Complete VtnServiceWebAPIController#get()");
		return responseJSON;
	}

	/**
	 * Post.This method will be required when we have to create any component
	 * using Java API. This method will work in the following sequence- Create
	 * session acquire config mode calling actual Java API which is requested
	 * release config lock release session
	 * 
	 * @param sessionJson
	 *            the session json
	 * @param reqObject
	 *            the req object
	 * @param resourcePath
	 *            the resource path
	 * @return the jSON object
	 * @throws VtnServiceWebAPIException
	 *             the vtn service web api exception
	 */
	public JSONObject post(final JsonObject sessionJson,
			final JsonObject reqObject, final String resourcePath)
			throws VtnServiceWebAPIException {
		LOG.trace("Start VtnServiceWebAPIController#post()");
		JSONObject responseJSON = null;
		String responseStr = null;
		long sessionId = 0;
		long configId = 0;
		boolean exceptionStatus = false;
		final RestResource resource = new RestResource();
		try {
			LOG.debug("acquiring session id and config mode from java API");
			// acquire session id
			resource.setPath(ResourcePathEnum.SESSION_PATH.getPath());
			if (resource.post(sessionJson) != ApplicationConstants.SUCCESS) {
				LOG.error("JAVA API returns error # " + resource.getInfo());
				responseStr = resource.getInfo() != null ? resource.getInfo()
						.toString() : null;
				throw new VtnServiceWebAPIException();
			}
			sessionId = VtnServiceCommonUtil.getSessionFromJson(resource
					.getInfo());
			// acquire configure mode
			resource.setPath(ResourcePathEnum.ACQUIRE_CONFIG_PATH.getPath());
			resource.setSessionID(sessionId);
			if (resource.post(VtnServiceWebUtil.prepareAquireConfigJSON()) != ApplicationConstants.SUCCESS) {
				LOG.error("JAVA API returns error # " + resource.getInfo());
				responseStr = resource.getInfo() != null ? resource.getInfo()
						.toString() : null;
				throw new VtnServiceWebAPIException();
			}
			configId = VtnServiceCommonUtil.getConfigIdFromJson(resource
					.getInfo());
			// Abort operation to clear candidate DB
			responseStr = performAbort(sessionId, configId, resource);
			if (responseStr != null) {
				throw new VtnServiceWebAPIException();
			}
			// send the api request json
			resource.setPath(resourcePath);
			resource.setSessionID(sessionId);
			resource.setConfigID(configId);
			LOG.debug("Request object passing to JAVA API  #" + reqObject);
			final int status = resource.post(reqObject);
			LOG.debug("JAVA API returned error code #" + status);
			if (status != ApplicationConstants.SUCCESS) {
				LOG.error("JAVA API returns error # " + resource.getInfo());
				responseStr = resource.getInfo() != null ? resource.getInfo()
						.toString() : null;
				throw new VtnServiceWebAPIException();// to save the below steps
														// execution
			}
			responseStr = resource.getInfo() != null ? resource.getInfo()
					.toString() : null;
			LOG.debug("JAVA API returned response # " + responseStr);
			// responseStr = performCommit(sessionId, configId, resource);
			final String commitResponse = performCommit(sessionId, configId,
					resource);
			if (commitResponse != null) {
				responseStr = commitResponse;
				throw new VtnServiceWebAPIException();
			}
		} catch (final Exception e) {
			LOG.error(e, "VTN Service error occurred : " + e.getMessage());
			exceptionStatus = true;
			if (!(e instanceof VtnServiceWebAPIException)) {
				throw new VtnServiceWebAPIException(
						HttpErrorCodeEnum.UNC_INTERNAL_SERVER_ERROR.getCode());
			}
		} finally {
			try {
				if (configId != ApplicationConstants.ZERO) {
					// release monitoring mode
					resource.setPath(ResourcePathEnum.RELEASE_CONFIGURATION
							.getPath() + configId);
					resource.setSessionID(sessionId);
					resource.setConfigID(configId);
					if (resource.delete() != ApplicationConstants.SUCCESS) {
						LOG.error("JAVA API returns error # "
								+ resource.getInfo());
						responseStr = resource.getInfo() != null ? resource
								.getInfo().toString() : null;
						throw new VtnServiceWebAPIException();
					}
				}
				if (sessionId != ApplicationConstants.ZERO) {
					// release session
					resource.setPath(ResourcePathEnum.RELEASE_SESSION.getPath()
							+ sessionId);
					resource.setSessionID(sessionId);
					if (resource.delete() != ApplicationConstants.SUCCESS) {
						LOG.error("JAVA API returns error # "
								+ resource.getInfo());
						responseStr = resource.getInfo() != null ? resource
								.getInfo().toString() : null;
						throw new VtnServiceWebAPIException();
					}
				}

				if (null == responseStr && !exceptionStatus) {
					responseJSON = VtnServiceWebUtil
							.prepareErrResponseJson(HttpErrorCodeEnum.UNC_STATUS_OK
									.getCode());
				} else {
					responseJSON = new JSONObject(responseStr);
				}
			} catch (final JSONException exception) {
				LOG.error(exception, "VTN Service error occurred : "
						+ exception.getMessage());
				throw new VtnServiceWebAPIException(
						HttpErrorCodeEnum.UNC_INTERNAL_SERVER_ERROR.getCode());
			}

		}
		LOG.trace("Complete VtnServiceWebAPIController#post()");
		return responseJSON;
	}

	/**
	 * Put.This method will be required when we have to update any component
	 * using Java API. This method will work in the following sequence- Create
	 * session acquire config mode calling actual Java API which is requested
	 * release config lock release session
	 * 
	 * @param sessionJson
	 *            the session json
	 * @param reqObject
	 *            the req object
	 * @param resourcePath
	 *            the resource path
	 * @return the jSON object
	 * @throws VtnServiceWebAPIException
	 *             the vtn service web api exception
	 */
	public JSONObject put(final JsonObject sessionJson,
			final JsonObject reqObject, final String resourcePath)
			throws VtnServiceWebAPIException {
		LOG.trace("Start VtnServiceWebAPIController#put()");
		JSONObject responseJSON = null;
		String responseStr = null;
		long sessionId = 0;
		long configId = 0;
		boolean exceptionStatus = false;
		final RestResource resource = new RestResource();
		try {
			LOG.debug("acquiring session id and config mode from java API");
			// acquire session id
			resource.setPath(ResourcePathEnum.SESSION_PATH.getPath());
			if (resource.post(sessionJson) != ApplicationConstants.SUCCESS) {
				LOG.error("JAVA API returns error # " + resource.getInfo());
				responseStr = resource.getInfo() != null ? resource.getInfo()
						.toString() : null;
				throw new VtnServiceWebAPIException();
			}
			sessionId = VtnServiceCommonUtil.getSessionFromJson(resource
					.getInfo());
			// acquire configure mode
			resource.setPath(ResourcePathEnum.ACQUIRE_CONFIG_PATH.getPath());
			resource.setSessionID(sessionId);
			if (resource.post(VtnServiceWebUtil.prepareAquireConfigJSON()) != ApplicationConstants.SUCCESS) {
				LOG.error("JAVA API returns error # " + resource.getInfo());
				responseStr = resource.getInfo() != null ? resource.getInfo()
						.toString() : null;
				throw new VtnServiceWebAPIException();
			}
			configId = VtnServiceCommonUtil.getConfigIdFromJson(resource
					.getInfo());
			responseStr = performAbort(sessionId, configId, resource);
			if (responseStr != null) {
				throw new VtnServiceWebAPIException();
			}
			// send the api request json
			resource.setPath(resourcePath);
			resource.setConfigID(configId);
			resource.setSessionID(sessionId);
			LOG.debug("Request object passing to JAVA API  #" + reqObject);
			final int status = resource.put(reqObject);
			LOG.debug("JAVA API returned error code #" + status);
			if (status != ApplicationConstants.SUCCESS) {
				LOG.error("JAVA API returns error # " + resource.getInfo());
				responseStr = resource.getInfo() != null ? resource.getInfo()
						.toString() : null;
				throw new VtnServiceWebAPIException();// to save the below steps
														// execution
			}
			responseStr = resource.getInfo() != null ? resource.getInfo()
					.toString() : null;
			LOG.debug("JAVA API returned response # " + responseStr);
			// responseStr = performCommit(sessionId, configId, resource);
			final String commitResponse = performCommit(sessionId, configId,
					resource);
			if (commitResponse != null) {
				responseStr = commitResponse;
				throw new VtnServiceWebAPIException();
			}
		} catch (final Exception e) {
			LOG.error(e, "VTN Service error occurred : " + e.getMessage());
			exceptionStatus = true;
			if (!(e instanceof VtnServiceWebAPIException)) {
				throw new VtnServiceWebAPIException(
						HttpErrorCodeEnum.UNC_INTERNAL_SERVER_ERROR.getCode());
			}
		} finally {
			try {
				if (configId != ApplicationConstants.ZERO) {
					// release monitoring mode
					resource.setPath(ResourcePathEnum.RELEASE_CONFIGURATION
							.getPath() + configId);
					resource.setSessionID(sessionId);
					resource.setConfigID(configId);
					if (resource.delete() != ApplicationConstants.SUCCESS) {
						LOG.error("JAVA API returns error # "
								+ resource.getInfo());
						responseStr = resource.getInfo() != null ? resource
								.getInfo().toString() : null;
						throw new VtnServiceWebAPIException();
					}
				}
				if (sessionId != ApplicationConstants.ZERO) {
					// release session
					resource.setPath(ResourcePathEnum.RELEASE_SESSION.getPath()
							+ sessionId);
					resource.setSessionID(sessionId);
					if (resource.delete() != ApplicationConstants.SUCCESS) {
						LOG.error("JAVA API returns error # "
								+ resource.getInfo());
						responseStr = resource.getInfo() != null ? resource
								.getInfo().toString() : null;
						throw new VtnServiceWebAPIException();
					}
				}

				if (null == responseStr && !exceptionStatus) {
					responseJSON = VtnServiceWebUtil
							.prepareErrResponseJson(HttpErrorCodeEnum.UNC_STATUS_OK
									.getCode());
				} else {
					responseJSON = new JSONObject(responseStr);
				}
			} catch (final JSONException exception) {
				LOG.error(exception, "VTN Service error occurred : "
						+ exception.getMessage());
				throw new VtnServiceWebAPIException(
						HttpErrorCodeEnum.UNC_INTERNAL_SERVER_ERROR.getCode());
			}

		}
		LOG.trace("Complete VtnServiceWebAPIController#put()");
		return responseJSON;
	}

	/**
	 * Delete.This method will be required when we have to delete any component
	 * using Java API. This method will work in the following sequence- Create
	 * session acquire config mode calling actual Java API which is requested
	 * release config lock release session
	 * 
	 * @param sessionJson
	 *            the session json
	 * @param reqObject
	 *            the req object
	 * @param resourcePath
	 *            the resource path
	 * @return the jSON object
	 * @throws VtnServiceWebAPIException
	 *             the vtn service web api exception
	 */
	public JSONObject delete(final JsonObject sessionJson,
			final JsonObject reqObject, final String resourcePath)
			throws VtnServiceWebAPIException {
		LOG.trace("Start VtnServiceWebAPIController#delete()");
		JSONObject responseJSON = null;
		String responseStr = null;
		long sessionId = 0;
		long configId = 0;
		boolean exceptionStatus = false;
		final RestResource resource = new RestResource();
		try {
			LOG.debug("acquiring session id and config mode from java API");
			// acquire session id
			resource.setPath(ResourcePathEnum.SESSION_PATH.getPath());
			if (resource.post(sessionJson) != ApplicationConstants.SUCCESS) {
				LOG.error("JAVA API returns error # " + resource.getInfo());
				responseStr = resource.getInfo() != null ? resource.getInfo()
						.toString() : null;
				throw new VtnServiceWebAPIException();
			}
			sessionId = VtnServiceCommonUtil.getSessionFromJson(resource
					.getInfo());
			// acquire configure mode
			resource.setPath(ResourcePathEnum.ACQUIRE_CONFIG_PATH.getPath());
			resource.setSessionID(sessionId);
			if (resource.post(VtnServiceWebUtil.prepareAquireConfigJSON()) != ApplicationConstants.SUCCESS) {
				LOG.error("JAVA API returns error # " + resource.getInfo());
				responseStr = resource.getInfo() != null ? resource.getInfo()
						.toString() : null;
				throw new VtnServiceWebAPIException();
			}
			configId = VtnServiceCommonUtil.getConfigIdFromJson(resource
					.getInfo());
			responseStr = performAbort(sessionId, configId, resource);
			if (responseStr != null) {
				throw new VtnServiceWebAPIException();
			}
			// send the api request json
			resource.setPath(resourcePath);
			resource.setConfigID(configId);
			resource.setSessionID(sessionId);
			LOG.debug("Request object passing to JAVA API  #" + reqObject);
			final int status = resource.delete(reqObject);
			LOG.debug("JAVA API returned error code #" + status);
			if (status != ApplicationConstants.SUCCESS) {
				LOG.error("JAVA API returns error # " + resource.getInfo());
				responseStr = resource.getInfo() != null ? resource.getInfo()
						.toString() : null;
				throw new VtnServiceWebAPIException();// to save the below steps
														// execution
			}
			responseStr = resource.getInfo() != null ? resource.getInfo()
					.toString() : null;
			LOG.debug("JAVA API returned response # " + responseStr);
			// responseStr = performCommit(sessionId, configId, resource);
			final String commitResponse = performCommit(sessionId, configId,
					resource);
			if (commitResponse != null) {
				responseStr = commitResponse;
				throw new VtnServiceWebAPIException();
			}
		} catch (final Exception e) {
			LOG.error(e, "VTN Service error occurred : " + e.getMessage());
			exceptionStatus = true;
			if (!(e instanceof VtnServiceWebAPIException)) {
				throw new VtnServiceWebAPIException(
						HttpErrorCodeEnum.UNC_INTERNAL_SERVER_ERROR.getCode());
			}
		} finally {
			try {
				if (configId != ApplicationConstants.ZERO) {
					// release monitoring mode
					resource.setPath(ResourcePathEnum.RELEASE_CONFIGURATION
							.getPath() + configId);
					resource.setSessionID(sessionId);
					resource.setConfigID(configId);
					if (resource.delete() != ApplicationConstants.SUCCESS) {
						LOG.error("JAVA API returns error # "
								+ resource.getInfo());
						responseStr = resource.getInfo() != null ? resource
								.getInfo().toString() : null;
						throw new VtnServiceWebAPIException();
					}
				}
				if (sessionId != ApplicationConstants.ZERO) {
					// release session
					resource.setPath(ResourcePathEnum.RELEASE_SESSION.getPath()
							+ sessionId);
					resource.setSessionID(sessionId);
					if (resource.delete() != ApplicationConstants.SUCCESS) {
						LOG.error("JAVA API returns error # "
								+ resource.getInfo());
						responseStr = resource.getInfo() != null ? resource
								.getInfo().toString() : null;
						throw new VtnServiceWebAPIException();
					}
				}
				if (null == responseStr && !exceptionStatus) {
					responseJSON = VtnServiceWebUtil
							.prepareErrResponseJson(HttpErrorCodeEnum.UNC_STATUS_OK
									.getCode());
				} else {
					responseJSON = new JSONObject(responseStr);
				}
			} catch (final JSONException exception) {
				LOG.error(exception, "VTN Service error occurred : "
						+ exception.getMessage());
				throw new VtnServiceWebAPIException(
						HttpErrorCodeEnum.UNC_INTERNAL_SERVER_ERROR.getCode());
			}

		}
		LOG.trace("Complete VtnServiceWebAPIController#delete()");
		return responseJSON;
	}

	/**
	 * Delete.This method will be required when we have to delete any component
	 * using Java API. This method will work in the following sequence- Create
	 * session acquire config mode calling actual Java API which is requested
	 * release config lock release session
	 * 
	 * @param sessionJson
	 *            the session json
	 * @param resourcePath
	 *            the resource path
	 * @return the jSON object
	 * @throws VtnServiceWebAPIException
	 *             the vtn service web api exception
	 */
	public JSONObject delete(final JsonObject sessionJson,
			final String resourcePath) throws VtnServiceWebAPIException {
		LOG.trace("Start VtnServiceWebAPIController#delete()");
		JSONObject responseJSON = null;
		String responseStr = null;
		long sessionId = 0;
		long configId = 0;
		boolean exceptionStatus = false;
		final RestResource resource = new RestResource();
		try {
			LOG.debug("acquiring session id and config mode from java API");
			// acquire session id
			resource.setPath(ResourcePathEnum.SESSION_PATH.getPath());
			if (resource.post(sessionJson) != ApplicationConstants.SUCCESS) {
				LOG.error("JAVA API returns error # " + resource.getInfo());
				responseStr = resource.getInfo() != null ? resource.getInfo()
						.toString() : null;
				throw new VtnServiceWebAPIException();
			}
			sessionId = VtnServiceCommonUtil.getSessionFromJson(resource
					.getInfo());
			// acquire configure mode
			resource.setPath(ResourcePathEnum.ACQUIRE_CONFIG_PATH.getPath());
			resource.setSessionID(sessionId);
			if (resource.post(VtnServiceWebUtil.prepareAquireConfigJSON()) != ApplicationConstants.SUCCESS) {
				LOG.error("JAVA API returns error # " + resource.getInfo());
				responseStr = resource.getInfo() != null ? resource.getInfo()
						.toString() : null;
				throw new VtnServiceWebAPIException();
			}
			configId = VtnServiceCommonUtil.getConfigIdFromJson(resource
					.getInfo());
			responseStr = performAbort(sessionId, configId, resource);
			if (responseStr != null) {
				throw new VtnServiceWebAPIException();
			}
			// send the api request json
			resource.setPath(resourcePath);
			resource.setConfigID(configId);
			resource.setSessionID(sessionId);
			final int status = resource.delete();
			LOG.debug("JAVA API returned error code #" + status);
			if (status != ApplicationConstants.SUCCESS) {
				LOG.error("JAVA API returns error # " + resource.getInfo());
				responseStr = resource.getInfo() != null ? resource.getInfo()
						.toString() : null;
				throw new VtnServiceWebAPIException();// to save the below steps
														// execution
			}
			responseStr = resource.getInfo() != null ? resource.getInfo()
					.toString() : null;
			LOG.debug("JAVA API returned response # " + responseStr);
			// responseStr = performCommit(sessionId, configId, resource);
			final String commitResponse = performCommit(sessionId, configId,
					resource);
			if (commitResponse != null) {
				responseStr = commitResponse;
				throw new VtnServiceWebAPIException();
			}
		} catch (final Exception e) {
			LOG.error(e, "VTN Service error occurred : " + e.getMessage());
			exceptionStatus = true;
			if (!(e instanceof VtnServiceWebAPIException)) {
				throw new VtnServiceWebAPIException(
						HttpErrorCodeEnum.UNC_INTERNAL_SERVER_ERROR.getCode());
			}
		} finally {
			try {
				if (configId != ApplicationConstants.ZERO) {
					// release monitoring mode
					resource.setPath(ResourcePathEnum.RELEASE_CONFIGURATION
							.getPath() + configId);
					resource.setSessionID(sessionId);
					resource.setConfigID(configId);
					if (resource.delete() != ApplicationConstants.SUCCESS) {
						LOG.error("JAVA API returns error # "
								+ resource.getInfo());
						responseStr = resource.getInfo() != null ? resource
								.getInfo().toString() : null;
						throw new VtnServiceWebAPIException();
					}
				}
				if (sessionId != ApplicationConstants.ZERO) {
					// release session
					resource.setPath(ResourcePathEnum.RELEASE_SESSION.getPath()
							+ sessionId);
					resource.setSessionID(sessionId);
					if (resource.delete() != ApplicationConstants.SUCCESS) {
						LOG.error("JAVA API returns error # "
								+ resource.getInfo());
						responseStr = resource.getInfo() != null ? resource
								.getInfo().toString() : null;
						throw new VtnServiceWebAPIException();
					}
				}
				if (null == responseStr && !exceptionStatus) {
					responseJSON = VtnServiceWebUtil
							.prepareErrResponseJson(HttpErrorCodeEnum.UNC_STATUS_OK
									.getCode());
				} else {
					responseJSON = new JSONObject(responseStr);
				}
			} catch (final JSONException exception) {
				LOG.error(exception, "VTN Service error occurred : "
						+ exception.getMessage());
				throw new VtnServiceWebAPIException(
						HttpErrorCodeEnum.UNC_INTERNAL_SERVER_ERROR.getCode());
			}
		}
		LOG.trace("Complete VtnServiceWebAPIController#delete()");
		return responseJSON;
	}

	/**
	 * Execute commit API for given resource, session and configuration id
	 * 
	 * @param responseStr
	 * @param sessionId
	 * @param configId
	 * @param resource
	 * @return : response from JavaAPI
	 * @throws VtnServiceWebAPIException
	 */
	private String performCommit(long sessionId, long configId,
			final RestResource resource) throws VtnServiceWebAPIException {
		LOG.trace("Start VtnServiceWebAPIController#performCommit()");
		String responseStr = null;
		resource.setPath(ResourcePathEnum.COMMIT_CONFIGURATION.getPath());
		resource.setSessionID(sessionId);
		resource.setConfigID(configId);
		final int commitStatus = resource
				.put(VtnServiceWebUtil
						.prepareConfigCommitOrSaveJSON(ApplicationConstants.OPERATION_COMMIT));
		if (commitStatus != ApplicationConstants.SUCCESS) {
			LOG.error("JAVA API returns error # " + resource.getInfo());
			responseStr = resource.getInfo() != null ? resource.getInfo()
					.toString() : null;
			performAbort(sessionId, configId, resource);
		}
		LOG.trace("Complete VtnServiceWebAPIController#performCommit()");
		return responseStr;
	}

	/**
	 * Execute abort API for given resource, session and configuration id
	 * 
	 * @param responseStr
	 * @param sessionId
	 * @param configId
	 * @param resource
	 * @return : response from JavaAPI
	 * @throws VtnServiceWebAPIException
	 */
	private String performAbort(long sessionId, long configId,
			final RestResource resource) throws VtnServiceWebAPIException {
		LOG.trace("Start VtnServiceWebAPIController#performAbort()");
		String responseStr = null;
		resource.setPath(ResourcePathEnum.ABORT_CONFIGURATION.getPath());
		resource.setSessionID(sessionId);
		resource.setConfigID(configId);
		if (resource
				.put(VtnServiceWebUtil
						.prepareCandidateAbortJSON(ApplicationConstants.OPERATION_ABORT)) != ApplicationConstants.SUCCESS) {
			LOG.error("JAVA API returns error # " + resource.getInfo());
			responseStr = resource.getInfo() != null ? resource.getInfo()
					.toString() : null;
		}
		LOG.trace("Complete VtnServiceWebAPIController#performAbort()");
		return responseStr;
	}
}
