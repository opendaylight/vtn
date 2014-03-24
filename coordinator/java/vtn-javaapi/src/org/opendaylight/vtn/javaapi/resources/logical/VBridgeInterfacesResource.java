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
import java.util.Iterator;
import java.util.List;

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.ipc.ClientSession;
import org.opendaylight.vtn.core.ipc.IpcDataUnit;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.annotation.UNCField;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceIpcConsts;
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
import org.opendaylight.vtn.javaapi.ipc.enums.UncOption2Enum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncUPLLEnums;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.validation.logical.InterfaceResourceValidator;

/**
 * The Class VBridgeInterfacesResource.
 * 
 * @author NHST
 * @version 1.0
 */
/* This class handles post and get methods */
@UNCVtnService(path = "/vtns/{vtn_name}/vbridges/{vbr_name}/interfaces")
public class VBridgeInterfacesResource extends AbstractResource {
	/** The vtn name. */
	@UNCField("vtn_name")
	private String vtnName;
	/** The vbr name. */
	@UNCField("vbr_name")
	private String vbrName;

	/**
	 * Gets the vtn name.
	 * 
	 * @return the vtn name
	 */
	public final String getVtnName() {
		return vtnName;
	}

	/**
	 * Gets the vbr name.
	 * 
	 * @return the vbr name
	 */
	public final String getVbrName() {
		return vbrName;
	}

	/** The LOG. */
	private static final Logger LOG = Logger
			.getLogger(VBridgeInterfacesResource.class.getName());

	/**
	 * Instantiates a new v bridge interfaces resource.
	 */
	public VBridgeInterfacesResource() {
		super();
		LOG.trace("Start VBridgeInterfacesResource#VBridgeInterfacesResource()");
		setValidator(new InterfaceResourceValidator(this));
		LOG.trace("Complete VBridgeInterfacesResource#VBridgeInterfacesResource()");
	}

	/**
	 * Implementation of Post method of VBridgeInterface API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public final int post(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VBridgeInterfacesResource#post()");
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
			final List<String> uriParameterList = getUriParameters(requestBody);
			requestProcessor.createIpcRequestPacket(
					IpcRequestPacketEnum.KT_VBR_IF_CREATE, requestBody,
					uriParameterList);
			LOG.debug("Request Packet created successfully");
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
		LOG.trace("Complete VBridgeInterfacesResourc#post()");
		return status;
	}

	/**
	 * Implementation of Get method of VBridgeInterface API
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
		LOG.trace("Start VBridgeInterfacesResource#get()");
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
			final List<String> uriParameterList = getUriParameters(requestBody);
			requestProcessor.createIpcRequestPacket(
					IpcRequestPacketEnum.KT_VBR_IF_GET, requestBody,
					uriParameterList);
			LOG.debug("Request Packet created successfully for 1st call");
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
			IpcDataUnit[] responsePacket = null;
			JsonObject neighbor = null;
			JsonObject vbrInterfaceJson = null;
			final IpcLogicalResponseFactory responseGenerator = new IpcLogicalResponseFactory();
			String dataType = VtnServiceJsonConsts.STATE;
			if (requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
				dataType = requestBody.get(VtnServiceJsonConsts.TARGETDB)
						.getAsString();
			}
			String opType = VtnServiceJsonConsts.NORMAL;
			if (requestBody.has(VtnServiceJsonConsts.OP)) {
				opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
			}
			JsonObject interfaceJson = null;
			final JsonArray interfaceArray = new JsonArray();
			responsePacket = requestProcessor.getIpcResponsePacket();
			vbrInterfaceJson = responseGenerator.getVBridgeInterfaceResponse(
					responsePacket, requestBody, VtnServiceJsonConsts.LIST);
			if (vbrInterfaceJson.get(VtnServiceJsonConsts.INTERFACES)
					.isJsonArray()) {
				final JsonArray responseArray = vbrInterfaceJson.get(
						VtnServiceJsonConsts.INTERFACES).getAsJsonArray();
				vbrInterfaceJson = getResponseJsonArrayLogical(requestBody,
						requestProcessor, responseGenerator, responseArray,
						VtnServiceJsonConsts.INTERFACES,
						VtnServiceJsonConsts.IFNAME,
						IpcRequestPacketEnum.KT_VBR_IF_GET, uriParameterList,
						VtnServiceIpcConsts.GET_VBRIDGE_INTERFACE_RESPONSE);
			}
			LOG.debug("Response object created successfully for 1st request");
			if ((VtnServiceJsonConsts.STATE).equalsIgnoreCase(dataType)
					&& opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
				final Iterator<JsonElement> interfaceIterator = vbrInterfaceJson
						.get(VtnServiceJsonConsts.INTERFACES).getAsJsonArray()
						.iterator();
				requestProcessor.setServiceInfo(
						UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
						UncUPLLEnums.ServiceID.UPLL_READ_SVC_ID.ordinal());
				while (interfaceIterator.hasNext()) {
					interfaceJson = interfaceIterator.next().getAsJsonObject();
					final String ifName = interfaceJson.get(
							VtnServiceJsonConsts.IFNAME).getAsString();
					requestBody.addProperty(VtnServiceJsonConsts.INDEX, ifName);
					requestProcessor.createIpcRequestPacket(
							IpcRequestPacketEnum.KT_VBR_IF_GET, requestBody,
							getUriParameters(requestBody));
					requestProcessor
							.getRequestPacket()
							.setOption2(
									IpcDataUnitWrapper
											.setIpcUint32Value((UncOption2Enum.UNC_OPT2_NEIGHBOR
													.ordinal())));
					requestProcessor
							.getRequestPacket()
							.setOperation(
									IpcDataUnitWrapper
											.setIpcUint32Value(UncOperationEnum.UNC_OP_READ
													.ordinal()));
					LOG.debug("Request packet created successfully for 2nd call");
					status = requestProcessor.processIpcRequest();
					LOG.debug("Request packet processed for 2nd call with status"
							+ status);
					responsePacket = requestProcessor.getIpcResponsePacket();
					neighbor = responseGenerator.getNeighborResponse(
							responsePacket, requestBody,
							VtnServiceJsonConsts.SHOW);
					interfaceJson.add(VtnServiceJsonConsts.NEIGHBOR, neighbor);
					interfaceArray.add(interfaceJson);
				}
				vbrInterfaceJson.add(VtnServiceJsonConsts.INTERFACES,
						interfaceArray);
			}
			LOG.debug("Response object created successfully for 2nd request");
			setInfo(vbrInterfaceJson);
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
		LOG.trace("Complete VBridgeInterfacesResource#get()");
		return status;
	}

	/**
	 * Add URI parameters to list
	 * 
	 * @return parameter list
	 */
	private List<String> getUriParameters(final JsonObject requestBody) {
		LOG.trace("Start VBridgeInterfacesResource#getUriParameters()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(vtnName);
		uriParameters.add(vbrName);
		if (requestBody != null && requestBody.has(VtnServiceJsonConsts.INDEX)) {
			uriParameters.add(requestBody.get(VtnServiceJsonConsts.INDEX)
					.getAsString());
		}
		LOG.trace("Completed VBridgeInterfacesResource#getUriParameters()");
		return uriParameters;
	}
}
