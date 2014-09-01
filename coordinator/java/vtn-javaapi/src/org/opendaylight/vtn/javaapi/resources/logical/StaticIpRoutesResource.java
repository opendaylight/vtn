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

import com.google.gson.JsonArray;
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
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcLogicalResponseFactory;
import org.opendaylight.vtn.javaapi.ipc.enums.IpcRequestPacketEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncUPLLEnums;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.validation.logical.StaticIpRouteResourceValidator;

/**
 * The Class StaticIpRoutesResource implements post, get method.
 */
@UNCVtnService(path = "/vtns/{vtn_name}/vrouters/{vrt_name}/static_iproutes")
public class StaticIpRoutesResource extends AbstractResource {

	/** The vtn name. */
	@UNCField("vtn_name")
	private String vtnName;

	/** The vrt name. */
	@UNCField("vrt_name")
	private String vrtName;

	/**
	 * 
	 * @return the vtn name
	 */
	public final String getVtnName() {
		return vtnName;
	}

	/**
	 * @return the vrt name
	 */
	public final String getVrtName() {
		return vrtName;
	}

	private static final Logger LOG = Logger
			.getLogger(StaticIpRoutesResource.class.getName());

	/**
	 * Instantiates a new static ip routes resource.
	 */
	public StaticIpRoutesResource() {
		super();
		LOG.trace("Start StaticIpRoutesResource#StaticIpRoutesResource()");
		setValidator(new StaticIpRouteResourceValidator(this));
		LOG.trace("Complete StaticIpRoutesResource#StaticIpRoutesResource()");
	}

	/**
	 * Implementation of post method of Static Ip Routes
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
		LOG.trace("Start StaticIpRoutesResource#post()");
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
					IpcRequestPacketEnum.KT_VRT_IPROUTE_CREATE, requestBody,
					uriParameterList);
			LOG.debug("Request Packet  created successfully");
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet processed with status" + status);
			// StaticIp Route
			String ipAddr = null;
			String nextHopAddr = null;
			String prefix = null;
			// String nmgName = null;
			String staticIpRouteId = null;
			if (requestBody != null
					&& requestBody.has(VtnServiceJsonConsts.STATIC_IPROUTE)
					&& ((JsonObject) requestBody
							.get(VtnServiceJsonConsts.STATIC_IPROUTE))
							.has(VtnServiceJsonConsts.IPADDR)
					&& ((JsonObject) requestBody
							.get(VtnServiceJsonConsts.STATIC_IPROUTE))
							.has(VtnServiceJsonConsts.PREFIX)
					&& ((JsonObject) requestBody
							.get(VtnServiceJsonConsts.STATIC_IPROUTE))
							.has(VtnServiceJsonConsts.NEXTHOPADDR)) {
				ipAddr = ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.STATIC_IPROUTE)).get(
						VtnServiceJsonConsts.IPADDR).getAsString();
				nextHopAddr = ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.STATIC_IPROUTE)).get(
						VtnServiceJsonConsts.NEXTHOPADDR).getAsString();
				prefix = ((JsonObject) requestBody
						.get(VtnServiceJsonConsts.STATIC_IPROUTE)).get(
						VtnServiceJsonConsts.PREFIX).getAsString();
				staticIpRouteId = ipAddr + VtnServiceJsonConsts.HYPHEN
						+ nextHopAddr + VtnServiceJsonConsts.HYPHEN + prefix;
			}
			final JsonObject root = new JsonObject();
			final JsonObject staticIpRoute = new JsonObject();
			staticIpRoute.addProperty(VtnServiceJsonConsts.STATICIPROUTEID,
					staticIpRouteId);
			root.add(VtnServiceJsonConsts.STATIC_IPROUTE, staticIpRoute);
			setInfo(root);
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
		LOG.trace("Complete StaticIpRoutesResource#post()");
		return status;
	}

	/**
	 * Implementation of get method of Static Ip Routes
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

		LOG.trace("Start StaticIpRoutesResource#get()");
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
					IpcRequestPacketEnum.KT_VRT_IPROUTE_GET, requestBody,
					uriParameterList);
			LOG.debug("Request Packet created successfully");
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet processed with status" + status);
			final IpcLogicalResponseFactory responseGenerator = new IpcLogicalResponseFactory();
			/*
			 * setInfo(responseGenerator.getStaticIpRouteResponse(
			 * requestProcessor.getIpcResponsePacket(), requestBody,
			 * VtnServiceJsonConsts.LIST));
			 */
			JsonObject responseJson = responseGenerator
					.getStaticIpRouteResponse(
							requestProcessor.getIpcResponsePacket(),
							requestBody, VtnServiceJsonConsts.LIST);
			if (responseJson.get(VtnServiceJsonConsts.STATIC_IPROUTES)
					.isJsonArray()) {
				final JsonArray responseArray = responseJson.get(
						VtnServiceJsonConsts.STATIC_IPROUTES).getAsJsonArray();
				responseJson = getResponseJsonArrayLogical(requestBody,
						requestProcessor, responseGenerator, responseArray,
						VtnServiceJsonConsts.STATIC_IPROUTES,
						VtnServiceJsonConsts.STATICIPROUTEID,
						IpcRequestPacketEnum.KT_VRT_IPROUTE_GET,
						uriParameterList,
						VtnServiceIpcConsts.GET_STATIC_IPROUTE_SERVER_RESPONSE);
			}
			setInfo(responseJson);
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
		LOG.trace("Complete StaticIpRoutesResource#get()");
		return status;
	}

	/**
	 * Add URI parameters to list
	 * 
	 * @return
	 */
	private List<String> getUriParameters(final JsonObject requestBody) {
		LOG.trace("Start StaticIpRoutesResource#getUriParameters()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(vtnName);
		uriParameters.add(vrtName);
		if (requestBody != null && requestBody.has(VtnServiceJsonConsts.INDEX)) {
			uriParameters.add(requestBody.get(VtnServiceJsonConsts.INDEX)
					.getAsString());
		}
		LOG.trace("Completed StaticIpRoutesResource#getUriParameters()");
		return uriParameters;
	}

}
