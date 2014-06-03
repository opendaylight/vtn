/*
 * Copyright (c) 2014 NEC Corporation
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
import org.opendaylight.vtn.core.ipc.IpcString;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.annotation.UNCField;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcDataUnitWrapper;
import org.opendaylight.vtn.javaapi.ipc.enums.UncBaseEnums;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncIpcErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncTCEnums;
import org.opendaylight.vtn.javaapi.validation.AuditControllerResourceValidator;

/**
 * The Class AuditControllerResource implements put method.
 */
@UNCVtnService(path = "/controllers/{controller_id}/audit")
public class AuditControllerResource extends AbstractResource {

	/** The LOG. */
	private static final Logger LOG = Logger
			.getLogger(AuditControllerResource.class.getName());

	/**
	 * Instantiates a new Audit Controller API.
	 */
	public AuditControllerResource() {
		super();
		LOG.trace("Start AuditControllerResource#AuditControllerResource()");
		setValidator(new AuditControllerResourceValidator(this));
		LOG.trace("Complete AuditControllerResource#AuditControllerResource()");
	}

	/** The controller Id. */
	@UNCField("controller_id")
	private String controllerId;

	/**
	 * @return the controllerId
	 */
	public final String getControllerId() {
		return controllerId;
	}

	/**
	 * Implementation of Put method of Audit Controller API
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
		LOG.trace("Starts AuditControllerResource#put()");
		ClientSession session = null;
		int status = ClientSession.RESP_FATAL;
		try {
			LOG.debug("Start Ipc framework call");
			session = getConnPool().getSession(UncTCEnums.UNC_CHANNEL_NAME,
					UncTCEnums.TC_SERVICE_NAME,
					UncTCEnums.ServiceID.TC_AUDIT_SERVICES.ordinal(),
					getExceptionHandler());
			LOG.info("Session created successfully");
			session.setTimeout(null);
			session.addOutput(IpcDataUnitWrapper
					.setIpcUint32Value(UncTCEnums.ServiceType.TC_OP_USER_AUDIT
							.ordinal()));
			session.addOutput(IpcDataUnitWrapper
					.setIpcUint32Value(getSessionID()));
			session.addOutput(new IpcString(controllerId));
			if (requestBody != null
					&& requestBody.has(VtnServiceJsonConsts.AUDIT)
					&& requestBody.getAsJsonObject(VtnServiceJsonConsts.AUDIT)
							.has(VtnServiceJsonConsts.FORCE)) {
				String force = requestBody
						.getAsJsonObject(VtnServiceJsonConsts.AUDIT)
						.getAsJsonPrimitive(VtnServiceJsonConsts.FORCE)
						.getAsString();
				if (VtnServiceJsonConsts.TRUE.equalsIgnoreCase(force)) {
					session.addOutput(IpcDataUnitWrapper
							.setIpcUint8Value(VtnServiceJsonConsts.VAL_1));
				} else {
					session.addOutput(IpcDataUnitWrapper
							.setIpcUint8Value(VtnServiceJsonConsts.VAL_0));
				}
			} else {
				session.addOutput(IpcDataUnitWrapper
						.setIpcUint8Value(VtnServiceJsonConsts.VAL_0));
			}

			LOG.info("Request packet created successfully");
			status = session.invoke();
			LOG.info("Request packet processed with status:" + status);
			final String operationType = IpcDataUnitWrapper
					.getIpcDataUnitValue(session
							.getResponse(VtnServiceJsonConsts.VAL_0));
			final String sessionId = IpcDataUnitWrapper.getIpcDataUnitValue(
					session.getResponse(VtnServiceJsonConsts.VAL_1)).toString();
			final int operationStatus = Integer.parseInt(IpcDataUnitWrapper
					.getIpcDataUnitValue(session
							.getResponse(VtnServiceJsonConsts.VAL_2)));
			final int auditResult = Integer.parseInt(IpcDataUnitWrapper
					.getIpcDataUnitValue(session
							.getResponse(VtnServiceJsonConsts.VAL_3)));
			LOG.info("Response retreived successfully");
			LOG.info("Operation type= " + operationType);
			LOG.info("SessionId= " + sessionId);
			LOG.info("OperationStatus= " + operationStatus);
			LOG.info("auditResult= " + auditResult);
			status = handleResponse(session, operationStatus, auditResult);
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
		LOG.trace("Completed AuditControllerResource#put()");
		return status;
	}

	/**
	 * Handle Response from TC
	 * 
	 * @param session
	 * @param operationStatus
	 * @param auditResult
	 * @return
	 * @throws IpcException
	 */
	private int handleResponse(ClientSession session,
			final int operationStatus, final int auditResult)
			throws IpcException {
		int status;
		final int responseCode;
		if (auditResult == UncTCEnums.TcAuditStatus.TC_AUDIT_OPER_SUCCESS
				.getValue()) {
			if (operationStatus == UncTCEnums.OperationStatus.TC_SYSTEM_FAILURE
					.getCode()) {
				createTcErrorInfo(UncIpcErrorCode.getTcCodes(operationStatus));
				LOG.info("Request not processed successfully : "
						+ operationStatus);
				status = UncCommonEnum.UncResultCode.UNC_SERVER_ERROR
						.getValue();
			} else {
				LOG.info("Request processed successfully");
				status = UncCommonEnum.UncResultCode.UNC_SUCCESS.getValue();
			}
		} else {
			if (operationStatus == UncTCEnums.OperationStatus.TC_INVALID_OPERATION_TYPE
					.getCode()) {
				createTcErrorInfo(UncIpcErrorCode.getTcCodes(operationStatus));
			} else if (operationStatus == UncTCEnums.OperationStatus.TC_INVALID_STATE
					.getCode()) {
				createTcErrorInfo(UncIpcErrorCode.getTcCodes(operationStatus));
			} else if (operationStatus == UncTCEnums.OperationStatus.TC_SYSTEM_BUSY
					.getCode()) {
				createTcErrorInfo(UncIpcErrorCode.getTcCodes(operationStatus));
			} else {
				LOG.info("Request not processed successfully : "
						+ operationStatus);
				responseCode = Integer.parseInt(IpcDataUnitWrapper
						.getIpcDataUnitValue(session
								.getResponse(VtnServiceJsonConsts.VAL_5)));
				LOG.info("Response Code : " + responseCode);
				if (responseCode == UncBaseEnums.UncRespCode.UNC_RC_NO_SUCH_INSTANCE
						.getCode()) {
					createErrorInfo(
							UncBaseEnums.UncRespCode.UNC_RC_NO_SUCH_INSTANCE
									.getErrorCode(),
							UncBaseEnums.UncRespCode.UNC_RC_NO_SUCH_INSTANCE
									.getMessage());
				} else if (responseCode == UncBaseEnums.UncRespCode.UNC_RC_CTR_BUSY
						.getCode()) {
					createErrorInfo(
							UncBaseEnums.UncRespCode.UNC_RC_CTR_BUSY
									.getErrorCode(),
							UncBaseEnums.UncRespCode.UNC_RC_CTR_BUSY
									.getMessage());
				} else {
					createErrorInfo(UncBaseEnums.UncRespCode.UNC_RC_INTERNAL_ERR
							.getErrorCode());
				}
			}
			status = UncCommonEnum.UncResultCode.UNC_SERVER_ERROR.getValue();
		}
		return status;
	}
}
