/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.resources.physical;

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
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcDataUnitWrapper;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcPhysicalResponseFactory;
import org.opendaylight.vtn.javaapi.ipc.enums.IpcRequestPacketEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncOperationEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncOption2Enum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncUPPLEnums;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.validation.physical.LinkResourceValidator;

/**
 * The Class LinkResource implements get method.
 */
@UNCVtnService(path = "/controllers/{controller_id}/links")
public class LinksResource extends AbstractResource {
	/**
	 * Logger for debugging purpose .
	 */
	private static final Logger LOG = Logger.getLogger(LinksResource.class
			.getName());
	/** The controller Id. */
	@UNCField("controller_id")
	private String controllerId;

	/**
	 * Instantiates a new Controller Resource.
	 */
	public LinksResource() {
		LOG.trace("Start LinksResource#LinksResource()");
		setValidator(new LinkResourceValidator(this));
		LOG.trace("Completed LinksResource#LinksResource()");
	}

	/**
	 * @return the controllerId
	 */
	public final String getControllerId() {
		return controllerId;
	}

	/**
	 * Implementation of get method of Link API.
	 * 
	 * @param requestBody
	 *            the request Json object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 *             , in case of error .
	 */
	@Override
	public final int get(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Starts LinksResource#get()");
		ClientSession session = null;
		IpcRequestProcessor requestProcessor = null;
		int status = ClientSession.RESP_FATAL;
		try {
			LOG.debug("Start Ipc framework call");
			session = getConnPool().getSession(UncUPPLEnums.UPPL_IPC_CHN_NAME,
					UncUPPLEnums.UPPL_IPC_SVC_NAME,
					UncUPPLEnums.ServiceID.UPPL_SVC_READREQ.ordinal(),
					getExceptionHandler());
			LOG.debug("Session created successfully");
			requestProcessor = new IpcRequestProcessor(session, getSessionID(),
					getConfigID(), getExceptionHandler());
			final List<String> uriParameterList = getUriParameters(requestBody);
			requestProcessor.createIpcRequestPacket(
					IpcRequestPacketEnum.KT_LINK_GET, requestBody,
					uriParameterList);
			if (!requestBody.has(VtnServiceJsonConsts.LINKNAME)) {
				IpcRequestPacketEnum ipcRequestPacketEnum = null;
				ipcRequestPacketEnum = getModifiedRequestPacket(requestBody, requestProcessor);
				LOG.debug("Request packet created successfully");
				status = requestProcessor.processIpcRequest();
				LOG.debug("Request packet processed with status:" + status);
				final IpcPhysicalResponseFactory responseGenerator = new IpcPhysicalResponseFactory();
				JsonObject responseJson = responseGenerator.getLinkResponse(
						requestProcessor.getIpcResponsePacket(), requestBody);
				if (responseJson.get(VtnServiceJsonConsts.LINKS).isJsonArray()) {
					final JsonArray responseArray = responseJson.get(
							VtnServiceJsonConsts.LINKS).getAsJsonArray();
					responseJson = getResponseJsonArrayPhysical(requestBody,
							requestProcessor, responseGenerator, responseArray,
							VtnServiceJsonConsts.LINKS,
							VtnServiceJsonConsts.LINKNAME,
							ipcRequestPacketEnum, uriParameterList,
							VtnServiceIpcConsts.GET_LINK_RESPONSE);
				}
				setInfo(responseJson);
			} else {
				requestProcessor.getRequestPacket().setOperation(
						IpcDataUnitWrapper
								.setIpcUint32Value(UncOperationEnum.UNC_OP_READ
										.ordinal()));
				status = requestProcessor.processIpcRequest();
				LOG.debug("Request packet processed with status:" + status);
				final IpcPhysicalResponseFactory responseGenerator = new IpcPhysicalResponseFactory();
				setInfo(responseGenerator.getLinkResponse(
						requestProcessor.getIpcResponsePacket(), requestBody));
			}
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
		LOG.trace("Completed LinksResource#get()");
		return status;
	}

	/**
	 * @param requestBody
	 *            , for handling request.
	 * @param requestProcessor
	 *            , for processing the request .
	 */
	private IpcRequestPacketEnum getModifiedRequestPacket(
			final JsonObject requestBody,
			final IpcRequestProcessor requestProcessor) {
		IpcRequestPacketEnum ipcRequestPacketEnum = IpcRequestPacketEnum.KT_LINK_GET;
		if ((requestBody.has(VtnServiceJsonConsts.SWITCH1ID) && requestBody
				.has(VtnServiceJsonConsts.SWITCH2ID))) {
			requestProcessor
					.getRequestPacket()
					.setOption2(
							IpcDataUnitWrapper
									.setIpcUint32Value(UncOption2Enum.UNC_OPT2_MATCH_BOTH_SWITCH
											.ordinal()));
			ipcRequestPacketEnum = IpcRequestPacketEnum.KT_LINK_GET_BOTH_SWITCH;
		} else if (requestBody.has(VtnServiceJsonConsts.SWITCH1ID)) {
			requestProcessor
					.getRequestPacket()
					.setOption2(
							IpcDataUnitWrapper
									.setIpcUint32Value(UncOption2Enum.UNC_OPT2_MATCH_SWITCH1
											.ordinal()));
			ipcRequestPacketEnum = IpcRequestPacketEnum.KT_LINK_GET_SWITCH1;
		} else if (requestBody.has(VtnServiceJsonConsts.SWITCH2ID)) {
			requestProcessor
					.getRequestPacket()
					.setOption2(
							IpcDataUnitWrapper
									.setIpcUint32Value(UncOption2Enum.UNC_OPT2_MATCH_SWITCH2
											.ordinal()));
			ipcRequestPacketEnum = IpcRequestPacketEnum.KT_LINK_GET_SWITCH2;
		}
		
		return ipcRequestPacketEnum;
	}

	/**
	 * Add URI parameters to list.
	 * 
	 * @param requestBody
	 *            , to handle the request .
	 * @return List , containing all URI paramters .
	 */
	private List<String> getUriParameters(final JsonObject requestBody) {
		LOG.trace("Starts LinksResource#getUriParameters()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(controllerId);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.LINKNAME)) {
			uriParameters.add(requestBody.get(VtnServiceJsonConsts.LINKNAME)
					.getAsString());
		} else if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.INDEX)) {
			uriParameters.add(requestBody.get(VtnServiceJsonConsts.INDEX)
					.getAsString());
		}
		LOG.trace("Completed LinksResource#getUriParameters()");
		return uriParameters;
	}
}
