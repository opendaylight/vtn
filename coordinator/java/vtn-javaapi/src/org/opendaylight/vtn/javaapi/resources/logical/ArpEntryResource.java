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
import org.opendaylight.vtn.javaapi.ipc.enums.UncOption2Enum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncUPLLEnums;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.validation.logical.ArpEntryResourceValidator;

/**
 * The Class ArpEntryResource implements get method.
 * 
 */
@UNCVtnService(path = "/vtns/{vtn_name}/vrouters/{vrt_name}/arpentries")
public class ArpEntryResource extends AbstractResource {
	/** The VTN name. */
	@UNCField("vtn_name")
	private String vtnName;
	/** The VRT name. */
	@UNCField("vrt_name")
	private String vrtName;
	/** The Constant LOG. */
	private static final Logger LOG = Logger.getLogger(ArpEntryResource.class
			.getName());

	/**
	 * Instantiates a new ARP entry resource.
	 */
	public ArpEntryResource() {
		super();
		LOG.trace("Start ArpEntryResource#ArpEntryResource()");
		setValidator(new ArpEntryResourceValidator(this));
		LOG.trace("Complete ArpEntryResource#ArpEntryResource()");
	}

	/**
	 * @return the vtnName
	 */
	public final String getVtnName() {
		return vtnName;
	}

	/**
	 * @return the vrtName
	 */
	public final String getVrtName() {
		return vrtName;
	}

	/**
	 * Implementation of get method of ArpEntry API
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
		LOG.trace("Start ArpEntryResource#get()");
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
			if (requestBody.has(VtnServiceJsonConsts.OP)
					&& requestBody.get(VtnServiceJsonConsts.OP).getAsString()
							.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
				requestProcessor.createIpcRequestPacket(
						IpcRequestPacketEnum.KT_VROUTER_ARP_ENTRY_COUNT,
						requestBody, getUriParameters());
			} else {
				requestProcessor.createIpcRequestPacket(
						IpcRequestPacketEnum.KT_VROUTER_ARP_ENTRY, requestBody,
						getUriParameters());
			}
			requestProcessor.getRequestPacket().setOperation(
					IpcDataUnitWrapper
							.setIpcUint32Value((UncOperationEnum.UNC_OP_READ
									.ordinal())));
			if (requestBody.has(VtnServiceJsonConsts.TYPE)
					&& requestBody.get(VtnServiceJsonConsts.TYPE).getAsString()
							.equalsIgnoreCase(VtnServiceJsonConsts.STATIC)) {
				requestProcessor
						.getRequestPacket()
						.setOption2(
								IpcDataUnitWrapper
										.setIpcUint32Value(UncOption2Enum.UNC_OPT2_ARP_ENTRY_STATIC
												.ordinal()));
			} else if (requestBody.has(VtnServiceJsonConsts.TYPE)
					&& requestBody.get(VtnServiceJsonConsts.TYPE).getAsString()
							.equalsIgnoreCase(VtnServiceJsonConsts.DYNAMIC)) {
				requestProcessor
						.getRequestPacket()
						.setOption2(
								IpcDataUnitWrapper
										.setIpcUint32Value(UncOption2Enum.UNC_OPT2_ARP_ENTRY_DYNAMIC
												.ordinal()));
			}
			LOG.debug("Request packet created successfully");
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet processed with status" + status);
			final IpcLogicalResponseFactory responseGenerator = new IpcLogicalResponseFactory();
			setInfo(responseGenerator.getARPEntryResponse(
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
		LOG.trace("Completed ArpEntryResource#get()");
		return status;
	}

	/**
	 * Add URI parameters to list
	 * 
	 * @return
	 */
	private List<String> getUriParameters() {
		LOG.trace("Start ArpEntryResource#getUriParameters()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(vtnName);
		uriParameters.add(vrtName);
		LOG.trace("Completed ArpEntryResource#getUriParameters()");
		return uriParameters;
	}
}
