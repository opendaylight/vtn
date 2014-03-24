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

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonNull;
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
 * The Class VTepGroupResource.
 */
/* This class handles delete and get methods */
@UNCVtnService(path = "/vtns/{vtn_name}/vtepgroups/{vtepgroup_name}")
public class VTepGroupResource extends AbstractResource {
	/** The vtn name. */
	@UNCField("vtn_name")
	private String vtnName;
	@UNCField("vtepgroup_name")
	private String vtepgroupName;
	private static final Logger LOG = Logger.getLogger(VTepGroupResource.class
			.getSimpleName());

	public VTepGroupResource() {
		super();
		LOG.trace("Start VTepGroupResource#VTepGroupResource()");
		setValidator(new VTepGroupResourceValidator(this));
		LOG.trace("Complete VTepGroupResource#VTepGroupResource()");
	}

	/**
	 * @return the vtnName
	 */
	public final String getVtnName() {
		return vtnName;
	}

	/**
	 * @return the vtepgroupName
	 */
	public final String getVtepgroupName() {
		return vtepgroupName;
	}

	/**
	 * Implementation of Put method of VTepGroup API
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
		LOG.trace("Starts VTepGroupResource#get()");
		ClientSession session = null;
		IpcRequestProcessor requestProcessor = null;
		JsonObject vTepGroup = null;
		int status = ClientSession.RESP_FATAL;
		final JsonObject getRequestBody = new JsonObject();
		// pending query for the same will update once query will be resolved
		getRequestBody.addProperty(VtnServiceJsonConsts.TARGETDB,
				VtnServiceJsonConsts.STATE);
		status = get(getRequestBody);
		LOG.debug("Request packet processed with status" + status);
		if (status == VtnServiceConsts.RESPONSE_SUCCESS) {
			final JsonObject root = getInfo();
			try {
				LOG.debug("Start Ipc framework call");
				// Deleting all the member data.
				LOG.debug("Deletion of member data started..");
				final IpcRollback ipcRollback = new IpcRollback();
				session = getConnPool().getSession(
						UncUPLLEnums.UPLL_IPC_CHANNEL_NAME,
						UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
						UncUPLLEnums.ServiceID.UPLL_EDIT_SVC_ID.ordinal(),
						getExceptionHandler());
				LOG.debug("Session created successfully");
				// getting all the member data along with vTepGroup
				requestProcessor = new IpcRequestProcessor(session,
						getSessionID(), getConfigID(), getExceptionHandler());

				if (null != root && root.has(VtnServiceJsonConsts.VTEPGROUP)
						&& null != root.get(VtnServiceJsonConsts.VTEPGROUP)) {
					if (root.getAsJsonObject(VtnServiceJsonConsts.VTEPGROUP)
							.has(VtnServiceJsonConsts.VTEPGROUPMEMBERNAME)
							&& null != root.getAsJsonObject(
									VtnServiceJsonConsts.VTEPGROUP).get(
									VtnServiceJsonConsts.VTEPGROUPMEMBERNAME)) {
						final JsonArray vTepGroupMemberArray = root
								.getAsJsonObject(VtnServiceJsonConsts.VTEPGROUP)
								.get(VtnServiceJsonConsts.VTEPGROUPMEMBERNAME)
								.getAsJsonArray();
						if (vTepGroupMemberArray.size() > 0) {
							for (final JsonElement jsonElement : vTepGroupMemberArray) {
								status = ClientSession.RESP_FATAL;
								requestProcessor.setServiceInfo(
										UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
										UncUPLLEnums.ServiceID.UPLL_EDIT_SVC_ID
												.ordinal());
								requestProcessor
										.createIpcRequestPacket(
												IpcRequestPacketEnum.KT_VTEP_GRP_MEMBER_DELETE,
												jsonElement.getAsJsonObject(),
												getUriParameters(requestBody));
								LOG.debug("Request packet deleted successfully");
								ipcRollback.pushIpcPacket(requestProcessor
										.getRequestPacket());
								status = requestProcessor.processIpcRequest();
								LOG.debug("Request packet processed with status"
										+ status);
								if (status == ClientSession.RESP_FATAL) {
									ipcRollback
											.rollBackIpcRequest(requestProcessor);
									break;
								}
							}
						}
					}
				}
				LOG.debug("Deletion of member data finished successfully..");
				// creating Data members along with VtepGroup
				// status = ClientSession.RESP_FATAL;
				if (null != requestBody
						&& requestBody.has(VtnServiceJsonConsts.VTEPGROUP)) {
					vTepGroup = requestBody
							.getAsJsonObject(VtnServiceJsonConsts.VTEPGROUP);
				}
				LOG.debug("Creation of member data started..");
				if (null != vTepGroup
						&& vTepGroup
								.has(VtnServiceJsonConsts.VTEPGROUPMEMBERNAME)
						&& null != vTepGroup
								.get(VtnServiceJsonConsts.VTEPGROUPMEMBERNAME)) {
					final JsonArray vTepGroupMemberArray = vTepGroup.get(
							VtnServiceJsonConsts.VTEPGROUPMEMBERNAME)
							.getAsJsonArray();
					if (vTepGroupMemberArray.size() > 0) {
						for (final JsonElement jsonElement : vTepGroupMemberArray) {
							status = ClientSession.RESP_FATAL;
							requestProcessor.setServiceInfo(
									UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
									UncUPLLEnums.ServiceID.UPLL_EDIT_SVC_ID
											.ordinal());
							requestProcessor
									.createIpcRequestPacket(
											IpcRequestPacketEnum.KT_VTEP_GRP_MEMBER_CREATE,
											jsonElement.getAsJsonObject(),
											getUriParameters(requestBody));
							ipcRollback.pushIpcPacket(requestProcessor
									.getRequestPacket());
							LOG.debug("Request packet created successfully");
							status = requestProcessor.processIpcRequest();
							LOG.debug("Request packet processed with status"
									+ status);
							if (status == ClientSession.RESP_FATAL) {
								ipcRollback
										.rollBackIpcRequest(requestProcessor);
								break;
							}
						}
					}
				}
				LOG.debug("Creation of member data finished successfully..");
			} catch (final VtnServiceException e) {
				getExceptionHandler()
						.raise(Thread.currentThread().getStackTrace()[1]
								.getClassName()
								+ VtnServiceConsts.HYPHEN
								+ Thread.currentThread().getStackTrace()[1]
										.getMethodName(),
								UncJavaAPIErrorCode.IPC_SERVER_ERROR
										.getErrorCode(),
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

			LOG.debug("Complete Ipc framework call");
			LOG.trace("Completed VTepGroupResource#delete()");
		}
		setInfo(null);
		return status;
	}

	/**
	 * Implementation of Delete method of VTep Group Resource API
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public final int delete() throws VtnServiceException {
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
					IpcRequestPacketEnum.KT_VTEP_GRP_DELETE,
					getNullJsonObject(), getUriParameters(getNullJsonObject()));
			LOG.debug("Request packet created successfully");
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet processed with status" + status);
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
		LOG.trace("Completed VTepGroupResource#delete()");
		return status;
	}

	/**
	 * Implementation of get method of Vtep group Interface API
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
		LOG.trace("starts VTepGroupResource#get()");
		ClientSession session = null;
		final JsonObject root = new JsonObject();
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
			LOG.debug("Request packet created successfully");
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
			JsonObject vtepGroup = responseGenerator.getVTepGroupResponse(
					requestProcessor.getIpcResponsePacket(), requestBody,
					VtnServiceJsonConsts.SHOW);
			if (!(vtepGroup.get(VtnServiceJsonConsts.VTEPGROUP) instanceof JsonNull)) {
				requestProcessor.setServiceInfo(
						UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
						UncUPLLEnums.ServiceID.UPLL_READ_SVC_ID.ordinal());

				requestProcessor.createIpcRequestPacket(
						IpcRequestPacketEnum.KT_VTEP_GRP_MEMBER_GET,
						requestBody, getUriParameters(requestBody));
				LOG.debug("Request packet created successfully for 2nd call");
				requestProcessor
						.getRequestPacket()
						.setOperation(
								IpcDataUnitWrapper
										.setIpcUint32Value(UncOperationEnum.UNC_OP_READ_SIBLING_BEGIN
												.ordinal()));
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
							UncJavaAPIErrorCode.IPC_SERVER_ERROR
									.getErrorMessage());
				}
				LOG.debug("Request packet created successfully");
				vtepGroup = responseGenerator
						.getVtepGroupMembers(
								requestProcessor.getIpcResponsePacket(),
								vtepGroup
										.getAsJsonObject(VtnServiceJsonConsts.VTEPGROUP));
				final JsonArray memberJsonArray = vtepGroup
						.getAsJsonArray(VtnServiceJsonConsts.MEMBERVTEPS);
				final VtnServiceConfiguration configuration = VtnServiceInitManager
						.getConfigurationMap();
				final int max_rep_count = Integer.parseInt(configuration
						.getConfigValue(VtnServiceConsts.MAX_REP_DEFAULT));
				int memberIndex = memberJsonArray.size();
				if (memberJsonArray.size() >= max_rep_count) {
					while (memberIndex >= max_rep_count) {
						memberIndex = memberJsonArray.size();
						final JsonObject memberJson = (JsonObject) vtepGroup
								.getAsJsonArray(
										VtnServiceJsonConsts.MEMBERVTEPS).get(
										memberIndex - 1);
						requestProcessor.createIpcRequestPacket(
								IpcRequestPacketEnum.KT_VTEP_GRP_MEMBER_GET,
								memberJson, getUriParameters(requestBody));
						requestProcessor
								.getRequestPacket()
								.setOperation(
										IpcDataUnitWrapper
												.setIpcUint32Value(UncOperationEnum.UNC_OP_READ_SIBLING
														.ordinal()));
						status = requestProcessor.processIpcRequest();
						vtepGroup = responseGenerator.getVtepGroupMembers(
								requestProcessor.getIpcResponsePacket(),
								vtepGroup);
						if (!vtepGroup.has(VtnServiceJsonConsts.MEMBERVTEPS)
								|| vtepGroup
										.getAsJsonArray(VtnServiceJsonConsts.MEMBERVTEPS) != null
								&& vtepGroup.getAsJsonArray(
										VtnServiceJsonConsts.MEMBERVTEPS)
										.size() == 0) {
							break;
						}
						memberIndex++;
					}
				}
				root.add(VtnServiceJsonConsts.VTEPGROUP, vtepGroup);
				setInfo(root);
			} else {
				setInfo(vtepGroup);
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
		LOG.trace("Completed VTepGroupResource#get()");
		return status;
	}

	/**
	 * Add URI parameters to list
	 * 
	 * @return parameter list
	 */
	private List<String> getUriParameters(final JsonObject requestBody) {
		LOG.trace("Start VTepGroupResource#getUriParameters()");
		LOG.info("request body : " + requestBody);
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(vtnName);
		if (null != requestBody
				&& requestBody.has(VtnServiceJsonConsts.VTEPGROUP)
				&& null != requestBody
						.getAsJsonObject(VtnServiceJsonConsts.VTEPGROUP)
				&& requestBody.getAsJsonObject(VtnServiceJsonConsts.VTEPGROUP)
						.has(VtnServiceJsonConsts.VTEPGROUPNAME)) {
			uriParameters.add(requestBody
					.getAsJsonObject(VtnServiceJsonConsts.VTEPGROUP)
					.get(VtnServiceJsonConsts.VTEPGROUPNAME).getAsString());
		} else {
			uriParameters.add(vtepgroupName);
		}
		LOG.trace("Complete VTepGroupResource#getUriParameters()");
		return uriParameters;
	}
}
