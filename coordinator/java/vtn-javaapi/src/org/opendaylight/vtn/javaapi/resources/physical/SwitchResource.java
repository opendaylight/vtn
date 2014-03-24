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

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.ipc.ClientSession;
import org.opendaylight.vtn.core.ipc.IpcUint32;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.annotation.UNCField;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.IpcRequestProcessor;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcPhysicalResponseFactory;
import org.opendaylight.vtn.javaapi.ipc.enums.IpcRequestPacketEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncOperationEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncOption1Enum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncUPPLEnums;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.resources.logical.VtnsResource;
import org.opendaylight.vtn.javaapi.validation.physical.SwitchResourceValidator;

/**
 * The Class SwitchResource implements get method.
 */
@UNCVtnService(path = "/controllers/{controller_id}/switches/{switch_id}")
public class SwitchResource extends AbstractResource {

	/** The Constant LOG. */
	private static final Logger LOG = Logger.getLogger(VtnsResource.class
			.getName());
	/** The controller Id. */
	@UNCField("controller_id")
	private String controllerId;
	/** The Switch Id */
	@UNCField("switch_id")
	private String switchId;

	/**
	 * Instantiates a new switch resource.
	 */
	public SwitchResource() {
		super();
		LOG.trace("Start SwitchResource#SwitchResource()");
		setValidator(new SwitchResourceValidator(this));
		LOG.trace("Completed SwitchResource#SwitchResource()");
	}

	/**
	 * Gets the controller Id.
	 * 
	 * @return the controller Id
	 */
	public final String getControllerId() {
		return controllerId;
	}

	/**
	 * Gets the Switch Id.
	 * 
	 * @return the Switch Id
	 */
	public final String getSwitchId() {
		return switchId;
	}

	/**
	 * Implementation of get method of Switch API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public final int get(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Starts SwitchResource#get()");
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
			requestProcessor.createIpcRequestPacket(
					IpcRequestPacketEnum.KT_SWITCH_GET, requestBody,
					getUriParameters());
			
			requestProcessor.getRequestPacket().setOperation(
					new IpcUint32(UncOperationEnum.UNC_OP_READ.ordinal()));
			
			if (requestBody.has(VtnServiceJsonConsts.OP)
					&& requestBody.get(VtnServiceJsonConsts.OP).getAsString()
							.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
				requestProcessor.getRequestPacket()
						.setOption1(
								new IpcUint32(UncOption1Enum.UNC_OPT1_DETAIL
										.ordinal()));
			}
			LOG.debug("Request packet created successfully");
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet processed with status:" + status);
			final IpcPhysicalResponseFactory responseGenerator = new IpcPhysicalResponseFactory();
			setInfo(responseGenerator.getSwitchResponse(
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
		LOG.trace("Completed SwitchResource#get()");
		return status;
	}

	/**
	 * Add URI parameters to list
	 * 
	 * @return
	 */
	private List<String> getUriParameters() {
		LOG.trace("Starts SwitchResource#getUriParameters()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(controllerId);
		uriParameters.add(switchId);
		LOG.trace("Completed SwitchResource#getUriParameters()");
		return uriParameters;
	}
}
