/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.resources.physical;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.ipc.ClientSession;
import org.opendaylight.vtn.core.ipc.IpcDataUnit;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.annotation.UNCField;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceIpcConsts;
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
 * The Class DomainLogicalPortsResource implements get method.
 */
@UNCVtnService(
		path = "/controllers/{controller_id}/domains/{domain_id}/logical_ports")
public class DomainLogicalPortsResource extends AbstractResource {
	/**
	 * @return the controllerId
	 */
	@UNCField("controller_id")
	private String controllerId;
	/**
	 * @return the domainId
	 */
	@UNCField("domain_id")
	private String domainId;
	private static final Logger LOG = Logger
			.getLogger(DomainLogicalPortsResource.class.getName());

	/**
	 * Instantiates a new DomainLogicalPortsResource resource.
	 */
	public DomainLogicalPortsResource() {
		super();
		LOG.trace("Start DomainLogicalPortsResource#DomainLogicalPortsResource()");
		setValidator(new DomainLogicalPortResourceValidator(this));
		LOG.trace("Complete DomainLogicalPortsResource#DomainLogicalPortsResource()");

	}

	public final String getcontrollerId() {
		return controllerId;
	}

	public final String getdomainId() {
		return domainId;
	}

	/**
	 * Implementation of get method of DomainLogicalPorts API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public final int get(final JsonObject queryString)
			throws VtnServiceException {
		LOG.trace("Starts DomainLogicalPortsResource#get()");
		ClientSession session = null;
		IpcRequestProcessor requestProcessor = null;
		JsonObject memberLastIndex = null;
		int status = ClientSession.RESP_FATAL;
		IpcDataUnit[] resp = null;
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
					getUriParameters(queryString));
			LOG.debug("Request packet for 1st created successfully");

			if (!queryString.has(VtnServiceJsonConsts.LOGICAL_PORT_ID)) {
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
				resp = requestProcessor.getIpcResponsePacket();
				JsonObject root = responseGenerator
						.getDomainLogicalPortResponse(resp, queryString);
				if (root.get(VtnServiceJsonConsts.LOGICALPORTS).isJsonArray()) {
					JsonArray domainLogicalPortArray = null;
					domainLogicalPortArray = root.getAsJsonArray(VtnServiceJsonConsts.LOGICALPORTS);
					
					root = getResponseJsonArrayPhysical(
							queryString,
							requestProcessor,
							responseGenerator,
							domainLogicalPortArray,
							VtnServiceJsonConsts.LOGICALPORTS,
							VtnServiceJsonConsts.LOGICAL_PORT_ID,
							IpcRequestPacketEnum.KT_LOGICAL_PORT_GET,
							getUriParameters(queryString),
							VtnServiceIpcConsts.GET_DOMAIN_LOGICAL_PORT_RESPONSE);
				}
				
				String opType = VtnServiceJsonConsts.NORMAL;
				if (queryString.has(VtnServiceJsonConsts.OP)) {
					opType = queryString.get(VtnServiceJsonConsts.OP)
							.getAsString();
				}
				final JsonArray logicalPortsArray = new JsonArray();
				JsonArray memberArray = null;
				JsonArray memberArrayNew = null;
				if (VtnServiceJsonConsts.STATE.equalsIgnoreCase(dataType)
						&& VtnServiceJsonConsts.DETAIL.equalsIgnoreCase(opType)) {
					final Iterator<JsonElement> logicalPortIterator = root
							.get(VtnServiceJsonConsts.LOGICALPORTS)
							.getAsJsonArray().iterator();
					requestProcessor.setServiceInfo(
							UncUPPLEnums.UPPL_IPC_SVC_NAME,
							UncUPPLEnums.ServiceID.UPPL_SVC_READREQ.ordinal());
					while (logicalPortIterator.hasNext()) {
						final JsonObject logicalPortsJson = (JsonObject) logicalPortIterator
								.next();
						requestProcessor
								.createIpcRequestPacket(
										IpcRequestPacketEnum.KT_LOGICAL_PORT_MEMBER_GET,
										queryString,
										getUriParametersMember(logicalPortsJson));
						requestProcessor
								.getRequestPacket()
								.setOperation(
										IpcDataUnitWrapper
												.setIpcUint32Value(UncOperationEnum.UNC_OP_READ_SIBLING_BEGIN
														.ordinal()));
						LOG.debug("Request packet for 2nd call created successfully");
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
						resp = requestProcessor.getIpcResponsePacket();
						memberArray = responseGenerator
								.getDomainLogicalPortMemberResponse(resp);
						int memberIndex = 0;
						memberIndex = memberArray.size();
						if (memberArray.size() > 0) {
							while (memberIndex > 0) {
								memberIndex = memberArray.size();
								memberLastIndex = (JsonObject) memberArray
										.get(memberIndex - 1);
								requestProcessor
										.createIpcRequestPacket(
												IpcRequestPacketEnum.KT_LOGICAL_PORT_MEMBER_GET,
												queryString,
												getUriParametersMemberGreaterThanDefault(
														logicalPortsJson,
														memberLastIndex));
								requestProcessor
										.getRequestPacket()
										.setOperation(
												IpcDataUnitWrapper
														.setIpcUint32Value(UncOperationEnum.UNC_OP_READ_SIBLING
																.ordinal()));
								status = requestProcessor.processIpcRequest();
								if (status == ClientSession.RESP_FATAL) {
									throw new VtnServiceException(
											Thread.currentThread()
													.getStackTrace()[1]
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
									memberArray.getAsJsonArray().addAll(
											memberArrayNew);
								} else {
									break;
								}
								memberIndex = memberArrayNew.size();
							}
						}
						//get neighbor information
						getNeighbor(queryString, responseGenerator,
										requestProcessor, memberArray, logicalPortsJson);
						//get logical port boundary informations
						getBoundary(queryString, responseGenerator, requestProcessor, logicalPortsJson);
						logicalPortsArray.add(logicalPortsJson);
					}
					root.add(VtnServiceJsonConsts.LOGICALPORTS,
							logicalPortsArray);
				}
				setInfo(root);
			} else {
				requestProcessor.getRequestPacket().setOperation(
						IpcDataUnitWrapper
								.setIpcUint32Value(UncOperationEnum.UNC_OP_READ
										.ordinal()));
				status = requestProcessor.processIpcRequest();
				LOG.debug("Request packet for 1st call processed with status:"
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
				final JsonObject root = responseGenerator
						.getDomainLogicalPortResponse(
								requestProcessor.getIpcResponsePacket(),
								queryString);
				JsonArray memberArray = null;
				JsonArray memberArrayNew = null;
				String opType = VtnServiceJsonConsts.NORMAL;
				if (queryString.has(VtnServiceJsonConsts.OP)) {
					opType = queryString.get(VtnServiceJsonConsts.OP)
							.getAsString();
				}
				if (VtnServiceJsonConsts.STATE.equalsIgnoreCase(dataType)
						&& opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)
						&& (root.get(VtnServiceJsonConsts.LOGICALPORTS)
								.getAsJsonArray().size() != 0)) {
					requestProcessor.setServiceInfo(
							UncUPPLEnums.UPPL_IPC_SVC_NAME,
							UncUPPLEnums.ServiceID.UPPL_SVC_READREQ.ordinal());
					requestProcessor.createIpcRequestPacket(
							IpcRequestPacketEnum.KT_LOGICAL_PORT_MEMBER_GET,
							queryString,
							getUriParametersMemberShow(queryString));
					requestProcessor
							.getRequestPacket()
							.setOperation(
									IpcDataUnitWrapper
											.setIpcUint32Value(UncOperationEnum.UNC_OP_READ_SIBLING_BEGIN
													.ordinal()));
					LOG.debug("Request packet  2nd call created successfully");
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
					memberArray = responseGenerator
							.getDomainLogicalPortMemberResponse(requestProcessor
									.getIpcResponsePacket());
					int memberIndex = 0;
					memberIndex = memberArray.size();
					JsonObject logicalPort = (JsonObject)root.getAsJsonArray(
								VtnServiceJsonConsts.LOGICALPORTS).get(0);
					if (memberArray.size() > 0) {
						while (memberIndex > 0) {
							memberIndex = memberArray.size();
							memberLastIndex = (JsonObject) memberArray
									.get(memberIndex - 1);
							requestProcessor
									.createIpcRequestPacket(
											IpcRequestPacketEnum.KT_LOGICAL_PORT_MEMBER_GET,
											queryString,
											getUriParametersMemberGreaterThanDefault(
													logicalPort, memberLastIndex));
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
								memberArray.getAsJsonArray().addAll(
										memberArrayNew);
							} else {
								break;
							}
							memberIndex = memberArrayNew.size();
						}
					}
					if (null != memberArray) {
						//get neighbor information
						getNeighbor(queryString, responseGenerator,
								requestProcessor, memberArray, logicalPort);
					}
					//get logical port boundary informations
					getBoundary(queryString, responseGenerator, requestProcessor, logicalPort);
					final JsonArray resultJsonArray = new JsonArray();
					resultJsonArray.add(logicalPort);
					root.add(VtnServiceJsonConsts.LOGICALPORTS, resultJsonArray);
				}
				setInfo(root);
			}
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
		LOG.trace("Completed LogicalPortsResource#get()");
		return status;
	}

	/**
	 * Getter for controller id
	 * 
	 * @return
	 */
	public final String getControllerId() {
		return controllerId;
	}

	/**
	 * Getter for domain id
	 * 
	 * @return
	 */
	public final String getDomainId() {
		return domainId;
	}

	/**
	 * Add URI parameters to list
	 * 
	 * @return parameter list
	 */
	private List<String> getUriParameters(final JsonObject requestBody) {
		LOG.trace("Starts LogicalPortsResource#getUriParameters()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(controllerId);
		uriParameters.add(domainId);
		if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.LOGICAL_PORT_ID)) {
			uriParameters.add(requestBody.get(
					VtnServiceJsonConsts.LOGICAL_PORT_ID).getAsString());
		} else if (requestBody != null
				&& requestBody.has(VtnServiceJsonConsts.INDEX)) {
			uriParameters.add(requestBody.get(VtnServiceJsonConsts.INDEX)
					.getAsString());
		}
		LOG.trace("Completed LogicalPortsResource#getUriParameters()");
		return uriParameters;
	}

	/**
	 * Add URI Member parameters to list
	 * 
	 * @return parameter list
	 */
	private List<String> getUriParametersMember(
			final JsonObject logicalPortsJson) {
		LOG.trace("Starts LogicalPortsResource#getUriParametersMember()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(controllerId);
		uriParameters.add(domainId);
		uriParameters.add(logicalPortsJson.get(
				VtnServiceJsonConsts.LOGICAL_PORT_ID).getAsString());

		LOG.trace("Completed LogicalPortsResource#getUriParametersMember()");
		return uriParameters;
	}

	/**
	 * Add URI Member parameters to Show
	 * 
	 * @return parameter list
	 */
	private List<String> getUriParametersMemberShow(
			final JsonObject logicalPortsJson) {
		LOG.trace("Starts DomainLogicalPortResource#getUriParametersMember()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(controllerId);
		uriParameters.add(domainId);
		if (logicalPortsJson.has(VtnServiceJsonConsts.LOGICAL_PORT_ID)) {
			uriParameters.add(logicalPortsJson.get(
					VtnServiceJsonConsts.LOGICAL_PORT_ID).getAsString());
		}
		LOG.trace("Completed DomainLogicalPortResource#getUriParametersMember()");
		return uriParameters;
	}

	private List<String>
			getUriParametersMemberGreaterThanDefault(
					final JsonObject logicalPortsJson,
					final JsonObject memberLastIndex) {
		LOG.trace("Starts LogicalPortsResource#getUriParametersMemberGreaterThanDefault()");
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
		LOG.trace("Completed LogicalPortsResource#getUriParametersMemberGreaterThanDefault()");
		return uriParameters;
	}

	private void getBoundary(final JsonObject queryString, IpcPhysicalResponseFactory responseGenerator,
					IpcRequestProcessor requestProcessor, final JsonObject logicalPortJson)
					throws VtnServiceException {
		int status = ClientSession.RESP_FATAL;

		requestProcessor.createIpcRequestPacket(
				IpcRequestPacketEnum.KT_LOGICAL_PORT_BOUNDARY_GET,
				queryString, getUriParametersMember(logicalPortJson));
		requestProcessor.getRequestPacket()
						.setOperation(IpcDataUnitWrapper
							.setIpcUint32Value(UncOperationEnum.UNC_OP_READ.ordinal()));
		LOG.debug("Request packet for boundary call created successfully");
		status = requestProcessor.processIpcRequest();
		LOG.debug("Request packet boundary call processed with status:" + status);
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
		JsonObject boundary = responseGenerator.getDomainLogicalPortBoundaryResponse(
												requestProcessor.getIpcResponsePacket());
		if (boundary != null) {
			logicalPortJson.add(VtnServiceJsonConsts.BOUNDARY, boundary);
		}
	}

	/**/
	private void getNeighbor(final JsonObject queryString, IpcPhysicalResponseFactory responseGenerator,
			IpcRequestProcessor requestProcessor, final JsonArray memberArray,
			final JsonObject logicalPortJson) throws VtnServiceException {
		int status = ClientSession.RESP_FATAL;
		JsonArray tmpMembArray = new JsonArray();

		for (int neighborIndex = 0; neighborIndex < memberArray.size(); neighborIndex++) {
			JsonObject member = (JsonObject) memberArray.get(neighborIndex);
			requestProcessor.createIpcRequestPacket(
					IpcRequestPacketEnum.KT_LOGICAL_PORT_NEIGHBOR_GET,
					queryString, getUriParametersMemberGreaterThanDefault(
					logicalPortJson, member));
			requestProcessor.getRequestPacket()
					.setOperation(IpcDataUnitWrapper
						.setIpcUint32Value(UncOperationEnum.UNC_OP_READ.ordinal()));
			LOG.debug("Request packet for neighbor call created successfully");	
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet neighbor call processed with status:" + status);
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
			JsonObject neighbor = responseGenerator.getDomainLogicalPortNeighborResponse(
												requestProcessor.getIpcResponsePacket());
			if (neighbor != null) {
				member.add(VtnServiceJsonConsts.NEIGHBOR, neighbor);
			}
			tmpMembArray.add(member);
		}

		if (tmpMembArray.size() > 0) {
			logicalPortJson.add(VtnServiceJsonConsts.MEMBERPORTS, tmpMembArray);
		}
	}
}
