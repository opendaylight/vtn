/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.resources.logical;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;

import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.ipc.ClientSession;
import org.opendaylight.vtn.core.ipc.IpcDataUnit;
import org.opendaylight.vtn.core.ipc.IpcStruct;
import org.opendaylight.vtn.core.ipc.IpcUint32;
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
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcLogicalResponseFactory;
import org.opendaylight.vtn.javaapi.ipc.enums.IpcRequestPacketEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncOperationEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncStructEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncStructIndexEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncUPLLEnums;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.validation.logical.VLinkResourceValidator;

/**
 * The Class VLinksResource.
 */
/* This class handles post and get methods */
@UNCVtnService(path = "/vtns/{vtn_name}/vlinks")
public class VLinksResource extends AbstractResource {
	/** The VTN name. */
	@UNCField("vtn_name")
	private String vtnName;

	/**
	 * Gets the VTN name.
	 * 
	 * @return the VTN name
	 */
	public final String getVtnName() {
		return vtnName;
	}

	private static final Logger LOG = Logger.getLogger(VLinksResource.class
			.getName());

	/**
	 * Instantiates a new v links resource.
	 */
	public VLinksResource() {
		LOG.trace("Start VLinksResource#VLinksResource()");
		setValidator(new VLinkResourceValidator(this));
		LOG.trace("Completed VLinksResource#VLinksResource()");
	}

	/**
	 * Implementation of Post method of Vlink API
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
		LOG.trace("Start VLinksResource#post()");
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
					IpcRequestPacketEnum.KT_VLINK_CREATE, requestBody,
					getUriParameters(requestBody));
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
		LOG.trace("Completed VLinksResource#post()");
		return status;
	}

	/**
	 * Implementation of Get method of Vlink API
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
		LOG.trace("Start VLinksResource#get()");
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
					IpcRequestPacketEnum.KT_VLINK_GET, requestBody,
					getUriParameters(requestBody));
			getModifiedRequestPacket(requestBody, requestProcessor);
			LOG.debug("Request packet created successfully");
			status = requestProcessor.processIpcRequest();
			final List<String> uriParameterList = getUriParameters(requestBody);
			LOG.debug("Request packet processed with status" + status);
			final IpcLogicalResponseFactory responseGenerator = new IpcLogicalResponseFactory();
			/*
			 * setInfo(responseGenerator.getVLinkResponse(
			 * requestProcessor.getIpcResponsePacket(), requestBody,
			 * VtnServiceJsonConsts.LIST));
			 */
			JsonObject responseJson = responseGenerator.getVLinkResponse(
					requestProcessor.getIpcResponsePacket(), requestBody,
					VtnServiceJsonConsts.LIST);
			if (responseJson.get(VtnServiceJsonConsts.VLINKS).isJsonArray()) {
				final JsonArray responseArray = responseJson.get(
						VtnServiceJsonConsts.VLINKS).getAsJsonArray();
				responseJson = getResponseJsonArrayLink(requestBody,
						requestProcessor, responseGenerator, responseArray,
						VtnServiceJsonConsts.VLINKS,
						VtnServiceJsonConsts.VLKNAME,
						IpcRequestPacketEnum.KT_VLINK_GET, uriParameterList,
						VtnServiceIpcConsts.GET_VLINKS_RESPONSE);
			}
			setInfo(responseJson);
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
		LOG.trace("Completed VLinksResource#get()");
		return status;
	}

	private void getModifiedRequestPacket(final JsonObject requestBody,
			final IpcRequestProcessor requestProcessor) {
		final IpcStruct valStruct = new IpcStruct(
				UncStructEnum.ValVlink.getValue());
		requestProcessor.getRequestPacket().setValStruct(valStruct);
		if (requestBody.has(VtnServiceJsonConsts.VNODE1NAME)
				|| requestBody.has(VtnServiceJsonConsts.VNODE2NAME)) {
			// IpcStruct valStruct =
			// requestProcessor.getRequestPacket().getValStruct();
			if (requestBody.has(VtnServiceJsonConsts.VNODE1NAME)) {
				valStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_VNODE1_NAME_VLNK
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valStruct.set(VtnServiceJsonConsts.VNODE1NAME,
						IpcDataUnitWrapper.setIpcUint8ArrayValue(requestBody
								.get(VtnServiceJsonConsts.VNODE1NAME)
								.getAsString()));
			}
			if (requestBody.has(VtnServiceJsonConsts.VNODE2NAME)) {
				valStruct
						.set(VtnServiceIpcConsts.VALID,
								UncStructIndexEnum.ValVlinkIndex.UPLL_IDX_VNODE2_NAME_VLNK
										.ordinal(),
								IpcDataUnitWrapper
										.setIpcUint8Value(UncStructIndexEnum.Valid.UNC_VF_VALID
												.ordinal()));
				valStruct.set(VtnServiceJsonConsts.VNODE2NAME,
						IpcDataUnitWrapper.setIpcUint8ArrayValue(requestBody
								.get(VtnServiceJsonConsts.VNODE2NAME)
								.getAsString()));
			}
		}
		if (requestBody.get(VtnServiceJsonConsts.TARGETDB).getAsString()
				.equalsIgnoreCase(VtnServiceJsonConsts.STATE)
				&& !requestBody.get(VtnServiceJsonConsts.OP).getAsString()
						.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
			LOG.debug("Add ValVlinkSt to Request Packet");
			final IpcDataUnit[] extraDataUnits = new IpcDataUnit[1];
			final IpcStruct valStructSt = new IpcStruct(
					UncStructEnum.ValVlinkSt.getValue());
			extraDataUnits[0] = valStructSt;
			requestProcessor.getRequestPacket().setExtraDataUnits(
					extraDataUnits);
		} else {
			requestProcessor.getRequestPacket().setExtraDataUnits(null);
		}
	}

	/**
	 * Add URI parameters to list
	 * 
	 * @return parameter list
	 */
	private List<String> getUriParameters(final JsonObject requestBody) {
		LOG.trace("Start VLinksResource#getUriParameters()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(vtnName);
		if (requestBody != null && requestBody.has(VtnServiceJsonConsts.INDEX)) {
			uriParameters.add(requestBody.get(VtnServiceJsonConsts.INDEX)
					.getAsString());
		}
		LOG.trace("Completed VLinksResource#getUriParameters()");
		return uriParameters;
	}

	private final JsonObject getResponseJsonArrayLink(
			final JsonObject requestBody,
			final IpcRequestProcessor requestProcessor,
			final Object responseGenerator, final JsonArray responseArray,
			final String JsonArrayName, final String IndexName,
			final IpcRequestPacketEnum requestPackeEnumName,
			final List<String> uriParameters, final String methodName)
			throws VtnServiceException {
		// session reset
		requestProcessor.setServiceInfo(UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
				UncUPLLEnums.ServiceID.UPLL_READ_SVC_ID.ordinal());
		int status = ClientSession.RESP_FATAL;
		long memberIndex = 0;
		long max_repetition = 0;
		//get max_repetition.
		max_repetition = Long.parseLong(requestBody
								.getAsJsonPrimitive(VtnServiceJsonConsts
								.MAX).getAsString());
		memberIndex = responseArray.size();
		if (memberIndex != 0 && memberIndex < max_repetition) {
			JsonObject memberLastIndex = (JsonObject) responseArray
					.get(responseArray.size() - 1);
			if (requestBody.has(VtnServiceJsonConsts.INDEX)) {
				uriParameters.remove(uriParameters.size() - 1);
				uriParameters.add(uriParameters.size(),
						memberLastIndex.get(IndexName).getAsString());
			} else {
				uriParameters.add(memberLastIndex.get(IndexName).getAsString());
			}
			// (noSuchInstanceFlag = 0) means that all data has been acquired.
			// (noSuchInstanceFlag != 0) means that there may be data.
			long noSuchInstanceFlag = 0;
			noSuchInstanceFlag = memberIndex;
			while (noSuchInstanceFlag != 0 && memberIndex < max_repetition) {

				JsonArray memberArray = null;
				memberLastIndex = (JsonObject) responseArray.get(responseArray
						.size() - 1);
				uriParameters.remove(uriParameters.size() - 1);
				uriParameters.add(uriParameters.size(),
						memberLastIndex.get(IndexName).getAsString());

				requestProcessor.createIpcRequestPacket(requestPackeEnumName,
						requestBody, uriParameters);
				// update the operation to read_sibling.
				if (requestProcessor.getRequestPacket().getOperation().intValue() !=
				    UncOperationEnum.UNC_OP_READ_SIBLING.ordinal()) {
					requestProcessor.getRequestPacket().setOperation(new IpcUint32
							(UncOperationEnum.UNC_OP_READ_SIBLING.ordinal()));
				}
				getModifiedRequestPacket(requestBody, requestProcessor);
				status = requestProcessor.processIpcRequest();
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
				try {
					Method method;

					final Class<IpcLogicalResponseFactory> sourceClass = IpcLogicalResponseFactory.class;
					// get the method name to get the IpcLogicalResponseFactory
					// object for given key
					method = sourceClass.getMethod(methodName,
							new Class<?>[] { IpcDataUnit[].class,
									JsonObject.class, String.class });
					// get IpcLogicalResponseFactory object
					memberArray = ((JsonObject) method.invoke(
							responseGenerator,
							requestProcessor.getIpcResponsePacket(),
							requestBody, VtnServiceJsonConsts.LIST))
							.getAsJsonArray(JsonArrayName);
					noSuchInstanceFlag = memberArray.size();
				} catch (final Exception e) {
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
				if (null != memberArray && !memberArray.isJsonNull()
						&& memberArray.size() > 0) {
					// all the required data have been acquired.
					if (memberIndex + memberArray.size() > max_repetition) {
						for (long i = memberIndex; i<max_repetition; i++) {
							responseArray.getAsJsonArray().add((memberArray.get((int)(i - memberIndex))));
						}
						memberIndex =  max_repetition;
					} else {
						responseArray.getAsJsonArray().addAll(memberArray);
						memberIndex += memberArray.size();
					}
				} else {
					break;
				}
			}
		}
		final JsonObject root = new JsonObject();
		root.add(JsonArrayName, responseArray);
		return root;
	}
}
