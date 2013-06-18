/*
 * Copyright (c) 2012-2013 NEC Corporation
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
import org.opendaylight.vtn.javaapi.ipc.enums.UncUPPLEnums;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.validation.physical.SwitchPortResourceValidator;

@UNCVtnService(path = "/controllers/{controller_id}/switches/{switch_id}/ports/{port_name}")
public class SwitchPortResource extends AbstractResource {
	private static final Logger LOG = Logger.getLogger(SwitchPortResource.class
			.getName());

	/** The controller Id. */
	@UNCField("controller_id")
	private String controllerId;
	/** The Switch Id */
	@UNCField("switch_id")
	private String switchId;
	/** The Switch Id */
	@UNCField("port_name")
	private String portName;

	/**
	 * Instantiates a new Port Resource.
	 */
	public SwitchPortResource() {
		LOG.trace("Start SwitchPortResource#SwitchPortResource()");
		setValidator(new SwitchPortResourceValidator(this));
		LOG.trace("Completed SwitchPortResource#SwitchPortResource()");
	}

	/**
	 * Get method for getting switch port details.
	 */
	@Override
	public int get(final JsonObject requestBody) throws VtnServiceException {
		LOG.trace("Starts SwitchPortResource#get()");
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
					IpcRequestPacketEnum.KT_PORT_GET, requestBody,
					getUriParameters());
			LOG.debug("Request Packet for 1st call created successfully");
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet 1st call processed with status:"+status);
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
			JsonObject root = null;
			final IpcPhysicalResponseFactory responseGenerator = new IpcPhysicalResponseFactory();
			root = responseGenerator.getSwitchPortResponse(
					requestProcessor.getIpcResponsePacket(), requestBody,
					VtnServiceJsonConsts.SHOW);
			requestProcessor.setServiceInfo(UncUPPLEnums.UPPL_IPC_SVC_NAME,
					UncUPPLEnums.ServiceID.UPPL_SVC_READREQ.ordinal());
			requestProcessor.createIpcRequestPacket(
					IpcRequestPacketEnum.KT_PORT_GET_MEMBER, requestBody,
					getUriParameters());
			LOG.debug("Request Packet 2nd call created successfully");
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet 2nd call processed with status:"+status);
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
			root = responseGenerator.getSwitchPortMemberResponse(
					requestProcessor.getIpcResponsePacket(), root,
					VtnServiceJsonConsts.SHOW);
			setInfo(root);
			LOG.debug("Response object created successfully");
			LOG.debug("Complete Ipc framework call");
		}
		catch (final VtnServiceException e) {
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
				if (null != requestProcessor
						&& null != requestProcessor.getErrorJson()) {
					setInfo(requestProcessor.getErrorJson());
				} else {
					createErrorInfo(UncCommonEnum.UncResultCode.UNC_SERVER_ERROR
							.getValue());
				}
				status = UncResultCode.UNC_SERVER_ERROR.getValue();
			}
			getConnPool().destroySession(session);
		}
		LOG.trace("Complete SwitchPortResource#get()");
		return status;
	}

	/**
	 * This method will add all the uri parameters to the parameter list
	 * 
	 * @return uriparameters
	 */
	private List<String> getUriParameters() {
		LOG.trace("Starts SwitchPortResource#getUriParameters()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(controllerId);
		uriParameters.add(switchId);
		uriParameters.add(portName);
		LOG.trace("Complete SwitchPortResource#getUriParameters()");
		return uriParameters;
	}

	/**
	 * @return the controllerId
	 */
	public String getControllerId() {
		return controllerId;
	}

	/**
	 * @return the Switch Id
	 */
	public String getSwitchId() {
		return switchId;
	}

	/**
	 * @return the Port Name
	 */
	public String getPortName() {
		return portName;
	}
}
