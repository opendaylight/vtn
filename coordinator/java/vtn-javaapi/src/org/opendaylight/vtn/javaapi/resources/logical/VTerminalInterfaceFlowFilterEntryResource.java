/*
 * Copyright (c) 2014 NEC Corporation
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
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcDataUnitWrapper;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcLogicalResponseFactory;
import org.opendaylight.vtn.javaapi.ipc.enums.IpcRequestPacketEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncOperationEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncOption1Enum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncUPLLEnums;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.validation.logical.VBridgeFlowFilterEntryResourceValidator;

@UNCVtnService(path = "/vtns/{vtn_name}/vterminals/{vterminal_name}/interfaces/{if_name}/flowfilters/{ff_type}/flowfilterentries/{seqnum}")
public class VTerminalInterfaceFlowFilterEntryResource extends AbstractResource{
	
	/** The Constant LOG. */
	private static final Logger LOG = Logger.getLogger(VTerminalInterfaceFlowFilterEntryResource.class
			.getName());
	/**
	 * Instantiates a new VTerminalInterfaceFlowFilterEntry Resource.
	 */
	public VTerminalInterfaceFlowFilterEntryResource() {
		super();
		LOG.trace("Start VTerminalInterfaceFlowFilterEntryResource#VTerminalInterfaceFlowFilterEntryResource()");
		setValidator(new VBridgeFlowFilterEntryResourceValidator(this));
		LOG.trace("Completed VTerminalInterfaceFlowFilterEntryResource#VTerminalInterfaceFlowFilterEntryResource()");
	}
	
	/** The vtn name. */
	@UNCField("vtn_name")
	private String vtnName;
	
	/** The vterminal name. */
	@UNCField("vterminal_name")
	private String vterminalName;
	
	/** The if name. */
	@UNCField("if_name")
	private String ifName;
	
	/** The ff type. */
	@UNCField("ff_type")
	private String ffType;
	
	/** The seqnum. */
	@UNCField("seqnum")
	private String seqnum;

	/**
	 * @return the vtnName
	 */
	public String getVtnName() {
		return vtnName;
	}

	/**
	 * @return the vterminalName
	 */
	public String getVterminalName() {
		return vterminalName;
	}

	/**
	 * @return the ifName
	 */
	public String getIfName() {
		return ifName;
	}
	
	/**
	 * @return the ffType
	 */
	public String getFfType() {
		return ffType;
	}
	
	/**
	 * @return the seqnum
	 */
	public String getSeqnum() {
		return seqnum;
	}

	/**
	 * Implementation of Put method of VTerminal Interface FlowFilter Entry API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public int put(final JsonObject requestBody) throws VtnServiceException {
		LOG.trace("Start VTerminalInterfaceFlowFilterEntryResource#put()");
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
					IpcRequestPacketEnum.KT_VTERM_IF_FLOWFILTER_ENTRY_UPDATE,
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
		LOG.trace("Completed VTerminalInterfaceFlowFilterEntryResource#put()");
		return status;
	}

	/**
	 * Implementation of Delete method of VTerminal Interface FlowFilterEntry API
	 * 
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public int delete() throws VtnServiceException {
		LOG.trace("Start VTerminalInterfaceFlowFilterEntryResource#delete()");
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
					IpcRequestPacketEnum.KT_VTERM_IF_FLOWFILTER_ENTRY_DELETE,
					getNullJsonObject(), getUriParameters());
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
		LOG.trace("Completed VTerminalInterfaceFlowFilterEntryResource#delete()");
		return status;
	}

	/**
	 * Implementation of Put method of VTerminal Interface FlowFilterEntry API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public int get(final JsonObject requestBody) throws VtnServiceException {
		LOG.trace("Start VTerminalInterfaceFlowFilterEntryResource#get()");
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
					IpcRequestPacketEnum.KT_VTERM_IF_FLOWFILTER_ENTRY_GET,
					requestBody, getUriParameters());
			requestProcessor.getRequestPacket().setOperation(
					IpcDataUnitWrapper
							.setIpcUint32Value(UncOperationEnum.UNC_OP_READ
									.ordinal()));
			if (!requestBody.has(VtnServiceJsonConsts.TARGETDB)
					|| (requestBody.has(VtnServiceJsonConsts.TARGETDB) && requestBody
							.get(VtnServiceJsonConsts.TARGETDB).getAsString()
							.equalsIgnoreCase(VtnServiceJsonConsts.STATE))) {
				if (requestBody.has(VtnServiceJsonConsts.OP)
						&& requestBody.get(VtnServiceJsonConsts.OP)
								.getAsString()
								.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
					requestProcessor
							.getRequestPacket()
							.setOption1(
									IpcDataUnitWrapper
											.setIpcUint32Value((UncOption1Enum.UNC_OPT1_DETAIL
													.ordinal())));
				}
			}
			LOG.debug("Request packet created successfully");
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet processed with status" + status);
			final IpcLogicalResponseFactory responseGenerator = new IpcLogicalResponseFactory();
			setInfo(responseGenerator
					.getVTerminalInterfaceFlowFilterEntryResponse(
							requestProcessor.getIpcResponsePacket(),
						requestBody, VtnServiceJsonConsts.SHOW));
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
		LOG.trace("Completed VTerminalInterfaceFlowFilterEntryResource#get()");
		return status;
	}

	/**
	 * Add URI parameters to list
	 * 
	 * @return
	 */
	private List<String> getUriParameters() {
		LOG.trace("Start VTerminalInterfaceFlowFilterEntryResource#getUriParameters()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(vtnName);
		uriParameters.add(vterminalName);
		uriParameters.add(ifName);
		uriParameters.add(ffType);
		uriParameters.add(seqnum);
		LOG.trace("Completed VTerminalInterfaceFlowFilterEntryResource#getUriParameters()");
		return uriParameters;
	}
}
