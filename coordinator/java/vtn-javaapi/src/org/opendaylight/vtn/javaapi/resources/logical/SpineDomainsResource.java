/*
 * Copyright (c) 2015 NEC Corporation
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
import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.ipc.ClientSession;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.annotation.UNCField;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceIpcConsts;
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
import org.opendaylight.vtn.javaapi.validation.logical.SpineDomainResourceValidator;

/**
 * The Class SpineDomainsResource implements post, get method.
 */
@UNCVtnService(path = "/unified_networks/{unified_network_name}/spine_domains")
public class SpineDomainsResource extends AbstractResource {

	/** The unified_network_name. */
	@UNCField("unified_network_name")
	private String unifiedNetworkName;

	/**
	 * 
	 * @return the unified_network_name
	 */
	public final String getUnifiedNetworkName() {
		return unifiedNetworkName;
	}

	private static final Logger LOG = Logger
			.getLogger(StaticIpRoutesResource.class.getName());

	/**
	 * Instantiates a new Spine Domains resource.
	 */
	public SpineDomainsResource() {
		super();
		LOG.trace("Start SpineDomainsResource#SpineDomainsResource()");
		setValidator(new SpineDomainResourceValidator(this));
		LOG.trace("Complete SpineDomainsResource#SpineDomainsResource()");
	}

	/**
	 * Implementation of post method of Spine Domains
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
		LOG.trace("Starts SpineDomainsResource#post()");
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
					IpcRequestPacketEnum.KT_UNW_SPINE_DOMAIN_CREATE,
					requestBody, getUriParameters(requestBody));
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
		LOG.trace("Completed SpineDomainsResource#post()");
		return status;
	}

	/**
	 * Implementation of get method of Spine Domains
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
		LOG.trace("Starts SpineDomainsResource#get()");
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

			String opType = VtnServiceJsonConsts.NORMAL;
			if (requestBody.has(VtnServiceJsonConsts.OP)) {
				opType = requestBody.get(VtnServiceJsonConsts.OP).getAsString();
			}

			String dataType = VtnServiceJsonConsts.STATE;
			if (requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
				dataType = requestBody.get(VtnServiceJsonConsts.TARGETDB).getAsString();
			}

			if (opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)
				&& dataType.equalsIgnoreCase(VtnServiceJsonConsts.STATE)) {
				requestProcessor = new IpcRequestProcessor(session,
						getSessionID(), getConfigID(), getExceptionHandler());
				// Uriparamter list
				final List<String> uriParameterList = getUriParameters(requestBody);
				requestProcessor.createIpcRequestPacket(
						IpcRequestPacketEnum.KT_UNW_SPINE_DOMAIN_GET_STATE,
						requestBody, uriParameterList);
				LOG.debug("Request packet created successfully");
				status = requestProcessor.processIpcRequest();
				LOG.debug("Request packet processed with status" + status);
				final IpcLogicalResponseFactory responseGenerator = new IpcLogicalResponseFactory();

				JsonObject responseJson = responseGenerator
						.getSpineDomainResponse(
								requestProcessor.getIpcResponsePacket(),
								requestBody, VtnServiceJsonConsts.LIST);
				if (responseJson.get(VtnServiceJsonConsts.SPINE_DOMAINS)
						.isJsonArray()) {
					final JsonArray responseArray = responseJson.get(
							VtnServiceJsonConsts.SPINE_DOMAINS).getAsJsonArray();

					responseJson = getResponseJsonArrayLogical(requestBody,
							requestProcessor, responseGenerator, responseArray,
							VtnServiceJsonConsts.SPINE_DOMAINS,
							VtnServiceJsonConsts.SPINE_DOMAIN_NAME,
							IpcRequestPacketEnum.KT_UNW_SPINE_DOMAIN_GET_STATE,
							uriParameterList,
							VtnServiceIpcConsts.GET_SPINE_DOMAIN_RESPONSE);
					setInfo(responseJson);
				}
			} else {
				requestProcessor = new IpcRequestProcessor(session,
						getSessionID(), getConfigID(), getExceptionHandler());
				// Uriparamter list
				final List<String> uriParameterList = getUriParameters(requestBody);
				requestProcessor.createIpcRequestPacket(
						IpcRequestPacketEnum.KT_UNW_SPINE_DOMAIN_GET,
						requestBody, uriParameterList);
				LOG.debug("Request packet created successfully");
				status = requestProcessor.processIpcRequest();
				LOG.debug("Request packet processed with status" + status);
				final IpcLogicalResponseFactory responseGenerator = new IpcLogicalResponseFactory();

				JsonObject responseJson = responseGenerator
						.getSpineDomainResponse(
								requestProcessor.getIpcResponsePacket(),
								requestBody, VtnServiceJsonConsts.LIST);
				if (responseJson.get(VtnServiceJsonConsts.SPINE_DOMAINS)
						.isJsonArray()) {
					final JsonArray responseArray = responseJson.get(
							VtnServiceJsonConsts.SPINE_DOMAINS).getAsJsonArray();

					responseJson = getResponseJsonArrayLogical(requestBody,
							requestProcessor, responseGenerator, responseArray,
							VtnServiceJsonConsts.SPINE_DOMAINS,
							VtnServiceJsonConsts.SPINE_DOMAIN_NAME,
							IpcRequestPacketEnum.KT_UNW_SPINE_DOMAIN_GET,
							uriParameterList,
							VtnServiceIpcConsts.GET_SPINE_DOMAIN_RESPONSE);
				}
				setInfo(responseJson);
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
		LOG.trace("Complete SpineDomainsResource#get()");
		return status;
	}

	/**
	 * Add URI parameters to list
	 * 
	 * @return
	 */
	private List<String> getUriParameters(final JsonObject requestBody) {
		LOG.trace("Start SpineDomainsResource#getUriParameters()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(unifiedNetworkName);
		if (requestBody != null && requestBody.has(VtnServiceJsonConsts.INDEX)) {
			uriParameters.add(requestBody.get(VtnServiceJsonConsts.INDEX)
					.getAsString());
		}
		LOG.trace("Completed SpineDomainsResource#getUriParameters()");
		return uriParameters;
	}

}
