/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.resources;

import java.sql.Connection;
import java.sql.SQLException;

import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.ipc.ClientSession;
import org.opendaylight.vtn.core.ipc.IpcDataUnit;
import org.opendaylight.vtn.core.ipc.IpcException;
import org.opendaylight.vtn.core.ipc.IpcUint32;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceIpcConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.init.VtnServiceInitManager;
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
	 * The number of header data in commit response.
	 */
	private final static int  COMMIT_RESPONSE_HEADER_SIZE = 4;

	/**
	 * The priority of commit response code.
	 * If the value is smaller, the priority is higher.
	 */
	private final static char  COMMIT_RESPONSE_CODE_PRI_1  = 0x1;
	private final static char  COMMIT_RESPONSE_CODE_PRI_2  = 0x2;
	private final static char  COMMIT_RESPONSE_CODE_PRI_3  = 0x3;
	private final static char  COMMIT_RESPONSE_CODE_PRI_FF = 0xff;

	private final static String MULTI_CONTROLLERS_FAIL_COMMON_MSG =
			"Error occurred in more than one controller";

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
	public final int put(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Starts ConfigResource#put()");
		ClientSession session = null;
		int status = ClientSession.RESP_FATAL;
		int operationStatus = VtnServiceIpcConsts.INVALID_OPEARTION_STATUS;
		String timeout = null;
		String cancelAudit = null;
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
								.equalsIgnoreCase(
										VtnServiceJsonConsts.COMMIT)) {
					session = getConnPool().getSession(
							UncTCEnums.UNC_CHANNEL_NAME,
							UncTCEnums.TC_SERVICE_NAME,
							UncTCEnums.ServiceID.TC_CANDIDATE_SERVICES
									.ordinal(), getExceptionHandler());
					LOG.info("Session created successfully");
					// set session timeout as infinity for commit operation
					session.setTimeout(null);
					final JsonObject configuration = requestBody.getAsJsonObject(
											VtnServiceJsonConsts.CONFIGURATION);
					if (configuration.has(VtnServiceJsonConsts.TIMEOUT)) {
						timeout = configuration.getAsJsonPrimitive(
								VtnServiceJsonConsts.TIMEOUT).getAsString();
					}

					if (configuration.has(VtnServiceJsonConsts.CANCEL_AUDIT)) {
						cancelAudit = configuration.getAsJsonPrimitive(
								VtnServiceJsonConsts.CANCEL_AUDIT).getAsString();
					}

					session.addOutput(new IpcUint32(UncTCEnums.ServiceType
									.TC_OP_CANDIDATE_COMMIT_TIMED.ordinal()));
					session.addOutput(IpcDataUnitWrapper
							.setIpcUint32Value(getSessionID()));
					session.addOutput(IpcDataUnitWrapper
							.setIpcUint32Value(getConfigID()));
					session.addOutput(IpcDataUnitWrapper.setIpcInt32Value(
							Integer.parseInt(timeout)));
					session.addOutput(IpcDataUnitWrapper
							.setIpcUint8Value(cancelAudit));
				} else if (requestBody.has(VtnServiceJsonConsts.CONFIGURATION)
						&& requestBody
								.getAsJsonObject(
										VtnServiceJsonConsts.CONFIGURATION)
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.OPERATION)
								.getAsString()
								.equalsIgnoreCase(
										VtnServiceJsonConsts.SAVE)) {
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
			long start = System.currentTimeMillis();
			status = session.invoke();
			LOG.debug("The treatment of under layer cost the following time: "
					+ (System.currentTimeMillis() - start) + "(ms)");
			LOG.info("Request packet processed with status:" + status);
			final int operationType = Integer.parseInt(IpcDataUnitWrapper
					.getIpcDataUnitValue(session
							.getResponse(VtnServiceJsonConsts.VAL_0)));
			final String sessionId = IpcDataUnitWrapper
					.getIpcDataUnitValue(session
							.getResponse(VtnServiceJsonConsts.VAL_1));
			String configId = null;
			if (operationType == UncTCEnums.ServiceType.TC_OP_CANDIDATE_COMMIT_TIMED
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
			LOG.info("OperationStatus=" + operationStatus);
			if (operationType == UncTCEnums.ServiceType.TC_OP_CANDIDATE_COMMIT_TIMED
					.ordinal()) {
				status = setCommitHttpResponse(session, operationStatus);
			} else {
				status = setSaveHttpResponse(operationStatus);
			}
			LOG.debug("Complete Ipc framework call");
		} catch (final VtnServiceException e) {
			LOG.error(e, "Error occured while performing getSession operation");
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
			LOG.error(e, "Error occured while performing addOutput operation");
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
			performOpenStackDBOperation(operationStatus);
		}
		LOG.trace("Completed ConfigResource#put()");
		return status;
	}

	/**
	 * Perform DB commit or rollback, as per status if UNC commit operation
	 * 
	 * @param operationStatus
	 * @throws VtnServiceException
	 * 
	 */
	private void performOpenStackDBOperation(int operationStatus)
			throws VtnServiceException {
		Connection connection = getOpenStackConnection();
		// perform only when connection is not null
		if (connection != null) {
			try {
				if (operationStatus == UncTCEnums.OperationStatus.TC_OPER_SUCCESS
						.getCode()) {
					LOG.debug("Commit operation");
					connection.commit();
				} else {
					LOG.debug("Rollback operation");
					connection.rollback();
				}
			} catch (SQLException e) {
				LOG.error(e, "Error occured performing database commit/rollback operation");
				getExceptionHandler()
						.raise(Thread.currentThread().getStackTrace()[1]
								.getClassName()
								+ VtnServiceConsts.HYPHEN
								+ Thread.currentThread().getStackTrace()[1]
										.getMethodName(),
								UncJavaAPIErrorCode.INTERNAL_ERROR
										.getErrorCode(),
								UncJavaAPIErrorCode.INTERNAL_ERROR
										.getErrorMessage(), e);
			} finally {
				LOG.info("Free connection...");
				VtnServiceInitManager.getDbConnectionPoolMap().freeConnection(
						connection);
			}
		}
	}

	/**
	 * Set commit command response to caller
	 * 
	 * @param session
	 * @param operationStatus
	 * @return execute code
	 * @throws IpcException
	 */
	private int setCommitHttpResponse(final ClientSession session,
			final int operationStatus) throws IpcException {
		LOG.trace("Starts ConfigResource#setCommitHttpResponse()");
		int status;
		boolean isOperationStatusChanged = false;
		JsonArray ctrls = new JsonArray();
		int finalOperationStatus = operationStatus;

		if (finalOperationStatus == 
				UncTCEnums.OperationStatus.TC_OPER_SUCCESS.getCode()
				|| finalOperationStatus == 
						UncTCEnums.OperationStatus.TC_OPER_ABORT.getCode()) {
			isOperationStatusChanged = true;

			int responseCode = handleCommitTcResponse(session,
					finalOperationStatus, ctrls);
			LOG.info("Final response code: " + responseCode);

			if (responseCode == UncTCEnums.OperationStatus.TC_OPER_SUCCESS
					.getCode()) {
				LOG.info("Commit successed.");
			} else {
				finalOperationStatus = getFinalOperStatusCode(finalOperationStatus,
						responseCode);
			}
		} else if (!(finalOperationStatus == 
					UncTCEnums.OperationStatus.TC_INVALID_STATE.getCode()
				|| finalOperationStatus == 
					UncTCEnums.OperationStatus.TC_INVALID_OPERATION_TYPE
					.getCode()
				|| finalOperationStatus == 
					UncTCEnums.OperationStatus.TC_CONFIG_NOT_PRESENT.getCode()
				|| finalOperationStatus == 
					UncTCEnums.OperationStatus.TC_INVALID_CONFIG_ID.getCode()
				|| finalOperationStatus == 
					UncTCEnums.OperationStatus.TC_INVALID_SESSION_ID.getCode()
				|| finalOperationStatus == 
					UncTCEnums.OperationStatus.TC_SYSTEM_BUSY.getCode()
				|| finalOperationStatus == 
					UncTCEnums.OperationStatus.TC_SYSTEM_FAILURE.getCode())) {
			finalOperationStatus = UncTCEnums.OperationStatus
					.TC_INTERNAL_SERVER_ERROR.getCode();
		}
		LOG.info("Final OperationStatus: " + finalOperationStatus);

		if (UncTCEnums.OperationStatus.TC_OPER_SUCCESS.getCode() == finalOperationStatus) {
			LOG.info("Request processed successfully");
			status = UncTCEnums.OperationStatus.TC_OPER_SUCCESS.getErrorCode();
		} else if (UncTCEnums.OperationStatus.TC_OPER_SUCCESS_CTR_DISCONNECTED.getCode() == finalOperationStatus ||
				   UncTCEnums.OperationStatus.TC_OPER_SUCCESS_CTRLAPI_FAILURE.getCode() == finalOperationStatus ||
				   UncTCEnums.OperationStatus.TC_OPER_SUCCESS_CTR_CONFIG_STATUS_ERR.getCode() == finalOperationStatus ||
				   UncTCEnums.OperationStatus.TC_OPER_SUCCESS_OTHER_ERROR.getCode() == finalOperationStatus) {
			UncTCEnums.OperationStatus errorEnum =
					UncIpcErrorCode.getTcCodes(finalOperationStatus);
			status = errorEnum.getErrorCode() / 100;
			LOG.info("Request processed, but resources not yet ready");
		} else {
			UncTCEnums.OperationStatus errorEnum =
					UncIpcErrorCode.getTcCodes(finalOperationStatus);
			if (null == errorEnum) {
				errorEnum = UncTCEnums.OperationStatus.TC_INTERNAL_SERVER_ERROR;
			}

			if (isOperationStatusChanged) {
				JsonObject response = createControllerErrorInfo(
						errorEnum.getErrorCode(), ctrls);
				setInfo(response);
			} else {
				createTcErrorInfo(errorEnum);
			}
			LOG.info("Request not processed successfully");
			status = errorEnum.getErrorCode() / 100;
		}

		LOG.trace("Completed ConfigResource#setCommitHttpResponse()");
		return status;
	}

	/**
	 * Handle response from TC
	 * 
	 * @param session
	 * @param operationStatus
	 * @return execute result
	 * @throws IpcException
	 */
	private int handleCommitTcResponse(final ClientSession session,
			final int operationStatus, JsonArray ctrls) throws IpcException {
		LOG.trace("Starts ConfigResource#handleCommitTcResponse()");
		int finalResponseCode = UncTCEnums.OperationStatus.TC_OPER_SUCCESS
				.getCode();

		/* total number of data */
		int total = session.getResponseCount();

		/* number of body data = number of total data - number of header data
		 * number of header data: OperationType, SessionID, ConfigID, OperationStatus
		 */
		int dataCount = total - COMMIT_RESPONSE_HEADER_SIZE;

		/* No body in response */
		if (0 == dataCount) {
			LOG.trace("Completed ConfigResource#handleCommitTcResponse()");
			return finalResponseCode;
		}

		LOG.info("Total number of data in response: " + total);
		LOG.info("Number of body data in response: " + dataCount);

		/* Parse response body
		 * body format:   parameter          type             comment
		 *              Controller ID   PFC_IPCTYPE_STRING   Mandatory
		 *              Response Code   PFC_IPCTYPE_UINT32   Mandatory
		 *              NoOfErrors      PFC_IPCTYPE_UINT32   Mandatory
		 *              KeyType         PFC_IPCTYPE_UINT32   Mandatory
		 *              Key             PFC_IPCTYPE_STRUCT   Mandatory
		 *              Value           PFC_IPCTYPE_STRUCT   Optional
		 */
		for (int idx = COMMIT_RESPONSE_HEADER_SIZE; idx < total;) {
			String strCtrlId = IpcDataUnitWrapper.getIpcDataUnitValue(
					session.getResponse(idx));
			LOG.trace("Controller ID: " + strCtrlId);

			String strRepCode = IpcDataUnitWrapper.getIpcDataUnitValue(
					session.getResponse(idx + 1));
			LOG.trace("Response Code: " + strRepCode);
			int curResponseCode = Integer.parseInt(strRepCode);
			String msg = getFinalOperStatusMsg(operationStatus,
					curResponseCode);
			LOG.trace("Error Msg: " + msg);

			/* Make controller info */
			JsonObject ctrl = new JsonObject();
			ctrl.addProperty(VtnServiceJsonConsts.CONTROLLERID, strCtrlId);
			ctrl.addProperty(VtnServiceJsonConsts.MSG, msg);
			ctrls.add(ctrl);

			/*
			 * If priority of curResponseCode more than priority of finalResponseCode,
			 * save curResponseCode to finalResponseCode.
			 */
			char oldPriority = getResponseCodePriority(operationStatus,
					finalResponseCode);
			char newPriority = getResponseCodePriority(operationStatus,
					curResponseCode);
			if (newPriority < oldPriority) {
				finalResponseCode = curResponseCode;
			}

			String strErrNumber = IpcDataUnitWrapper.getIpcDataUnitValue(
					session.getResponse(idx + 2));
			LOG.trace("NoOfErrors: " + strErrNumber);
			int errNumber =  Integer.parseInt(strErrNumber);

			/* Offset to the next controller */
			/* offset 1: Controller ID + Response Code + NoOfErrors */
			idx = idx + 3;

			/* offset 2: according to NoOfErrors, do the following */
			for (int i = 0; i < errNumber; i++) {
				/* offset 2-1: KeyType + Key */
				idx = idx + 2;

				/* Judge is the last one */
				if (idx >= total) {
					break;
				}

				/* Because the value may not be present for a few key Types,
				 * do the following
				 */
				int type = session.getResponse(idx).getType();
				if (IpcDataUnit.STRUCT == type) {
					/* offset 2-2: Value */
					idx = idx + 1;
				}
			}
		}

		LOG.trace("Completed ConfigResource#handleCommitTcResponse()");
		return finalResponseCode;
	}

	/**
	 * Judge priority of response code.
	 * 
	 * @param operationStatus
 	 * @param response code
	 * @return priority
	 */
	private char getResponseCodePriority(final int operationStatus,
			final int code) {
		LOG.trace("Starts ConfigResource#getResponseCodePriority()");
		if (code == UncTCEnums.OperationStatus.TC_OPER_SUCCESS
				.getCode()) {
			return COMMIT_RESPONSE_CODE_PRI_FF;
		}

		if (operationStatus == 
				UncTCEnums.OperationStatus.TC_OPER_SUCCESS.getCode()) {
			/*
			 * TcResponseCode                 JavaAPI Code   Priority
			 * UNC_RC_CTR_DISCONNECTED        202            1
			 * UNC_RC_CTRLAPI_FAILURE         202            1
			 * UNC_RC_CTR_CONFIG_STATUS_ERR   202            1
			 * Other code                     202            1
			 */
			if (code == UncTCEnums.TcResponseCode.TC_CTR_DISCONNECTED
					.getCode()) {
				return COMMIT_RESPONSE_CODE_PRI_1;
			} else if (code == UncTCEnums.TcResponseCode.TC_CTRLAPI_FAILURE
					.getCode()) {
				return COMMIT_RESPONSE_CODE_PRI_1;
			} else if (code == UncTCEnums.TcResponseCode
					.TC_CTR_CONFIG_STATUS_ERR.getCode()) {
				return COMMIT_RESPONSE_CODE_PRI_1;
			} else {
				return COMMIT_RESPONSE_CODE_PRI_1;
			}
		} else {
			/*
			 * TcResponseCode                 JavaAPI Code   Priority
			 * UNC_RC_INTERNAL_ERR            500            1
			 * UNC_RC_CONFIG_INVAL            409            3
			 * UNC_RC_CTR_BUSY                503            2
			 * UNC_RC_CTRLAPI_FAILURE         500            1
			 * UNC_RC_REQ_NOT_SENT_TO_CTR     500            1
			 * UNC_RC_ERR_DRIVER_NOT_PRESENT  500            1
			 * Other code                     500            1
			 */
			if (code == UncTCEnums.TcResponseCode.TC_INTERNAL_ERR.getCode()) {
				return COMMIT_RESPONSE_CODE_PRI_1;
			} else if (code == UncTCEnums.TcResponseCode.TC_CONFIG_INVAL
					.getCode()) {
				return COMMIT_RESPONSE_CODE_PRI_3;
			} else if (code == UncTCEnums.TcResponseCode.TC_CTR_BUSY
					.getCode()) {
				return COMMIT_RESPONSE_CODE_PRI_2;
			} else if (code == UncTCEnums.TcResponseCode.TC_CTRLAPI_FAILURE
					.getCode()) {
				return COMMIT_RESPONSE_CODE_PRI_1;
			} else if (code == UncTCEnums.TcResponseCode.TC_ERR_DRIVER_NOT_PRESENT
					.getCode()) {
				return COMMIT_RESPONSE_CODE_PRI_1;
			} else {
				return COMMIT_RESPONSE_CODE_PRI_1;
			}
		}
	}

	/**
	 * According to response code, get operationStatus.
	 * 
	 * @param operationStatus
	 * @param responseCode
	 * @return final operationStatus
	 */
	private int getFinalOperStatusCode(final int operationStatus,
			final int responseCode) {
		LOG.trace("Starts ConfigResource#getFinalOperStatusCode()");
		if (operationStatus == 
				UncTCEnums.OperationStatus.TC_OPER_SUCCESS.getCode()) {
			if (responseCode == UncTCEnums.TcResponseCode
					.TC_CTR_DISCONNECTED.getCode()){
				return UncTCEnums.OperationStatus
						.TC_OPER_SUCCESS_CTR_DISCONNECTED.getCode();
			} else if (responseCode == UncTCEnums.TcResponseCode
					.TC_CTRLAPI_FAILURE.getCode()){
				return UncTCEnums.OperationStatus
						.TC_OPER_SUCCESS_CTRLAPI_FAILURE.getCode();
			} else if (responseCode == UncTCEnums.TcResponseCode
					.TC_CTR_CONFIG_STATUS_ERR.getCode()){
				return UncTCEnums.OperationStatus
						.TC_OPER_SUCCESS_CTR_CONFIG_STATUS_ERR.getCode();
			} else {
				return UncTCEnums.OperationStatus
						.TC_OPER_SUCCESS_OTHER_ERROR.getCode();
			}
		} else {
			if (responseCode == UncTCEnums.TcResponseCode.TC_INTERNAL_ERR
					.getCode()) {
				return UncTCEnums.OperationStatus
						.TC_OPER_ABORT_INTERNAL_ERR.getCode();
			} else if (responseCode == UncTCEnums.TcResponseCode
					.TC_CONFIG_INVAL.getCode()) {
				return UncTCEnums.OperationStatus
						.TC_OPER_ABORT_CONFIG_INVAL.getCode();
			} else if (responseCode == UncTCEnums.TcResponseCode
					.TC_CTR_BUSY.getCode()) {
				return UncTCEnums.OperationStatus
						.TC_OPER_ABORT_CTR_BUSY.getCode();
			} else if (responseCode == UncTCEnums.TcResponseCode
					.TC_CTRLAPI_FAILURE.getCode()) {
				return UncTCEnums.OperationStatus
						.TC_OPER_ABORT_CTRLAPI_FAILURE.getCode();
			} else if (responseCode == UncTCEnums.TcResponseCode
					.TC_ERR_DRIVER_NOT_PRESENT.getCode()) {
				return UncTCEnums.OperationStatus
						.TC_OPER_ABORT_ERR_DRIVER_NOT_PRESENT.getCode();
			} else {
				return UncTCEnums.OperationStatus
						.TC_OPER_ABORT_OTHER_ERROR.getCode();
			}
		}
	}

	/**
	 * According to response code, get message.
	 * 
	 * @param operationStatus
	 * @param responseCode
	 * @return message
	 */
	private String getFinalOperStatusMsg(final int operationStatus,
			final int responseCode) {
		LOG.trace("Starts ConfigResource#getFinalOperStatusMsg()");
		if (operationStatus == 
				UncTCEnums.OperationStatus.TC_OPER_SUCCESS.getCode()) {
			if (responseCode == UncTCEnums.TcResponseCode
					.TC_CTR_DISCONNECTED.getCode()){
				return UncTCEnums.OperationStatus
						.TC_OPER_SUCCESS_CTR_DISCONNECTED.getMessage();
			} else if (responseCode == UncTCEnums.TcResponseCode
					.TC_CTRLAPI_FAILURE.getCode()){
				return UncTCEnums.OperationStatus
						.TC_OPER_SUCCESS_CTRLAPI_FAILURE.getMessage();
			} else if (responseCode == UncTCEnums.TcResponseCode
					.TC_CTR_CONFIG_STATUS_ERR.getCode()){
				return UncTCEnums.OperationStatus
						.TC_OPER_SUCCESS_CTR_CONFIG_STATUS_ERR.getMessage();
			} else {
				return UncTCEnums.OperationStatus
						.TC_OPER_SUCCESS_OTHER_ERROR.getMessage();
			}
		} else {
			if (responseCode == UncTCEnums.TcResponseCode.TC_INTERNAL_ERR
					.getCode()) {
				return UncTCEnums.OperationStatus
						.TC_OPER_ABORT_INTERNAL_ERR.getMessage();
			} else if (responseCode == UncTCEnums.TcResponseCode
					.TC_CONFIG_INVAL.getCode()) {
				return UncTCEnums.OperationStatus
						.TC_OPER_ABORT_CONFIG_INVAL.getMessage();
			} else if (responseCode == UncTCEnums.TcResponseCode
					.TC_CTR_BUSY.getCode()) {
				return UncTCEnums.OperationStatus
						.TC_OPER_ABORT_CTR_BUSY.getMessage();
			} else if (responseCode == UncTCEnums.TcResponseCode
					.TC_CTRLAPI_FAILURE.getCode()) {
				return UncTCEnums.OperationStatus
						.TC_OPER_ABORT_CTRLAPI_FAILURE.getMessage();
			} else if (responseCode == UncTCEnums.TcResponseCode
					.TC_ERR_DRIVER_NOT_PRESENT.getCode()) {
				return UncTCEnums.OperationStatus
						.TC_OPER_ABORT_ERR_DRIVER_NOT_PRESENT.getMessage();
			} else {
				return UncTCEnums.OperationStatus
						.TC_OPER_ABORT_OTHER_ERROR.getMessage();
			}
		}
	}

	/**
	 * Create error info of controllers
	 * 
	 * @param operationStatus
	 * @return response body
	 */
	private JsonObject createControllerErrorInfo(final int errCode,
			final JsonArray ctrls) {
		LOG.trace("Starts ConfigResource#createControllerErrorInfo()");
		JsonObject root = new JsonObject();
		JsonObject error = new JsonObject();

		error.addProperty(VtnServiceJsonConsts.CODE, errCode);
		if (ctrls.size() == 1) {
			JsonObject ctrl = ctrls.get(0).getAsJsonObject();
			error.addProperty(VtnServiceJsonConsts.MSG,
					ctrl.get(VtnServiceJsonConsts.MSG).getAsString());
		} else {
			error.addProperty(VtnServiceJsonConsts.MSG,
					MULTI_CONTROLLERS_FAIL_COMMON_MSG);
		}
		error.add(VtnServiceJsonConsts.CONTROLLERS, ctrls);

		root.add(VtnServiceJsonConsts.ERROR, error);

		LOG.trace("Completed ConfigResource#createControllerErrorInfo()");
		return root;
	}

	/**
	 * Set save command response to caller
	 * 
	 * @param operationStatus
	 * @return execute result
	 */
	private int setSaveHttpResponse(final int operationStatus) {
		LOG.trace("Starts ConfigResource#setSaveHttpResponse()");
		int status;
		if (operationStatus != UncTCEnums.OperationStatus.TC_OPER_SUCCESS
				.getCode()) {
			createTcErrorInfo(UncIpcErrorCode.getTcCodes(operationStatus));
			LOG.info("Request not processed successfully");
			status = UncCommonEnum.UncResultCode.UNC_SERVER_ERROR.getValue();
		} else {
			LOG.info("Request processed successfully");
			status = UncCommonEnum.UncResultCode.UNC_SUCCESS.getValue();
		}

		LOG.trace("Completed ConfigResource#setSaveHttpResponse()");
		return status;
	}
}
