/*
 * Copyright (c) 2015 NEC Corporation
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
import org.opendaylight.vtn.javaapi.validation.logical.VBridgePortMapResourceValidator;

/**
 * The Class VBridgePortMapsResource implements post and get methods.
 * 
 */

@UNCVtnService(
		path = "/vtns/{vtn_name}/vbridges/{vbr_name}/portmaps")
public class VBridgePortMapsResource extends AbstractResource {
	
	/** The vtn name. */
	@UNCField("vtn_name")
	private String vtnName;
	
	/** The vbr name. */
	@UNCField("vbr_name")
	private String vbrName;

	/** The Constant LOG. */
	private static final Logger LOG = Logger
			.getLogger(VBridgePortMapsResource.class.getName());

	/**
	 * Instantiates a new v bridge portMaps resource.
	 */
	public VBridgePortMapsResource() {
		super();
		LOG.trace("Start VBridgePortMapsResource#VBridgePortMapsResource()");
		setValidator(new VBridgePortMapResourceValidator(this));
		LOG.trace("Complete VBridgePortMapsResource#VBridgePortMapsResource()");
	}

	/**
	 * @return the vtnName
	 */
	public final String getVtnName() {
		return vtnName;
	}

	/**
	 * @return the vbrName
	 */
	public final String getVbrName() {
		return vbrName;
	}

	/**
	 * Implementation of Post method of VBridgePortMaps API
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
		LOG.trace("Start VBridgePortMapsResource#post()");
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
					IpcRequestPacketEnum.KT_VBR_PORTMAP_CREATE,
					requestBody, uriParameterList);
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
					createErrorInfo(UncCommonEnum.UncResultCode.UNC_INTERNAL_SERVER_ERROR
							.getValue());
				}
				status = UncResultCode.UNC_SERVER_ERROR.getValue();
			}
			getConnPool().destroySession(session);
		}
		LOG.trace("Complete VBridgePortMapsResource#post()");
		return status;
	}

	/**
	 * Implementation of Get method of VBridgePortMaps API
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
		LOG.trace("Start VBridgePortMapsResource#get()");
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
					IpcRequestPacketEnum.KT_VBR_PORTMAP_GET,
					requestBody, uriParameterList);
			LOG.debug("Request packet created successfully");
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet processed with status" + status);
			final IpcLogicalResponseFactory responseGenerator = new IpcLogicalResponseFactory();
			JsonObject responseJson = responseGenerator
					.getVBridgePortMapResponse(
							requestProcessor.getIpcResponsePacket(),
							requestBody, VtnServiceJsonConsts.LIST);
			if (responseJson.get(VtnServiceJsonConsts.PORTMAPS)
					.isJsonArray()) {
				final JsonArray responseArray = responseJson.get(
						VtnServiceJsonConsts.PORTMAPS)
						.getAsJsonArray();
				responseJson = getResponseJsonArrayLogical(
						requestBody, requestProcessor, responseGenerator, responseArray,
						VtnServiceJsonConsts.PORTMAPS,
						VtnServiceJsonConsts.PORTMAP_NAME,
						IpcRequestPacketEnum.KT_VBR_PORTMAP_GET,
						uriParameterList,
						VtnServiceIpcConsts.GET_VBRIDGE_PORTMAP_RESPONSE);
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
					createErrorInfo(UncCommonEnum.UncResultCode.UNC_INTERNAL_SERVER_ERROR
							.getValue());
				}
				status = UncResultCode.UNC_SERVER_ERROR.getValue();
			}
			getConnPool().destroySession(session);
		}
		LOG.trace("Complete VBridgePortMapsResource#get()");
		return status;
	}

	/**
	 * Add URI parameters to list
	 * 
	 * @param requestBody
	 *            the request Json object
	 *            
	 * @return parameter list
	 */
	private List<String> getUriParameters(final JsonObject requestBody) {
		LOG.trace("Start VBridgePortMapsResource#getUriParameters()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(vtnName);
		uriParameters.add(vbrName);
		if (requestBody != null && requestBody.has(VtnServiceJsonConsts.INDEX)) {
			uriParameters.add(requestBody.get(VtnServiceJsonConsts.INDEX)
					.getAsString());
		}
		LOG.trace("Completed VBridgePortMapsResource#getUriParameters()");
		return uriParameters;
	}

}
