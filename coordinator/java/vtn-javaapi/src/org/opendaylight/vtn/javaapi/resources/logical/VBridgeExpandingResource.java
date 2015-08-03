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

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.ipc.ClientSession;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.annotation.UNCField;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
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
import org.opendaylight.vtn.javaapi.validation.logical.VBridgeResourceValidator;

/**
 * The Class VBridgeExpandingResource.
 * 
 */
/* This class handles get methods */
@UNCVtnService(path = "/vtns/{vtn_name}/vbridges/{vbr_name}/expanding")
public class VBridgeExpandingResource extends AbstractResource {
	/** The vtn name. */
	@UNCField("vtn_name")
	private String vtnName;
	/** The vbr name. */
	@UNCField("vbr_name")
	private String vbrName;
	/** The LOG. */
	private static final Logger LOG = Logger.getLogger(VBridgeExpandingResource.class
			.getName());

	/**
	 * Instantiates a new vbridge expanding resource.
	 */
	public VBridgeExpandingResource() {
		super();
		LOG.trace("Start VBridgeExpandingResource#VBridgeExpandingResource()");
		setValidator(new VBridgeResourceValidator(this));
		LOG.trace("Complete VBridgeExpandingResource#VBridgeExpandingResource()");
	}

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

	/**
	 * Implementation of get method of VBridge expanding API
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
		LOG.trace("Start VBridgeExpandingResource#get()");
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
					IpcRequestPacketEnum.KT_VBRIDGE_EXPAND, requestBody,
					getUriParameters());
			LOG.debug("Request Packet created successfully");
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet processed with status" + status);
			final IpcLogicalResponseFactory responseGenerator = new IpcLogicalResponseFactory();
			setInfo(responseGenerator.getVbridgeExpandResourceResponse(
					requestProcessor.getIpcResponsePacket(), requestBody,
					VtnServiceJsonConsts.SHOW));
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
		LOG.trace("Complete VBridgeExpandingResource#get()");
		return status;
	}

	/**
	 * Add URI parameters to list
	 * 
	 * @return parameter list
	 */
	private List<String> getUriParameters() {
		LOG.trace("Start VBridgeExpandingResource#getUriParameters()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(vtnName);
		uriParameters.add(vbrName);
		LOG.trace("Completed VBridgeExpandingResource#getUriParameters()");
		return uriParameters;
	}
}
