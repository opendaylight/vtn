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
import org.opendaylight.vtn.core.ipc.IpcUint32;
import org.opendaylight.vtn.core.ipc.IpcUint8;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcDataUnitWrapper;
import org.opendaylight.vtn.javaapi.ipc.enums.*;
import org.opendaylight.vtn.javaapi.validation.DifferenceConfigResourceValidator;

/**
 * The Class DifferenceConfigResource implements the get method of Difference
 * Configuration API
 */

@UNCVtnService(path = "/configuration/diff")
public class DifferenceConfigResource extends AbstractResource {

	private static final Logger LOG = Logger
			.getLogger(DifferenceConfigResource.class.getName());

	private boolean dirty = true;

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
	 * @param queryString
	 *            the request Json object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	@Override
	public final int get(final JsonObject queryString) throws VtnServiceException {
		LOG.trace("Starts DifferenceConfigResource#get()");
		int status = ClientSession.RESP_FATAL;

		final String modeType = queryString.getAsJsonPrimitive(
				VtnServiceJsonConsts.MODE_TYPE).getAsString();

		if (VtnServiceJsonConsts.REAL_MODE.equals(modeType)) {
			status = checkUppl();
		} else if (VtnServiceJsonConsts.VIRTUAL_MODE.equals(modeType) ||
				   VtnServiceJsonConsts.VTN_MODE.equals(modeType)) {
			status = checkUpll();
		} else {
			status = checkUpll();
			if (!dirty) {
				status = checkUppl();
			}
		}

		LOG.trace("Completed DifferenceConfigResource#get()");
		return status;
	}


	/**
	 * check configuration of UPLL whether it is dirty.
	 * 
	 * @return int
	 * @throws VtnServiceException
	 */
	private int checkUpll() throws VtnServiceException {
		ClientSession session = null;
		int status = ClientSession.RESP_FATAL;
		LOG.debug("Start Ipc framework call");

		try {
			session = getConnPool().getSession(
					UncUPLLEnums.UPLL_IPC_CHANNEL_NAME,
					UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
					UncUPLLEnums.ServiceID.UPLL_GLOBAL_CONFIG_SVC_ID.ordinal(),
					getExceptionHandler());
			LOG.debug("Session created successfully");
			// set session timeout as infinity for config diff operation
			session.setTimeout(null);
			session.addOutput(IpcDataUnitWrapper.setIpcUint32Value(
					UncUPLLEnums.UpllGlobalConfigOpT.UPLL_IS_CANDIDATE_DIRTY_OP
							.getValue()));
			session.addOutput(IpcDataUnitWrapper
					.setIpcUint32Value(getSessionID()));
			session.addOutput(IpcDataUnitWrapper
					.setIpcUint32Value(getConfigID()));
			LOG.info("Request packet created successfully");
			long start = System.currentTimeMillis();
			status = session.invoke();
			LOG.debug("The treatment of under layer cost the following time: "
					+ (System.currentTimeMillis() - start) + "(ms)");
			LOG.info("Request packet processed with status:" + status);

			long operationType = ((IpcUint32) session
					.getResponse(VtnServiceJsonConsts.VAL_0)).longValue();
			long resultCode = ((IpcUint32) session
					.getResponse(VtnServiceJsonConsts.VAL_1)).longValue();
			int dirtyStatus = ((IpcUint8) session
					.getResponse(VtnServiceJsonConsts.VAL_2)).intValue();

			LOG.debug("Response retreived successfully");
			LOG.debug("Operation type: " + operationType);
			LOG.debug("Result Code: " + resultCode);
			LOG.debug("DirtyStatus: " + dirtyStatus);

			if (resultCode != UncIpcErrorCode.RC_SUCCESS) {
				UncErrorBean uncErrorBean = UncIpcErrorCode.getLogicalError((int) resultCode);
				createErrorInfo(Integer.parseInt(uncErrorBean.getErrorCode()),
						uncErrorBean.getJavaAPIErrorMessage());
				status = UncCommonEnum.UncResultCode.UNC_SERVER_ERROR
						.getValue();
			}  else {
				createResponse(Integer.toString(dirtyStatus));
				LOG.debug("Request processed successfully");
				status = UncCommonEnum.UncResultCode.UNC_SUCCESS
						.getValue();
			}
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

		return status;
	}

	/**
	 * check configuration of UPPL whether it is dirty.
	 * 
	 * @return int
	 * @throws VtnServiceException
	 */
	private int checkUppl() throws VtnServiceException {
		ClientSession session = null;
		int status = ClientSession.RESP_FATAL;
		LOG.debug("Start Ipc framework call");

		try {
			session = getConnPool().getSession(
					UncUPPLEnums.UPPL_IPC_CHN_NAME,
					UncUPPLEnums.UPPL_IPC_SVC_NAME,
					UncUPPLEnums.ServiceID.UPPL_SVC_GLOBAL_CONFIG.ordinal(),
					getExceptionHandler());
			LOG.debug("Session created successfully");
			// set session timeout as infinity for config diff operation
			session.setTimeout(null);
			session.addOutput(IpcDataUnitWrapper.setIpcUint32Value(
					UncUPPLEnums.UncAddlOperationT.UNC_OP_IS_CANDIDATE_DIRTY
									.ordinal()));
			session.addOutput(IpcDataUnitWrapper
					.setIpcUint32Value(getSessionID()));
			session.addOutput(IpcDataUnitWrapper
					.setIpcUint32Value(getConfigID()));
			LOG.info("Request packet created successfully");
			long start = System.currentTimeMillis();
			status = session.invoke();
			LOG.debug("The treatment of under layer cost the following time: "
					+ (System.currentTimeMillis() - start) + "(ms)");
			LOG.info("Request packet processed with status:" + status);

			long operationType = ((IpcUint32) session
					.getResponse(VtnServiceJsonConsts.VAL_0)).longValue();
			long resultCode = ((IpcUint32) session
					.getResponse(VtnServiceJsonConsts.VAL_1)).longValue();
			int dirtyStatus = ((IpcUint8) session
					.getResponse(VtnServiceJsonConsts.VAL_2)).intValue();

			LOG.debug("Response retreived successfully");
			LOG.debug("Operation type: " + operationType);
			LOG.debug("Result Code: " + resultCode);
			LOG.debug("DirtyStatus: " + dirtyStatus);

			if (resultCode != UncIpcErrorCode.RC_SUCCESS) {
				UncErrorBean uncErrorBean = UncIpcErrorCode.getPhysicalError((int) resultCode);
				createErrorInfo(Integer.parseInt(uncErrorBean.getErrorCode()),
						uncErrorBean.getJavaAPIErrorMessage());
				status = UncCommonEnum.UncResultCode.UNC_SERVER_ERROR
						.getValue();
			}  else {
				createResponse(Integer.toString(dirtyStatus));
				LOG.debug("Request processed successfully");
				status = UncCommonEnum.UncResultCode.UNC_SUCCESS
						.getValue();
			}
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
		} else {
			dirtyJson.addProperty(VtnServiceJsonConsts.DIFF_STATUS,
					VtnServiceJsonConsts.FALSE);
			dirty = false;
		}
		response.add(VtnServiceJsonConsts.DIFF, dirtyJson);
		setInfo(response);
		LOG.trace("Completed DifferenceConfigResource#createResponse()");
	}
}
