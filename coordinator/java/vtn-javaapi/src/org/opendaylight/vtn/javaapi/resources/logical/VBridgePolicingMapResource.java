/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.resources.logical;

import java.util.ArrayList;
import java.util.List;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.ipc.ClientSession;
import org.opendaylight.vtn.core.ipc.IpcUint32;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.annotation.UNCField;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.IpcRequestProcessor;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcDataUnitWrapper;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcLogicalResponseFactory;
import org.opendaylight.vtn.javaapi.ipc.enums.IpcRequestPacketEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncOperationEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncOption1Enum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncUPLLEnums;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.validation.logical.VBridgePolicingMapResourceValidator;

/**
 * The Class VBridgePolicingMap implements DELETE/PUT/GET methods.
 * 
 */

@UNCVtnService(path = "/vtns/{vtn_name}/vbridges/{vbr_name}/policingmap")
public class VBridgePolicingMapResource extends AbstractResource {

	/** The Constant LOG. */
	private static final Logger LOG = Logger
			.getLogger(VBridgePolicingMapResource.class.getName());

	/**
	 * Instantiates a new PolicingProfiles Resource.
	 */
	public VBridgePolicingMapResource() {
		super();
		LOG.trace("Start VBridgePolicingMapResource#VBridgePolicingMapResource()");
		setValidator(new VBridgePolicingMapResourceValidator(this));
		LOG.trace("Completed VBridgePolicingMapResource#VBridgePolicingMapResource()");

	}

	/** The vtn_name. */
	@UNCField("vtn_name")
	private String vtnName;

	/** The vbr_name. */
	@UNCField("vbr_name")
	private String vbrName;

	/**
	 * @return the vtnName
	 */
	public String getVtnName() {
		return vtnName;
	}

	/**
	 * @return the vbrName
	 */
	public String getVbrName() {
		return vbrName;
	}

	/**
	 * Implementation of Delete method of VBridgePolicingMap Resource API
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public int delete() throws VtnServiceException {
		LOG.trace("Start VBridgePolicingMapResource#delete()");
		ClientSession session = null;
		IpcRequestProcessor requestProcessor = null;
		int status = ClientSession.RESP_FATAL;
		try {
			LOG.info("Start Ipc framework call");
			session = getConnPool().getSession(
					UncUPLLEnums.UPLL_IPC_CHANNEL_NAME,
					UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
					UncUPLLEnums.ServiceID.UPLL_EDIT_SVC_ID.ordinal(),
					getExceptionHandler());
			LOG.info("Session created successfully");
			requestProcessor = new IpcRequestProcessor(session, getSessionID(),
					getConfigID(), getExceptionHandler());
			requestProcessor.createIpcRequestPacket(
					IpcRequestPacketEnum.KT_VBR_POLICINGMAP_DELETE,
					getNullJsonObject(), getUriParameters());
			LOG.info("Request packet created successfully");
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet processed with status" + status);
			LOG.info("Complete Ipc framework call");
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
		} finally {
			if (status == ClientSession.RESP_FATAL) {
				if (null != requestProcessor.getErrorJson()) {
					setInfo(requestProcessor.getErrorJson());
				} else {
					createErrorInfo(UncCommonEnum.UncResultCode.UNC_SERVER_ERROR
							.getValue());
				}
				status = UncResultCode.UNC_SERVER_ERROR.getValue();
			}
			getConnPool().destroySession(session);
		}
		LOG.trace("Completed VBridgePolicingMapResource#delete()");
		return status;
	}

	/**
	 * Implementation of Put method of VBridge PolicingMap API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public final int put(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VBridgePolicingMapResource#put()");
		// set 0 for create and 1 for update
		int creatOrUpdateOperation = -1;
		try {
			/*
			 * check that policing map is already set or not. if already set
			 * then invoke update operation other wise invoke create operation
			 */
			int getOpStatus = get(new JsonObject());
			if (getOpStatus == UncResultCode.UNC_SUCCESS.getValue()) {
				LOG.info("Instance exists, Update Operation required.");
				creatOrUpdateOperation = 1;
			}
		} catch (final VtnServiceException e) {
			if (getInfo() != null
					&& Integer.parseInt(getInfo()
							.get(VtnServiceJsonConsts.ERROR).getAsJsonObject()
							.get(VtnServiceJsonConsts.CODE).getAsString()) == (UncResultCode.UNC_NOT_FOUND
							.getValue())) {
				LOG.info("Instance does not exist, Create Operation required.");
				creatOrUpdateOperation = 0;
			} else {
				LOG.error("operation failure while check instance existence");
				throw e;
			}
		}

		ClientSession session = null;
		IpcRequestProcessor requestProcessor = null;
		int status = ClientSession.RESP_FATAL;
		try {
			setInfo(null);
			LOG.info("Start Ipc framework call for Set vBridge  Policing Map");
			session = getConnPool().getSession(
					UncUPLLEnums.UPLL_IPC_CHANNEL_NAME,
					UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
					UncUPLLEnums.ServiceID.UPLL_EDIT_SVC_ID.ordinal(),
					getExceptionHandler());
			LOG.info("Session created successfully");
			requestProcessor = new IpcRequestProcessor(session, getSessionID(),
					getConfigID(), getExceptionHandler());
			requestProcessor.createIpcRequestPacket(
					IpcRequestPacketEnum.KT_VBR_POLICINGMAP_SET, requestBody,
					getUriParameters());
			requestProcessor.setServiceInfo(UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
					UncUPLLEnums.ServiceID.UPLL_EDIT_SVC_ID.ordinal());
			// Update operation will be set if creatOrUpdateOperation is 1
			if (creatOrUpdateOperation == 1) {
				requestProcessor.getRequestPacket()
						.setOperation(
								new IpcUint32(UncOperationEnum.UNC_OP_UPDATE
										.ordinal()));
			}
			LOG.info("Request Packet created successfully");
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet processed with status" + status);
			LOG.info("Complete Ipc framework call");
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
		} finally {
			if (status == ClientSession.RESP_FATAL) {
				if (null != requestProcessor.getErrorJson()) {
					setInfo(requestProcessor.getErrorJson());
				} else {
					createErrorInfo(UncCommonEnum.UncResultCode.UNC_SERVER_ERROR
							.getValue());
				}
				status = UncResultCode.UNC_SERVER_ERROR.getValue();
			}
			getConnPool().destroySession(session);
		}
		LOG.trace("Complete VBridgePolicingMapResource#put()");
		return status;
	}

	/**
	 * Implementation of get method of VBridgePolicingMap Resource API
	 * 
	 * @param requestBody
	 *            the request JSON object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public int get(final JsonObject requestBody) throws VtnServiceException {
		LOG.trace("Start VBridgePolicingMapResource#get()");
		ClientSession session = null;
		IpcRequestProcessor requestProcessor = null;
		int status = ClientSession.RESP_FATAL;
		try {
			LOG.info("Start Ipc framework call for VBridgePolicingMapResource");
			session = getConnPool().getSession(
					UncUPLLEnums.UPLL_IPC_CHANNEL_NAME,
					UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
					UncUPLLEnums.ServiceID.UPLL_READ_SVC_ID.ordinal(),
					getExceptionHandler());
			LOG.info("Session created successfully");
			requestProcessor = new IpcRequestProcessor(session, getSessionID(),
					getConfigID(), getExceptionHandler());
			requestProcessor.createIpcRequestPacket(
					IpcRequestPacketEnum.KT_VBR_POLICINGMAP_GET, requestBody,
					getUriParameters());

			if (!requestBody.has(VtnServiceJsonConsts.OP)) {
				requestBody.addProperty(VtnServiceJsonConsts.OP,
						VtnServiceJsonConsts.NORMAL);
			}

			if (!requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
				requestBody.addProperty(VtnServiceJsonConsts.TARGETDB,
						VtnServiceJsonConsts.STATE);
			}

			/*
			 * set option1 as "UNC_OPT1_DETAIL" only when datatype is "state"
			 * and op is "detail"
			 */
			if (requestBody.get(VtnServiceJsonConsts.TARGETDB).getAsString()
					.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
				LOG.info("Request invoked for state db, option1 should be set as detail.");
				requestProcessor.getRequestPacket()
						.setOption1(
								new IpcUint32(UncOption1Enum.UNC_OPT1_DETAIL
										.ordinal()));
			}

			requestProcessor.getRequestPacket().setOperation(
					IpcDataUnitWrapper
							.setIpcUint32Value((UncOperationEnum.UNC_OP_READ
									.ordinal())));

			LOG.info("Request packet created successfully");
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet processed with status: " + status);
			LOG.info("Complete Ipc framework call");
			final IpcLogicalResponseFactory responseGenerator = new IpcLogicalResponseFactory();
			setInfo(responseGenerator.getPolicingMapResponse(
					requestProcessor.getIpcResponsePacket(), requestBody,
					VtnServiceJsonConsts.SHOW));
			LOG.info("Response JSON object created successfully");
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
		} finally {
			if (status == ClientSession.RESP_FATAL) {
				if (null != requestProcessor.getErrorJson()) {
					setInfo(requestProcessor.getErrorJson());
				} else {
					createErrorInfo(UncCommonEnum.UncResultCode.UNC_SERVER_ERROR
							.getValue());
				}
				status = UncResultCode.UNC_SERVER_ERROR.getValue();
			}
			getConnPool().destroySession(session);
		}
		LOG.trace("Completed VBridgePolicingMapResource#get()");
		return status;
	}

	/**
	 * Add URI parameters to list
	 * 
	 * @return
	 */
	private List<String> getUriParameters() {
		LOG.trace("Start VBridgePolicingMapResource#getUriParameters()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(vtnName);
		uriParameters.add(vbrName);
		LOG.trace("Completed VBridgePolicingMapResource#getUriParameters()");
		return uriParameters;
	}

}
