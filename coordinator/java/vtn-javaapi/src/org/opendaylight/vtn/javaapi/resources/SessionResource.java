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
import org.opendaylight.vtn.core.ipc.IpcStruct;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.annotation.UNCField;
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
import org.opendaylight.vtn.javaapi.ipc.enums.UncSessionEnums;
import org.opendaylight.vtn.javaapi.ipc.enums.UncStructEnum;
import org.opendaylight.vtn.javaapi.validation.SessionResourceValidator;

/**
 * The Class SessionResource implements delete and get method of session API.
 */

@UNCVtnService(path = "/sessions/{session_id}")
public class SessionResource extends AbstractResource {

	/** The session id. */
	@UNCField("session_id")
	private String sessionId;

	private static final Logger LOG = Logger.getLogger(SessionResource.class
			.getName());

	/**
	 * Instantiates a new session resource.
	 */
	public SessionResource() {
		super();
		LOG.trace("Start SessionResource#SessionResource()");
		setValidator(new SessionResourceValidator(this));
		LOG.trace("Complete SessionResource#SessionResource()");
	}

	/**
	 * Gets the session id.
	 * 
	 * @return the session id
	 */
	public String getSessionId() {
		return sessionId;
	}

	/**
	 * Implementation of delete method of session API
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	@Override
	public int delete() throws VtnServiceException {
		LOG.trace("Starts SessionResource#delete()");
		ClientSession session = null;
		int status = ClientSession.RESP_FATAL;
		try {
			LOG.debug("Start Ipc framework call");
			session = getConnPool().getSession(
					UncSessionEnums.UNCD_IPC_CHANNEL,
					UncSessionEnums.UNCD_IPC_SERVICE,
					UncSessionEnums.ServiceID.kUsessSessDel.ordinal(),
					getExceptionHandler());
			LOG.info("Session created successfully");
			// create request IPC Structure for current session from which user
			// logged in
			final IpcStruct usessIpcReqSessIdCurrent = new IpcStruct(
					UncStructEnum.UsessIpcSessId.getValue());
			usessIpcReqSessIdCurrent.set(VtnServiceIpcConsts.ID,
					IpcDataUnitWrapper.setIpcUint32Value(String
							.valueOf(getSessionID())));
			// create request IPC Structure for session which needs to be
			// deleted
			final IpcStruct usessIpcReqSessIdDelsess = new IpcStruct(
					UncStructEnum.UsessIpcSessId.getValue());
			usessIpcReqSessIdDelsess.set(VtnServiceIpcConsts.ID,
					IpcDataUnitWrapper.setIpcUint32Value(String
							.valueOf(sessionId)));
			final IpcStruct usessIpcReqSessDel = new IpcStruct(
					UncStructEnum.UsessIpcReqSessDel.getValue());
			usessIpcReqSessDel.set(VtnServiceIpcConsts.CURRENT,
					usessIpcReqSessIdCurrent);
			usessIpcReqSessDel.set(VtnServiceIpcConsts.DELSESS,
					usessIpcReqSessIdDelsess);
			// create request packet for IPC call based on API key and
			// JsonObject
			session.addOutput(usessIpcReqSessDel);
			LOG.info("Request packet created successfully");
			status = session.invoke();
			LOG.info("Request packet processed with status:"+status);
			if (status != UncSessionEnums.UsessIpcErrE.USESS_E_OK.ordinal()) {
				LOG.info("Error occurred while performing operation");
				createErrorInfo(
						UncCommonEnum.UncResultCode.UNC_SERVER_ERROR.getValue(),
						UncIpcErrorCode.getSessionCodes(status));
				status = UncResultCode.UNC_SERVER_ERROR.getValue();
			} else {
				LOG.info("Opeartion successfully performed");
				status = UncResultCode.UNC_SUCCESS.getValue();
			}
			LOG.debug("Complete Ipc framework call");
		} catch (final VtnServiceException e) {
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
		LOG.trace("Completed SessionResource#delete()");
		return status;
	}

	/**
	 * Implementation of get method of session API
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	@Override
	public int get() throws VtnServiceException {
		LOG.trace("Starts SessionResource#get()");
		ClientSession session = null;
		int status = ClientSession.RESP_FATAL;
		try {
			LOG.debug("Start Ipc framework call");
			session = getConnPool().getSession(
					UncSessionEnums.UNCD_IPC_CHANNEL,
					UncSessionEnums.UNCD_IPC_SERVICE,
					UncSessionEnums.ServiceID.kUsessSessDetail.ordinal(),
					getExceptionHandler());
			LOG.info("Session created successfully");
			// create request packet for IPC call based on API key and
			// JsonObject
			// create request IPC Structure for current session from which user
			// logged in
			final IpcStruct usessIpcReqSessIdCurrent = new IpcStruct(
					UncStructEnum.UsessIpcSessId.getValue());
			usessIpcReqSessIdCurrent.set(VtnServiceIpcConsts.ID,
					IpcDataUnitWrapper.setIpcUint32Value(String
							.valueOf(getSessionID())));

			// create request IPC Structure for session foe which information is
			// required
			final IpcStruct usessIpcReqSessIdDetail = new IpcStruct(
					UncStructEnum.UsessIpcSessId.getValue());
			usessIpcReqSessIdDetail.set(VtnServiceIpcConsts.ID,
					IpcDataUnitWrapper.setIpcUint32Value(String
							.valueOf(sessionId)));
			final IpcStruct usessIpcReqSessDetail = new IpcStruct(
					UncStructEnum.UsessIpcReqSessDetail.getValue());
			usessIpcReqSessDetail.set(VtnServiceIpcConsts.CURRENT,
					usessIpcReqSessIdCurrent);
			usessIpcReqSessDetail.set(VtnServiceIpcConsts.DETAIL,
					usessIpcReqSessIdDetail);
			session.addOutput(usessIpcReqSessDetail);
			LOG.info("Request packet created successfully");
			status = session.invoke();
			LOG.info("Request packet processed with status:"+status);
			if (status != UncSessionEnums.UsessIpcErrE.USESS_E_OK.ordinal()) {
				LOG.info("Error occurred while performing operation");
				createErrorInfo(
						UncCommonEnum.UncResultCode.UNC_SERVER_ERROR.getValue(),
						UncIpcErrorCode.getSessionCodes(status));
				status = UncResultCode.UNC_SERVER_ERROR.getValue();
			} else {
				LOG.info("Opeartion successfully performed");
				final IpcStruct responseStruct = (IpcStruct) session
						.getResponse(VtnServiceJsonConsts.VAL_0);
				LOG.info("Response:" + responseStruct.toString());
				final JsonObject response = createGetResponse(responseStruct);
				setInfo(response);
				status = UncResultCode.UNC_SUCCESS.getValue();
			}
			LOG.debug("Complete Ipc framework call");
		} catch (final VtnServiceException e) {
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
		LOG.trace("Completed SessionResource#delete()");
		return status;
	}

	/**
	 * Creates the response Json for API user
	 * 
	 * @param responseStruct
	 *            response received from lower layer
	 * @return response Json Object
	 */
	private JsonObject createGetResponse(final IpcStruct responseStruct) {
		LOG.trace("Starts SessionResource#createGetResponse()");
		// convert IPC Structure into Json
		final JsonObject response = new JsonObject();
		final JsonObject sessJson = new JsonObject();
		final IpcStruct ipcResponseStructId = responseStruct
				.getInner(VtnServiceIpcConsts.SESS);
		LOG.debug("user_type:" + responseStruct.get("user_type"));
		// LOG.debug("login_name:" + responseStruct.get("login_name"));
		// LOG.debug("sess_uname:" + responseStruct.get("sess_uname"));

		// add session id to response json
		sessJson.addProperty(
				VtnServiceJsonConsts.SESSIONID,
				IpcDataUnitWrapper.getIpcStructUint32Value(ipcResponseStructId,
						VtnServiceIpcConsts.ID).toString());
		LOG.debug("session_id:"
				+ IpcDataUnitWrapper.getIpcStructUint32Value(
						ipcResponseStructId, VtnServiceIpcConsts.ID));
		// add type to response json
		if (IpcDataUnitWrapper
				.getIpcStructUint32Value(responseStruct,
						VtnServiceIpcConsts.SESS_TYPE)
				.toString()
				.equals(UncSessionEnums.UsessTypeE.USESS_TYPE_WEB_API
						.getValue())) {
			sessJson.addProperty(VtnServiceJsonConsts.TYPE,
					VtnServiceJsonConsts.WEBAPI);
		} else if (IpcDataUnitWrapper
				.getIpcStructUint32Value(responseStruct,
						VtnServiceIpcConsts.SESS_TYPE)
				.toString()
				.equals(UncSessionEnums.UsessTypeE.USESS_TYPE_WEB_UI.getValue())) {
			sessJson.addProperty(VtnServiceJsonConsts.TYPE,
					VtnServiceJsonConsts.WEBUI);
		}
		LOG.debug("type:"
				+ IpcDataUnitWrapper.getIpcStructUint32Value(responseStruct,
						VtnServiceIpcConsts.SESS_TYPE));
		// add user name to response json
		final String userName = IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
				responseStruct, VtnServiceIpcConsts.SESS_UNAME);
		LOG.debug("user_name:" + userName);
		if (VtnServiceIpcConsts.USESS_USER_WEB_ADMIN.equals(userName)) {
			sessJson.addProperty(VtnServiceJsonConsts.USERNAME,
					VtnServiceJsonConsts.ADMIN);
		} else if (VtnServiceIpcConsts.USESS_USER_WEB_OPER.equals(userName)) {
			sessJson.addProperty(VtnServiceJsonConsts.USERNAME,
					VtnServiceJsonConsts.OPER);
		}
		// add user type to response json
		if (IpcDataUnitWrapper
				.getIpcStructUint32Value(responseStruct,
						VtnServiceIpcConsts.USER_TYPE).toString()
				.equals(UncSessionEnums.UserTypeE.USER_TYPE_ADMIN.getValue())) {
			sessJson.addProperty(VtnServiceJsonConsts.USERTYPE,
					VtnServiceJsonConsts.ADMIN);
		} else if (IpcDataUnitWrapper
				.getIpcStructUint32Value(responseStruct,
						VtnServiceIpcConsts.USER_TYPE).toString()
				.equals(UncSessionEnums.UserTypeE.USER_TYPE_OPER.getValue())) {
			sessJson.addProperty(VtnServiceJsonConsts.USERTYPE,
					VtnServiceJsonConsts.OPER);
		}
		LOG.debug("usertype:"
				+ IpcDataUnitWrapper.getIpcStructUint32Value(responseStruct,
						VtnServiceIpcConsts.USER_TYPE));
		// add ipaddress to response json
		sessJson.addProperty(
				VtnServiceJsonConsts.IPADDR,
				IpcDataUnitWrapper.getIpcStructIpv4Value(responseStruct,
						VtnServiceJsonConsts.IPADDR).toString());
		LOG.debug("ipaddr:"
				+ IpcDataUnitWrapper.getIpcStructIpv4Value(responseStruct,
						VtnServiceJsonConsts.IPADDR));
		// add login name to response json
		sessJson.addProperty(
				VtnServiceJsonConsts.LOGIN_NAME,
				IpcDataUnitWrapper.getIpcStructUint8ArrayValue(responseStruct,
						VtnServiceJsonConsts.LOGIN_NAME).toString());
		LOG.debug("login_name:"
				+ IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
						responseStruct, VtnServiceJsonConsts.LOGIN_NAME)
						.toString());
		// add login time to response json
		final IpcStruct ipcResponseTimeStruct = responseStruct
				.getInner(VtnServiceIpcConsts.LOGIN_TIME);
		sessJson.addProperty(
				VtnServiceJsonConsts.LOGIN_TIME,
				IpcDataUnitWrapper.getIpcStructInt64Value(
						ipcResponseTimeStruct, VtnServiceIpcConsts.TV_SEC)
						.toString());
		LOG.debug("login_time:"
				+ IpcDataUnitWrapper.getIpcStructInt64Value(
						ipcResponseTimeStruct, VtnServiceIpcConsts.TV_SEC));
		// add info to response json
		sessJson.addProperty(
				VtnServiceJsonConsts.INFO,
				IpcDataUnitWrapper.getIpcStructUint8ArrayValue(responseStruct,
						VtnServiceJsonConsts.INFO).toString());
		LOG.debug("info:"
				+ IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
						responseStruct, VtnServiceJsonConsts.INFO));
		response.add(VtnServiceJsonConsts.SESSION, sessJson);
		LOG.trace("Completed SessionResource#createGetResponse()");
		return response;
	}

}
