/*
 * Copyright (c) 2012-2015 NEC Corporation
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
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcDataUnitWrapper;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncIpcErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncTCEnums;
import org.opendaylight.vtn.javaapi.validation.ReadLockResourceValidator;

/**
 * The Class ReadLockResource implements the put and delete method of Read Lock
 * API.
 * 
 */
@UNCVtnService(path = "/configuration/readlock")
public class ReadLockResource extends AbstractResource {

	private static final Logger LOG = Logger.getLogger(ReadLockResource.class
			.getName());

	/**
	 * Instantiates a new read lock resource.
	 */
	public ReadLockResource() {
		super();
		LOG.trace("Start ReadLockResource#VReadLockResource()");
		setValidator(new ReadLockResourceValidator(this));
		LOG.trace("Complete ReadLockResource#ReadLockResource()");
	}

	/**
	 * Implementation of Put method of read lock API
	 * 
	 * @param requestBody
	 *            the request Json Object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	@Override
	public final int put(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Starts ReadLockResource#put()");
		ClientSession session = null;
		int status = ClientSession.RESP_FATAL;
		try {
			LOG.debug("Start Ipc framework call");
			session = getConnPool().getSession(UncTCEnums.UNC_CHANNEL_NAME,
					UncTCEnums.TC_SERVICE_NAME,
					UncTCEnums.ServiceID.TC_READ_ACCESS_SERVICES.ordinal(),
					getExceptionHandler());
			LOG.info("Session created successfully");
			session.addOutput(IpcDataUnitWrapper
					.setIpcUint32Value(UncTCEnums.ServiceType.TC_OP_READ_ACQUIRE
							.ordinal()));
			session.addOutput(IpcDataUnitWrapper
					.setIpcUint32Value(getSessionID()));
			if (requestBody != null
					&& requestBody.has(VtnServiceJsonConsts.READLOCK)
					&& requestBody.getAsJsonObject(
							VtnServiceJsonConsts.READLOCK).has(
							VtnServiceJsonConsts.TIMEOUT)) {
				session.addOutput(IpcDataUnitWrapper
						.setIpcUint32Value(requestBody
								.getAsJsonObject(VtnServiceJsonConsts.READLOCK)
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.TIMEOUT)
								.getAsString()));
			} else {
				session.addOutput(IpcDataUnitWrapper
						.setIpcUint32Value(VtnServiceJsonConsts.VAL_0));
			}
			LOG.info("Request packet created successfully");
			long start = System.currentTimeMillis();
			status = session.invoke();
			LOG.debug("The treatment of under layer cost the following time: "
					+ (System.currentTimeMillis() - start) + "(ms)");
			LOG.info("Request packet processed with status:" + status);
			final String operationType = IpcDataUnitWrapper
					.getIpcDataUnitValue(session
							.getResponse(VtnServiceJsonConsts.VAL_0));
			final String sessionId = IpcDataUnitWrapper
					.getIpcDataUnitValue(session
							.getResponse(VtnServiceJsonConsts.VAL_1));
			final int operationStatus = Integer.parseInt(IpcDataUnitWrapper
					.getIpcDataUnitValue(session
							.getResponse(VtnServiceJsonConsts.VAL_2)));
			LOG.info("Response received successfully");
			LOG.info("OperationType" + operationType);
			LOG.info("SessionId" + sessionId);
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
				status = UncCommonEnum.UncResultCode.UNC_SERVER_ERROR
						.getValue();
			}
			// destroy session by common handler
			getConnPool().destroySession(session);
		}
		LOG.trace("Completed ReadLockResource#put()");
		return status;
	}

	/**
	 * Implementation of Get method of read lock API
	 * 
	 * @param requestBody
	 *            the request Json Object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	@Override
	public final int delete() throws VtnServiceException {
		LOG.trace("Starts ReadLockResource#delete()");
		int status = ClientSession.RESP_FATAL;
		ClientSession session = null;
		try {
			LOG.debug("Start Ipc framework call");
			session = getConnPool().getSession(UncTCEnums.UNC_CHANNEL_NAME,
					UncTCEnums.TC_SERVICE_NAME,
					UncTCEnums.ServiceID.TC_READ_ACCESS_SERVICES.ordinal(),
					getExceptionHandler());
			LOG.info("Session created successfully");
			session.addOutput(IpcDataUnitWrapper
					.setIpcUint32Value(UncTCEnums.ServiceType.TC_OP_READ_RELEASE
							.ordinal()));
			session.addOutput(IpcDataUnitWrapper
					.setIpcUint32Value(getSessionID()));
			LOG.info("Request packet created successfully");
			long start = System.currentTimeMillis();
			status = session.invoke();
			LOG.debug("The treatment of under layer cost the following time: "
					+ (System.currentTimeMillis() - start) + "(ms)");
			LOG.info("Request packet processed with status:" + status);
			final String operationType = IpcDataUnitWrapper
					.getIpcDataUnitValue(session.getResponse(0));
			final String sessionId = IpcDataUnitWrapper
					.getIpcDataUnitValue(session.getResponse(1));
			final int operationStatus = Integer.parseInt(IpcDataUnitWrapper
					.getIpcDataUnitValue(session.getResponse(2)));
			LOG.info("Response retreived successfully");
			LOG.info("Operation type= " + operationType);
			LOG.info("SessionId=" + sessionId);
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
				status = UncCommonEnum.UncResultCode.UNC_SERVER_ERROR
						.getValue();
			}
			// destroy session by common handler
			getConnPool().destroySession(session);
		}
		LOG.trace("Completed ReadLockResource#delete()");
		return status;
	}
}
