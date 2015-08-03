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
import org.opendaylight.vtn.javaapi.validation.logical.LabelRangeResourceValidator;

/**
 * The Class LabelRangesResource.
 * 
 */
/* This class handles post and get methods */
@UNCVtnService(path = "/unified_networks/{unified_network_name}/labels/{label_name}/ranges")
public class LabelRangesResource extends AbstractResource {
	/** The unified network name. */
	@UNCField("unified_network_name")
	private String unifiedNetworkName;
	/** The label name. */
	@UNCField("label_name")
	private String labelName;
	/** The LOG. */
	private static final Logger LOG = Logger.getLogger(LabelRangesResource.class
			.getName());

	/**
	 * Instantiates a new Label Ranges resource.
	 */
	public LabelRangesResource() {
		super();
		LOG.trace("Start LabelRangesResource#LabelRangesResource()");
		setValidator(new LabelRangeResourceValidator(this));
		LOG.trace("Complete LabelRangesResource#LabelRangesResource()");
	}

	/**
	 * Get the unified network name.
	 * 
	 * @return the unified network name
	 */
	public final String getUnifiedNetworkName() {
		return unifiedNetworkName;
	}

	/**
	 * Get the label name.
	 * 
	 * @return the label name
	 */
	public final String getLabelName() {
		return labelName;
	}

	/**
	 * Implementation the Post method of Label Ranges API
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
		LOG.trace("Start LabelRangesResource#post()");
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
					IpcRequestPacketEnum.KT_LABEL_RANGE_CREATE, requestBody,
					getUriParameters(requestBody));
			LOG.debug("Request Packet created successfully");
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet processed with status" + status);
			LOG.debug("Complete Ipc framework call");

			final JsonObject root = new JsonObject();
			final JsonObject rangId = new JsonObject();
			String min = requestBody.getAsJsonObject(
					VtnServiceJsonConsts.RANGE).getAsJsonPrimitive(
							VtnServiceJsonConsts.RANGE_MIN).getAsString();
			String max = requestBody.getAsJsonObject(
					VtnServiceJsonConsts.RANGE).getAsJsonPrimitive(
							VtnServiceJsonConsts.RANGE_MAX).getAsString();
			rangId.addProperty(VtnServiceJsonConsts.RANGE_ID,
					min + VtnServiceConsts.UNDERSCORE + max);
			root.add(VtnServiceJsonConsts.RANGE, rangId);
			setInfo(root);

			LOG.debug("Response object created successfully");
			LOG.debug("Ipc framework call complete");

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
		LOG.trace("Complete LabelRangesResource#post()");
		return status;
	}

	/**
	 * Implementation the Get method of Label Ranges API
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
		LOG.trace("Start LabelRangesResource#get()");
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
					IpcRequestPacketEnum.KT_LABEL_RANGE_GET, requestBody,
					uriParameterList);
			LOG.debug("Request Packet created successfully");
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet processed with status" + status);
			final IpcLogicalResponseFactory responseGenerator = new IpcLogicalResponseFactory();
			JsonObject responseJson = responseGenerator.getLabelRangeResponse(
					requestProcessor.getIpcResponsePacket(), requestBody,
					VtnServiceJsonConsts.LIST);
			if (responseJson.get(VtnServiceJsonConsts.RANGES).isJsonArray()) {
				final JsonArray responseArray = responseJson.get(
						VtnServiceJsonConsts.RANGES).getAsJsonArray();

				responseJson = getResponseJsonArrayLogical(requestBody,
						requestProcessor, responseGenerator, responseArray,
						VtnServiceJsonConsts.RANGES,
						VtnServiceJsonConsts.RANGE_ID,
						IpcRequestPacketEnum.KT_LABEL_RANGE_GET, uriParameterList,
						VtnServiceIpcConsts.GET_LABEL_RANGE_RESPONSE);
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
		LOG.trace("Complete LabelRangesResource#get()");
		return status;
	}

	/**
	 * Add URI parameters to list
	 * 
	 * @return parameter list
	 */
	private List<String> getUriParameters(final JsonObject requestBody) {
		LOG.trace("Start LabelRangesResource#getUriParameters()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(unifiedNetworkName);
		uriParameters.add(labelName);
		if (requestBody != null && requestBody.has(VtnServiceJsonConsts.INDEX)) {
			uriParameters.add(requestBody.get(VtnServiceJsonConsts.INDEX)
					.getAsString());
		}
		LOG.trace("Completed LabelRangesResource#getUriParameters()");
		return uriParameters;
	}
}
