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
import org.opendaylight.vtn.core.ipc.IpcString;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcDataUnitWrapper;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncIpcErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncTCEnums;
import org.opendaylight.vtn.javaapi.validation.ConfigModeResourceValidator;

/**
 * The Class AcquireConfigModeResource implements the post method of
 * Configuration Mode API.
 * 
 */

@UNCVtnService(path = "/configuration/configmode")
public class AcquireConfigModeResource extends AbstractResource {

	private static final Logger LOG = Logger
			.getLogger(AcquireConfigModeResource.class.getName());

	/**
	 * Instantiates a new acquire config mode resource.
	 */
	public AcquireConfigModeResource() {
		super();
		LOG.trace("Start AcquireConfigModeResource#VReadLockResource()");
		setValidator(new ConfigModeResourceValidator(this));
		LOG.trace("Complete AcquireConfigModeResource#ReadLockResource()");
	}

	/**
	 * Implementation of Post method of configuration mode API.
	 * 
	 * @param requestBody
	 *            the request Json Object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	@Override
	public final int post(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Starts AcquireConfigModeResource#post()");
		ClientSession session = null;
		int status = ClientSession.RESP_FATAL;
		boolean isForce = false;
		boolean isVtnMode = false;
		String mode = null;
		try {
			LOG.debug("Start Ipc framework call");
			session = getConnPool().getSession(UncTCEnums.UNC_CHANNEL_NAME,
					UncTCEnums.TC_SERVICE_NAME,
					UncTCEnums.ServiceID.TC_CONFIG_SERVICES.ordinal(),
					getExceptionHandler());
			LOG.info("Session created successfully");
			if (requestBody != null
					&& requestBody.has(VtnServiceJsonConsts.OP)
					&& requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.OP)
							.getAsString()
							.equalsIgnoreCase(VtnServiceJsonConsts.FORCE)) {
				session.addOutput(IpcDataUnitWrapper
						.setIpcUint32Value(UncTCEnums.ServiceType.TC_OP_CONFIG_ACQUIRE_FORCE
								.ordinal()));
				isForce = true;
			} else if (requestBody != null
					&& requestBody.has(VtnServiceJsonConsts.TIMEOUT)) {
				session.setTimeout(null);
				session.addOutput(IpcDataUnitWrapper
						.setIpcUint32Value(UncTCEnums.ServiceType.TC_OP_CONFIG_ACQUIRE_TIMED
								.ordinal()));
			} else if (requestBody != null
					&& requestBody.has(VtnServiceJsonConsts.MODE)){
				mode = requestBody.getAsJsonPrimitive(
						VtnServiceJsonConsts.MODE).getAsString();
				session.addOutput(IpcDataUnitWrapper
							.setIpcUint32Value(UncTCEnums.ServiceType.TC_OP_CONFIG_ACQUIRE_PARTIAL
									.ordinal()));
			} else {
				session.addOutput(IpcDataUnitWrapper
						.setIpcUint32Value(UncTCEnums.ServiceType.TC_OP_CONFIG_ACQUIRE
								.ordinal()));
			}
			session.addOutput(IpcDataUnitWrapper
					.setIpcUint32Value(getSessionID()));
			if (!isForce && requestBody != null
					&& requestBody.has(VtnServiceJsonConsts.TIMEOUT)) {
				String str = requestBody.getAsJsonPrimitive(
						VtnServiceJsonConsts.TIMEOUT).getAsString();
				int timeout = Integer.parseInt(str);
				session.addOutput(IpcDataUnitWrapper.setIpcInt32Value(timeout));
			}

			if (!isForce && requestBody != null
					&& !requestBody.has(VtnServiceJsonConsts.TIMEOUT)
					&& requestBody.has(VtnServiceJsonConsts.MODE)) {
				if(mode.equals(VtnServiceJsonConsts.VIRTUAL_MODE)) {
					session.addOutput(IpcDataUnitWrapper
						.setIpcUint32Value(UncTCEnums.ConfigMode
						.TC_CONFIG_VIRTUAL.ordinal()));
				} else if (mode.equals(VtnServiceJsonConsts.REAL_MODE)) {
					session.addOutput(IpcDataUnitWrapper
						.setIpcUint32Value(UncTCEnums.ConfigMode
						.TC_CONFIG_REAL.ordinal()));
				} else if (mode.equals(VtnServiceJsonConsts.VTN_MODE)) {
					isVtnMode = true;
					session.addOutput(IpcDataUnitWrapper
						.setIpcUint32Value(UncTCEnums.ConfigMode
						.TC_CONFIG_VTN.ordinal()));
				} else {
					session.addOutput(IpcDataUnitWrapper
						.setIpcUint32Value(UncTCEnums.ConfigMode
						.TC_CONFIG_GLOBAL.ordinal()));
				}

				if (isVtnMode) {
					String vtnName = requestBody.getAsJsonPrimitive(
						VtnServiceJsonConsts.VTNNAME).getAsString();
					session.addOutput(new IpcString(vtnName));
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
			final String sessionId = IpcDataUnitWrapper
					.getIpcDataUnitValue(session
							.getResponse(VtnServiceJsonConsts.VAL_1));
			final int operationStatus = Integer.parseInt(IpcDataUnitWrapper
					.getIpcDataUnitValue(session
							.getResponse(VtnServiceJsonConsts.VAL_2)));
			LOG.info("Response  received successfully");
			LOG.info("OperationType " + operationType);
			LOG.info("SessionId " + sessionId);
			LOG.info("OperationStatus " + operationStatus);
			if (operationStatus != UncTCEnums.OperationStatus.TC_OPER_SUCCESS
					.getCode()) {
				createTcErrorInfo(UncIpcErrorCode.getTcCodes(operationStatus));
				LOG.info("Request not processed successfully");
				status = UncCommonEnum.UncResultCode.UNC_SERVER_ERROR
						.getValue();
			} else {
				final String configId = IpcDataUnitWrapper
						.getIpcDataUnitValue(session
								.getResponse(VtnServiceJsonConsts.VAL_3));
				LOG.info("ConfigId " + configId);
				final JsonObject response = new JsonObject();
				final JsonObject config = new JsonObject();
				config.addProperty(VtnServiceJsonConsts.CONFIGID, configId);
				response.add(VtnServiceJsonConsts.CONFIGMODE, config);
				setInfo(response);
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
		LOG.trace("Completed AcquireConfigModeResource#post()");
		return status;
	}

}
