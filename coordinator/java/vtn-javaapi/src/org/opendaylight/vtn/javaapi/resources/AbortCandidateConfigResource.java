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
import org.opendaylight.vtn.javaapi.validation.AbortCandidateConfigResourceValidator;

/**
 * The Class AbortCandidateConfigResource implements the put method of Abort
 * Candidate Configuration API
 * 
 */

@UNCVtnService(path = "/configuration/candidate")
public class AbortCandidateConfigResource extends AbstractResource {

	private static final Logger LOG = Logger
			.getLogger(AbortCandidateConfigResource.class.getName());

	/**
	 * Instantiates a new abort candidate config resource.
	 */
	public AbortCandidateConfigResource() {
		super();
		LOG.trace("Start AbortCandidateConfigResource#AbortCandidateConfigResource()");
		setValidator(new AbortCandidateConfigResourceValidator(this));
		LOG.trace("Complete AbortCandidateConfigResource#AbortCandidateConfigResource()");
	}

	/**
	 * Implementation of Put method of Abort Candidate Configuration API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	@Override
	public final int put(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Starts AbortCandidateConfigResource#put()");
		ClientSession session = null;
		int status = ClientSession.RESP_FATAL;
		String timeout = null;
		String cancelAudit = null;
		try {
			LOG.debug("Start Ipc framework call");
			session = getConnPool().getSession(UncTCEnums.UNC_CHANNEL_NAME,
					UncTCEnums.TC_SERVICE_NAME,
					UncTCEnums.ServiceID.TC_CANDIDATE_SERVICES.ordinal(),
					getExceptionHandler());
			LOG.info("Session created successfully");
			// set session timeout as infinity for abort operation
			session.setTimeout(null);
			if (requestBody != null
					&& requestBody.has(VtnServiceJsonConsts.CANDIDATE)
					&& requestBody
							.getAsJsonObject(VtnServiceJsonConsts.CANDIDATE)
							.getAsJsonPrimitive(VtnServiceJsonConsts.OPERATION)
							.getAsString()
							.equalsIgnoreCase(VtnServiceJsonConsts.ABORT)) {
				final JsonObject candidate = requestBody
						.getAsJsonObject(VtnServiceJsonConsts.CANDIDATE);
				if (candidate.has(VtnServiceJsonConsts.TIMEOUT)) {
					timeout = candidate.getAsJsonPrimitive(
							VtnServiceJsonConsts.TIMEOUT).getAsString();
				}

				if (candidate.has(VtnServiceJsonConsts.CANCEL_AUDIT)) {
					cancelAudit = candidate.getAsJsonPrimitive(
							VtnServiceJsonConsts.CANCEL_AUDIT).getAsString();
				}

				session.addOutput(IpcDataUnitWrapper
							.setIpcUint32Value(UncTCEnums.ServiceType
									.TC_OP_CANDIDATE_ABORT_TIMED.ordinal()));
			} else {
				LOG.warning("Request body is not correct");
			}
			session.addOutput(IpcDataUnitWrapper
					.setIpcUint32Value(getSessionID()));
			session.addOutput(IpcDataUnitWrapper
					.setIpcUint32Value(getConfigID()));
			session.addOutput(IpcDataUnitWrapper.setIpcInt32Value(
					Integer.parseInt(timeout)));
			session.addOutput(IpcDataUnitWrapper
					.setIpcUint8Value(cancelAudit));
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
			final String configId = IpcDataUnitWrapper.getIpcDataUnitValue(
					session.getResponse(VtnServiceJsonConsts.VAL_2)).toString();
			final int operationStatus = Integer.parseInt(IpcDataUnitWrapper
					.getIpcDataUnitValue(session
							.getResponse(VtnServiceJsonConsts.VAL_3)));
			LOG.info("Response retreived successfully");
			LOG.info("Operation type= " + operationType);
			LOG.info("ConfigId=" + configId);
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
		LOG.trace("Completed AbortCandidateConfigResource#put()");
		return status;
	}
}
