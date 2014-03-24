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
 * The Class VBypassInterfacesResource.
 */
/* This class handles post and get methods */
@UNCVtnService(path = "/vtns/{vtn_name}/vbypasses/{vbypass_name}/interfaces")
public class VBypassInterfacesResource extends AbstractResource {
	/** The vtn name. */
	@UNCField("vtn_name")
	private String vtnName;
	/** The vbypass name. */
	@UNCField("vbypass_name")
	private String vbypassName;

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

	private static final Logger LOG = Logger
			.getLogger(VBypassInterfacesResource.class.getName());

	/**
	 * Instantiates a new vBypass interfaces resource.
	 */
	public VBypassInterfacesResource() {
		super();
		LOG.trace("Start VBypassInterfacesResource#VBypassInterfacesResource()");
		setValidator(new InterfaceResourceValidator(this));
		LOG.trace("Completed VBypassInterfacesResource#VBypassInterfacesResource()");
	}

	/**
	 * Implementation of Post method of VBypass Interface API
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
		LOG.trace("Start VBypassInterfacesResource#post()");
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
					IpcRequestPacketEnum.KT_VUNK_IF_CREATE, requestBody,
					getUriParameters(requestBody));
			LOG.debug("Request packet created successfully");
			status = requestProcessor.processIpcRequest();
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
		LOG.trace("Completed VBypassInterfacesResource#post()");
		return status;
	}

	/**
	 * Implementation of Get method of VBypass Interface API
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
			// Uriparamter list
			final List<String> uriParameterList = getUriParameters(requestBody);
			requestProcessor.createIpcRequestPacket(
					IpcRequestPacketEnum.KT_VUNK_IF_GET, requestBody,
					uriParameterList);
			LOG.debug("Request packet created successfully for 1st request");
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet for 1st request processed with status"
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
			JsonObject vbypassInterfacesJson = null;
			final IpcLogicalResponseFactory responseGenerator = new IpcLogicalResponseFactory();
			String dataType = null;
			if (requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
				dataType = requestBody.get(VtnServiceJsonConsts.TARGETDB)
						.getAsString();
			}
			String opType = VtnServiceJsonConsts.NORMAL;
			if (requestBody.has(VtnServiceJsonConsts.OP)) {
				opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
			}
			JsonObject interfacesJson = null;
			final JsonArray interfaceArray = new JsonArray();
			/*
			 * vbypassInterfacesJson =
			 * responseGenerator.getVBypassInterfaceResponse(
			 * requestProcessor.getIpcResponsePacket(), requestBody,
			 * VtnServiceJsonConsts.LIST);
			 */
			vbypassInterfacesJson = responseGenerator
					.getVBypassInterfaceResponse(
							requestProcessor.getIpcResponsePacket(),
							requestBody, VtnServiceJsonConsts.LIST);
			if (vbypassInterfacesJson.get(VtnServiceJsonConsts.INTERFACES)
					.isJsonArray()) {
				final JsonArray responseArray = vbypassInterfacesJson.get(
						VtnServiceJsonConsts.INTERFACES).getAsJsonArray();

				vbypassInterfacesJson = getResponseJsonArrayLogical(
						requestBody, requestProcessor, responseGenerator,
						responseArray, VtnServiceJsonConsts.INTERFACES,
						VtnServiceJsonConsts.IFNAME,
						IpcRequestPacketEnum.KT_VUNK_IF_GET, uriParameterList,
						VtnServiceIpcConsts.GET_VBYPASS_INTERFACE_RESPONSE);
			}
			if (VtnServiceJsonConsts.STATE.equalsIgnoreCase(dataType)
					&& opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
				final Iterator<JsonElement> interfaceIterator = vbypassInterfacesJson
						.get(VtnServiceJsonConsts.INTERFACES).getAsJsonArray()
						.iterator();
				requestProcessor.setServiceInfo(
						UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
						UncUPLLEnums.ServiceID.UPLL_READ_SVC_ID.ordinal());
				while (interfaceIterator.hasNext()) {
					interfacesJson = interfaceIterator.next().getAsJsonObject();
					final String ifName = interfacesJson.get(
							VtnServiceJsonConsts.IFNAME).getAsString();
					requestBody.addProperty(VtnServiceJsonConsts.INDEX, ifName);
					requestProcessor.createIpcRequestPacket(
							IpcRequestPacketEnum.KT_VUNK_IF_GET, requestBody,
							getUriParameters(requestBody));
					requestProcessor
							.getRequestPacket()
							.setOption2(
									IpcDataUnitWrapper
											.setIpcUint32Value(UncOption2Enum.UNC_OPT2_NEIGHBOR
													.ordinal()));
					requestProcessor
							.getRequestPacket()
							.setOperation(
									IpcDataUnitWrapper
											.setIpcUint32Value(UncOperationEnum.UNC_OP_READ
													.ordinal()));
					LOG.debug("Request packet created successfully for 2nd request");
					status = requestProcessor.processIpcRequest();
					LOG.debug("Request packet for 2nd request processed with status"
							+ status);
					neighbor = responseGenerator.getNeighborResponse(
							requestProcessor.getIpcResponsePacket(),
							requestBody, VtnServiceJsonConsts.SHOW);
					interfacesJson.add(VtnServiceJsonConsts.NEIGHBOR, neighbor);
					interfaceArray.add(interfacesJson);
				}
				vbypassInterfacesJson.add(VtnServiceJsonConsts.INTERFACES,
						interfaceArray);
			}
			LOG.trace("Response Packet created successfully for 2nd request");
			setInfo(vbypassInterfacesJson);
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
		LOG.trace("Completed VBypassInterfacesResource#get()");
		return status;
	}

	/**
	 * Add URI parameters to list
	 * 
	 * @return parameter list
	 */
	private List<String> getUriParameters(final JsonObject requestBody) {
		LOG.trace("Start VBypassInterfacesResource#getUriParameters()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(vtnName);
		uriParameters.add(vbypassName);
		if (requestBody != null && requestBody.has(VtnServiceJsonConsts.INDEX)) {
			uriParameters.add(requestBody.get(VtnServiceJsonConsts.INDEX)
					.getAsString());
		}
		LOG.trace("Completed VBypassInterfacesResource#getUriParameters()");
		return uriParameters;
	}
}
