/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.resources.logical;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.ipc.ClientSession;
import org.opendaylight.vtn.core.ipc.IpcDataUnit;
import org.opendaylight.vtn.core.util.Logger;
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
import org.opendaylight.vtn.javaapi.validation.logical.VtnStationsResourceValidator;

/**
 * The Class VtnStationsResource implements get methods.
 */
@UNCVtnService(path = "/vtnstations")
public class VtnStationsResource extends AbstractResource {
	private static final Logger LOG = Logger
			.getLogger(VtnStationsResource.class.getName());

	/**
	 * Instantiates a new vtn stations resource.
	 */
	public VtnStationsResource() {
		LOG.trace("Start VtnStationsResource#VtnStationsResource()");
		setValidator(new VtnStationsResourceValidator(this));
		LOG.trace("Completed VtnStationsResource#VtnStationsResource()");
	}

	/**
	 * Implementation of get method of VtnStations
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
		LOG.trace("Start VtnStationsResource#get()");
		ClientSession session = null;
		IpcRequestProcessor requestProcessor = null;
		int status = ClientSession.RESP_FATAL;
		try {
			LOG.debug("Start Ipc framework call");
			// Get session from connection pool of UPLL server connections
			session = getConnPool().getSession(
					UncUPLLEnums.UPLL_IPC_CHANNEL_NAME,
					UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
					UncUPLLEnums.ServiceID.UPLL_READ_SVC_ID.ordinal(),
					getExceptionHandler());
			LOG.debug("Session created successfully");
			requestProcessor = new IpcRequestProcessor(session, getSessionID(),
					getConfigID(), getExceptionHandler());
			String opType = VtnServiceJsonConsts.NORMAL;
			if (requestBody.has(VtnServiceJsonConsts.OP)) {
				opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
			}
			if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
				requestProcessor
						.createIpcRequestPacket(
								IpcRequestPacketEnum.KT_VTNSTATION_CONTROLLER_GET_COUNT,
								requestBody, getNullListObject());

			} else {
				requestProcessor.createIpcRequestPacket(
						IpcRequestPacketEnum.KT_VTNSTATION_CONTROLLER_GET,
						requestBody, getNullListObject());
			}
			/*
			 * Set IP address in request packet
			 */
			final IpcDataUnit extraDataUnits[] = new IpcDataUnit[1];
			if (requestBody.has(VtnServiceJsonConsts.IPADDR)) {
				extraDataUnits[0] = IpcDataUnitWrapper
						.setIpcInet4AddressValue(requestBody.get(
								VtnServiceJsonConsts.IPADDR).getAsString());
				requestProcessor.getRequestPacket().setExtraDataUnits(
						extraDataUnits);
			} else if (requestBody.has(VtnServiceJsonConsts.IPV6ADDR)) {
				extraDataUnits[0] = IpcDataUnitWrapper
						.setIpcInet6AddressValue(requestBody.get(
								VtnServiceJsonConsts.IPV6ADDR).getAsString());
				requestProcessor.getRequestPacket().setExtraDataUnits(
						extraDataUnits);
			} else {
				LOG.debug("No need to set IPV4 or IPV6 address");
			}
			/*
			 * Exception case : "op" parameter is coming Show API Update
			 * operation type in case "op" parameter is coming from request body
			 */

			if (opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
				requestProcessor
						.getRequestPacket()
						.setOption1(
								IpcDataUnitWrapper
										.setIpcUint32Value(UncOption1Enum.UNC_OPT1_DETAIL
												.ordinal()));
			}
			requestProcessor.getRequestPacket().setOperation(
					IpcDataUnitWrapper
							.setIpcUint32Value(UncOperationEnum.UNC_OP_READ
									.ordinal()));
			LOG.debug("Request packet created successfully");
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet processed with status" + status);
			/*
			 * Create the Json after getting the key and value structure from
			 * IpcRequestProcessor After getting the response, set the same to
			 * info so that it can be accessed by next call
			 */
			final IpcLogicalResponseFactory responseGenerator = new IpcLogicalResponseFactory();
			setInfo(responseGenerator.getVtnStationResponse(
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
			/*
			 * Handle the case, where error info required to be set for failures
			 * If error cause can be found from UPLL layer then, set the error
			 * according to error code Otherwise set the error as IPC server
			 * error
			 */
			if (status == ClientSession.RESP_FATAL) {
				if (null != requestProcessor.getErrorJson()) {
					setInfo(requestProcessor.getErrorJson());
				} else {
					createErrorInfo(UncCommonEnum.UncResultCode.UNC_SERVER_ERROR
							.getValue());
				}
				status = UncResultCode.UNC_SERVER_ERROR.getValue();
			}
			// destroy the session in all cases
			getConnPool().destroySession(session);
		}
		LOG.trace("Completed VtnStationsResource#get()");
		return status;
	}
}
