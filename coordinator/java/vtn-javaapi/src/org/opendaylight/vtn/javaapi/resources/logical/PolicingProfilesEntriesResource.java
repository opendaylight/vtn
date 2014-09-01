/*
 * Copyright (c) 2014 NEC Corporation
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
import org.opendaylight.vtn.javaapi.validation.logical.PolicingProfilesEntriesResourceValidator;

@UNCVtnService(path = "/policingprofiles/{profile_name}/policingprofileentries")
public class PolicingProfilesEntriesResource extends AbstractResource {

	/** The Constant LOG. */
	private static final Logger LOG = Logger
			.getLogger(PolicingProfilesEntriesResource.class.getName());

	/**
	 * Instantiates a new PolicingProfilesEntries Resource.
	 */
	public PolicingProfilesEntriesResource() {
		super();
		LOG.trace("Start PolicingProfilesEntriesResource#PolicingProfilesEntriesResource()");
		setValidator(new PolicingProfilesEntriesResourceValidator(this));
		LOG.trace("Completed PolicingProfilesEntriesResource#PolicingProfilesEntriesResource()");
	}

	/** The profile_name name. */
	@UNCField("profile_name")
	private String profileName;

	/**
	 * @return the profile_name
	 */
	public String getProfileName() {
		return profileName;
	}

	/**
	 * Implementation of Post method of Policing Profiles Entries Resource API
	 * 
	 * @param requestBody
	 *            the request JSON object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public int post(final JsonObject requestBody) throws VtnServiceException {
		LOG.trace("Start PolicingProfilesEntriesResource#post()");
		ClientSession session = null;
		IpcRequestProcessor requestProcessor = null;
		int status = ClientSession.RESP_FATAL;
		try {
			LOG.info("Start Ipc framework call PolicingProfilesEntriesResource");
			session = getConnPool().getSession(
					UncUPLLEnums.UPLL_IPC_CHANNEL_NAME,
					UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
					UncUPLLEnums.ServiceID.UPLL_EDIT_SVC_ID.ordinal(),
					getExceptionHandler());
			LOG.info("Session created successfully");
			requestProcessor = new IpcRequestProcessor(session, getSessionID(),
					getConfigID(), getExceptionHandler());
			requestProcessor.createIpcRequestPacket(
					IpcRequestPacketEnum.KT_POLICING_PROFILE_ENTRY_CREATE,
					requestBody, getUriParameters(requestBody));
			LOG.info("Request packet created successfully");
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet processed with status" + status);
			LOG.info("Complete Ipc framework call");
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
		LOG.trace("Completed PolicingProfilesEntriesResource#post()");
		return status;
	}

	/**
	 * Implementation of Get method of Policing Profiles Entries Resource
	 * 
	 * @param requestBody
	 *            the request JSON object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public int get(final JsonObject requestBody) throws VtnServiceException {
		LOG.trace("Start PolicingProfilesEntriesResource#get()");
		ClientSession session = null;
		IpcRequestProcessor requestProcessor = null;
		int status = ClientSession.RESP_FATAL;
		try {
			LOG.info("Start Ipc framework call for PolicingProfilesEntriesResource");
			session = getConnPool().getSession(
					UncUPLLEnums.UPLL_IPC_CHANNEL_NAME,
					UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
					UncUPLLEnums.ServiceID.UPLL_READ_SVC_ID.ordinal(),
					getExceptionHandler());
			LOG.info("Session created successfully");
			requestProcessor = new IpcRequestProcessor(session, getSessionID(),
					getConfigID(), getExceptionHandler());
			final List<String> uriParameterList = getUriParameters(requestBody);
			requestProcessor.createIpcRequestPacket(
					IpcRequestPacketEnum.KT_POLICING_PROFILE_ENTRY_GET,
					requestBody, uriParameterList);
			LOG.info("Request packet created successfully");
			status = requestProcessor.processIpcRequest();
			LOG.debug("Request packet processed with status" + status);
			final IpcLogicalResponseFactory responseGenerator = new IpcLogicalResponseFactory();
			JsonObject responseJson = responseGenerator
					.getPolicingProfileEntryResponse(
							requestProcessor.getIpcResponsePacket(),
							requestBody, VtnServiceJsonConsts.LIST);
			if (responseJson.get(VtnServiceJsonConsts.POLICINGPROFILEENTRIES)
					.isJsonArray()) {
				final JsonArray responseArray = responseJson.get(
						VtnServiceJsonConsts.POLICINGPROFILEENTRIES)
						.getAsJsonArray();
				responseJson = getResponseJsonArrayLogical(requestBody,
						requestProcessor, responseGenerator, responseArray,
						VtnServiceJsonConsts.POLICINGPROFILEENTRIES,
						VtnServiceJsonConsts.SEQNUM,
						IpcRequestPacketEnum.KT_POLICING_PROFILE_ENTRY_GET,
						uriParameterList,
						VtnServiceIpcConsts.GET_POLICING_PROFILE_ENTRY_RESPONSE);
			}
			setInfo(responseJson);
			LOG.info("Response object created successfully");
			LOG.info("Complete Ipc framework call");
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
		LOG.trace("Completed PolicingProfilesEntriesResource#get()");
		return status;
	}

	/**
	 * Add URI parameters to list
	 * 
	 * @return
	 */
	private List<String> getUriParameters(final JsonObject requestBody) {
		LOG.trace("Start PolicingProfilesEntriesResource#getUriParameters()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(profileName);
		if (requestBody != null && requestBody.has(VtnServiceJsonConsts.INDEX)) {
			uriParameters.add(requestBody.get(VtnServiceJsonConsts.INDEX)
					.getAsString());
		}
		LOG.trace("Completed PolicingProfilesEntriesResource#getUriParameters()");
		return uriParameters;
	}

}
