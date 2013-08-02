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
import org.opendaylight.vtn.javaapi.validation.UserResourceValidator;

/**
 * The Class UserResource implements the put method of password API.
 */

@UNCVtnService(path = "/users/{username}/password")
public class UserResource extends AbstractResource {

	private static final Logger LOG = Logger.getLogger(ConfigResource.class
			.getName());

	@UNCField("username")
	private String userName;

	/**
	 * Gets the user name.
	 * 
	 * @return the user name
	 */
	public String getUserName() {
		return userName;
	}

	/**
	 * Instantiates a new user resource.
	 */
	public UserResource() {
		super();
		LOG.trace("Start UserResource#UserResource()");
		setValidator(new UserResourceValidator(this));
		LOG.trace("Complete UserResource#UserResource()");
	}

	/**
	 * Implementation of put method of password API
	 * 
	 * @param requestBody
	 *            the request JsonObject
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	@Override
	public int put(final JsonObject requestBody) throws VtnServiceException {
		LOG.trace("Starts UserResource#put()");
		ClientSession session = null;
		int status = ClientSession.RESP_FATAL;
		try {
			LOG.debug("Start Ipc framework call");
			session = getConnPool().getSession(
					UncSessionEnums.UNCD_IPC_CHANNEL,
					UncSessionEnums.UNCD_IPC_SERVICE,
					UncSessionEnums.ServiceID.kUserUserPasswd.ordinal(),
					getExceptionHandler());
			LOG.info("Session created successfully");
			// create request packet for IPC call based on API key and
			// JsonObject
			final IpcStruct usessIpcReqUserPasswd = new IpcStruct(
					UncStructEnum.usessIpcReqUserPasswd.getValue());
			// create request IPC Structure for session from which user
			// logged in
			final IpcStruct usessIpcReqSessId = new IpcStruct(
					UncStructEnum.UsessIpcSessId.getValue());
			usessIpcReqSessId.set(VtnServiceIpcConsts.ID, IpcDataUnitWrapper
					.setIpcUint32Value(String.valueOf(getSessionID())));
			usessIpcReqUserPasswd.set(VtnServiceJsonConsts.CURRENT,
					usessIpcReqSessId);

			// set user name
			usessIpcReqUserPasswd.set(VtnServiceIpcConsts.SESS_UNAME,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(getUserName()));

			// set password
			usessIpcReqUserPasswd.set(
					VtnServiceJsonConsts.PASSWORD,
					IpcDataUnitWrapper.setIpcUint8ArrayValue(requestBody
							.getAsJsonPrimitive(VtnServiceJsonConsts.PASSWORD)
							.getAsString().trim()));

			session.addOutput(usessIpcReqUserPasswd);
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
			LOG.info("Error occured while performing invoke operation");
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
		LOG.trace("Completed UserResource#put()");
		return status;
	}
}
