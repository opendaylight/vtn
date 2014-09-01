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
import org.opendaylight.vtn.core.ipc.IpcStruct;
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
import org.opendaylight.vtn.javaapi.ipc.enums.UncStructEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncUPLLEnums;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.validation.logical.VtnPolicingMapResourceValidator;

/**
 * Class provides implementation for GET/PUT/DELETE method of policing-map under
 * VTN
 */
@UNCVtnService(path = "/vtns/{vtn_name}/policingmap")
public class VtnPolicingMapResource extends AbstractResource {

	/** The Constant LOG. */
	private static final Logger LOG = Logger
			.getLogger(VtnPolicingMapResource.class.getName());

	/** The vtn name. */
	@UNCField("vtn_name")
	private String vtnName;

	/**
	 * @return the vtnName
	 */
	public final String getVtnName() {
		return vtnName;
	}

	/**
	 * Instantiates a new VTN PolicingMap resource.
	 */
	public VtnPolicingMapResource() {
		LOG.trace("Start VtnPolicingMapResource#VtnPolicingMap()");
		setValidator(new VtnPolicingMapResourceValidator(this));
		LOG.trace("Completed VtnPolicingMapResource#VtnPolicingMap()");
	}

	/**
	 * Implementation of Delete method of Policing Map API
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public final int delete() throws VtnServiceException {
		LOG.trace("Start VtnPolicingMapResource#delete()");
		ClientSession session = null;
		IpcRequestProcessor requestProcessor = null;
		int status = ClientSession.RESP_FATAL;
		try {
			LOG.info("Start Ipc framework call for Delete VTN Policing Map");
			session = getConnPool().getSession(
					UncUPLLEnums.UPLL_IPC_CHANNEL_NAME,
					UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
					UncUPLLEnums.ServiceID.UPLL_EDIT_SVC_ID.ordinal(),
					getExceptionHandler());
			LOG.info("Session created successfully");
			requestProcessor = new IpcRequestProcessor(session, getSessionID(),
					getConfigID(), getExceptionHandler());
			requestProcessor.createIpcRequestPacket(
					IpcRequestPacketEnum.KT_VTN_POLICINGMAP_DELETE,
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
		LOG.trace("Complete VtnPolicingMapResource#delete()");
		return status;
	}

	/**
	 * Implementation of get method of Policing Map API
	 * 
	 * @param requestBody
	 *            the request JSON object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public final int get(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VtnPolicingMapResource#get()");
		ClientSession session = null;
		IpcRequestProcessor requestProcessor = null;
		int status = ClientSession.RESP_FATAL;
		try {
			LOG.info("Start Ipc framework call for Get VTN Policing Map");
			session = getConnPool().getSession(
					UncUPLLEnums.UPLL_IPC_CHANNEL_NAME,
					UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
					UncUPLLEnums.ServiceID.UPLL_READ_SVC_ID.ordinal(),
					getExceptionHandler());
			LOG.info("Session created successfully");
			requestProcessor = new IpcRequestProcessor(session, getSessionID(),
					getConfigID(), getExceptionHandler());

			// set default value of targetdb
			if (!requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
				requestBody.addProperty(VtnServiceJsonConsts.TARGETDB,
						VtnServiceJsonConsts.STATE);
			}

			// set default value of op
			if (!requestBody.has(VtnServiceJsonConsts.OP)) {
				requestBody.addProperty(VtnServiceJsonConsts.OP,
						VtnServiceJsonConsts.NORMAL);
			}

			/*
			 * create request-body as per op and targetdb parameters
			 */
			if (!requestBody.get(VtnServiceJsonConsts.TARGETDB).getAsString()
					.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
				LOG.info("request-packet creation for : KT_VTN_POLICINGMAP");
				requestProcessor.createIpcRequestPacket(
						IpcRequestPacketEnum.KT_VTN_POLICINGMAP_GET,
						requestBody, getUriParameters());
			} else {
				if (requestBody.has(VtnServiceJsonConsts.CONTROLLERID)
						&& requestBody.has(VtnServiceJsonConsts.DOMAINID)) {
					LOG.info("request-packet creation for : KT_VTN_POLICINGMAP_CONTROLLER");
					requestProcessor
							.createIpcRequestPacket(
									IpcRequestPacketEnum.KT_VTN_POLICINGMAP_CONTROLLER_GET,
									requestBody, getUriParameters());
					requestProcessor.getRequestPacket().setValStruct(
							new IpcStruct(
									UncStructEnum.ValPolicingMapController
											.getValue()));
				} else {
					LOG.info("request-packet creation for : KT_VTN_POLICINGMAP");
					requestProcessor.createIpcRequestPacket(
							IpcRequestPacketEnum.KT_VTN_POLICINGMAP_GET,
							requestBody, getUriParameters());
				}
			}

			// update operation as READ
			requestProcessor.getRequestPacket().setOperation(
					IpcDataUnitWrapper
							.setIpcUint32Value((UncOperationEnum.UNC_OP_READ
									.ordinal())));

			LOG.info("Request packet created successfully");
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet processed with status" + status);
			final IpcLogicalResponseFactory responseGenerator = new IpcLogicalResponseFactory();
			setInfo(responseGenerator.getVtnPolicingMapResponse(
					requestProcessor.getIpcResponsePacket(), requestBody,
					VtnServiceJsonConsts.SHOW));
			LOG.info("Response object created successfully");
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
		LOG.trace("Complete VtnPolicingMapResource#get()");
		return status;
	}

	/**
	 * Implementation of Put method of Policing Map API
	 * 
	 * @param requestBody
	 *            the request JSON object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public final int put(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VtnPolicingMapResource#put()");
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
				LOG.error(e, "operation failure while check instance existence");
				throw e;
			}
		}

		ClientSession session = null;
		IpcRequestProcessor requestProcessor = null;
		int status = ClientSession.RESP_FATAL;
		try {
			setInfo(null);
			LOG.info("Start Ipc framework call for Set VTN Policing Map");
			session = getConnPool().getSession(
					UncUPLLEnums.UPLL_IPC_CHANNEL_NAME,
					UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
					UncUPLLEnums.ServiceID.UPLL_EDIT_SVC_ID.ordinal(),
					getExceptionHandler());
			LOG.info("Session created successfully");
			requestProcessor = new IpcRequestProcessor(session, getSessionID(),
					getConfigID(), getExceptionHandler());
			requestProcessor.createIpcRequestPacket(
					IpcRequestPacketEnum.KT_VTN_POLICINGMAP_SET, requestBody,
					getUriParameters());
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
		LOG.trace("Complete VtnPolicingMapResource#put()");
		return status;
	}

	/**
	 * Add URI parameters to list
	 * 
	 * @return
	 */
	private List<String> getUriParameters() {
		LOG.trace("Start VtnPolicingMapResource#getUriParameters()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(vtnName);
		LOG.trace("Completed VtnPolicingMapResource#getUriParameters()");
		return uriParameters;
	}
}
