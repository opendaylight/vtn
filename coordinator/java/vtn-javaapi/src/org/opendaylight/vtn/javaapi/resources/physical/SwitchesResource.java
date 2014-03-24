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
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcPhysicalResponseFactory;
import org.opendaylight.vtn.javaapi.ipc.enums.IpcRequestPacketEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncUPPLEnums;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.resources.logical.VtnsResource;
import org.opendaylight.vtn.javaapi.validation.physical.SwitchResourceValidator;

/**
 * The Class SwitchesResource implements get method.
 */
@UNCVtnService(path = "/controllers/{controller_id}/switches")
public class SwitchesResource extends AbstractResource {

	/** The Constant LOG. */
	private static final Logger LOG = Logger.getLogger(VtnsResource.class
			.getName());

	/** The controller Id. */
	@UNCField("controller_id")
	private String controllerId;

	/**
	 * Gets the controller Id.
	 * 
	 * @return the controller Id
	 */
	public String getControllerId() {
		return controllerId;
	}

	/**
	 * Instantiates a new switches resource.
	 */
	public SwitchesResource() {
		super();
		LOG.trace("Start SwitchesResource#SwitchesResource()");
		setValidator(new SwitchResourceValidator(this));
		LOG.trace("Completed SwitchesResource#SwitchesResource()");
	}

	/**
	 * Implementation of get method of Switch API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public int get(final JsonObject requestBody) throws VtnServiceException {
		LOG.trace("Starts SwitchesResource#get()");
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
			// Uriparamter list
			final List<String> uriParameterList = getUriParameters(requestBody);
			requestProcessor.createIpcRequestPacket(
					IpcRequestPacketEnum.KT_SWITCH_GET, requestBody,
					uriParameterList);
			LOG.debug("Request packet created successfully");
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet processed with status:" + status);
			final IpcPhysicalResponseFactory responseGenerator = new IpcPhysicalResponseFactory();
			JsonObject responseJson = responseGenerator.getSwitchResponse(
					requestProcessor.getIpcResponsePacket(), requestBody,
					VtnServiceJsonConsts.LIST);
			if (responseJson.get(VtnServiceJsonConsts.SWITCHES).isJsonArray()) {
				final JsonArray responseArray = responseJson.get(
						VtnServiceJsonConsts.SWITCHES).getAsJsonArray();

				responseJson = getResponseJsonArrayPhysical(requestBody,
						requestProcessor, responseGenerator, responseArray,
						VtnServiceJsonConsts.SWITCHES,
						VtnServiceJsonConsts.SWITCHID,
						IpcRequestPacketEnum.KT_SWITCH_GET, uriParameterList,
						VtnServiceIpcConsts.GET_SWITCH_RESPONSE);
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
		LOG.trace("Completed SwitchesResource#get()");
		return status;
	}

	/**
	 * Add URI parameters to list
	 * 
	 * @param requestBody
	 *            ,for handling the request.
	 * @return
	 */
	private List<String> getUriParameters(final JsonObject requestBody) {
		LOG.trace("Starts SwitchesResource#getUriParameters()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(controllerId);
		if (requestBody != null && requestBody.has(VtnServiceJsonConsts.INDEX)) {
			uriParameters.add(requestBody.get(VtnServiceJsonConsts.INDEX)
					.getAsString());
		}
		LOG.trace("Completed SwitchesResource#getUriParameters()");
		return uriParameters;
	}
}
