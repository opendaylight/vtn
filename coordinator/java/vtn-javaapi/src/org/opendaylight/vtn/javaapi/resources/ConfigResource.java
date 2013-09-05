/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.resources;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.ipc.ClientSession;
import org.opendaylight.vtn.core.ipc.IpcException;
import org.opendaylight.vtn.core.ipc.IpcUint32;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceIpcConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcDataUnitWrapper;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncIpcErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncTCEnums;
import org.opendaylight.vtn.javaapi.validation.ConfigResourceValidator;

/**
 * The Class ConfigResource implements the put method of configuration API.
 * 
 */

@UNCVtnService(path = "/configuration")
public class ConfigResource extends AbstractResource {

	private static final Logger LOG = Logger.getLogger(ConfigResource.class
			.getName());

	/**
	 * Instantiates a new config resource.
	 */
	public ConfigResource() {
		super();
		LOG.trace("Start ConfigResource#ConfigResource()");
		setValidator(new ConfigResourceValidator(this));
		LOG.trace("Complete ConfigResource#ConfigResource()");
	}

	/**
	 * Implementation of Put method of configuration API
	 * 
	 * @param requestBody
	 *            the request Json Object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	@Override
	public int put(final JsonObject requestBody) throws VtnServiceException {
		LOG.trace("Starts ConfigResource#put()");
		ClientSession session = null;
		int status = ClientSession.RESP_FATAL;
		try {
			LOG.debug("Start Ipc framework call");
			if (requestBody != null) {
				if (requestBody.has(VtnServiceJsonConsts.CONFIGURATION)
						&& requestBody
								.getAsJsonObject(
										VtnServiceJsonConsts.CONFIGURATION)
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.OPERATION)
								.getAsString()
								.equalsIgnoreCase(VtnServiceJsonConsts.COMMIT)) {
					session = getConnPool().getSession(
							UncTCEnums.UNC_CHANNEL_NAME,
							UncTCEnums.TC_SERVICE_NAME,
							UncTCEnums.ServiceID.TC_CANDIDATE_SERVICES
									.ordinal(), getExceptionHandler());
					LOG.info("Session created successfully");
					// set session timeout as infinity for commit operation
					session.setTimeout(null);
					session.addOutput(new IpcUint32(
							UncTCEnums.ServiceType.TC_OP_CANDIDATE_COMMIT
									.ordinal()));
					session.addOutput(IpcDataUnitWrapper
							.setIpcUint32Value(getSessionID()));
					session.addOutput(IpcDataUnitWrapper
							.setIpcUint32Value(getConfigID()));
				} else if (requestBody.has(VtnServiceJsonConsts.CONFIGURATION)
						&& requestBody
								.getAsJsonObject(
										VtnServiceJsonConsts.CONFIGURATION)
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.OPERATION)
								.getAsString()
								.equalsIgnoreCase(VtnServiceJsonConsts.SAVE)) {
					session = getConnPool().getSession(
							UncTCEnums.UNC_CHANNEL_NAME,
							UncTCEnums.TC_SERVICE_NAME,
							UncTCEnums.ServiceID.TC_STARTUP_DB_SERVICES
									.ordinal(), getExceptionHandler());
					LOG.info("Session created successfully");
					// set session timeout as infinity for save operation
					session.setTimeout(null);
					session.addOutput(IpcDataUnitWrapper
							.setIpcUint32Value(UncTCEnums.ServiceType.TC_OP_RUNNING_SAVE
									.ordinal()));
					session.addOutput(IpcDataUnitWrapper
							.setIpcUint32Value(getSessionID()));
				}
				LOG.info("Request packet created successfully");
			} else {
				LOG.warning("Request body is not correct");
			}
			status = session.invoke();
			LOG.info("Request packet processed with status:"+status);
			final int operationType = Integer.parseInt(IpcDataUnitWrapper
					.getIpcDataUnitValue(session
							.getResponse(VtnServiceJsonConsts.VAL_0)));
			final String sessionId = IpcDataUnitWrapper
					.getIpcDataUnitValue(session
							.getResponse(VtnServiceJsonConsts.VAL_1));
			String configId = null;
			int operationStatus = VtnServiceIpcConsts.INVALID_OPEARTION_STATUS;
			if (operationType == UncTCEnums.ServiceType.TC_OP_CANDIDATE_COMMIT
					.ordinal()) {
				configId = IpcDataUnitWrapper.getIpcDataUnitValue(session
						.getResponse(VtnServiceJsonConsts.VAL_2));
				operationStatus = Integer.parseInt(IpcDataUnitWrapper
						.getIpcDataUnitValue(session
								.getResponse(VtnServiceJsonConsts.VAL_3)));
			} else if (operationType == UncTCEnums.ServiceType.TC_OP_RUNNING_SAVE
					.ordinal()) {
				operationStatus = Integer.parseInt(IpcDataUnitWrapper
						.getIpcDataUnitValue(session
								.getResponse(VtnServiceJsonConsts.VAL_2)));
			}
			LOG.info("Response retreived successfully");
			LOG.info("Operation type= " + operationType);
			LOG.info("SessionId=" + sessionId);
			LOG.info("ConfigId=" + configId);
			LOG.info("OperationStatus" + operationStatus);
			if (operationStatus != UncTCEnums.OperationStatus.TC_OPER_SUCCESS
					.getCode()) {
				createTcErrorInfo(UncIpcErrorCode.getTcCodes(operationStatus));
				LOG.info("Request not processed successfully");
				status = UncCommonEnum.UncResultCode.UNC_SERVER_ERROR
						.getValue();
			} else {
				LOG.info("Request processed successfully");
				status = UncCommonEnum.UncResultCode.UNC_SUCCESS.getValue();
			}
			LOG.debug("Complete Ipc framework call");
		} catch (final VtnServiceException e) {
			LOG.info("Error occured while performing getSession operation");
			getExceptionHandler()
					.raise(Thread.currentThread().getStackTrace()[1]
							.getClassName()
							+ VtnServiceConsts.HYPHEN
							+ Thread.currentThread().getStackTrace()[1]
									.getMethodName(),
							UncJavaAPIErrorCode.IPC_SERVER_ERROR.getErrorCode(),
							UncJavaAPIErrorCode.IPC_SERVER_ERROR
									.getErrorMessage(), e);
			throw e;
		} catch (final IpcException e) {
			LOG.info("Error occured while performing addOutput operation");
			getExceptionHandler()
					.raise(Thread.currentThread().getStackTrace()[1]
							.getClassName()
							+ VtnServiceConsts.HYPHEN
							+ Thread.currentThread().getStackTrace()[1]
									.getMethodName(),
							UncJavaAPIErrorCode.IPC_SERVER_ERROR.getErrorCode(),
							UncJavaAPIErrorCode.IPC_SERVER_ERROR
									.getErrorMessage(), e);
		} finally {
			if (status == ClientSession.RESP_FATAL) {
				createErrorInfo(UncCommonEnum.UncResultCode.UNC_SERVER_ERROR
						.getValue());
				status = UncResultCode.UNC_SERVER_ERROR.getValue();
			}
			// destroy session by common handler
			getConnPool().destroySession(session);
		}
		LOG.trace("Completed ConfigResource#put()");
		return status;
	}
}
