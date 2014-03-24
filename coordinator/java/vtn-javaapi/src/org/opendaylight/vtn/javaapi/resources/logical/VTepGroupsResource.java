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
import java.util.Iterator;
import java.util.List;

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.ipc.ClientSession;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.annotation.UNCField;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.init.VtnServiceConfiguration;
import org.opendaylight.vtn.javaapi.init.VtnServiceInitManager;
import org.opendaylight.vtn.javaapi.ipc.IpcRequestProcessor;
import org.opendaylight.vtn.javaapi.ipc.IpcRollback;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcDataUnitWrapper;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcLogicalResponseFactory;
import org.opendaylight.vtn.javaapi.ipc.enums.IpcRequestPacketEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncOperationEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncUPLLEnums;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.validation.logical.VTepGroupResourceValidator;

/**
 * The Class VTepGroupsResource.
 */
/* This class handles post and get methods */

@UNCVtnService(path = "/vtns/{vtn_name}/vtepgroups")
public class VTepGroupsResource extends AbstractResource {
	/** The vtn name. */
	@UNCField("vtn_name")
	private String vtnName;
	private static final Logger LOG = Logger.getLogger(VTepGroupsResource.class
			.getSimpleName());

	public VTepGroupsResource() {
		super();
		LOG.trace("Start VTepGroupsResource#VTunnelsResource()");
		setValidator(new VTepGroupResourceValidator(this));
		LOG.trace("Complete VTepGroupsResource#VTunnelsResource()");
	}

	/**
	 * @return the vtnName
	 */
	public final String getVtnName() {
		return vtnName;
	}

	/**
	 * Implementation of Post method of VTepGroup API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public final int post(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("starts VTepGroupResource#get()");
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
					IpcRequestPacketEnum.KT_VTEP_GRP_CREATE, requestBody,
					getUriParameters(requestBody));
			LOG.debug("Request packet created successfully");
			final IpcRollback ipcRollback = new IpcRollback();
			ipcRollback.pushIpcPacket(requestProcessor.getRequestPacket());
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet processed with status" + status);
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
			if (null != requestBody
					&& requestBody.getAsJsonObject(
							VtnServiceJsonConsts.VTEPGROUP).has(
							VtnServiceJsonConsts.VTEPGROUPMEMBERNAME)
					&& null != requestBody.getAsJsonObject(
							VtnServiceJsonConsts.VTEPGROUP).get(
							VtnServiceJsonConsts.VTEPGROUPMEMBERNAME)) {
				final JsonArray vTepGroupMemberArray = requestBody
						.getAsJsonObject(VtnServiceJsonConsts.VTEPGROUP)
						.get(VtnServiceJsonConsts.VTEPGROUPMEMBERNAME)
						.getAsJsonArray();
				if (vTepGroupMemberArray.size() > 0) {
					requestProcessor.setServiceInfo(
							UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
							UncUPLLEnums.ServiceID.UPLL_EDIT_SVC_ID.ordinal());

					for (final JsonElement jsonElement : vTepGroupMemberArray) {
						requestBody.add(
								VtnServiceJsonConsts.INDEX,
								requestBody.getAsJsonObject(
										VtnServiceJsonConsts.VTEPGROUP).get(
										VtnServiceJsonConsts.VTEPGROUPNAME));
						requestProcessor.createIpcRequestPacket(
								IpcRequestPacketEnum.KT_VTEP_GRP_MEMBER_CREATE,
								jsonElement.getAsJsonObject(),
								getUriParameters(requestBody));
						LOG.debug("Request packet created successfully");
						status = requestProcessor.processIpcRequest();
						if (status == ClientSession.RESP_FATAL) {
							ipcRollback.rollBackIpcRequest(requestProcessor);
							break;
						}
					}

				}
			}
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
		LOG.debug("Complete Ipc framework call");
		LOG.trace("Completed VTepGroupsResource#create()");
		return status;
	}

	/**
	 * Implementation of Get method of VTepGroup API
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
		LOG.trace("Start VTepGroupResource#get()");
		ClientSession session = null;
		JsonObject root = new JsonObject();
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
					IpcRequestPacketEnum.KT_VTEP_GRP_GET, requestBody,
					getUriParameters(requestBody));
			LOG.debug("Request packet created successfully for 1st call");
			/*
			 * operation type will be required to resolve the response type
			 */
			String opType = VtnServiceJsonConsts.NORMAL;
			if (requestBody.has(VtnServiceJsonConsts.OP)) {
				opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
			}
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet processed for 1st call with status"
					+ status);
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
			final IpcLogicalResponseFactory responseGenerator = new IpcLogicalResponseFactory();
			final JsonObject VtepGrp = responseGenerator.getVTepGroupResponse(
					requestProcessor.getIpcResponsePacket(), requestBody,
					VtnServiceJsonConsts.LIST);

			requestProcessor.setServiceInfo(UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
					UncUPLLEnums.ServiceID.UPLL_READ_SVC_ID.ordinal());

			JsonArray vTepGroupArr = null;

			if (VtnServiceJsonConsts.DETAIL.equalsIgnoreCase(opType)) {
				final Iterator<JsonElement> vtepGrpItr = VtepGrp
						.getAsJsonArray(VtnServiceJsonConsts.VTEPGROUPS)
						.iterator();
				vTepGroupArr = new JsonArray();
				final VtnServiceConfiguration configuration = VtnServiceInitManager
						.getConfigurationMap();
				final int max_rep_count = Integer.parseInt(configuration
						.getConfigValue(VtnServiceConsts.MAX_REP_DEFAULT));
				while (vtepGrpItr.hasNext()) {
					JsonObject vTepGroup = (JsonObject) vtepGrpItr.next();
					requestBody.addProperty(VtnServiceJsonConsts.INDEX,
							vTepGroup.get(VtnServiceJsonConsts.VTEPGROUPNAME)
									.getAsString());
					requestProcessor.createIpcRequestPacket(
							IpcRequestPacketEnum.KT_VTEP_GRP_MEMBER_GET,
							requestBody, getUriParameters(requestBody));
					LOG.debug("Request packet created for 2nd call successfully");
					requestProcessor
							.getRequestPacket()
							.setOperation(
									IpcDataUnitWrapper
											.setIpcUint32Value(UncOperationEnum.UNC_OP_READ_SIBLING_BEGIN
													.ordinal()));
					status = requestProcessor.processIpcRequest();
					LOG.debug("Request packet processed for 2nd call with status"
							+ status);
					vTepGroup = responseGenerator.getVtepGroupMembers(
							requestProcessor.getIpcResponsePacket(), vTepGroup);
					final JsonArray memberJsonArray = vTepGroup
							.getAsJsonArray(VtnServiceJsonConsts.MEMBERVTEPS);
					if (memberJsonArray.size() >= max_rep_count) {
						int memberIndex = memberJsonArray.size();
						while (memberIndex >= max_rep_count) {
							memberIndex = memberJsonArray.size();
							final JsonObject memberJson = (JsonObject) vTepGroup
									.getAsJsonArray(
											VtnServiceJsonConsts.MEMBERVTEPS)
									.get(memberIndex - 1);
							requestBody.addProperty(
									VtnServiceJsonConsts.VTEPNAME, memberJson
											.get(VtnServiceJsonConsts.VTEPNAME)
											.getAsString());
							requestProcessor
									.createIpcRequestPacket(
											IpcRequestPacketEnum.KT_VTEP_GRP_MEMBER_GET,
											requestBody,
											getUriParameters(requestBody));
							requestProcessor
									.getRequestPacket()
									.setOperation(
											IpcDataUnitWrapper
													.setIpcUint32Value(UncOperationEnum.UNC_OP_READ_SIBLING
															.ordinal()));
							status = requestProcessor.processIpcRequest();
							vTepGroup = responseGenerator.getVtepGroupMembers(
									requestProcessor.getIpcResponsePacket(),
									vTepGroup);
							if (!vTepGroup
									.has(VtnServiceJsonConsts.MEMBERVTEPS)
									|| vTepGroup
											.getAsJsonArray(VtnServiceJsonConsts.MEMBERVTEPS) != null
									&& vTepGroup.getAsJsonArray(
											VtnServiceJsonConsts.MEMBERVTEPS)
											.size() == 0) {
								break;
							}
							memberIndex++;
						}
					}
					vTepGroupArr.add(vTepGroup);
				}
				root.add(VtnServiceJsonConsts.VTEPGROUPS, vTepGroupArr);
			} else {
				root = VtepGrp;
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
		LOG.trace("Completed VTepGroupsResource#get()");
		return status;
	}

	/**
	 * Add URI parameters to list
	 * 
	 * @return parameter list
	 */
	private List<String> getUriParameters(final JsonObject requestBody) {
		LOG.trace("Start VTepGroupsResource#getUriParameters()");
		LOG.info("request body : " + requestBody);
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(vtnName);
		if (requestBody != null && requestBody.has(VtnServiceJsonConsts.INDEX)) {
			uriParameters.add(requestBody.get(VtnServiceJsonConsts.INDEX)
					.getAsString());
		} else if (null != requestBody
				&& requestBody.has(VtnServiceJsonConsts.VTEPGROUP)
				&& null != requestBody
						.getAsJsonObject(VtnServiceJsonConsts.VTEPGROUP)
				&& requestBody.getAsJsonObject(VtnServiceJsonConsts.VTEPGROUP)
						.has(VtnServiceJsonConsts.VTEPGROUPNAME)) {
			uriParameters.add(requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VTEPGROUP)
					.get(VtnServiceJsonConsts.VTEPGROUPNAME).getAsString());
		}
		LOG.trace("Complete VTepGroupsResource#getUriParameters()");
		return uriParameters;
	}
}
