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

import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.ipc.ClientSession;
import org.opendaylight.vtn.core.ipc.IpcDataUnit;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.annotation.UNCField;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.init.VtnServiceConfiguration;
import org.opendaylight.vtn.javaapi.init.VtnServiceInitManager;
import org.opendaylight.vtn.javaapi.ipc.IpcRequestProcessor;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcDataUnitWrapper;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcPhysicalResponseFactory;
import org.opendaylight.vtn.javaapi.ipc.enums.IpcRequestPacketEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncOperationEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncUPPLEnums;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.validation.physical.DomainLogicalPortResourceValidator;

/**
 * The Class DomainLogicalPortResource implements get method.
 */
@UNCVtnService(path = "/controllers/{controller_id}/domains/{domain_id}/logical_ports/{logical_port_id}")
public class DomainLogicalPortResource extends AbstractResource {
	/**
	 * @return the controllerId
	 */
	@UNCField("controller_id")
	private String controllerId;
	/**
	 * Getter for domainId
	 * 
	 * @return
	 */
	@UNCField("domain_id")
	private String domainId;
	/**
	 * Getter for logicalPortId
	 * 
	 * @return
	 */
	@UNCField("logical_port_id")
	private String logicalPortId;
	private static final Logger LOG = Logger
			.getLogger(DomainLogicalPortResource.class.getName());

	/**
	 * Instantiates a new DomainLogicalPortResource resource.
	 */
	public DomainLogicalPortResource() {
		super();
		LOG.trace("Start DomainLogicalPortResourceValidator()");
		setValidator(new DomainLogicalPortResourceValidator(this));
		LOG.trace("Complete DomainLogicalPortResourceValidator()");
	}

	/**
	 * Implementation of get method of DomainLogicalPort API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public int get(JsonObject queryString) throws VtnServiceException {
		LOG.trace("Start DomainLogicalPortResource#get()");
		ClientSession session = null;
		JsonObject memberLastIndex = null;
		IpcRequestProcessor requestProcessor = null;
		IpcDataUnit[] resp = null;
		int status = ClientSession.RESP_FATAL;
		try {
			LOG.debug("Start Ipc framework call");
			// Get session from connection pool of UPLL server connections
			session = getConnPool().getSession(UncUPPLEnums.UPPL_IPC_CHN_NAME,
					UncUPPLEnums.UPPL_IPC_SVC_NAME,
					UncUPPLEnums.ServiceID.UPPL_SVC_READREQ.ordinal(),
					getExceptionHandler());
			LOG.debug("Session created successfully");
			final IpcPhysicalResponseFactory responseGenerator = new IpcPhysicalResponseFactory();
			String dataType = VtnServiceJsonConsts.STATE;
			if (queryString.has(VtnServiceJsonConsts.TARGETDB)) {
				dataType = queryString.get(VtnServiceJsonConsts.TARGETDB)
						.getAsString();
			}
			requestProcessor = new IpcRequestProcessor(session, getSessionID(),
					getConfigID(), getExceptionHandler());
			requestProcessor.createIpcRequestPacket(
					IpcRequestPacketEnum.KT_LOGICAL_PORT_GET, queryString,
					getUriParameters());
			LOG.debug("Request packet for 1st call created successfully");
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet for 1st call processed with status:"+status);
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
			JsonObject root = responseGenerator.getDomainLogicalPortResponse(
					requestProcessor.getIpcResponsePacket(), queryString,
					VtnServiceJsonConsts.SHOW);
			JsonArray memberArray = null;
			JsonArray memberArrayNew = null;
			if (VtnServiceJsonConsts.STATE.equalsIgnoreCase(dataType)) {
				requestProcessor.setServiceInfo(UncUPPLEnums.UPPL_IPC_SVC_NAME,
						UncUPPLEnums.ServiceID.UPPL_SVC_READREQ.ordinal());
				requestProcessor.createIpcRequestPacket(
						IpcRequestPacketEnum.KT_LOGICAL_PORT_MEMBER_GET,
						queryString, getUriParametersMember(root));
				requestProcessor
						.getRequestPacket()
						.setOperation(
								IpcDataUnitWrapper
										.setIpcUint32Value(UncOperationEnum.UNC_OP_READ_SIBLING_BEGIN
												.ordinal()));
				LOG.debug("Request packet  2nd call created successfully");
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
							UncJavaAPIErrorCode.IPC_SERVER_ERROR
									.getErrorMessage());
				}
				memberArray = responseGenerator
						.getDomainLogicalPortMemberResponse(requestProcessor
								.getIpcResponsePacket());
				int memberIndex = 0;
				VtnServiceConfiguration configuration = VtnServiceInitManager
						.getConfigurationMap();
				int max_rep_count = Integer.parseInt(configuration
						.getConfigValue(VtnServiceConsts.MAX_REP_DEFAULT));
				if (memberArray.size() >= max_rep_count) {
					while (memberIndex >= max_rep_count) {
						memberIndex = memberArray.size();
						memberLastIndex = (JsonObject) memberArray
								.get(memberIndex - 1);
						requestProcessor
								.createIpcRequestPacket(
										IpcRequestPacketEnum.KT_LOGICAL_PORT_MEMBER_GET,
										queryString,
										getUriParametersMemberGreaterThanDefault(
												root, memberLastIndex));
						requestProcessor
								.getRequestPacket()
								.setOperation(
										IpcDataUnitWrapper
												.setIpcUint32Value(UncOperationEnum.UNC_OP_READ_SIBLING
														.ordinal()));
						status = requestProcessor.processIpcRequest();
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
						resp = requestProcessor.getIpcResponsePacket();
						memberArrayNew = responseGenerator
								.getDomainLogicalPortMemberResponse(resp);
						if (null != memberArrayNew
								&& !memberArrayNew.isJsonNull()) {
							memberArray.getAsJsonArray().addAll(memberArrayNew);
						} else {
							break;
						}
						memberIndex++;
					}
				}
				if (null != memberArray) {
					root.get(VtnServiceJsonConsts.LOGICALPORT)
							.getAsJsonObject()
							.add(VtnServiceJsonConsts.MEMBER_PORTS, memberArray);
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
		LOG.trace("Completed DomainLogicalPortResource#get()");
		return status;
	}

	/**
	 * Getter for controller id
	 * 
	 * @return
	 */
	public String getControllerId() {
		return controllerId;
	}

	/**
	 * Getter for domain id
	 * 
	 * @return
	 */
	public String getDomainId() {
		return domainId;
	}

	/**
	 * Getter for port id
	 * 
	 * @return
	 */
	public String getLogicalPortId() {
		return logicalPortId;
	}

	/**
	 * Add URI parameters to list
	 * 
	 * @return parameter list
	 */
	private List<String> getUriParameters() {
		LOG.trace("Starts DomainLogicalPortResource#getUriParameters()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(controllerId);
		uriParameters.add(domainId);
		uriParameters.add(logicalPortId);
		LOG.trace("Completed DomainLogicalPortResource#getUriParameters()");
		return uriParameters;
	}

	/**
	 * Add URI Member parameters to list
	 * 
	 * @return parameter list
	 */
	private List<String> getUriParametersMember(JsonObject root) {
		LOG.trace("Starts DomainLogicalPortResource#getUriParametersMember()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(controllerId);
		uriParameters.add(domainId);
		uriParameters.add(logicalPortId);
		// uriParameters.add(((JsonObject)root.get(VtnServiceJsonConsts.LOGICALPORT)).get(VtnServiceJsonConsts.SWITCHID).getAsString());
		// uriParameters.add(((JsonObject)root.get(VtnServiceJsonConsts.LOGICALPORT)).get(VtnServiceJsonConsts.PORTNAME).getAsString());
		LOG.trace("Completed DomainLogicalPortResource#getUriParametersMember()");
		return uriParameters;
	}

	private List<String> getUriParametersMemberGreaterThanDefault(
			JsonObject logicalPortsJson, final JsonObject memberLastIndex) {
		LOG.trace("Starts DomainLogicalPortResource#getUriParametersMemberGreaterThanDefault()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(controllerId);
		uriParameters.add(domainId);
		uriParameters.add(logicalPortsJson.get(
				VtnServiceJsonConsts.LOGICAL_PORT_ID).getAsString());
		if (memberLastIndex.has(VtnServiceJsonConsts.SWITCHID)) {
			uriParameters.add(memberLastIndex
					.get(VtnServiceJsonConsts.SWITCHID).getAsString());
		} else {
			uriParameters.add(VtnServiceJsonConsts.SWITCHID_NOT_FOUND);
		}
		if (memberLastIndex.has(VtnServiceJsonConsts.PORTNAME)) {
			uriParameters.add(memberLastIndex
					.get(VtnServiceJsonConsts.PORTNAME).getAsString());
		} else {
			uriParameters.add(VtnServiceJsonConsts.PORTID_NOT_FOUND);
		}
		LOG.trace("Completed DomainLogicalPortResource#getUriParametersMemberGreaterThanDefault()");
		return uriParameters;
	}
}
