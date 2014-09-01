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

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.ipc.ClientSession;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.annotation.UNCField;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
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

/**
 * The Class PolicingProfilesEntryResource implements delete, get and put
 * methods
 */

@UNCVtnService(
		path = "/policingprofiles/{profile_name}/policingprofileentries/{seqnum}")
public class PolicingProfilesEntryResource extends AbstractResource {
	/** The profile_name name. */
	@UNCField("profile_name")
	private String profileName;

	/** The seqnum. */
	@UNCField("seqnum")
	private String seqnum;

	/**
	 * @return the seqnum
	 */
	public final String getSeqnum() {
		return seqnum;
	}

	/**
	 * @return the profile_name
	 */
	public String getProfileName() {
		return profileName;
	}

	private static final Logger LOG = Logger
			.getLogger(PolicingProfilesEntryResource.class.getName());

	/**
	 * Instantiates a new PolicingProfilesEntry Resource.
	 */
	public PolicingProfilesEntryResource() {
		super();
		LOG.trace("Start PolicingProfilesEntry#PolicingProfilesEntryResource()");
		setValidator(new PolicingProfilesEntriesResourceValidator(this));
		LOG.trace("Completed PolicingProfilesEntry#PolicingProfilesEntryResource()");
	}

	/**
	 * Implementation of put method of PolicingProfilesEntry
	 * 
	 * @param requestBody
	 *            the request JSON object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public final int put(final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Starts PolicingProfilesEntryResource#put()");
		ClientSession session = null;
		IpcRequestProcessor requestProcessor = null;
		int status = ClientSession.RESP_FATAL;
		try {
			LOG.info("Start Ipc framework call for PolicingProfilesEntryResource");
			session = getConnPool().getSession(
					UncUPLLEnums.UPLL_IPC_CHANNEL_NAME,
					UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
					UncUPLLEnums.ServiceID.UPLL_EDIT_SVC_ID.ordinal(),
					getExceptionHandler());
			LOG.info("Session created successfully");
			requestProcessor = new IpcRequestProcessor(session, getSessionID(),
					getConfigID(), getExceptionHandler());
			requestProcessor.createIpcRequestPacket(
					IpcRequestPacketEnum.KT_POLICING_PROFILE_ENTRY_UPDATE,
					requestBody, getUriParameters());
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
		LOG.trace("Completed PolicingProfilesEntryResource#put()");
		return status;
	}

	/**
	 * Implementation of delete method of PolicingProfilesEntryResource
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public final int delete() throws VtnServiceException {
		LOG.trace("Starts PolicingProfilesEntryResource#delete()");
		ClientSession session = null;
		IpcRequestProcessor requestProcessor = null;
		int status = ClientSession.RESP_FATAL;
		try {
			LOG.info("Start Ipc framework call for PolicingProfilesEntryResource");
			session = getConnPool().getSession(
					UncUPLLEnums.UPLL_IPC_CHANNEL_NAME,
					UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
					UncUPLLEnums.ServiceID.UPLL_EDIT_SVC_ID.ordinal(),
					getExceptionHandler());
			LOG.info("Session created successfully");
			requestProcessor = new IpcRequestProcessor(session, getSessionID(),
					getConfigID(), getExceptionHandler());
			requestProcessor.createIpcRequestPacket(
					IpcRequestPacketEnum.KT_POLICING_PROFILE_ENTRY_DELETE,
					getNullJsonObject(), getUriParameters());
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
		LOG.trace("Complete PolicingProfilesEntryResource#delete()");
		return status;
	}

	/**
	 * Implementation of get method of PolicingProfilesEntryResource
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
		LOG.trace("Start PolicingProfilesEntryResource#get()");
		ClientSession session = null;
		IpcRequestProcessor requestProcessor = null;
		int status = ClientSession.RESP_FATAL;
		try {
			LOG.info("Start Ipc framework call for PolicingProfilesEntryResource");
			session = getConnPool().getSession(
					UncUPLLEnums.UPLL_IPC_CHANNEL_NAME,
					UncUPLLEnums.UPLL_IPC_SERVICE_NAME,
					UncUPLLEnums.ServiceID.UPLL_READ_SVC_ID.ordinal(),
					getExceptionHandler());
			LOG.info("Session created successfully");
			requestProcessor = new IpcRequestProcessor(session, getSessionID(),
					getConfigID(), getExceptionHandler());
			final List<String> uriParameterList = getUriParameters();
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
							requestBody, VtnServiceJsonConsts.SHOW);
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
		LOG.trace("Completed PolicingProfilesEntryResource#get()");
		return status;
	}

	/**
	 * Add URI parameters to list
	 * 
	 * @return parameter list
	 */
	private List<String> getUriParameters() {
		LOG.trace("Start PolicingProfilesEntryResource#getUriParameters()");
		final List<String> uriParameters = new ArrayList<String>();
		uriParameters.add(profileName);
		uriParameters.add(seqnum);
		LOG.trace("Completed PolicingProfilesEntryResource#getUriParameters()");
		return uriParameters;
	}
}
