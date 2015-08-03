/*
 * Copyright (c) 2012-2015 NEC Corporation
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
import org.opendaylight.vtn.core.ipc.IpcException;
import org.opendaylight.vtn.core.ipc.IpcStruct;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.RestResource;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceIpcConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcDataUnitWrapper;
import org.opendaylight.vtn.javaapi.ipc.enums.*;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.validation.SessionResourceValidator;

/**
 * The Class SessionsResource implements post and get methods of Session API.
 * 
 */

@UNCVtnService(path = "/sessions")
public class SessionsResource extends AbstractResource {

	private static final Logger LOG = Logger.getLogger(SessionsResource.class
			.getName());

	/**
	 * Instantiates a new sessions resource.
	 */
	public SessionsResource() {
		super();
		LOG.trace("Start SessionsResource#SessionsResource()");
		setValidator(new SessionResourceValidator(this));
		LOG.trace("Complete SessionsResource#SessionsResource()");
	}

	/**
	 * Implementation of post method of session API
	 * 
	 * @param requestBody
	 *            the request JsonObject
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	@Override
	public final int post(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Starts SessionsResource#post()");
		ClientSession session = null;
		ClientSession sessionEnable = null;
		int status = ClientSession.RESP_FATAL;
		boolean exceptionStatus = false;
		String sessionId = null;
		try {
			JsonObject sessionJson = null;
			// unauthorized user check
			if (requestBody != null
					&& requestBody.has(VtnServiceJsonConsts.SESSION)) {
				sessionJson = requestBody
						.getAsJsonObject(VtnServiceJsonConsts.SESSION);
				if (sessionJson.has(VtnServiceJsonConsts.USERNAME)) {
					String username = sessionJson.get(
							VtnServiceJsonConsts.USERNAME).getAsString();
					if (!(username.equalsIgnoreCase(VtnServiceJsonConsts.ADMIN) || username
							.equalsIgnoreCase(VtnServiceJsonConsts.OPER))) {
						createErrorInfo(UncCommonEnum.UncResultCode.UNC_UNAUTHORIZED
								.getValue());
						status = UncResultCode.UNC_CLIENT_ERROR.getValue();
						return status;
					}
				}
			}
			LOG.debug("Start Ipc framework call");
			session = getConnPool().getSession(
					UncSessionEnums.UNCD_IPC_CHANNEL,
					UncSessionEnums.UNCD_IPC_SERVICE,
					UncSessionEnums.ServiceID.kUsessSessAdd.ordinal(),
					getExceptionHandler());
			LOG.info("Session created successfully");
			// set IPC timeout
			session.setTimeout(null);
			// create request packet for IPC call based on API key and
			// JsonObject
			final IpcStruct usessIpcReqSessAdd = new IpcStruct(
					UncStructEnum.UsessIpcReqSessAdd.getValue());
			if (requestBody != null
					&& requestBody.has(VtnServiceJsonConsts.SESSION)) {
				// add user name to usess_ipc_req_sess_add structure
				if (sessionJson.has(VtnServiceJsonConsts.USERNAME)
						&& sessionJson
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.USERNAME)
								.getAsString()
								.equalsIgnoreCase(VtnServiceJsonConsts.ADMIN)) {
					usessIpcReqSessAdd
							.set(VtnServiceIpcConsts.SESS_UNAME,
									IpcDataUnitWrapper
											.setIpcUint8ArrayValue(VtnServiceIpcConsts.USESS_USER_WEB_ADMIN));
					LOG.debug("Login from admin user"
							+ sessionJson.getAsJsonPrimitive(
									VtnServiceJsonConsts.USERNAME)
									.getAsString());
				} else {
					usessIpcReqSessAdd
							.set(VtnServiceIpcConsts.SESS_UNAME,
									IpcDataUnitWrapper
											.setIpcUint8ArrayValue(VtnServiceIpcConsts.USESS_USER_WEB_OPER));
					LOG.debug("Login from oper user"
							+ sessionJson.getAsJsonPrimitive(
									VtnServiceJsonConsts.USERNAME)
									.getAsString());
				}
				LOG.debug("sess_uname: "
						+ IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								usessIpcReqSessAdd,
								VtnServiceIpcConsts.SESS_UNAME));
				// add password to usess_ipc_req_sess_add structure
				usessIpcReqSessAdd.set(
						VtnServiceIpcConsts.SESS_PASSWD,
						IpcDataUnitWrapper.setIpcUint8ArrayValue(sessionJson
								.getAsJsonPrimitive(
										VtnServiceJsonConsts.PASSWORD)
								.getAsString()));

				// add login name to usess_ipc_req_sess_add structure
				if (sessionJson.has(VtnServiceJsonConsts.LOGINNAME)
						&& sessionJson.getAsJsonPrimitive(
								VtnServiceJsonConsts.LOGINNAME).getAsString() != null) {
					usessIpcReqSessAdd
							.set(VtnServiceIpcConsts.LOGIN_NAME,
									IpcDataUnitWrapper
											.setIpcUint8ArrayValue(sessionJson
													.getAsJsonPrimitive(
															VtnServiceJsonConsts.LOGINNAME)
													.getAsString()));
				} else {
					usessIpcReqSessAdd
							.set(VtnServiceIpcConsts.LOGIN_NAME,
									IpcDataUnitWrapper
											.setIpcUint8ArrayValue(VtnServiceConsts.EMPTY_STRING));
				}
				// add type to usess_ipc_req_sess_add structure
				if (sessionJson.has(VtnServiceJsonConsts.TYPE)
						&& sessionJson.getAsJsonPrimitive(
								VtnServiceJsonConsts.TYPE).getAsString() != null
						&& sessionJson
								.getAsJsonPrimitive(VtnServiceJsonConsts.TYPE)
								.getAsString()
								.equalsIgnoreCase(VtnServiceJsonConsts.WEBAPI)) {
					usessIpcReqSessAdd
							.set(VtnServiceIpcConsts.SESS_TYPE,
									IpcDataUnitWrapper
											.setIpcInt32Value(UncSessionEnums.UsessTypeE.USESS_TYPE_WEB_API
													.ordinal()));
				} else {
					usessIpcReqSessAdd
							.set(VtnServiceIpcConsts.SESS_TYPE,
									IpcDataUnitWrapper
											.setIpcInt32Value(UncSessionEnums.UsessTypeE.USESS_TYPE_WEB_UI
													.ordinal()));
				}
				LOG.debug("sess_type: "
						+ IpcDataUnitWrapper.getIpcStructUint32Value(
								usessIpcReqSessAdd,
								VtnServiceIpcConsts.SESS_TYPE));

				// add ip address to usess_ipc_req_sess_add structure
				usessIpcReqSessAdd.set(VtnServiceJsonConsts.IPADDR,
						IpcDataUnitWrapper
								.setIpcInet4AddressValue(sessionJson
										.getAsJsonPrimitive(
												VtnServiceJsonConsts.IPADDR)
										.getAsString()));

				// add info to usess_ipc_req_sess_add structure
				if (sessionJson.has(VtnServiceJsonConsts.INFO)
						&& sessionJson.getAsJsonPrimitive(
								VtnServiceJsonConsts.INFO).getAsString() != null) {
					usessIpcReqSessAdd.set(VtnServiceJsonConsts.INFO,
							IpcDataUnitWrapper
									.setIpcUint8ArrayValue(sessionJson
											.getAsJsonPrimitive(
													VtnServiceJsonConsts.INFO)
											.getAsString()));
				} else {
					usessIpcReqSessAdd
							.set(VtnServiceJsonConsts.INFO,
									IpcDataUnitWrapper
											.setIpcUint8ArrayValue(VtnServiceConsts.EMPTY_STRING));
				}
				session.addOutput(usessIpcReqSessAdd);
				LOG.info("Request packet created successfully");
				long start = System.currentTimeMillis();
				status = session.invoke();
				LOG.debug("The treatment of under layer cost the following time: "
						+ (System.currentTimeMillis() - start) + "(ms)");
				LOG.info("Request packet processed with status:" + status);
				if (status != UncSessionEnums.UsessIpcErrE.USESS_E_OK.ordinal()) {
					LOG.info("Error occurred while performing operation");
					createSessionErrorInfo(UncIpcErrorCode
							.getSessionCodes(status));
					status = UncResultCode.UNC_SERVER_ERROR.getValue();
				} else {
					LOG.info("Opeartion successfully performed");
					final JsonObject response = new JsonObject();
					final IpcStruct responseStruct = (IpcStruct) session
							.getResponse(0);
					final JsonObject sessionInfo = new JsonObject();
					sessionId = IpcDataUnitWrapper
							.getIpcStructUint32Value(responseStruct,
									VtnServiceIpcConsts.ID).toString();
					sessionInfo.addProperty(VtnServiceJsonConsts.SESSIONID,
							sessionId);
					response.add(VtnServiceJsonConsts.SESSION, sessionInfo);
					setInfo(response);

					if (sessionJson.has(VtnServiceJsonConsts.USERNAME)
							&& sessionJson
									.getAsJsonPrimitive(
											VtnServiceJsonConsts.USERNAME)
									.getAsString()
									.equalsIgnoreCase(
											VtnServiceJsonConsts.ADMIN)) {
						sessionEnable = getConnPool().getSession(
								UncSessionEnums.UNCD_IPC_CHANNEL,
								UncSessionEnums.UNCD_IPC_SERVICE,
								UncSessionEnums.ServiceID.kUsessEnable
										.ordinal(), getExceptionHandler());
						LOG.info("Session created successfully");
						//set IPC timeout
						sessionEnable.setTimeout(null);
						// create request packet for IPC call based on API key
						// and JsonObject
						final IpcStruct usessIpcReqSessEnable = new IpcStruct(
								UncStructEnum.UsessIpcReqSessEnable.getValue());
						// create request IPC Structure for current session from
						// which user logged in
						final IpcStruct usessIpcReqSessIdCurrent = new IpcStruct(
								UncStructEnum.UsessIpcSessId.getValue());
						usessIpcReqSessIdCurrent
								.set(VtnServiceIpcConsts.ID, IpcDataUnitWrapper
										.setIpcUint32Value(sessionId));
						usessIpcReqSessEnable.set(VtnServiceIpcConsts.CURRENT,
								usessIpcReqSessIdCurrent);
						usessIpcReqSessEnable
								.set(VtnServiceIpcConsts.ENABLE_PASSWORD,
										IpcDataUnitWrapper
												.setIpcUint8ArrayValue(sessionJson
														.getAsJsonPrimitive(
																VtnServiceJsonConsts.PASSWORD)
														.getAsString()));
						sessionEnable.addOutput(usessIpcReqSessEnable);
						LOG.info("Request packet created successfully");
						start = System.currentTimeMillis();
						status = sessionEnable.invoke();
						LOG.debug("The treatment of under layer cost the following time: "
								+ (System.currentTimeMillis() - start) + "(ms)");
						LOG.info("Request packet processed with status:"
								+ status);
						if (status != UncSessionEnums.UsessIpcErrE.USESS_E_OK
								.ordinal()) {
							exceptionStatus = true;
							LOG.info("Error occurred while performing operation");
							createSessionErrorInfo(UncIpcErrorCode
									.getSessionCodes(status));
							status = UncResultCode.UNC_SERVER_ERROR.getValue();
						} else {
							status = UncResultCode.UNC_SUCCESS.getValue();
						}
					} else {
						status = UncResultCode.UNC_SUCCESS.getValue();
					}
				}
			}
			LOG.debug("Complete Ipc framework call");
		} catch (final VtnServiceException e) {
			exceptionStatus = true;
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
			exceptionStatus = true;
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
			if(exceptionStatus && (sessionId != null)) {
				final RestResource resource = new RestResource();
				resource.setPath("/sessions/"+ sessionId);
				resource.setSessionID(Long.parseLong(sessionId));
				resource.delete();
			}
			// destroy session by common handler
			getConnPool().destroySession(session);
			if (sessionEnable != null) {
				getConnPool().destroySession(sessionEnable);
			}
		}
		LOG.trace("Completed SessionsResource#post()");
		return status;
	}

	/**
	 * Implementation of Get method of session API
	 * 
	 * @param requestBody
	 *            the request JsonObject
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	@Override
	public final int get(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("starts SessionsResource#get()");
		ClientSession session = null;
		int status = ClientSession.RESP_FATAL;
		try {
			LOG.debug("Start Ipc framework call");
			if (requestBody != null
					&& requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.OP)
							.getAsString()
							.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
				session = getConnPool().getSession(
						UncSessionEnums.UNCD_IPC_CHANNEL,
						UncSessionEnums.UNCD_IPC_SERVICE,
						UncSessionEnums.ServiceID.kUsessSessCount.ordinal(),
						getExceptionHandler());
			} else {
				session = getConnPool().getSession(
						UncSessionEnums.UNCD_IPC_CHANNEL,
						UncSessionEnums.UNCD_IPC_SERVICE,
						UncSessionEnums.ServiceID.kUsessSessList.ordinal(),
						getExceptionHandler());
			}
			LOG.info("Session created successfully");
			// create request packet for IPC call based on API key and
			// JsonObject
			final IpcStruct usessIpcReqSessId = new IpcStruct(
					UncStructEnum.UsessIpcSessId.getValue());
			usessIpcReqSessId.set(VtnServiceJsonConsts.ID, IpcDataUnitWrapper
					.setIpcUint32Value(String.valueOf(getSessionID())));
			session.addOutput(usessIpcReqSessId);
			LOG.info("Request packet created successfully");
			long start = System.currentTimeMillis();
			status = session.invoke();
			LOG.debug("The treatment of under layer cost the following time: "
					+ (System.currentTimeMillis() - start) + "(ms)");
			LOG.info("Request packet processed with status:" + status);
			if (status != UncSessionEnums.UsessIpcErrE.USESS_E_OK.ordinal()) {
				LOG.info("Error occurred while performing operation");
				createSessionErrorInfo(UncIpcErrorCode.getSessionCodes(status));
				status = UncResultCode.UNC_SERVER_ERROR.getValue();
			} else {
				LOG.info("Opeartion successfully performed");
				final JsonObject response = new JsonObject();

				String opType = VtnServiceJsonConsts.NORMAL;
				if (requestBody.has(VtnServiceJsonConsts.OP)) {
					opType = requestBody.get(VtnServiceJsonConsts.OP)
							.getAsString();
				}
				if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
					/*
					 * Create Json for Count
					 */
					JsonObject sessJson = null;
					sessJson = new JsonObject();
					sessJson.addProperty(VtnServiceJsonConsts.COUNT,
							IpcDataUnitWrapper.getIpcDataUnitValue(session
									.getResponse(0)));
					response.add(VtnServiceJsonConsts.SESSIONS, sessJson);
				} else {
					createGetResponse(session, response, opType);
				}
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
		LOG.trace("Completed SessionsResource#delete()");
		return status;
	}

	/**
	 * Creates the response Json for API user
	 * 
	 * @param session
	 *            session object created
	 * @param response
	 *            response Json Object
	 * @param opType
	 *            operation type
	 * 
	 * @throws IpcException
	 */
	private void createGetResponse(final ClientSession session,
			final JsonObject response, final String opType) throws IpcException {
		LOG.trace("Starts SessionsResource#createGetResponse()");
		final JsonArray sessArray = new JsonArray();
		final int count = session.getResponseCount();
		LOG.debug("Operation:" + opType);
		for (int i = VtnServiceJsonConsts.VAL_0; i < count; i++) {
			final IpcStruct responseStruct = (IpcStruct) session.getResponse(i);
			LOG.info("Response:" + responseStruct.toString());
			JsonObject sessJson = null;
			sessJson = new JsonObject();
			final IpcStruct ipcResponseStructId = responseStruct
					.getInner(VtnServiceIpcConsts.SESS);
			sessJson.addProperty(
					VtnServiceJsonConsts.SESSIONID,
					IpcDataUnitWrapper.getIpcStructUint32Value(
							ipcResponseStructId, VtnServiceIpcConsts.ID)
							.toString());
			LOG.debug("session_id:"
					+ IpcDataUnitWrapper.getIpcStructUint32Value(
							ipcResponseStructId, VtnServiceIpcConsts.ID));

			if (opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
				LOG.debug("Case : detail");
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
						.equals(UncSessionEnums.UsessTypeE.USESS_TYPE_WEB_UI
								.getValue())) {
					sessJson.addProperty(VtnServiceJsonConsts.TYPE,
							VtnServiceJsonConsts.WEBUI);
				} else {
					LOG.debug("Incorrect value for type");
				}
				LOG.debug("type:"
						+ IpcDataUnitWrapper.getIpcStructUint32Value(
								responseStruct, VtnServiceIpcConsts.SESS_TYPE));
				// add user name to response json
				final String userName = IpcDataUnitWrapper
						.getIpcStructUint8ArrayValue(responseStruct,
								VtnServiceIpcConsts.SESS_UNAME);
				LOG.debug("user_name:" + userName);
				if (VtnServiceIpcConsts.USESS_USER_WEB_ADMIN.equals(userName)) {
					sessJson.addProperty(VtnServiceJsonConsts.USERNAME,
							VtnServiceJsonConsts.ADMIN);
				} else if (VtnServiceIpcConsts.USESS_USER_WEB_OPER
						.equals(userName)) {
					sessJson.addProperty(VtnServiceJsonConsts.USERNAME,
							VtnServiceJsonConsts.OPER);
				} else {
					LOG.debug("Incorrect value for user_name");
				}
				// add user type to response json
				if (IpcDataUnitWrapper
						.getIpcStructUint32Value(responseStruct,
								VtnServiceIpcConsts.USER_TYPE)
						.toString()
						.equals(UncSessionEnums.UserTypeE.USER_TYPE_ADMIN
								.getValue())) {
					sessJson.addProperty(VtnServiceJsonConsts.USERTYPE,
							VtnServiceJsonConsts.ADMIN);
				} else if (IpcDataUnitWrapper
						.getIpcStructUint32Value(responseStruct,
								VtnServiceIpcConsts.USER_TYPE)
						.toString()
						.equals(UncSessionEnums.UserTypeE.USER_TYPE_OPER
								.getValue())) {
					sessJson.addProperty(VtnServiceJsonConsts.USERTYPE,
							VtnServiceJsonConsts.OPER);
				} else {
					LOG.debug("Incorrect value for usertype");
				}
				LOG.debug("usertype:"
						+ IpcDataUnitWrapper.getIpcStructUint32Value(
								responseStruct, VtnServiceIpcConsts.USER_TYPE));
				// add ipaddress to response json
				sessJson.addProperty(
						VtnServiceJsonConsts.IPADDR,
						IpcDataUnitWrapper.getIpcStructIpv4Value(
								responseStruct, VtnServiceJsonConsts.IPADDR)
								.toString());
				LOG.debug("ipaddr:"
						+ IpcDataUnitWrapper.getIpcStructIpv4Value(
								responseStruct, VtnServiceJsonConsts.IPADDR));
				// add login name to response json
				sessJson.addProperty(
						VtnServiceJsonConsts.LOGIN_NAME,
						IpcDataUnitWrapper
								.getIpcStructUint8ArrayValue(responseStruct,
										VtnServiceJsonConsts.LOGIN_NAME)
								.toString());
				LOG.debug("login_name:"
						+ IpcDataUnitWrapper
								.getIpcStructUint8ArrayValue(responseStruct,
										VtnServiceJsonConsts.LOGIN_NAME)
								.toString());
				// add login time to response json
				final IpcStruct ipcResponseTimeStruct = responseStruct
						.getInner(VtnServiceIpcConsts.LOGIN_TIME);
				sessJson.addProperty(
						VtnServiceJsonConsts.LOGIN_TIME,
						IpcDataUnitWrapper.getIpcStructInt64Value(
								ipcResponseTimeStruct,
								VtnServiceIpcConsts.TV_SEC).toString());
				LOG.debug("login_time:"
						+ IpcDataUnitWrapper.getIpcStructInt64Value(
								ipcResponseTimeStruct,
								VtnServiceIpcConsts.TV_SEC));
				// add info to response json
				sessJson.addProperty(
						VtnServiceJsonConsts.INFO,
						IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								responseStruct, VtnServiceJsonConsts.INFO)
								.toString());
				LOG.debug("info:"
						+ IpcDataUnitWrapper.getIpcStructUint8ArrayValue(
								responseStruct, VtnServiceJsonConsts.INFO));
				// add mode to response json
				if (IpcDataUnitWrapper
						.getIpcStructUint32Value(responseStruct,
								VtnServiceIpcConsts.SESS_MODE)
						.toString()
						.equals(UncSessionEnums.UsessModeE.USESS_MODE_OPER
								.getValue())) {
					sessJson.addProperty(VtnServiceJsonConsts.MODE,
							VtnServiceJsonConsts.OPER);
				} else if (IpcDataUnitWrapper
						.getIpcStructUint32Value(responseStruct,
								VtnServiceIpcConsts.SESS_MODE)
						.toString()
						.equals(UncSessionEnums.UsessModeE.USESS_MODE_ENABLE
								.getValue())) {
					sessJson.addProperty(VtnServiceJsonConsts.MODE,
							VtnServiceJsonConsts.ENABLE);
				} else if (IpcDataUnitWrapper
						.getIpcStructUint32Value(responseStruct,
								VtnServiceIpcConsts.SESS_MODE)
						.toString()
						.equals(UncSessionEnums.UsessModeE.USESS_MODE_DEL
								.getValue())) {
					sessJson.addProperty(VtnServiceJsonConsts.MODE,
							VtnServiceJsonConsts.DEL);
				} else if (IpcDataUnitWrapper
						.getIpcStructUint32Value(responseStruct,
								VtnServiceIpcConsts.SESS_MODE)
						.toString()
						.equals(UncSessionEnums.UsessModeE.USESS_MODE_UNKNOWN
								.getValue())) {
					sessJson.addProperty(VtnServiceJsonConsts.MODE,
							VtnServiceJsonConsts.UNKNOWN);
				} else {
					LOG.debug("Incorrect value for mode");
				}
				LOG.debug("mode: "
						+ IpcDataUnitWrapper.getIpcStructUint32Value(
								responseStruct, VtnServiceIpcConsts.SESS_MODE));
				// add configstatus to response json
				boolean enableFlag = false;
				if (IpcDataUnitWrapper
						.getIpcStructUint32Value(responseStruct,
								VtnServiceIpcConsts.CONFIG_STATUS)
						.toString()
						.equals(UncSessionEnums.UsessConfigModeE.CONFIG_STATUS_NONE
								.getValue())) {
					sessJson.addProperty(VtnServiceJsonConsts.CONFIGSTATUS,
							VtnServiceJsonConsts.DISABLE);
				} else if (IpcDataUnitWrapper
						.getIpcStructUint32Value(responseStruct,
								VtnServiceIpcConsts.CONFIG_STATUS)
						.toString()
						.equals(UncSessionEnums.UsessConfigModeE.CONFIG_STATUS_TCLOCK
								.getValue())) {
					sessJson.addProperty(VtnServiceJsonConsts.CONFIGSTATUS,
							VtnServiceJsonConsts.ENABLE);
					enableFlag = true;
				} else if (IpcDataUnitWrapper
						.getIpcStructUint32Value(responseStruct,
								VtnServiceIpcConsts.CONFIG_STATUS)
						.equals(UncSessionEnums.UsessConfigModeE.CONFIG_STATUS_TCLOCK_PART
								.getValue())) {
					sessJson.addProperty(VtnServiceJsonConsts.CONFIGSTATUS,
							VtnServiceJsonConsts.ENABLE);
					enableFlag = true;
				} else{
					LOG.debug("Incorrect value for config_status");
				}
				LOG.debug("configstatus: "
						+ IpcDataUnitWrapper.getIpcStructUint32Value(
								responseStruct,
								VtnServiceIpcConsts.CONFIG_STATUS));

				// add config_mode to response json
				if (enableFlag) {
					if (IpcDataUnitWrapper
							.getIpcStructUint32Value(responseStruct,
									VtnServiceIpcConsts.CONFIG_MODE)
							.equals(String.valueOf(UncTCEnums.ConfigMode.TC_CONFIG_GLOBAL
									.ordinal()))) {
						sessJson.addProperty(VtnServiceJsonConsts.CONFIGMODE,
								VtnServiceJsonConsts.GLOBAL_MODE);
					} else if (IpcDataUnitWrapper
							.getIpcStructUint32Value(responseStruct,
									VtnServiceIpcConsts.CONFIG_MODE)
							.equals(String.valueOf(UncTCEnums.ConfigMode.TC_CONFIG_REAL
									.ordinal()))) {
						sessJson.addProperty(VtnServiceJsonConsts.CONFIGMODE,
								VtnServiceJsonConsts.REAL_MODE);
					} else if (IpcDataUnitWrapper
							.getIpcStructUint32Value(responseStruct,
									VtnServiceIpcConsts.CONFIG_MODE)
							.equals(String.valueOf(UncTCEnums.ConfigMode.TC_CONFIG_VIRTUAL
									.ordinal()))) {
						sessJson.addProperty(VtnServiceJsonConsts.CONFIGMODE,
								VtnServiceJsonConsts.VIRTUAL_MODE);
					} else if (IpcDataUnitWrapper
							.getIpcStructUint32Value(responseStruct,
									VtnServiceIpcConsts.CONFIG_MODE)
							.equals(String.valueOf(UncTCEnums.ConfigMode.TC_CONFIG_VTN
									.ordinal()))) {
						sessJson.addProperty(VtnServiceJsonConsts.CONFIGMODE,
								VtnServiceJsonConsts.VTN_MODE);
						sessJson.addProperty(VtnServiceJsonConsts.VTNNAME,
								IpcDataUnitWrapper
										.getIpcStructUint8ArrayValue(responseStruct,
												VtnServiceIpcConsts.VTNNAME));
					} else {
						LOG.debug("Incorrect value for config_mode");
					}
				}
			}
			sessArray.add(sessJson);
		}
		response.add(VtnServiceJsonConsts.SESSIONS, sessArray);
		LOG.trace("Completed SessionsResource#createGetResponse()");
	}
}
