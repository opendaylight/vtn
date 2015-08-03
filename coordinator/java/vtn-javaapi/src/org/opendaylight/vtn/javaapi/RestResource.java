/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi;

import java.sql.Connection;
import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.exception.VtnServiceExceptionHandler;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.reflect.AnnotationReflect;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.resources.ConfigResource;
import org.opendaylight.vtn.javaapi.util.VtnServiceUtil;

/**
 * The Class RestResource. JavaAPI interfaces for post, put, create and delete
 * provided by this class. There are some other interfaces also provided for the
 * JavaAPI functionality.
 */
public class RestResource implements VtnServiceResource {

	private static final Logger LOG = Logger.getLogger(RestResource.class
			.getName());

	/* Session ID and Configuration ID instance variables */
	private long sessionID;
	private long configID;

	private transient AbstractResource resource;
	private final transient VtnServiceExceptionHandler exceptionHandler;

	/* Resource path and Response information instances */
	private String path;
	private transient JsonObject info;

	/* OpenStack DB Connection instance */
	Connection openStackConnection;

	/**
	 * Instantiates a new rest resource.
	 */
	public RestResource() {
		LOG.trace("Start RestResource#RestResource()");
		exceptionHandler = new VtnServiceExceptionHandler();
		LOG.trace("Complete RestResource#RestResource()");
	}

	/**
	 * Instantiates a new rest resource. And Sets the resource path
	 * 
	 * @param resourcePath
	 *            the resource path
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	public RestResource(final String resourcePath) {
		LOG.trace("Start RestResource#RestResource()");
		exceptionHandler = new VtnServiceExceptionHandler();
		setPath(resourcePath);
		LOG.trace("Complete RestResource#RestResource()");
	}

	/**
	 * Sets the resource path
	 * 
	 * @param resourcePath
	 *            the new resource path
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	public final void setPath(final String resourcePath) {
		LOG.trace("Start RestResource#setResourcePath()");
		boolean resourceFoundStatus = false;
		try {
			LOG.debug("Input value for resourcePath : " + resourcePath);
			this.path = resourcePath.replaceAll(VtnServiceConsts.WHITESPACE,
					VtnServiceConsts.EMPTY_STRING);
			// Resolve the resource class class object corresponding to current
			// resource path
			if (VtnServiceUtil.isValidString(this.path)) {
				this.resource = AnnotationReflect.getResource(this.path,
						exceptionHandler);
				//when FDB support in U18, delete the code 
				String[] uri = this.path.substring(1).split("/");
				if (uri.length == 5) {
					if (uri[0].equals("unified_networks")&& uri[2].equals("spine_domains")
							&& uri[4].equals("fdbusage")) {
						this.resource = null;
					} 
				}

				if (this.resource == null) {
					resourceFoundStatus = true;
					LOG.error("Resource not found");
					throw new VtnServiceException(
							Thread.currentThread().getStackTrace()[1]
									.getClassName()
									+ VtnServiceConsts.HYPHEN
									+ Thread.currentThread().getStackTrace()[1]
											.getMethodName(),
							UncJavaAPIErrorCode.RESOURCE_NOT_FOUND_ERROR
									.getErrorCode(),
							UncJavaAPIErrorCode.RESOURCE_NOT_FOUND_ERROR
									.getErrorMessage());
				}
				if (this.resource.getExceptionHandler() == null) {
					resource.setExceptionHandler(exceptionHandler);
				}
				resource.setConnPool();
				if (resource instanceof ConfigResource) {
					resource.setOpenStackConnection(openStackConnection);
				}
			}
		} catch (final RuntimeException exception) {
			exceptionHandler.handle(
					Thread.currentThread().getStackTrace()[1].getClassName()
							+ VtnServiceConsts.HYPHEN
							+ Thread.currentThread().getStackTrace()[1]
									.getMethodName(),
					UncJavaAPIErrorCode.RESOURCE_PATH_ERROR.getErrorCode(),
					UncJavaAPIErrorCode.RESOURCE_PATH_ERROR.getErrorMessage(),
					exception);
			resourceFoundStatus = true;
		} catch (final VtnServiceException exception) {
			exceptionHandler
					.handle(Thread.currentThread().getStackTrace()[1]
							.getClassName()
							+ VtnServiceConsts.HYPHEN
							+ Thread.currentThread().getStackTrace()[1]
									.getMethodName(),
							UncJavaAPIErrorCode.RESOURCE_NOT_FOUND_ERROR
									.getErrorCode(),
							UncJavaAPIErrorCode.RESOURCE_NOT_FOUND_ERROR
									.getErrorMessage(), exception);
			resourceFoundStatus = true;
		} finally {
			/*
			 * resource can not be resolved then set error Json with common
			 * resource path not correct message
			 */
			if (resourceFoundStatus) {
				final JsonObject errorJsonObject = new JsonObject();
				final JsonObject error = new JsonObject();
				error.addProperty(VtnServiceJsonConsts.CODE,
						UncCommonEnum.UncResultCode.UNC_NOT_FOUND.getValue());
				error.addProperty(VtnServiceJsonConsts.MSG,
						UncCommonEnum.UncResultCode.UNC_NOT_FOUND.getMessage());
				errorJsonObject.add(VtnServiceJsonConsts.ERROR, error);
				info = errorJsonObject;
			}
		}
		LOG.trace("Complete RestResource#setResourcePath()");
	}

	/**
	 * Gets the resource path.
	 * 
	 * @return the resource path
	 */
	public final String getPath() {
		LOG.trace("Return from RestResource#getResourcePath()");
		LOG.debug("Return value for path : " + path);
		return path;
	}

	/**
	 * Deletes resource and prepares the response information
	 * 
	 */
	@Override
	public final int delete() {
		LOG.trace("Start RestResource#delete()");
		int responseCode = UncCommonEnum.UncResultCode.UNC_CLIENT_ERROR
				.getValue();

		/*
		 * If Resource not found then ERROR CODE (400) will be returned
		 */
		if (resource != null) {
			try {
				validateURI(VtnServiceConsts.DELETE, resource);

				/*
				 * ERROR CODE (500) will be returned for any other type of
				 * errors
				 */
				responseCode = UncCommonEnum.UncResultCode.UNC_SERVER_ERROR
						.getValue();
				responseCode = resource.delete();
				// if response is not success, then set the info for error
				if (responseCode != UncResultCode.UNC_SUCCESS.getValue()) {
					info = resource.getInfo();
				} else {
					openStackConnection = resource.getOpenStackConnection();
				}
			} catch (final VtnServiceException exception) {
				exceptionHandler.handle(
						Thread.currentThread().getStackTrace()[1]
								.getClassName()
								+ VtnServiceConsts.HYPHEN
								+ Thread.currentThread().getStackTrace()[1]
										.getMethodName(),
						UncJavaAPIErrorCode.DELETE_ERROR.getErrorCode(),
						UncJavaAPIErrorCode.DELETE_ERROR.getErrorMessage(),
						exception);
			} catch (final RuntimeException exception) {
				exceptionHandler.handle(
						Thread.currentThread().getStackTrace()[1]
								.getClassName()
								+ VtnServiceConsts.HYPHEN
								+ Thread.currentThread().getStackTrace()[1]
										.getMethodName(),
						UncJavaAPIErrorCode.INTERNAL_ERROR.getErrorCode(),
						UncJavaAPIErrorCode.INTERNAL_ERROR.getErrorMessage(),
						exception);
			}
		}
		responseCode = convertResponseCode(responseCode);
		LOG.trace("Complete RestResource#delete()");
		return responseCode;
	}

	/**
	 * Deletes resource and prepares the response information
	 * 
	 */
	@Override
	public final int delete(final JsonObject queryString) {
		LOG.trace("Start RestResource#delete()");
		int responseCode = UncResultCode.UNC_CLIENT_ERROR.getValue();
		/*
		 * If Resource not found then ERROR CODE (400) will be returned
		 */
		if (resource != null && queryString != null) {
			LOG.debug("Input value for queryString : " + queryString);
			try {
				validateJson(VtnServiceConsts.DELETE, queryString, resource);
				/*
				 * ERROR CODE (500) will be returned for any other type of
				 * errors Performs validation before initiating the delete
				 */
				responseCode = UncCommonEnum.UncResultCode.UNC_SERVER_ERROR
						.getValue();
				responseCode = resource.delete(queryString);
				// if response is not success, then set the info for error
				if (responseCode != UncResultCode.UNC_SUCCESS.getValue()) {
					info = resource.getInfo();
				} else {
					openStackConnection = resource.getOpenStackConnection();
				}
			} catch (final VtnServiceException exception) {
				exceptionHandler.handle(
						Thread.currentThread().getStackTrace()[1]
								.getClassName()
								+ VtnServiceConsts.HYPHEN
								+ Thread.currentThread().getStackTrace()[1]
										.getMethodName(),
						UncJavaAPIErrorCode.DELETE_ERROR.getErrorCode(),
						UncJavaAPIErrorCode.DELETE_ERROR.getErrorMessage(),
						exception);
			} catch (final RuntimeException exception) {
				exceptionHandler.handle(
						Thread.currentThread().getStackTrace()[1]
								.getClassName()
								+ VtnServiceConsts.HYPHEN
								+ Thread.currentThread().getStackTrace()[1]
										.getMethodName(),
						UncJavaAPIErrorCode.INTERNAL_ERROR.getErrorCode(),
						UncJavaAPIErrorCode.INTERNAL_ERROR.getErrorMessage(),
						exception);
			}
		}
		responseCode = convertResponseCode(responseCode);
		LOG.trace("Complete RestResource#delete()");
		return responseCode;
	}

	/**
	 * Retrieves resource and prepares the response information
	 * 
	 */
	@Override
	public final int get() {
		LOG.trace("Start RestResource#get()");
		int responseCode = UncResultCode.UNC_CLIENT_ERROR.getValue();
		/*
		 * If Resource not found then ERROR CODE (400) will be returned
		 */
		if (resource != null) {
			try {
				validateURI(VtnServiceConsts.GET, resource);
				/*
				 * ERROR CODE (500) will be returned for any other type of
				 * errors
				 */
				responseCode = UncCommonEnum.UncResultCode.UNC_SERVER_ERROR
						.getValue();
				responseCode = resource.get();
				info = resource.getInfo();
				openStackConnection = resource.getOpenStackConnection();
			} catch (final VtnServiceException exception) {
				exceptionHandler.handle(
						Thread.currentThread().getStackTrace()[1]
								.getClassName()
								+ VtnServiceConsts.HYPHEN
								+ Thread.currentThread().getStackTrace()[1]
										.getMethodName(),
						UncJavaAPIErrorCode.GET_ERROR.getErrorCode(),
						UncJavaAPIErrorCode.GET_ERROR.getErrorMessage(),
						exception);
			} catch (final RuntimeException exception) {
				exceptionHandler.handle(
						Thread.currentThread().getStackTrace()[1]
								.getClassName()
								+ VtnServiceConsts.HYPHEN
								+ Thread.currentThread().getStackTrace()[1]
										.getMethodName(),
						UncJavaAPIErrorCode.INTERNAL_ERROR.getErrorCode(),
						UncJavaAPIErrorCode.INTERNAL_ERROR.getErrorMessage(),
						exception);
				// resource.createErrorInfo();
			}
		}
		responseCode = convertResponseCode(responseCode);
		LOG.trace("Complete RestResource#get()");
		return responseCode;
	}

	/**
	 * Retrieves resource and prepares the response information
	 * 
	 */
	@Override
	public final int get(final JsonObject queryString) {
		LOG.trace("Start RestResource#get()");
		int responseCode = UncResultCode.UNC_CLIENT_ERROR.getValue();
		/*
		 * If Resource not found then ERROR CODE (400) will be returned
		 */
		if (resource != null && queryString != null) {
			LOG.debug("Input value for queryString : " + queryString);
			try {
				validateJson(VtnServiceConsts.GET, queryString, resource);
				validateJsonOp(queryString);
				/*
				 * ERROR CODE (500) will be returned for any other type of
				 * errors Performs validation before initiating the get
				 */
				responseCode = UncCommonEnum.UncResultCode.UNC_SERVER_ERROR
						.getValue();
				responseCode = resource.get(queryString);
				info = resource.getInfo();
				openStackConnection = resource.getOpenStackConnection();
			} catch (final VtnServiceException exception) {
				exceptionHandler.handle(
						Thread.currentThread().getStackTrace()[1]
								.getClassName()
								+ VtnServiceConsts.HYPHEN
								+ Thread.currentThread().getStackTrace()[1]
										.getMethodName(),
						UncJavaAPIErrorCode.GET_ERROR.getErrorCode(),
						UncJavaAPIErrorCode.GET_ERROR.getErrorMessage(),
						exception);
			} catch (final RuntimeException exception) {
				exceptionHandler.handle(
						Thread.currentThread().getStackTrace()[1]
								.getClassName()
								+ VtnServiceConsts.HYPHEN
								+ Thread.currentThread().getStackTrace()[1]
										.getMethodName(),
						UncJavaAPIErrorCode.INTERNAL_ERROR.getErrorCode(),
						UncJavaAPIErrorCode.INTERNAL_ERROR.getErrorMessage(),
						exception);
				// resource.createErrorInfo();
			}
		}
		responseCode = convertResponseCode(responseCode);
		LOG.trace("Complete RestResource#get()");
		return responseCode;
	}

	/**
	 * Creates resource and prepares the response information
	 * 
	 */
	@Override
	public final int post(final JsonObject requestBody) {
		LOG.trace("Start RestResource#post()");
		int responseCode = UncResultCode.UNC_CLIENT_ERROR.getValue();
		/*
		 * If Resource not found then ERROR CODE (400) will be returned
		 */
		if (resource != null && requestBody != null) {
			LOG.debug("Input value for requestBody : " + requestBody);
			try {
				validateJson(VtnServiceConsts.POST, requestBody, resource);
				/*
				 * ERROR CODE (500) will be returned for any other type of
				 * errors Performs validation before initiating the post
				 */
				responseCode = UncCommonEnum.UncResultCode.UNC_SERVER_ERROR
						.getValue();
				responseCode = resource.post(requestBody);
				// if response is not success, then set the info for error
				if (responseCode != UncResultCode.UNC_SUCCESS.getValue()) {
					info = resource.getInfo();
				} else {
					openStackConnection = resource.getOpenStackConnection();
				}
			} catch (final VtnServiceException exception) {
				exceptionHandler.handle(
						Thread.currentThread().getStackTrace()[1]
								.getClassName()
								+ VtnServiceConsts.HYPHEN
								+ Thread.currentThread().getStackTrace()[1]
										.getMethodName(),
						UncJavaAPIErrorCode.POST_ERROR.getErrorCode(),
						UncJavaAPIErrorCode.POST_ERROR.getErrorMessage(),
						exception);
			} catch (final RuntimeException exception) {
				exceptionHandler.handle(
						Thread.currentThread().getStackTrace()[1]
								.getClassName()
								+ VtnServiceConsts.HYPHEN
								+ Thread.currentThread().getStackTrace()[1]
										.getMethodName(),
						UncJavaAPIErrorCode.INTERNAL_ERROR.getErrorCode(),
						UncJavaAPIErrorCode.INTERNAL_ERROR.getErrorMessage(),
						exception);
			}
		}
		responseCode = convertResponseCode(responseCode);
		LOG.trace("Complete RestResource#post()");
		return responseCode;
	}

	/**
	 * Updates resource and prepares the response information
	 * 
	 */
	@Override
	public final int put(final JsonObject requestBody) {
		LOG.trace("Start RestResource#put()");
		int responseCode = UncResultCode.UNC_CLIENT_ERROR.getValue();
		/*
		 * If Resource not found then ERROR CODE (400) will be returned
		 */
		if (resource != null && requestBody != null) {
			LOG.debug("Input value for requestBody : " + requestBody);
			try {
				validateJson(VtnServiceConsts.PUT, requestBody, resource);
				/*
				 * ERROR CODE (500) will be returned for any other type of
				 * errors Performs validation before initiating the put
				 */
				responseCode = UncCommonEnum.UncResultCode.UNC_SERVER_ERROR
						.getValue();
				responseCode = resource.put(requestBody);
				// if response is not success, then set the info for error
				if (responseCode != UncResultCode.UNC_SUCCESS.getValue()) {
					info = resource.getInfo();
				} else {
					openStackConnection = resource.getOpenStackConnection();
				}
			} catch (final VtnServiceException exception) {
				exceptionHandler.handle(
						Thread.currentThread().getStackTrace()[1]
								.getClassName()
								+ VtnServiceConsts.HYPHEN
								+ Thread.currentThread().getStackTrace()[1]
										.getMethodName(),
						UncJavaAPIErrorCode.PUT_ERROR.getErrorCode(),
						UncJavaAPIErrorCode.PUT_ERROR.getErrorMessage(),
						exception);
			} catch (final RuntimeException exception) {
				exceptionHandler.handle(
						Thread.currentThread().getStackTrace()[1]
								.getClassName()
								+ VtnServiceConsts.HYPHEN
								+ Thread.currentThread().getStackTrace()[1]
										.getMethodName(),
						UncJavaAPIErrorCode.INTERNAL_ERROR.getErrorCode(),
						UncJavaAPIErrorCode.INTERNAL_ERROR.getErrorMessage(),
						exception);
			}
		}
		responseCode = convertResponseCode(responseCode);
		LOG.trace("Complete RestResource#put()");
		return responseCode;
	}

	/**
	 * Gets the config id.
	 * 
	 * @return the config id
	 */
	public final long getConfigID() {
		LOG.trace("Return from RestResource#getConfigId()");
		LOG.debug("Return value for configID : " + configID);
		return configID;
	}

	/**
	 * Sets the config id.
	 * 
	 * @param configId
	 *            the new config id
	 */
	public final void setConfigID(final long configId) {
		LOG.trace("Start RestResource#setConfigId()");
		LOG.debug("Input value for configId : " + configId);
		this.configID = configId;
		if (null != resource) {
			resource.setConfigID(this.configID);
		}
		LOG.trace("Complete RestResource#setConfigId()");
	}

	/**
	 * Gets the session id.
	 * 
	 * @return the session id
	 */
	public final long getSessionID() {
		LOG.trace("Return from RestResource#getSessionId()");
		LOG.debug("Return value for sessionID : " + sessionID);
		return sessionID;
	}

	/**
	 * Sets the session id.
	 * 
	 * @param sessionId
	 *            the new session id
	 */
	public final void setSessionID(final long sessionId) {
		LOG.trace("Start RestResource#setSessionId()");
		LOG.debug("Input value for sessionId : " + sessionId);
		this.sessionID = sessionId;
		if (null != resource) {
			resource.setSessionID(this.sessionID);
		}
		LOG.trace("Complete RestResource#setSessionId()");
	}

	/**
	 * Gets the info.
	 * 
	 * @return the info
	 */
	public final JsonObject getInfo() {
		LOG.trace("Return from RestResource#getInfo()");
		if (resource != null) {
			info = resource.getInfo();
		}
		LOG.debug("Return value for info : " + info);
		return info;
	}

	/**
	 * Log the returned value for response code and convert response code as per
	 * error code
	 * 
	 * @param responseCode
	 */
	private int convertResponseCode(final int responseCode) {
		LOG.debug("Return value for responseCode : " + responseCode);
		int finalresponseCode = responseCode;
		if (responseCode == UncResultCode.UNC_SERVER_ERROR.getValue()
				&& null != resource.getInfo()) {
			if (String.valueOf(VtnServiceJsonConsts.VAL_4).equals(
					resource.getInfo().get(VtnServiceJsonConsts.ERROR)
							.getAsJsonObject().get(VtnServiceJsonConsts.CODE)
							.getAsString().substring(0, 1))) {
				finalresponseCode = UncResultCode.UNC_CLIENT_ERROR.getValue();
			}
		}
		LOG.debug("Return value for finalresponseCode : " + finalresponseCode);
		return finalresponseCode;
	}

	/**
	 * Calls the respective validator to execute the validation of URI
	 * parameters and create the error Json if validation fails
	 * 
	 * @param requestType
	 * @param resource
	 * @return
	 */
	private void validateURI(final String requestType,
			final AbstractResource resource) throws VtnServiceException {
		try {
			resource.getValidator().validate(requestType, resource);
		} catch (final Exception exception) {
			resource.createErrorInfo(UncCommonEnum.UncResultCode.UNC_INVALID_ARGUMENT
					.getValue());
			final JsonObject error = resource.getInfo().getAsJsonObject(
					VtnServiceJsonConsts.ERROR);
			error.addProperty(VtnServiceJsonConsts.MSG,
					error.get(VtnServiceJsonConsts.MSG).getAsString()
							+ resource.getValidator().getInvalidParameter()
							+ VtnServiceConsts.CLOSE_SMALL_BRACES);
			exceptionHandler.raise(
					Thread.currentThread().getStackTrace()[1].getClassName()
							+ VtnServiceConsts.HYPHEN
							+ Thread.currentThread().getStackTrace()[1]
									.getMethodName(),
					UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorCode(),
					UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorMessage(),
					exception);
			if (exception instanceof VtnServiceException) {
				throw (VtnServiceException) exception;
			}
		}
	}

	/**
	 * Calls the respective validator to execute the validation of Json and URI
	 * parameters and create the error Json if validation fails
	 * 
	 * @param requestType
	 * @param resource
	 * @return
	 */
	private void validateJson(final String requestType,
			final JsonObject requestBody, AbstractResource resource)
			throws VtnServiceException {
		try {
			VtnServiceUtil.trimParamValues(requestBody, resource);
			/*
			 * remove the parameters which contains empty strings
			 */
			if (VtnServiceConsts.POST.equals(requestType)
					&& !VtnServiceUtil.isOpenStackResurce(resource)) {
				VtnServiceUtil.removeEmptyParamas(requestBody);
			}
			resource.getValidator().validate(requestType, requestBody);
		} catch (final Exception exception) {
			if (resource
					.getValidator()
					.getInvalidParameter()
					.equals(UncCommonEnum.UncResultCode.UNC_INVALID_FORMAT
							.getMessage())) {
				resource.createErrorInfo(UncCommonEnum.UncResultCode.UNC_INVALID_FORMAT
						.getValue());
			} else if (resource
					.getValidator()
					.getInvalidParameter()
					.equals(UncCommonEnum.UncResultCode.UNC_METHOD_NOT_ALLOWED
							.getMessage())) {
				resource.createErrorInfo(UncCommonEnum.UncResultCode.UNC_METHOD_NOT_ALLOWED
						.getValue());
			} else {
				resource.createErrorInfo(UncCommonEnum.UncResultCode.UNC_INVALID_ARGUMENT
						.getValue());
				final JsonObject error = resource.getInfo().getAsJsonObject(
						VtnServiceJsonConsts.ERROR);
				error.addProperty(VtnServiceJsonConsts.MSG,
						error.get(VtnServiceJsonConsts.MSG).getAsString()
								+ resource.getValidator().getInvalidParameter()
								+ VtnServiceConsts.CLOSE_SMALL_BRACES);
			}
			exceptionHandler.raise(
					Thread.currentThread().getStackTrace()[1].getClassName()
							+ VtnServiceConsts.HYPHEN
							+ Thread.currentThread().getStackTrace()[1]
									.getMethodName(),
					UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorCode(),
					UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorMessage(),
					exception);
			if (exception instanceof VtnServiceException) {
				throw (VtnServiceException) exception;
			}
		}
	}

	/**
	 * 
	 * @param requestBody
	 * @throws VtnServiceException
	 */
	private void validateJsonOp(final JsonObject requestBody)
			throws VtnServiceException {
		try {
			if (requestBody.has(VtnServiceJsonConsts.OP)
					&& requestBody.get(VtnServiceJsonConsts.OP).getAsString()
							.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)
					&& requestBody.has(VtnServiceJsonConsts.INDEX)) {
				LOG.error("Validation failed in case of op : count with index");
				throw new VtnServiceException(Thread.currentThread()
						.getStackTrace()[1].getMethodName(),
						UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorCode(),
						UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorMessage());
			}
		} catch (final VtnServiceException exception) {
			resource.createErrorInfo(UncCommonEnum.UncResultCode.UNC_CLIENT_ERROR
					.getValue());
			final JsonObject error = getInfo().getAsJsonObject(
					VtnServiceJsonConsts.ERROR);
			error.addProperty(VtnServiceJsonConsts.MSG,
					VtnServiceConsts.INDEX_ERROR_MSG);
			exceptionHandler.raise(
					Thread.currentThread().getStackTrace()[1].getClassName()
							+ VtnServiceConsts.HYPHEN
							+ Thread.currentThread().getStackTrace()[1]
									.getMethodName(),
					UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorCode(),
					UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorMessage(),
					exception);
			throw exception;
		}
	}

	/**
	 * Return the resource instance corresponding to resourcePath set by
	 * setPath() interface
	 * 
	 * @return
	 */
	public AbstractResource getResource() {
		return resource;
	}
}
