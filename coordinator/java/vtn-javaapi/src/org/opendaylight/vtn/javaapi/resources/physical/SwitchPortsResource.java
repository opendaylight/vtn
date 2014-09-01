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

import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.ipc.ClientSession;
import org.opendaylight.vtn.core.ipc.IpcStruct;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.annotation.UNCField;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceIpcConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.IpcRequestProcessor;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcDataUnitWrapper;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcPhysicalResponseFactory;
import org.opendaylight.vtn.javaapi.ipc.enums.IpcRequestPacketEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncOperationEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncOption1Enum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncPhysicalStructIndexEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncStructEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncStructIndexEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncUPPLEnums;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.validation.physical.SwitchPortResourceValidator;

/**
 *
 */
@UNCVtnService(path = "/controllers/{controller_id}/switches/{switch_id}/ports")
public class SwitchPortsResource extends AbstractResource {
	/**
 *
 */
	private static final Logger LOG = Logger
			.getLogger(SwitchPortsResource.class.getName());

	/** The controller Id. */
	@UNCField("controller_id")
	private String controllerId;
	/** The Switch Id. */
	@UNCField("switch_id")
	private String switchId;

	/**
	 * Instantiates a new Port Resource.
	 */
	public SwitchPortsResource() {
		LOG.trace("Start SwitchPortsResource#SwitchPortsResource()");
		setValidator(new SwitchPortResourceValidator(this));
		LOG.trace("Completed SwitchPortsResource#SwitchPortsResource()");
	}

	/**
	 * Get Switch Port list information by calling get API of SwitchPorts
	 * resources.
	 * 
	 * @param requestBody
	 *            , handles request Json.
	 * @throws VtnServiceException
	 *             , In case any Exception is thrown.
	 * @return result code ,as an integer for status of opetration
	 */
	@Override
	public final int get(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Starts SwitchPortsResource#get()");
		ClientSession session = null;
		IpcRequestProcessor requestProcessor = null;
		JsonObject root = null;
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
					getUriParameters(requestBody));

			LOG.debug("Request Packet 1st call created successfully");

			final IpcPhysicalResponseFactory responseGenerator = new IpcPhysicalResponseFactory();
			JsonArray switchPortArray = null;
			IpcRequestPacketEnum ipcRequestPacketEnum = IpcRequestPacketEnum.KT_PORT_GET;
            if (requestBody.has(VtnServiceJsonConsts.OP)
                    && requestBody.get(VtnServiceJsonConsts.OP).getAsString()
                            .equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)) {
                requestProcessor
                        .getRequestPacket()
                        .setOption1(
                                IpcDataUnitWrapper
                                        .setIpcUint32Value(UncOption1Enum.UNC_OPT1_DETAIL
                                                .ordinal()));
                ipcRequestPacketEnum = IpcRequestPacketEnum.KT_PORT_GET_DETAIL;
            }
			if (!requestBody.has(VtnServiceJsonConsts.PORTNAME)) {

				if (requestBody.has(VtnServiceJsonConsts.PORT_ID)) {
					final IpcStruct valPortStruct = new IpcStruct(
							UncStructEnum.ValPort.getValue());
					valPortStruct
							.set(VtnServiceIpcConsts.VALID,
									UncPhysicalStructIndexEnum.UpplValPortIndex.kIdxPortNumber
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
													.ordinal()));

					valPortStruct.set(VtnServiceIpcConsts.PORT_NUMBER,
							IpcDataUnitWrapper.setIpcUint32Value(requestBody
									.get(VtnServiceJsonConsts.PORT_ID)
									.getAsInt()));

					final IpcStruct valPortStStruct = new IpcStruct(
							UncStructEnum.ValPortSt.getValue());
					valPortStStruct
							.set(VtnServiceIpcConsts.VALID,
									UncPhysicalStructIndexEnum.UpplValPortStIndex.kIdxPortSt
											.ordinal(),
									IpcDataUnitWrapper
											.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
													.ordinal()));

					valPortStStruct.set(VtnServiceJsonConsts.PORT,
							valPortStruct);

					requestProcessor.getRequestPacket().setValStruct(
							valPortStStruct);
				}
				status = requestProcessor.processIpcRequest();
				LOG.debug("Request packet 1st call processed with status:"
						+ status);
				if (status == ClientSession.RESP_FATAL) {
					throw new VtnServiceException(
							Thread.currentThread().getStackTrace()[1]
									.getClassName()
									+ VtnServiceConsts.HYPHEN
									+ Thread.currentThread().getStackTrace()[1]
											.getMethodName(),
							UncJavaAPIErrorCode.IPC_SERVER_ERROR.getErrorCode(),
							UncJavaAPIErrorCode.IPC_SERVER_ERROR
									.getErrorMessage());
				}

				root = responseGenerator.getSwitchPortResponse(
						requestProcessor.getIpcResponsePacket(), requestBody);

				//if port_id is specified,no need to send requestion to UPPL many times
				if (!requestBody.has(VtnServiceJsonConsts.PORT_ID)) {
					if (root.get(VtnServiceJsonConsts.PORTS).isJsonArray()) {
						switchPortArray = root
								.getAsJsonArray(VtnServiceJsonConsts.PORTS);

						root = getResponseJsonArrayPhysical(
								requestBody,
								requestProcessor,
								responseGenerator,
								switchPortArray,
								VtnServiceJsonConsts.PORTS,
								VtnServiceJsonConsts.PORTNAME,
								ipcRequestPacketEnum,
								getUriParameters(requestBody),
								VtnServiceIpcConsts.GET_SWITCH_PORT_INTERFACE_RESPONSE);
					}
				}
			} else {
				requestProcessor.getRequestPacket().setOperation(
						IpcDataUnitWrapper
								.setIpcUint32Value(UncOperationEnum.UNC_OP_READ
										.ordinal()));
				status = requestProcessor.processIpcRequest();
				LOG.debug("Request packet 1st call processed with status:"
						+ status);
				if (status == ClientSession.RESP_FATAL) {
					throw new VtnServiceException(
							Thread.currentThread().getStackTrace()[1]
									.getClassName()
									+ VtnServiceConsts.HYPHEN
									+ Thread.currentThread().getStackTrace()[1]
											.getMethodName(),
							UncJavaAPIErrorCode.IPC_SERVER_ERROR.getErrorCode(),
							UncJavaAPIErrorCode.IPC_SERVER_ERROR
									.getErrorMessage());
				}

				root = responseGenerator.getSwitchPortResponse(
						requestProcessor.getIpcResponsePacket(), requestBody);
			}

			String opType = VtnServiceJsonConsts.NORMAL;
			if (requestBody.has(VtnServiceJsonConsts.OP)) {
				opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
			}

			if (opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)
					|| opType.equalsIgnoreCase(VtnServiceJsonConsts.INFO)) {
				requestProcessor.setServiceInfo(UncUPPLEnums.UPPL_IPC_SVC_NAME,
						UncUPPLEnums.ServiceID.UPPL_SVC_READREQ.ordinal());
				switchPortArray = root
						.getAsJsonArray(VtnServiceJsonConsts.PORTS);
				for (int index = 0; index < switchPortArray.size(); index++) {
					final JsonObject switchPort = (JsonObject) switchPortArray
							.get(index);
					requestBody.addProperty(VtnServiceJsonConsts.INDEX,
							switchPort.get(VtnServiceJsonConsts.PORTNAME)
									.getAsString());
					requestProcessor.createIpcRequestPacket(
							IpcRequestPacketEnum.KT_PORT_GET_MEMBER,
							requestBody, getUriParameters(requestBody));
					requestProcessor
							.getRequestPacket()
							.setOperation(
									IpcDataUnitWrapper
											.setIpcUint32Value(UncOperationEnum.UNC_OP_READ
													.ordinal()));
					LOG.debug("Request Packet for 2nd created successfully");
					status = requestProcessor.processIpcRequest();
					LOG.debug("Request packet 2nd call processed with status:"
							+ status);
					if (status == ClientSession.RESP_FATAL) {
						throw new VtnServiceException(
								Thread.currentThread().getStackTrace()[1]
										.getClassName()
										+ VtnServiceConsts.HYPHEN
										+ Thread.currentThread()
												.getStackTrace()[1]
												.getMethodName(),
								UncJavaAPIErrorCode.IPC_SERVER_ERROR
										.getErrorCode(),
								UncJavaAPIErrorCode.IPC_SERVER_ERROR
										.getErrorMessage());
					}
					responseGenerator.getSwitchPortMemberResponse(
							requestProcessor.getIpcResponsePacket(),
							switchPort, VtnServiceJsonConsts.LIST);
				}
			}
			setInfo(root);
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
		LOG.trace("Complete SwitchPortsResource#get()");
		return status;
	}

	/**
	 * Add URI parameters to list.
	 * 
	 * @param requestBody
	 *            , handles request Json
	 * @return List , as list of strings consisting all URI paramters.
	 */
	private List<String> getUriParameters(final JsonObject requestBody) {
		LOG.trace("Starts SwitchPortsResource#getUriParameters()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(controllerId);
		uriParameters.add(switchId);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.PORTNAME)) {
			uriParameters.add(requestBody.get(VtnServiceJsonConsts.PORTNAME)
					.getAsString());
		} else if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.INDEX)) {
			uriParameters.add(requestBody.get(VtnServiceJsonConsts.INDEX)
					.getAsString());
		}
		LOG.trace("Complete SwitchPortsResource#getUriParameters()");
		return uriParameters;
	}

	/**
	 * @return the controllerId
	 */
	public final String getControllerId() {
		return controllerId;
	}

	/**
	 * @return the controllerId
	 */
	public final String getSwitchId() {
		return switchId;
	}

}
