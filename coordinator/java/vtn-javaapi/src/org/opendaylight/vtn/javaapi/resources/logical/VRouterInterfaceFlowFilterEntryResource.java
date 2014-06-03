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
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.IpcRequestProcessor;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcDataUnitWrapper;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcLogicalResponseFactory;
import org.opendaylight.vtn.javaapi.ipc.enums.IpcRequestPacketEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncOperationEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncOption1Enum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncUPLLEnums;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.validation.logical.VBridgeFlowFilterEntryResourceValidator;

/**
 * The Class VRouterInterfaceFlowFilterEntryResource implements delete ,put and
 * get methods.
 * 
 */

@UNCVtnService(
		path = "/vtns/{vtn_name}/vrouters/{vrt_name}/interfaces/{if_name}/flowfilters/{ff_type}/flowfilterentries/{seqnum}")
public class VRouterInterfaceFlowFilterEntryResource extends AbstractResource {

	/** The VTN name. */
	@UNCField("vtn_name")
	private String vtnName;
	/** The VRT name. */
	@UNCField("vrt_name")
	private String vrtName;
	/** The if name. */
	@UNCField("if_name")
	private String ifName;
	/** The FF type. */
	@UNCField("ff_type")
	private String ffType;
	/** The Sequence Number. */
	@UNCField("seqnum")
	private String seqnum;

	/** The Constant LOG. */
	private static final Logger LOG = Logger
			.getLogger(VRouterInterfaceFlowFilterEntryResource.class.getName());

	/**
	 * Instantiates a new v router interface flow filter entry resource.
	 */
	public VRouterInterfaceFlowFilterEntryResource() {
		super();
		LOG.trace("Start VRouterInterfaceFlowFilterEntryResource#VRouterInterfaceFlowFilterEntryResource()");
		setValidator(new VBridgeFlowFilterEntryResourceValidator(this));
		LOG.trace("Completed VRouterInterfaceFlowFilterEntryResource#VRouterInterfaceFlowFilterEntryResource()");
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
	 * Gets the VRT name.
	 * 
	 * @return the VRT name
	 */
	public final String getVrtName() {
		return vrtName;
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
	 * Gets the FF type.
	 * 
	 * @return the FF type
	 */
	public final String getFfType() {
		return ffType;
	}

	/**
	 * Gets the Sequence Number.
	 * 
	 * @return the Sequence Number
	 */
	public final String getSeqnum() {
		return seqnum;
	}

	/**
	 * Implementation of Put method of VRouterInterfaceFlowFilterEntry API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public final int put(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start VRouterInterfaceFlowFilterEntryResource#put()");
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
					IpcRequestPacketEnum.KT_VRTIF_FLOWFILTER_ENTRY_UPDATE,
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
		LOG.trace("Completed VRouterInterfaceFlowFilterEntryResource#put()");
		return status;
	}

	/**
	 * Implementation of Delete method of VRouterInterfaceFlowFilterEntry API
	 * 
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public final int delete() throws VtnServiceException {
		LOG.trace("Start VRouterInterfaceFlowFilterEntryResource#delete()");
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
					IpcRequestPacketEnum.KT_VRTIF_FLOWFILTER_ENTRY_DELETE,
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
		LOG.trace("Completed VRouterInterfaceFlowFilterEntryResource#delete()");
		return status;
	}

	/**
	 * Implementation of Put method of VRouterInterfaceFlowFilterEntry API
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
		LOG.trace("Start VRouterInterfaceFlowFilterEntryResource#get()");
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
					IpcRequestPacketEnum.KT_VRTIF_FLOWFILTER_ENTRY_GET,
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
					.getVRouterInterfaceFlowFilterEntryResponse(
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
		LOG.trace("Completed VRouterInterfaceFlowFilterEntryResource#get()");
		return status;
	}

	/**
	 * Add URI parameters to list
	 * 
	 * @return
	 */
	private List<String> getUriParameters() {
		LOG.trace("Start VRouterInterfaceFlowFilterEntryResource#getUriParameters()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(vtnName);
		uriParameters.add(vrtName);
		uriParameters.add(ifName);
		uriParameters.add(ffType);
		uriParameters.add(seqnum);
		LOG.trace("Completed VRouterInterfaceFlowFilterEntryResource#getUriParameters()");
		return uriParameters;
	}
}
