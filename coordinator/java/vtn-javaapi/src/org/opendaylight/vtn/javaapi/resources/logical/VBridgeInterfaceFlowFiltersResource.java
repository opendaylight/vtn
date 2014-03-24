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

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.ipc.ClientSession;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.annotation.UNCField;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.IpcRequestProcessor;
import org.opendaylight.vtn.javaapi.ipc.enums.IpcRequestPacketEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncUPLLEnums;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.validation.logical.FlowFilterResourceValidator;

/**
 * The Class VBridgeInterfaceFlowFiltersResource implements post method.
 */

@UNCVtnService(
		path = "/vtns/{vtn_name}/vbridges/{vbr_name}/interfaces/{if_name}/flowfilters")
public class VBridgeInterfaceFlowFiltersResource extends AbstractResource {
	/** The VTN name. */
	@UNCField("vtn_name")
	private String vtnName;
	/** The VBR name. */
	@UNCField("vbr_name")
	private String vbrName;
	/** The if name. */
	@UNCField("if_name")
	private String ifName;

	/** The Constant LOG. */
	private static final Logger LOG = Logger
			.getLogger(VBridgeInterfaceFlowFiltersResource.class.getName());

	/**
	 * Instantiates a new v bridge interface flow filters resource.
	 */
	public VBridgeInterfaceFlowFiltersResource() {
		super();
		LOG.trace("Start VBridgeInterfaceFlowFiltersResource#VBridgeInterfaceFlowFiltersResource()");
		setValidator(new FlowFilterResourceValidator(this));
		LOG.trace("Completed VBridgeInterfaceFlowFiltersResource#VBridgeInterfaceFlowFiltersResource()");
	}

	/**
	 * Gets the VTN name.
	 * 
	 * @return the VTN name
	 */
	public final String getVtnName() {
		return vtnName;
	}

	/**
	 * Gets the VBR name.
	 * 
	 * @return the VBR name
	 */
	public final String getVbrName() {
		return vbrName;
	}

	/**
	 * Gets the if name.
	 * 
	 * @return the if name
	 */
	public final String getIfName() {
		return ifName;
	}

	/**
	 * Implementation of Post method of VBridgeInterfaceFlowFilter API
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
		LOG.trace("Start VBridgeInterfaceFlowFiltersResource#post()");
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
					IpcRequestPacketEnum.KT_VBRIF_FLOWFILTER_CREATE,
					requestBody, getUriParameters());
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
		LOG.trace("Completed VBridgeInterfaceFlowFiltersResource#post()");
		return status;
	}

	/**
	 * Add URI parameters to list
	 * 
	 * @return
	 */
	private List<String> getUriParameters() {
		LOG.trace("Start VBridgeInterfaceFlowFiltersResource#getUriParameters()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(vtnName);
		uriParameters.add(vbrName);
		uriParameters.add(ifName);
		LOG.trace("Completed VBridgeInterfaceFlowFiltersResource#getUriParameters()");
		return uriParameters;
	}
}
