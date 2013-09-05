/*
 * Copyright (c) 2012-2013 NEC Corporation
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
import org.opendaylight.vtn.javaapi.validation.logical.PortMapResourceValidator;

/**
 * The Class VTepInterfacePortMapResource implements delete, put and get methods.
 *
 */

@UNCVtnService(path = "/vtns/{vtn_name}/vteps/{vtep_name}/interfaces/{if_name}/portmap")
public class VTepInterfacePortMapResource extends AbstractResource {
	/** The vtn name. */
	@UNCField("vtn_name")
	private String vtnName;
	/** The vbr name. */
	@UNCField("vtep_name")
	private String vtepName;
	/** The if name. */
	@UNCField("if_name")
	private String ifName;

	/** The Constant LOG. */
	private static final Logger LOG = Logger.getLogger(VTepInterfacePortMapResource.class.getSimpleName());
	/**
	 * Gets the vtn name.
	 *
	 * @return the vtn name
	 */
	public String getVtnName() {
		return vtnName;
	}
	/**
	 * Gets the vbr name.
	 *
	 * @return the vbr name
	 */
	public String getVtepName() {
		return vtepName;
	}
	/**
	 * Gets the if name.
	 *
	 * @return the if name
	 */
	public String getIfName() {
		return ifName;
	}
	/**
	 * Instantiates a new port map resource.
	 */
	public VTepInterfacePortMapResource() {
		super();
		LOG.trace("Start VTepInterfacePortMapResource#VTepInterfacePortMapResource()");
		setValidator(new PortMapResourceValidator(this));
		LOG.trace("Complete VTepInterfacePortMapResource#VTepInterfacePortMapResource()");
	}

	/**
	 * Implementation of Delete method of VTep Interface PortMap API
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public int delete() throws VtnServiceException {
		LOG.trace("Start VTepInterfacePortMapResource#delete()");
		ClientSession session = null;
		IpcRequestProcessor requestProcessor = null;
		int status = ClientSession.RESP_FATAL;
		try {
			session = getConnPool().getSession(UncUPLLEnums.UPLL_IPC_CHANNEL_NAME,UncUPLLEnums.UPLL_IPC_SERVICE_NAME,UncUPLLEnums.ServiceID.UPLL_EDIT_SVC_ID.ordinal(),getExceptionHandler());
			LOG.debug("Session created successfully");
			requestProcessor = new IpcRequestProcessor(session, getSessionID(), getConfigID(),getExceptionHandler());
			requestProcessor.createIpcRequestPacket(IpcRequestPacketEnum.KT_VTEP_IF_UPDATE,getNullJsonObject(),getUriParameters());
			LOG.debug("Request packet created successfully");
			status= requestProcessor.processIpcRequest();
		} catch (final VtnServiceException e) {
			getExceptionHandler().raise(
					Thread.currentThread().getStackTrace()[1].getClassName()
					+ VtnServiceConsts.HYPHEN
					+ Thread.currentThread().getStackTrace()[1].getMethodName(),
					UncJavaAPIErrorCode.IPC_SERVER_ERROR.getErrorCode(),
					UncJavaAPIErrorCode.IPC_SERVER_ERROR.getErrorMessage(), e);
			throw e;
		} finally {
			if (status == ClientSession.RESP_FATAL) {
				if(null != requestProcessor.getErrorJson()){
					setInfo(requestProcessor.getErrorJson());
				} else {
					createErrorInfo(UncCommonEnum.UncResultCode.UNC_SERVER_ERROR.getValue());
				}
				status = UncResultCode.UNC_SERVER_ERROR.getValue();
			}
			getConnPool().destroySession(session);
		}
		LOG.trace("Complete VTepInterfacePortMapResource#delete()");
		return status;
	}
	/**
	 * Implementation of get method of VTep Interface PortMap API
	 * 
	 * @param requestBody the request Json object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public int get(final JsonObject requestBody) throws VtnServiceException {
		LOG.trace("Start VTepInterfacePortMapResource#get()");
		ClientSession session = null;
		IpcRequestProcessor requestProcessor = null;
		int status = ClientSession.RESP_FATAL;
		try {
			session = getConnPool().getSession(UncUPLLEnums.UPLL_IPC_CHANNEL_NAME,UncUPLLEnums.UPLL_IPC_SERVICE_NAME,UncUPLLEnums.ServiceID.UPLL_READ_SVC_ID.ordinal(),getExceptionHandler());
			LOG.debug("Session created successfully");
			requestProcessor = new IpcRequestProcessor(session, getSessionID(), getConfigID(),getExceptionHandler());
			requestProcessor.createIpcRequestPacket(IpcRequestPacketEnum.KT_VTEP_IF_GET, requestBody,getUriParameters());
			LOG.debug("Request Packet created successfully");
			status= requestProcessor.processIpcRequest();
			IpcLogicalResponseFactory responseGenerator = new IpcLogicalResponseFactory();
			setInfo(responseGenerator.getPortMapResponse(requestProcessor.getIpcResponsePacket(),requestBody,VtnServiceJsonConsts.SHOW,VtnServiceJsonConsts.VTEP_INTERFACE_PORTMAP));
			LOG.debug("response object created successfully");
			LOG.debug("Ipc framework call complete");
		} catch (final VtnServiceException e) {
			getExceptionHandler().raise(
					Thread.currentThread().getStackTrace()[1].getClassName()
					+ VtnServiceConsts.HYPHEN
					+ Thread.currentThread().getStackTrace()[1].getMethodName(),
					UncJavaAPIErrorCode.IPC_SERVER_ERROR.getErrorCode(),
					UncJavaAPIErrorCode.IPC_SERVER_ERROR.getErrorMessage(), e);
			throw e;
		} finally {
			if (status == ClientSession.RESP_FATAL) {
				if(null != requestProcessor.getErrorJson()){
					setInfo(requestProcessor.getErrorJson());
				} else {
					createErrorInfo(UncCommonEnum.UncResultCode.UNC_SERVER_ERROR.getValue());
				}
				status = UncResultCode.UNC_SERVER_ERROR.getValue();
			}
			getConnPool().destroySession(session);
		}
		LOG.trace("Complete VTepInterfacePortMapResource#get()");
		return status;
	}

	/**
	 * Implementation of Put method of PortMap API
	 * 
	 * @param requestBody the request Json object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public int put(final JsonObject requestBody) throws VtnServiceException {
		LOG.trace("Start VTepInterfacePortMapResource#put()");
		ClientSession session = null;
		IpcRequestProcessor requestProcessor = null;
		int status = ClientSession.RESP_FATAL;
		try {
			session = getConnPool().getSession(UncUPLLEnums.UPLL_IPC_CHANNEL_NAME,UncUPLLEnums.UPLL_IPC_SERVICE_NAME,UncUPLLEnums.ServiceID.UPLL_EDIT_SVC_ID.ordinal(),getExceptionHandler());
			LOG.debug("Session created successfully");
			requestProcessor = new IpcRequestProcessor(session, getSessionID(), getConfigID(),getExceptionHandler());
			requestProcessor.createIpcRequestPacket(IpcRequestPacketEnum.KT_VTEP_IF_UPDATE, requestBody,getUriParameters());
			LOG.debug("Request Packet created successfully");
			status = requestProcessor.processIpcRequest();
		}catch (final VtnServiceException e) {
			getExceptionHandler().raise(
					Thread.currentThread().getStackTrace()[1].getClassName()
					+ VtnServiceConsts.HYPHEN
					+ Thread.currentThread().getStackTrace()[1].getMethodName(),
					UncJavaAPIErrorCode.IPC_SERVER_ERROR.getErrorCode(),
					UncJavaAPIErrorCode.IPC_SERVER_ERROR.getErrorMessage(), e);
			throw e;
		} finally {
			if (status == ClientSession.RESP_FATAL) {
				if(null != requestProcessor.getErrorJson()){
					setInfo(requestProcessor.getErrorJson());
				} else {
					createErrorInfo(UncCommonEnum.UncResultCode.UNC_SERVER_ERROR.getValue());
				}
				status = UncResultCode.UNC_SERVER_ERROR.getValue();
			}
			getConnPool().destroySession(session);
		}
		LOG.trace("Complete VTepInterfacePortMapResource#put()");
		return status;
	}
	/**
	 * Add URI parameters to list
	 * @return
	 */
	private List<String> getUriParameters() {
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(vtnName);
		uriParameters.add(vtepName);
		uriParameters.add(ifName);
		return uriParameters;
	}
}
