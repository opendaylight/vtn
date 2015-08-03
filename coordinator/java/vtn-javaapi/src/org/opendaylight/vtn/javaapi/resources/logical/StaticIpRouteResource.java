/*
 * Copyright (c) 2012-2015 NEC Corporation
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
import org.opendaylight.vtn.javaapi.validation.logical.StaticIpRouteResourceValidator;

/**
 * The Class StaticIpRouteResource implements delete, get and put methods.
 */
/* This class handles delete, get and put methods */
@UNCVtnService(
		path = "/vtns/{vtn_name}/vrouters/{vrt_name}/static_iproutes/{static_iproute_id}")
public class StaticIpRouteResource extends AbstractResource {

	/** The vtn name. */
	@UNCField("vtn_name")
	private String vtnName;

	/** The vrt name. */
	@UNCField("vrt_name")
	private String vrtName;

	/** The static ip route id. */
	@UNCField("static_iproute_id")
	private String staticIpRouteId;

	/**
	 * 
	 * @return the vtn name
	 */
	public final String getVtnName() {
		return vtnName;
	}

	/**
	 * 
	 * @return the vrt name
	 */
	public final String getVrtName() {
		return vrtName;
	}

	/**
	 * 
	 * @return the static ip route id
	 */
	public final String getStaticIpRouteId() {
		return staticIpRouteId;
	}

	private static final Logger LOG = Logger
			.getLogger(StaticIpRouteResource.class.getName());

	/**
	 * Instantiates a new static ip route resource.
	 */
	public StaticIpRouteResource() {
		super();
		LOG.trace("Start StaticIpRouteResource#StaticIpRouteResource()");
		setValidator(new StaticIpRouteResourceValidator(this));
		LOG.trace("Complete StaticIpRouteResource#StaticIpRouteResource()");
	}

	/**
	 * Implementation of delete method of Static Ip Routes
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public final int delete() throws VtnServiceException {
		LOG.trace("Start StaticIpRouteResource#delete()");
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
					IpcRequestPacketEnum.KT_VRT_IPROUTE_DELETE,
					getNullJsonObject(), getUriParameters());
			LOG.debug("Request Packet created successfully");
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
		LOG.trace("Complete StaticIpRouteResource#delete()");
		return status;
	}

	/**
	 * Implementation of get method of Static Ip Routes
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
		LOG.trace("Start StaticIpRouteResource#get()");
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
					IpcRequestPacketEnum.KT_VRT_IPROUTE_GET_SHOW, requestBody,
					getUriParameters());
			LOG.debug("Request Packet created successfully");
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet processed with status" + status);
			final IpcLogicalResponseFactory responseGenerator = new IpcLogicalResponseFactory();
			setInfo(responseGenerator.getStaticIpRouteResponse(
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
		LOG.trace("Complete StaticIpRouteResource#get()");
		return status;
	}

	/**
	 * Implementation of put method of Static Ip Routes
	 *
	 * @param requestBody
	 * the request Json object
	 *
	 * @return Error code
	 * @throws VtnServiceException
	 */
	 @Override
	public int put(final JsonObject requestBody) throws VtnServiceException {
		LOG.trace("Start StaticIpRouteResource#put()");
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
					IpcRequestPacketEnum.KT_VRT_IPROUTE_UPDATE, requestBody,
					getUriParameters());
			LOG.debug("Request Packet created successfully");
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
		LOG.trace("Complete StaticIpRouteResource#put()");
		return status;
	}

	/**
	 * Add URI parameters to list
	 * 
	 * @return
	 */
	private List<String> getUriParameters() {
		LOG.trace("Start StaticIpRouteResource#getUriParameters()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(vtnName);
		uriParameters.add(vrtName);
		uriParameters.add(staticIpRouteId);
		LOG.trace("Completed StaticIpRouteResource#getUriParameters()");
		return uriParameters;
	}
}
