/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.resources.logical;

import java.util.ArrayList;
import java.util.List;

import com.google.gson.JsonNull;
import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.ipc.ClientSession;
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
import org.opendaylight.vtn.javaapi.ipc.enums.UncOption2Enum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncUPLLEnums;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.validation.logical.InterfaceResourceValidator;

/**
 * The Class VBypassInterfaceResource.
 */
/* This class handles put, delete and get methods */
@UNCVtnService(
		path = "/vtns/{vtn_name}/vbypasses/{vbypass_name}/interfaces/{if_name}")
public class VBypassInterfaceResource extends AbstractResource {
	/** The vtn name. */
	@UNCField("vtn_name")
	private String vtnName;
	/** The vbypass name. */
	@UNCField("vbypass_name")
	private String vbypassName;
	/** The interface name. */
	@UNCField("if_name")
	private String ifName;

	/**
	 * Gets the vtn name.
	 * 
	 * @return the vtn name
	 */
	public final String getVtnName() {
		return vtnName;
	}

	/**
	 * Gets the vbypass name.
	 * 
	 * @return the vbypass name
	 */
	public final String getVbypassName() {
		return vbypassName;
	}

	/**
	 * Gets the if name.
	 * 
	 * @return the if name
	 */
	public final String getIfName() {
		return ifName;
	}

	private static final Logger LOG = Logger
			.getLogger(VBypassInterfaceResource.class.getName());

	/**
	 * Instantiates a new VBypass interface resource.
	 */
	public VBypassInterfaceResource() {
		super();
		LOG.trace("Start VBypassInterfaceResource#VBypassInterfacesResource()");
		setValidator(new InterfaceResourceValidator(this));
		LOG.trace("Completed VBypassInterfaceResource#VBypassInterfacesResource()");
	}

	/**
	 * Implementation of Put method of VBypass Interface API
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
		LOG.trace("Starts VBypassInterfaceResource#put()");
		ClientSession session = null;
		IpcRequestProcessor requestProcessor = null;
		int status = ClientSession.RESP_FATAL;
		try {
			LOG.debug("Start Ipc framework call");
			session = getConnPool().getSession(
					UncUPLLEnums.UPLL_IPC_CHANNEL_NAME,
					UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
					UncUPLLEnums.ServiceID.UPLL_EDIT_SVC_ID.ordinal(),
					getExceptionHandler());
			LOG.debug("Session created successfully");
			requestProcessor = new IpcRequestProcessor(session, getSessionID(),
					getConfigID(), getExceptionHandler());
			requestProcessor.createIpcRequestPacket(
					IpcRequestPacketEnum.KT_VUNK_IF_UPDATE, requestBody,
					getUriParameters());
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet processed with status" + status);
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
		LOG.trace("Completed VBypassInterfaceResource#put()");
		return status;
	}

	/**
	 * Implementation of Delete method of VBypass Interface API
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public final int delete() throws VtnServiceException {
		LOG.trace("starts VBypassInterfaceResource#delete()");
		ClientSession session = null;
		IpcRequestProcessor requestProcessor = null;
		int status = ClientSession.RESP_FATAL;
		try {
			LOG.debug("Start Ipc framework call");
			session = getConnPool().getSession(
					UncUPLLEnums.UPLL_IPC_CHANNEL_NAME,
					UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
					UncUPLLEnums.ServiceID.UPLL_EDIT_SVC_ID.ordinal(),
					getExceptionHandler());
			LOG.debug("Session created successfully");
			requestProcessor = new IpcRequestProcessor(session, getSessionID(),
					getConfigID(), getExceptionHandler());
			requestProcessor.createIpcRequestPacket(
					IpcRequestPacketEnum.KT_VUNK_IF_DELETE,
					getNullJsonObject(), getUriParameters());
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet processed with status" + status);
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
		LOG.trace("Completed VBypassInterfaceResource#delete()");
		return status;
	}

	/**
	 * Implementation of get method of VBypass Interface API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public final int get(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Starts VBypassInterfaceResource#get()");
		ClientSession session = null;
		IpcRequestProcessor requestProcessor = null;
		int status = ClientSession.RESP_FATAL;
		try {
			LOG.debug("Start Ipc framework call");
			session = getConnPool().getSession(
					UncUPLLEnums.UPLL_IPC_CHANNEL_NAME,
					UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
					UncUPLLEnums.ServiceID.UPLL_READ_SVC_ID.ordinal(),
					getExceptionHandler());
			LOG.debug("Session created successfully");
			requestProcessor = new IpcRequestProcessor(session, getSessionID(),
					getConfigID(), getExceptionHandler());
			requestProcessor.createIpcRequestPacket(
					IpcRequestPacketEnum.KT_VUNK_IF_GET, requestBody,
					getUriParameters());
			LOG.debug("Request packet created successfully for 1st request");
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet processed for 1st call with status"
					+ status);
			if (status == ClientSession.RESP_FATAL) {
				throw new VtnServiceException(
						Thread.currentThread().getStackTrace()[1]
								.getClassName()
								+ VtnServiceConsts.HYPHEN
								+ Thread.currentThread().getStackTrace()[1]
										.getMethodName(),
						UncJavaAPIErrorCode.IPC_SERVER_ERROR.getErrorCode(),
						UncJavaAPIErrorCode.IPC_SERVER_ERROR.getErrorMessage());
			}
			JsonObject neighbor = null;
			JsonObject vbypassInterfaceJson = null;
			final IpcLogicalResponseFactory responseGenerator = new IpcLogicalResponseFactory();
			String dataType = null;
			if (requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
				dataType = requestBody.get(VtnServiceJsonConsts.TARGETDB)
						.getAsString();
			}
			vbypassInterfaceJson = responseGenerator
					.getVBypassInterfaceResponse(
							requestProcessor.getIpcResponsePacket(),
							requestBody, VtnServiceJsonConsts.SHOW);
			if (VtnServiceJsonConsts.STATE.equalsIgnoreCase(dataType)
					&& !(vbypassInterfaceJson
							.get(VtnServiceJsonConsts.INTERFACE) instanceof JsonNull)) {
				requestProcessor.setServiceInfo(
						UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
						UncUPLLEnums.ServiceID.UPLL_READ_SVC_ID.ordinal());
				requestProcessor.createIpcRequestPacket(
						IpcRequestPacketEnum.KT_VUNK_IF_GET, requestBody,
						getUriParameters());
				requestProcessor
						.getRequestPacket()
						.setOption2(
								IpcDataUnitWrapper
										.setIpcUint32Value(UncOption2Enum.UNC_OPT2_NEIGHBOR
												.ordinal()));
				LOG.debug("Request packet created successfully for 2nd request");
				status = requestProcessor.processIpcRequest();
				LOG.debug("Request packet for 2nd request processed with status"
						+ status);
				neighbor = responseGenerator.getNeighborResponse(
						requestProcessor.getIpcResponsePacket(), requestBody,
						VtnServiceJsonConsts.SHOW);
				vbypassInterfaceJson.get(VtnServiceJsonConsts.INTERFACE)
						.getAsJsonObject()
						.add(VtnServiceJsonConsts.NEIGHBOR, neighbor);
			}
			setInfo(vbypassInterfaceJson);
			LOG.debug("Response object created successfully");
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
		LOG.trace("Completed VBypassInterfaceResource#get()");
		return status;
	}

	/**
	 * Add URI parameters to list
	 * 
	 * @return parameter list
	 */
	private List<String> getUriParameters() {
		LOG.trace("Start VBypassInterfaceResource#getUriParameters()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(vtnName);
		uriParameters.add(vbypassName);
		uriParameters.add(ifName);
		LOG.trace("Completed VBypassInterfaceResource#getUriParameters()");
		return uriParameters;
	}
}
