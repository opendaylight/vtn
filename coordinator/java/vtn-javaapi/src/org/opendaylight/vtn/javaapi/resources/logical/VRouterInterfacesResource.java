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
import org.opendaylight.vtn.javaapi.validation.logical.VRouterInterfaceResourceValidator;

/**
 * The Class VRouterInterfacesResource implements post and get methods.
 */
@UNCVtnService(path = "/vtns/{vtn_name}/vrouters/{vrt_name}/interfaces")
public class VRouterInterfacesResource extends AbstractResource {

	/** The vtn name. */
	@UNCField("vtn_name")
	private String vtnName;

	/** The vrt name. */
	@UNCField("vrt_name")
	private String vrtName;

	/**
	 * @return the vtnName
	 */
	public final String getVtnName() {
		return vtnName;
	}

	/**
	 * @return the vrtName
	 */
	public final String getVrtName() {
		return vrtName;
	}

	private static final Logger LOG = Logger
			.getLogger(VRouterInterfacesResource.class.getName());

	/**
	 * Instantiates a new VRouterInterface resource.
	 */
	public VRouterInterfacesResource() {
		super();
		LOG.trace("Start VRouterInterfacesResource#VRouterInterfacesResource()");
		setValidator(new VRouterInterfaceResourceValidator(this));
		LOG.trace("Complete VRouterInterfacesResource#VRouterInterfacesResource()");
	}

	/**
	 * Implementation of get method of VRouterInterface
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
		LOG.trace("Start VRouterInterfacesResource#get()");
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
					IpcRequestPacketEnum.KT_VRT_IF_GET, requestBody,
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
			JsonObject neighbor = null;
			JsonObject vrtInterfacesJson = null;
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
			vrtInterfacesJson = responseGenerator.getVRouterInterfaceResponse(
					requestProcessor.getIpcResponsePacket(), requestBody,
					VtnServiceJsonConsts.LIST);
			if (vrtInterfacesJson.get(VtnServiceJsonConsts.INTERFACES)
					.isJsonArray()) {
				final JsonArray responseArray = vrtInterfacesJson.get(
						VtnServiceJsonConsts.INTERFACES).getAsJsonArray();
				vrtInterfacesJson = getResponseJsonArrayLogical(requestBody,
						requestProcessor, responseGenerator, responseArray,
						VtnServiceJsonConsts.INTERFACES,
						VtnServiceJsonConsts.IFNAME,
						IpcRequestPacketEnum.KT_VRT_IF_GET, uriParameterList,
						VtnServiceIpcConsts.GET_VROUTER_INTERFACE_RESPONSE);
			}
			LOG.debug("Response object created successfully for 1st request");
			if ((VtnServiceJsonConsts.STATE).equalsIgnoreCase(dataType)
					&& opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
				final Iterator<JsonElement> interfaceIterator = vrtInterfacesJson
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
							IpcRequestPacketEnum.KT_VRT_IF_GET, requestBody,
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
					neighbor = responseGenerator.getNeighborResponse(
							requestProcessor.getIpcResponsePacket(),
							requestBody, VtnServiceJsonConsts.SHOW);
					interfacesJson.add(VtnServiceJsonConsts.NEIGHBOR, neighbor);
					interfaceArray.add(interfacesJson);
				}
				vrtInterfacesJson.add(VtnServiceJsonConsts.INTERFACES,
						interfaceArray);
			}
			LOG.debug("Response object created successfully for 2nd request");
			setInfo(vrtInterfacesJson);
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

		LOG.trace("Complete VRouterInterfacesResource#get()");
		return status;
	}

	/**
	 * Implementation of post method of VRouterInterface
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
		LOG.trace("Start VRouterInterfacesResource#post()");
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
					IpcRequestPacketEnum.KT_VRT_IF_CREATE, requestBody,
					getUriParameters(requestBody));
			LOG.debug("Request packet created successfully");
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
		LOG.trace("Complete VRouterInterfacesResource#post()");
		return status;
	}

	/**
	 * Add URI parameters to list
	 * 
	 * @return
	 */
	private List<String> getUriParameters(final JsonObject requestBody) {
		LOG.trace("Start VRouterInterfacesResource#getUriParameters()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(vtnName);
		uriParameters.add(vrtName);
		if (requestBody != null && requestBody.has(VtnServiceJsonConsts.INDEX)) {
			uriParameters.add(requestBody.get(VtnServiceJsonConsts.INDEX)
					.getAsString());
		}
		LOG.trace("Completed VRouterInterfacesResource#getUriParameters()");
		return uriParameters;
	}
}
