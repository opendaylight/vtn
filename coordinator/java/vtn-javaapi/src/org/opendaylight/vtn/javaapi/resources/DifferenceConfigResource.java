/*
 * Copyright (c) 2012-2014 NEC Corporation
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
import org.opendaylight.vtn.javaapi.ipc.enums.UncStructIndexEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncUPLLEnums;
import org.opendaylight.vtn.javaapi.ipc.enums.UncUPPLEnums;
import org.opendaylight.vtn.javaapi.validation.DifferenceConfigResourceValidator;

/**
 * The Class DifferenceConfigResource implements the put method of Difference
 * Configuration API
 */

@UNCVtnService(path = "/configuration/diff")
public class DifferenceConfigResource extends AbstractResource {

	private static final Logger LOG = Logger
			.getLogger(DifferenceConfigResource.class.getName());

	/**
	 * Instantiates a new difference config resource.
	 */
	public DifferenceConfigResource() {
		super();
		LOG.trace("Start DifferenceConfigResource#DifferenceConfigResource()");
		setValidator(new DifferenceConfigResourceValidator(this));
		LOG.trace("Complete DifferenceConfigResource#DifferenceConfigResource()");
	}

	/**
	 * Implementation of Put method of Difference Configuration API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	@Override
	public final int get() throws VtnServiceException {
		LOG.trace("Starts DifferenceConfigResource#get()");
		ClientSession session = null;
		ClientSession sessionUppl = null;
		int status = ClientSession.RESP_FATAL;
		try {
			LOG.debug("Start Ipc framework call");
			session = getConnPool().getSession(
					UncUPLLEnums.UPLL_IPC_CHANNEL_NAME,
					UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
					UncUPLLEnums.ServiceID.UPLL_GLOBAL_CONFIG_SVC_ID.ordinal(),
					getExceptionHandler());
			LOG.debug("Session created successfully");
			// set session timeout as infinity for config diff operation
			session.setTimeout(null);
			session.addOutput(IpcDataUnitWrapper
					.setIpcUint32Value(UncUPLLEnums.UpllGlobalConfigOpT.UPLL_IS_CANDIDATE_DIRTY_OP
							.getValue()));
			LOG.info("Request packet created successfully");
			status = session.invoke();
			LOG.info("Request packet processed with status:" + status);
			String operationType = IpcDataUnitWrapper
					.getIpcDataUnitValue(session
							.getResponse(VtnServiceJsonConsts.VAL_0));
			int result = Integer.parseInt(IpcDataUnitWrapper
					.getIpcDataUnitValue(session
							.getResponse(VtnServiceJsonConsts.VAL_1)));
			String dirtyStatus = IpcDataUnitWrapper.getIpcDataUnitValue(session
					.getResponse(VtnServiceJsonConsts.VAL_2));
			LOG.debug("Response retreived successfully");
			LOG.debug("Operation type: " + operationType);
			LOG.debug("Result Code: " + result);
			LOG.debug("DirtyStatus: " + dirtyStatus);
			if (result != UncIpcErrorCode.RC_SUCCESS) {
				createTcErrorInfo(UncIpcErrorCode.getTcCodes(result));
				status = UncCommonEnum.UncResultCode.UNC_SERVER_ERROR
						.getValue();
			} else {
				if (UncStructIndexEnum.DirtyStatus.FALSE.getValue().equals(
						dirtyStatus)) {
					sessionUppl = getConnPool().getSession(
							UncUPPLEnums.UPPL_IPC_CHN_NAME,
							UncUPPLEnums.UPPL_IPC_SVC_NAME,
							UncUPPLEnums.ServiceID.UPPL_SVC_GLOBAL_CONFIG
									.ordinal(), getExceptionHandler());
					// set session timeout as infinity for config diff operation
					sessionUppl.setTimeout(null);
					sessionUppl.addOutput(IpcDataUnitWrapper
							.setIpcUint32Value(UncUPPLEnums.UncAddlOperationT.UNC_OP_IS_CANDIDATE_DIRTY
									.ordinal()));
					LOG.debug("Request packet created successfully");
					status = sessionUppl.invoke();
					LOG.debug("Request packet processed with status:" + status);
					operationType = IpcDataUnitWrapper
							.getIpcDataUnitValue(sessionUppl
									.getResponse(VtnServiceJsonConsts.VAL_0));
					result = Integer.parseInt(IpcDataUnitWrapper
							.getIpcDataUnitValue(sessionUppl
									.getResponse(VtnServiceJsonConsts.VAL_1)));
					dirtyStatus = IpcDataUnitWrapper
							.getIpcDataUnitValue(sessionUppl
									.getResponse(VtnServiceJsonConsts.VAL_2));
					LOG.debug("Response retreived successfully");
					LOG.debug("Operation type: " + operationType);
					LOG.debug("Result Code: " + result);
					LOG.debug("DirtyStatus: " + dirtyStatus);
					if (result != UncIpcErrorCode.RC_SUCCESS) {
						createTcErrorInfo(UncIpcErrorCode.getTcCodes(result));
						status = UncCommonEnum.UncResultCode.UNC_SERVER_ERROR
								.getValue();
					} else {
						createResponse(dirtyStatus);
						LOG.debug("Request processed successfully");
						status = UncCommonEnum.UncResultCode.UNC_SUCCESS
								.getValue();
					}
				} else {
					createResponse(dirtyStatus);
					LOG.debug("Request processed successfully");
					status = UncCommonEnum.UncResultCode.UNC_SUCCESS.getValue();
				}
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
			getConnPool().destroySession(sessionUppl);
		}
		LOG.trace("Completed DifferenceConfigResource#get()");
		return status;
	}

	/**
	 * Creates Json response.
	 * 
	 * @param dirtyStatus
	 *            the dirty status
	 */
	private void createResponse(final String dirtyStatus) {
		LOG.trace("Starts DifferenceConfigResource#createResponse()");
		final JsonObject response = new JsonObject();
		final JsonObject dirtyJson = new JsonObject();
		if (UncStructIndexEnum.DirtyStatus.TRUE.getValue().equals(dirtyStatus)) {
			dirtyJson.addProperty(VtnServiceJsonConsts.DIFF_STATUS,
					VtnServiceJsonConsts.TRUE);
		} else if (UncStructIndexEnum.DirtyStatus.FALSE.getValue().equals(
				dirtyStatus)) {
			dirtyJson.addProperty(VtnServiceJsonConsts.DIFF_STATUS,
					VtnServiceJsonConsts.FALSE);
		} else {
			LOG.debug("dirtyStatus: invalid");
		}
		response.add(VtnServiceJsonConsts.DIFF, dirtyJson);
		setInfo(response);
		LOG.trace("Completed DifferenceConfigResource#createResponse()");
	}
}
