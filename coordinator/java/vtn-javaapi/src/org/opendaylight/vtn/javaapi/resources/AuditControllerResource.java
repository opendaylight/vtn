/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.resources;

import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.ipc.ClientSession;
import org.opendaylight.vtn.core.ipc.IpcDataUnit;
import org.opendaylight.vtn.core.ipc.IpcException;
import org.opendaylight.vtn.core.ipc.IpcString;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.annotation.UNCField;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcDataUnitWrapper;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncTCEnums;
import org.opendaylight.vtn.javaapi.ipc.enums.UncTCEnumsForAudit;
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

			if (requestBody != null
					&& requestBody.has(VtnServiceJsonConsts.AUDIT)
					&& requestBody.getAsJsonObject(VtnServiceJsonConsts.AUDIT)
							.has(VtnServiceJsonConsts.REAL_NETWORK_AUDIT)) {
				String audit = requestBody
						.getAsJsonObject(VtnServiceJsonConsts.AUDIT)
						.getAsJsonPrimitive(VtnServiceJsonConsts.REAL_NETWORK_AUDIT)
						.getAsString();
				if (VtnServiceJsonConsts.TRUE.equalsIgnoreCase(audit)) {
					session.addOutput(IpcDataUnitWrapper
							.setIpcUint8Value(VtnServiceJsonConsts.VAL_1));
				}
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
		int status = UncCommonEnum.UncResultCode.UNC_SUCCESS.getValue();

		if (auditResult == UncTCEnums.TcAuditStatus.TC_AUDIT_OPER_SUCCESS
				.getValue()) {
				LOG.info("Request processed successfully");
				status = UncCommonEnum.UncResultCode.UNC_SUCCESS.getValue();
		} else {
			LOG.info("Request not processed successfully : " + operationStatus);
			setInfo(operationStatusProcess(session, operationStatus));
			status = UncCommonEnum.UncResultCode.UNC_SERVER_ERROR.getValue();
			}
		return status;
	}

	private JsonObject operationStatusProcess(ClientSession session,
			final int operationStatus) throws IpcException {
		JsonObject info = new JsonObject();
		
		UncTCEnumsForAudit.OperationStatus operationStatusEnum = UncTCEnumsForAudit.OperationStatus.UNKNOWN;
		if (operationStatus == UncTCEnums.OperationStatus.TC_INVALID_STATE
					.getCode()) {
			operationStatusEnum = UncTCEnumsForAudit.OperationStatus.INVALID_STATE;
		} else if (operationStatus == UncTCEnums.OperationStatus.TC_SYSTEM_BUSY
				.getCode()) {
			operationStatusEnum = UncTCEnumsForAudit.OperationStatus.SYSTEM_BUSY;
		} else if (operationStatus == UncTCEnums.OperationStatus.TC_INVALID_OPERATION_TYPE
				.getCode()) {
			operationStatusEnum = UncTCEnumsForAudit.OperationStatus.INVALID_OPERATION_TYPE;
		} else if (operationStatus == UncTCEnums.OperationStatus.TC_SYSTEM_FAILURE
				.getCode()) {
			operationStatusEnum = UncTCEnumsForAudit.OperationStatus.SYSTEM_FAILURE;
		} else if (operationStatus == UncTCEnums.OperationStatus.TC_AUDIT_CANCELLED
				.getCode()) {
			operationStatusEnum = UncTCEnumsForAudit.OperationStatus.AUDIT_CANCELLED;
		} else if (operationStatus == UncTCEnums.OperationStatus.TC_OPER_ABORT.getCode() || 
				   operationStatus == UncTCEnums.OperationStatus.TC_OPER_SUCCESS.getCode()) {
			// No response body from IPC.
			if (session.getResponseCount() <= 4) {
				operationStatusEnum =  UncTCEnumsForAudit.OperationStatus.OPER_ABORT;
			} else {
				return responseCodeProcess(session);
			}
		} else {
			operationStatusEnum = UncTCEnumsForAudit.OperationStatus.UNKNOWN;
		}

		JsonObject error = new JsonObject();
		error.addProperty(VtnServiceJsonConsts.CODE, operationStatusEnum.getErrorCode());
		error.addProperty(VtnServiceJsonConsts.MSG, operationStatusEnum.getMessage());

		info.add(VtnServiceJsonConsts.ERROR, error);
		return info;
	}

	private JsonObject responseCodeProcess(ClientSession session) throws IpcException {
		int index = VtnServiceJsonConsts.VAL_4;
		int responseCode = -1;
		int noOfErrors = -1;
		UncTCEnumsForAudit.TcResponseCode errorEnum = UncTCEnumsForAudit.TcResponseCode.NO_SUCH_INSTANCE;
		int count = session.getResponseCount();
		JsonArray errInfos = new JsonArray();
		UncTCEnumsForAudit.TcResponseCode responseCodeEnum = UncTCEnumsForAudit.TcResponseCode.NO_SUCH_INSTANCE;

		do {
			if (index >= count) {
				break;
			}

			IpcDataUnit ipcDataUnit = session.getResponse(index);
			if (IpcDataUnit.STRING != ipcDataUnit.getType()) {
				index++;
				continue;
			}

			index += 1;
			responseCode = Integer.parseInt(IpcDataUnitWrapper
					.getIpcDataUnitValue(session.getResponse(index)));
			if (UncTCEnums.TcResponseCode.TC_INTERNAL_ERR.getCode() == responseCode) {
				responseCodeEnum = UncTCEnumsForAudit.TcResponseCode.INTERNAL_ERR;
			} else if (UncTCEnums.TcResponseCode.TC_CONFIG_INVAL.getCode() == responseCode) {
				responseCodeEnum = UncTCEnumsForAudit.TcResponseCode.CONFIG_INVAL;
			} else if (UncTCEnums.TcResponseCode.TC_CTR_BUSY.getCode() == responseCode) {
				responseCodeEnum = UncTCEnumsForAudit.TcResponseCode.CTR_BUSY;
			} else if (UncTCEnums.TcResponseCode.TC_CTR_DISCONNECTED.getCode() == responseCode) {
				responseCodeEnum = UncTCEnumsForAudit.TcResponseCode.CTR_DISCONNECTED;
			} else if (UncTCEnums.TcResponseCode.TC_CTRLAPI_FAILURE.getCode() == responseCode) {
				responseCodeEnum = UncTCEnumsForAudit.TcResponseCode.CTRLAPI_FAILURE;
			} else if (UncTCEnums.TcResponseCode.TC_CTR_CONFIG_STATUS_ERR.getCode() == responseCode) {
				responseCodeEnum = UncTCEnumsForAudit.TcResponseCode.CTR_CONFIG_STATUS_ERR;
			} else if (UncTCEnums.TcResponseCode.TC_NO_SUCH_INSTANCE.getCode() == responseCode) {
				responseCodeEnum = UncTCEnumsForAudit.TcResponseCode.NO_SUCH_INSTANCE;
			} else if (UncTCEnums.TcResponseCode.TC_ERR_DRIVER_NOT_PRESENT.getCode() == responseCode) {
				responseCodeEnum = UncTCEnumsForAudit.TcResponseCode.ERR_DRIVER_NOT_PRESENT;
			} else {
				responseCodeEnum = UncTCEnumsForAudit.TcResponseCode.UNKNOWN;
			}
			//add err message
			JsonObject info = new JsonObject();
			info.addProperty(VtnServiceJsonConsts.MSG, responseCodeEnum.getMessage());
			errInfos.add(info);

			if (responseCodeEnum.getErrorCode() > errorEnum.getErrorCode()) {
				errorEnum = responseCodeEnum;
			}

			index += 1;
			noOfErrors = Integer.parseInt(IpcDataUnitWrapper
					.getIpcDataUnitValue(session.getResponse(index)));
			if (noOfErrors > 0) {
				index += noOfErrors * 2;
			}
		} while (true);

		JsonObject error = new JsonObject();
		error.addProperty(VtnServiceJsonConsts.CODE, errorEnum.getErrorCode());
		error.addProperty(VtnServiceJsonConsts.MSG, errorEnum.getMessage());
		error.add(VtnServiceJsonConsts.INFOS, errInfos);

		JsonObject info = new JsonObject();
		info.add(VtnServiceJsonConsts.ERROR, error);
		return info;
	}
}
