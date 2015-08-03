/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.resources;

import org.opendaylight.vtn.core.ipc.ClientSession;
import org.opendaylight.vtn.core.ipc.IpcException;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.annotation.UNCField;
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
 * The Class ReleaseConfigModeResource implements delete method of configuration
 * mode API.
 * 
 */
@UNCVtnService(path = "/configuration/configmode/{config_id}")
public class ReleaseConfigModeResource extends AbstractResource {

	@UNCField("config_id")
	private String configId;

	/**
	 * Gets the config id.
	 * 
	 * @return the config id
	 */
	public final String getConfigId() {
		return configId;
	}

	private static final Logger LOG = Logger
			.getLogger(ReleaseConfigModeResource.class.getName());

	/**
	 * Instantiates a new release config mode resource.
	 * 
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	public ReleaseConfigModeResource() {
		LOG.trace("Start ReleaseConfigModeResource#ReleaseConfigModeResource()");
		setValidator(new ConfigModeResourceValidator(this));
		LOG.trace("Complete ReleaseConfigModeResource#ReleaseConfigModeResource()");
	}

	/**
	 * Implementation of delete method of configuration mode API
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
		LOG.trace("Starts ReleaseConfigModeResource#delete()");
		ClientSession session = null;
		int status = ClientSession.RESP_FATAL;
		try {
			session = getConnPool().getSession(UncTCEnums.UNC_CHANNEL_NAME,
					UncTCEnums.TC_SERVICE_NAME,
					UncTCEnums.ServiceID.TC_CONFIG_SERVICES.ordinal(),
					getExceptionHandler());
			LOG.info("Session created successfully");
			session.addOutput(IpcDataUnitWrapper
					.setIpcUint32Value(UncTCEnums.ServiceType.TC_OP_CONFIG_RELEASE
							.ordinal()));
			session.addOutput(IpcDataUnitWrapper
					.setIpcUint32Value(getSessionID()));
			session.addOutput(IpcDataUnitWrapper
					.setIpcUint32Value(getConfigId()));
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
			LOG.info("Response receieved successfully");
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
			LOG.trace("Completed AcquireConfigModeResource#post()");
		}
		LOG.trace("Completed ReleaseConfigModeResource#delete()");
		return status;
	}
}
