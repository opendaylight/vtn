/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.resources;

import java.lang.reflect.Method;
import java.util.List;

import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.ipc.ClientSession;
import org.opendaylight.vtn.core.ipc.IpcDataUnit;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.VtnServiceResource;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.connection.IpcConnPool;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.exception.VtnServiceExceptionHandler;
import org.opendaylight.vtn.javaapi.init.VtnServiceConfiguration;
import org.opendaylight.vtn.javaapi.init.VtnServiceInitManager;
import org.opendaylight.vtn.javaapi.ipc.IpcRequestProcessor;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcDataUnitWrapper;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcLogicalResponseFactory;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcPhysicalResponseFactory;
import org.opendaylight.vtn.javaapi.ipc.enums.IpcRequestPacketEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncOption2Enum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncSYSMGEnums;
import org.opendaylight.vtn.javaapi.ipc.enums.UncSessionEnums;
import org.opendaylight.vtn.javaapi.ipc.enums.UncTCEnums;
import org.opendaylight.vtn.javaapi.ipc.enums.UncUPLLEnums;
import org.opendaylight.vtn.javaapi.ipc.enums.UncUPPLEnums;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class AbstractResource.
 */
public abstract class AbstractResource implements VtnServiceResource {

	private static final Logger LOG = Logger.getLogger(AbstractResource.class
			.getName());
	private VtnServiceExceptionHandler exceptionHandler;
	private VtnServiceValidator validator;
	private JsonObject info;
	private long configID = VtnServiceConsts.INVALID_CONFIGID;
	private long sessionID = VtnServiceConsts.INVALID_SESSIONID;
	private IpcConnPool connPool;

	public AbstractResource() {
	}

	public void setConnPool() {
		this.connPool = VtnServiceInitManager.getConnectionPoolMap();
	}

	public IpcConnPool getConnPool() {
		return connPool;
	}

	/**
	 * Default implementation for delete APIs
	 */
	@Override
	public int delete() throws VtnServiceException {
		LOG.trace("Return from AbstractResource#delete");
		createErrorInfo(UncResultCode.UNC_CLIENT_ERROR.getValue(),
				VtnServiceConsts.RESOURCE_METHOD_INCORRECT);
		return UncResultCode.UNC_CLIENT_ERROR.getValue();
	}

	/**
	 * Default implementation for delete APIs with Json Query string
	 */
	@Override
	public int delete(final JsonObject queryString) throws VtnServiceException {
		LOG.trace("Return from AbstractResource#delete");
		createErrorInfo(UncResultCode.UNC_CLIENT_ERROR.getValue(),
				VtnServiceConsts.RESOURCE_METHOD_INCORRECT);
		return UncResultCode.UNC_CLIENT_ERROR.getValue();
	}

	/**
	 * Default implementation for get APIs
	 */
	@Override
	public int get() throws VtnServiceException {
		LOG.trace("Return from AbstractResource#get");
		createErrorInfo(UncResultCode.UNC_CLIENT_ERROR.getValue(),
				VtnServiceConsts.RESOURCE_METHOD_INCORRECT);
		return UncResultCode.UNC_CLIENT_ERROR.getValue();
	}

	/**
	 * Default implementation for get APIs with Json Query string
	 */
	@Override
	public int get(final JsonObject queryString) throws VtnServiceException {
		LOG.trace("Return from AbstractResource#get");
		createErrorInfo(UncResultCode.UNC_CLIENT_ERROR.getValue(),
				VtnServiceConsts.RESOURCE_METHOD_INCORRECT);
		return UncResultCode.UNC_CLIENT_ERROR.getValue();
	}

	/**
	 * Gets the config id.
	 * 
	 * @return the config id
	 */
	public long getConfigID() {
		LOG.trace("Return from AbstractResource#getConfigID");
		return configID;
	}

	/**
	 * Gets the session id.
	 * 
	 * @return the session id
	 */
	public long getSessionID() {
		LOG.trace("Return from AbstractResource#getSessionID");
		return sessionID;
	}

	/**
	 * Default implementation for post APIs with Json request body
	 */
	@Override
	public int post(final JsonObject requestBody) throws VtnServiceException {
		LOG.trace("Return from AbstractResource#post");
		createErrorInfo(UncResultCode.UNC_CLIENT_ERROR.getValue(),
				VtnServiceConsts.RESOURCE_METHOD_INCORRECT);
		return UncResultCode.UNC_CLIENT_ERROR.getValue();
	}

	/**
	 * Default implementation for put APIs with Json request body
	 */
	@Override
	public int put(final JsonObject requestBody) throws VtnServiceException {
		LOG.trace("Return from AbstractResource#put");
		createErrorInfo(UncResultCode.UNC_CLIENT_ERROR.getValue(),
				VtnServiceConsts.RESOURCE_METHOD_INCORRECT);
		return UncResultCode.UNC_CLIENT_ERROR.getValue();
	}

	/**
	 * Sets the config id.
	 * 
	 * @param configId
	 *            the new config id
	 */
	public void setConfigID(final long configId) {
		LOG.trace("Start AbstractResource#setConfigID");
		this.configID = configId;
		LOG.trace("Complete AbstractResource#setConfigID");
	}

	/**
	 * Sets the session id.
	 * 
	 * @param sessionId
	 *            the new session id
	 */
	public void setSessionID(final long sessionId) {
		LOG.trace("Start AbstractResource#setSessionID");
		this.sessionID = sessionId;
		LOG.trace("Complete AbstractResource#setSessionID");
	}

	/**
	 * Gets the info.
	 * 
	 * @return the info
	 */
	public JsonObject getInfo() {
		LOG.trace("Return from AbstractResource#getInfo");
		return info;
	}

	/**
	 * Sets the info.
	 * 
	 * @param info
	 *            the new info
	 */
	public void setInfo(final JsonObject info) {
		LOG.trace("Start AbstractResource#setInfo");
		this.info = info;
		LOG.trace("Complete AbstractResource#setInfo");
	}

	/**
	 * Gets the validator.
	 * 
	 * @return the validator
	 */
	public VtnServiceValidator getValidator() {
		LOG.trace("Return from AbstractResource#getValidator");
		return validator;
	}

	/**
	 * Sets the validator.
	 * 
	 * @param validator
	 *            the new validator
	 */
	public void setValidator(final VtnServiceValidator validator) {
		LOG.trace("Start AbstractResource#setValidator");
		this.validator = validator;
		LOG.trace("Complete AbstractResource#setValidator");
	}

	/**
	 * Gets the exception handler.
	 * 
	 * @return the exception handler
	 */
	public VtnServiceExceptionHandler getExceptionHandler() {
		LOG.trace("Return from AbstractResource#getExceptionHandler");
		return exceptionHandler;
	}

	/**
	 * Sets the exception handler.
	 * 
	 * @param exceptionHandler
	 *            the new exception handler
	 */
	public void setExceptionHandler(
			final VtnServiceExceptionHandler exceptionHandler) {
		LOG.trace("Start AbstractResource#setExceptionHandler");
		this.exceptionHandler = exceptionHandler;
		LOG.trace("Complete AbstractResource#setExceptionHandler");
	}

	/**
	 * Create error Json in case of IPC server returns the RESP_FATAL (-1)
	 */
	public void createErrorInfo(final int code) {
		LOG.trace("Start AbstractResource#createErrorInfo");
		// error code and message required to be update after confirmation
		final JsonObject errorJsonObject = new JsonObject();
		final JsonObject error = new JsonObject();
		error.addProperty(VtnServiceJsonConsts.CODE, code);
		if (code == UncCommonEnum.UncResultCode.UNC_SERVER_ERROR.getValue()) {
			error.addProperty(VtnServiceJsonConsts.MSG,
					UncCommonEnum.UncResultCode.UNC_SERVER_ERROR.getMessage());
		} else if (code == UncCommonEnum.UncResultCode.UNC_CLIENT_ERROR
				.getValue()) {
			error.addProperty(VtnServiceJsonConsts.MSG,
					UncCommonEnum.UncResultCode.UNC_CLIENT_ERROR.getMessage());
		} else {
			error.addProperty(VtnServiceJsonConsts.MSG, "Invalid error code");
		}
		errorJsonObject.add(VtnServiceJsonConsts.ERROR, error);
		setInfo(errorJsonObject);
		LOG.trace("Complete AbstractResource#createErrorInfo");
	}

	/**
	 * Set error Json in case server side error require to return with JavaAPI
	 * specific message
	 * 
	 * @param errorMessage
	 */
	public void createErrorInfo(int errorCode, String errorMessage) {
		LOG.trace("Complete AbstractResource#createErrorInfo");
		final JsonObject errorJsonObject = new JsonObject();
		final JsonObject error = new JsonObject();
		error.addProperty(VtnServiceJsonConsts.CODE, errorCode);
		error.addProperty(VtnServiceJsonConsts.MSG, errorMessage);
		errorJsonObject.add(VtnServiceJsonConsts.ERROR, error);
		setInfo(errorJsonObject);
		LOG.trace("Complete AbstractResource#createErrorInfo");
	}

	/**
	 * Create error Json specific to session error codes
	 * 
	 * @param errorMessage
	 */
	public void createSessionErrorInfo(UncSessionEnums.UsessIpcErrE errorEnum) {
		LOG.trace("Complete AbstractResource#createSessionErrorInfo");
		final JsonObject errorJsonObject = new JsonObject();
		final JsonObject error = new JsonObject();
		error.addProperty(VtnServiceJsonConsts.CODE, errorEnum.getCode());
		error.addProperty(VtnServiceJsonConsts.MSG, errorEnum.getMessage());
		errorJsonObject.add(VtnServiceJsonConsts.ERROR, error);
		setInfo(errorJsonObject);
		LOG.trace("Complete AbstractResource#createSessionErrorInfo");
	}

	/**
	 * Create error Json specific to tc error codes
	 * 
	 * @param errorMessage
	 */
	public void createTcErrorInfo(UncTCEnums.OperationStatus errorEnum) {
		LOG.trace("Complete AbstractResource#createTcErrorInfo");
		final JsonObject errorJsonObject = new JsonObject();
		final JsonObject error = new JsonObject();
		error.addProperty(VtnServiceJsonConsts.CODE, errorEnum.getErrorCode());
		error.addProperty(VtnServiceJsonConsts.MSG, errorEnum.getMessage());
		errorJsonObject.add(VtnServiceJsonConsts.ERROR, error);
		setInfo(errorJsonObject);
		LOG.trace("Complete AbstractResource#createTcErrorInfo");
	}

	/**
	 * Create error Json specific to node manager error codes
	 * 
	 * @param errorMessage
	 */
	public void createNoMgErrorInfo(UncSYSMGEnums.NodeIpcErrorT errorEnum) {
		LOG.trace("Complete AbstractResource#createNoMgErrorInfo");
		final JsonObject errorJsonObject = new JsonObject();
		final JsonObject error = new JsonObject();
		error.addProperty(VtnServiceJsonConsts.CODE, errorEnum.getCode());
		error.addProperty(VtnServiceJsonConsts.MSG, errorEnum.getMessage());
		errorJsonObject.add(VtnServiceJsonConsts.ERROR, error);
		setInfo(errorJsonObject);
		LOG.trace("Complete AbstractResource#createNoMgErrorInfo");
	}

	/**
	 * Return a Null Json Object
	 * 
	 * @return
	 */
	protected JsonObject getNullJsonObject() {
		LOG.debug("Return NULL Json Object");
		return null;
	}

	/**
	 * Return a Null List
	 * 
	 * @return
	 */
	protected List<String> getNullListObject() {
		LOG.debug("Return NULL List Object");
		return null;
	}

	/**
	 * Invoke the UPPL services, if the previous call contains more than 10K
	 * (configurable)
	 * 
	 * @param requestBody
	 * @param requestProcessor
	 * @param responseGenerator
	 * @param responseArray
	 * @param JsonArrayName
	 * @param IndexName
	 * @param requestPackeEnumName
	 * @param uriParameters
	 * @param methodName
	 * @return
	 * @throws VtnServiceException
	 */
	public JsonObject getResponseJsonArrayPhysical(
			final JsonObject requestBody, IpcRequestProcessor requestProcessor,
			Object responseGenerator, JsonArray responseArray,
			String JsonArrayName, String IndexName,
			IpcRequestPacketEnum requestPackeEnumName,
			List<String> uriParameters, String methodName)
			throws VtnServiceException {
		// session reset
		requestProcessor.setServiceInfo(UncUPPLEnums.UPPL_IPC_SVC_NAME,
				UncUPPLEnums.ServiceID.UPPL_SVC_READREQ.ordinal());
		int status = ClientSession.RESP_FATAL;
		int memberIndex = 0;
		VtnServiceConfiguration configuration = VtnServiceInitManager
				.getConfigurationMap();
		int max_rep_count = Integer.parseInt(configuration
				.getConfigValue(VtnServiceConsts.MAX_REP_DEFAULT));
		memberIndex = responseArray.size();
		if (memberIndex != 0) {
			JsonObject memberLastIndex = (JsonObject) responseArray
					.get(responseArray.size() - 1);
			if (requestBody.has(VtnServiceJsonConsts.INDEX)) {
				uriParameters.remove(uriParameters.size() - 1);
				uriParameters.add(uriParameters.size(),
						memberLastIndex.get(IndexName).getAsString());
			} else {
				uriParameters.add(memberLastIndex.get(IndexName).getAsString());
			}
			while (memberIndex == max_rep_count) {

				JsonArray memberArray = null;
				memberLastIndex = (JsonObject) responseArray.get(responseArray
						.size() - 1);
				uriParameters.remove(uriParameters.size() - 1);
				uriParameters.add(uriParameters.size(),
						memberLastIndex.get(IndexName).getAsString());

				requestProcessor.createIpcRequestPacket(requestPackeEnumName,
						requestBody, uriParameters);
				// for testing only
				requestProcessor
						.getRequestPacket()
						.setOption2(
								IpcDataUnitWrapper
										.setIpcUint32Value(UncOption2Enum.UNC_OPT2_NEIGHBOR
												.ordinal()));

				status = requestProcessor.processIpcRequest();
				if (status == ClientSession.RESP_FATAL) {
					throw new VtnServiceException(
							Thread.currentThread().getStackTrace()[1]
									.getClassName()
									+ VtnServiceConsts.HYPHEN
									+ Thread.currentThread().getStackTrace()[1]
											.getMethodName(),
							UncJavaAPIErrorCode.IPC_SERVER_ERROR.getErrorCode(),
							UncJavaAPIErrorCode.IPC_SERVER_ERROR
									.getErrorMessage());
				}
				try {
					Method method;

					final Class<IpcPhysicalResponseFactory> sourceClass = IpcPhysicalResponseFactory.class;
					// get the method name to get the IpcLogicalResponseFactory
					// object for given key
					method = sourceClass.getMethod(methodName,
							new Class<?>[] { IpcDataUnit[].class,
									JsonObject.class, String.class });
					// get IpcLogicalResponseFactory object

					memberArray = ((JsonObject) method.invoke(
							responseGenerator,
							requestProcessor.getIpcResponsePacket(),
							requestBody, VtnServiceJsonConsts.LIST))
							.getAsJsonArray(JsonArrayName);
				} catch (final Exception e) {
					exceptionHandler.raise(
							Thread.currentThread().getStackTrace()[1]
									.getClassName()
									+ VtnServiceConsts.HYPHEN
									+ Thread.currentThread().getStackTrace()[1]
											.getMethodName(),
							UncJavaAPIErrorCode.INTERNAL_ERROR.getErrorCode(),
							UncJavaAPIErrorCode.INTERNAL_ERROR
									.getErrorMessage(), e);
				}
				if (null != memberArray && !memberArray.isJsonNull()
						&& memberArray.size() > 0) {
					responseArray.getAsJsonArray().addAll(memberArray);
				} else {
					break;
				}
				memberIndex = memberArray.size();
			}
		}
		JsonObject root = new JsonObject();
		root.add(JsonArrayName, responseArray);
		return root;
	}

	/**
	 * Invoke the UPLL services, if the previous call contains more than 10K
	 * (configurable)
	 * 
	 * @param requestBody
	 * @param requestProcessor
	 * @param responseGenerator
	 * @param responseArray
	 * @param JsonArrayName
	 * @param IndexName
	 * @param requestPackeEnumName
	 * @param uriParameters
	 * @param methodName
	 * @return
	 * @throws VtnServiceException
	 */
	public JsonObject getResponseJsonArrayLogical(final JsonObject requestBody,
			IpcRequestProcessor requestProcessor, Object responseGenerator,
			JsonArray responseArray, String JsonArrayName, String IndexName,
			IpcRequestPacketEnum requestPackeEnumName,
			List<String> uriParameters, String methodName)
			throws VtnServiceException {
		// session reset
		requestProcessor.setServiceInfo(UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
				UncUPLLEnums.ServiceID.UPLL_READ_SVC_ID.ordinal());
		int status = ClientSession.RESP_FATAL;
		int memberIndex = 0;
		VtnServiceConfiguration configuration = VtnServiceInitManager
				.getConfigurationMap();
		int max_rep_count = Integer.parseInt(configuration
				.getConfigValue(VtnServiceConsts.MAX_REP_DEFAULT));
		memberIndex = responseArray.size();
		if (memberIndex != 0) {
			JsonObject memberLastIndex = (JsonObject) responseArray
					.get(responseArray.size() - 1);
			if (requestBody.has(VtnServiceJsonConsts.INDEX)) {
				uriParameters.remove(uriParameters.size() - 1);
				uriParameters.add(uriParameters.size(),
						memberLastIndex.get(IndexName).getAsString());
			} else {
				uriParameters.add(memberLastIndex.get(IndexName).getAsString());
			}
			while (memberIndex == max_rep_count) {

				JsonArray memberArray = null;
				memberLastIndex = (JsonObject) responseArray.get(responseArray
						.size() - 1);
				uriParameters.remove(uriParameters.size() - 1);
				uriParameters.add(uriParameters.size(),
						memberLastIndex.get(IndexName).getAsString());

				requestProcessor.createIpcRequestPacket(requestPackeEnumName,
						requestBody, uriParameters);
				// for testing only
				requestProcessor
						.getRequestPacket()
						.setOption2(
								IpcDataUnitWrapper
										.setIpcUint32Value(UncOption2Enum.UNC_OPT2_NEIGHBOR
												.ordinal()));

				status = requestProcessor.processIpcRequest();
				if (status == ClientSession.RESP_FATAL) {
					throw new VtnServiceException(
							Thread.currentThread().getStackTrace()[1]
									.getClassName()
									+ VtnServiceConsts.HYPHEN
									+ Thread.currentThread().getStackTrace()[1]
											.getMethodName(),
							UncJavaAPIErrorCode.IPC_SERVER_ERROR.getErrorCode(),
							UncJavaAPIErrorCode.IPC_SERVER_ERROR
									.getErrorMessage());
				}
				try {
					Method method;

					final Class<IpcLogicalResponseFactory> sourceClass = IpcLogicalResponseFactory.class;
					// get the method name to get the IpcLogicalResponseFactory
					// object for given key
					method = sourceClass.getMethod(methodName,
							new Class<?>[] { IpcDataUnit[].class,
									JsonObject.class, String.class });
					// get IpcLogicalResponseFactory object

					memberArray = ((JsonObject) method.invoke(
							responseGenerator,
							requestProcessor.getIpcResponsePacket(),
							requestBody, VtnServiceJsonConsts.LIST))
							.getAsJsonArray(JsonArrayName);
				} catch (final Exception e) {
					exceptionHandler.raise(
							Thread.currentThread().getStackTrace()[1]
									.getClassName()
									+ VtnServiceConsts.HYPHEN
									+ Thread.currentThread().getStackTrace()[1]
											.getMethodName(),
							UncJavaAPIErrorCode.INTERNAL_ERROR.getErrorCode(),
							UncJavaAPIErrorCode.INTERNAL_ERROR
									.getErrorMessage(), e);
				}
				if (null != memberArray && !memberArray.isJsonNull()
						&& memberArray.size() > 0) {
					responseArray.getAsJsonArray().addAll(memberArray);
				} else {
					break;
				}
				memberIndex = memberArray.size();
			}
		}
		JsonObject root = new JsonObject();
		root.add(JsonArrayName, responseArray);
		return root;
	}
}
