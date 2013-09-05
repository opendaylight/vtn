/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.resources.physical;

import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.ipc.ClientSession;
import org.opendaylight.vtn.core.ipc.IpcException;
import org.opendaylight.vtn.core.ipc.IpcStruct;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.annotation.UNCVtnService;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceIpcConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcDataUnitWrapper;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncIpcErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncSYSMGEnums;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.validation.physical.VersionResourceValidator;

/**
 * The Class VersionResource get method.
 */
@UNCVtnService(path = "/unc/version")
public class VersionResource extends AbstractResource {

	/** The Constant LOG. */
	private static final Logger LOG = Logger.getLogger(VersionResource.class
			.getName());

	/**
	 * Instantiates a new Version resource.
	 */
	public VersionResource() {
		super();
		LOG.trace("Start VersionResource#VersionResource()");
		setValidator(new VersionResourceValidator(this));
		LOG.trace("Complete VersionResource#VersionResource()");
	}

	/**
	 * Implementation of Get method of Version API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public int get() throws VtnServiceException {
		LOG.trace("Starts VersionResource#get()");
		ClientSession session = null;
		int status = ClientSession.RESP_FATAL;
		try {
			LOG.debug("Start Ipc framework call");
			session = getConnPool().getSession(
					UncSYSMGEnums.SYSMG_IPC_CHANNEL_NAME,
					UncSYSMGEnums.NOMG_IPC_SERVICE_NAME,
					UncSYSMGEnums.NodeMgrServiceID.kNomgSoftVersionGet
							.ordinal(), getExceptionHandler());
			LOG.info("Session created successfully");
			status = session.invoke();
			LOG.info("Request packet processed with status:"+status);
			if (status != UncSYSMGEnums.NodeIpcErrorT.NOMG_E_OK.ordinal()) {
				LOG.info("error occurred while performing operation");
				createNoMgErrorInfo(UncIpcErrorCode.getNodeCodes(status));
				status = UncResultCode.UNC_SERVER_ERROR.getValue();
			} else {
				LOG.info("Operation successfully performed");
				final JsonObject response = createGetResponse(session);
				setInfo(response);
				status = UncResultCode.UNC_SUCCESS.getValue();
			}
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
		} catch (final IpcException e) {
			LOG.info("Error occured while performing addOutput operation");
			getExceptionHandler()
					.raise(Thread.currentThread().getStackTrace()[1]
							.getClassName()
							+ VtnServiceConsts.HYPHEN
							+ Thread.currentThread().getStackTrace()[1]
									.getMethodName(),
							UncJavaAPIErrorCode.IPC_SERVER_ERROR.getErrorCode(),
							UncJavaAPIErrorCode.IPC_SERVER_ERROR
									.getErrorMessage(), e);
		} finally {
			if (status == ClientSession.RESP_FATAL) {
				createErrorInfo(UncCommonEnum.UncResultCode.UNC_SERVER_ERROR
						.getValue());
				status = UncResultCode.UNC_SERVER_ERROR.getValue();
			}
			// destroy session by common handler
			getConnPool().destroySession(session);
		}
		LOG.trace("Completed VersionResource#get()");
		return status;
	}

	/**
	 * Creating Response for get Version API
	 * 
	 * @param ClientSession
	 *            Object
	 * @return Response Json Object
	 */
	private JsonObject createGetResponse(final ClientSession session) throws IpcException {
		LOG.trace("Starts VersionResource#createGetResponse()");
		final int count = session.getResponseCount();
		final JsonObject response = new JsonObject();
		final JsonObject versionJson = new JsonObject();
		final IpcStruct versionStruct = (IpcStruct) session
				.getResponse(VtnServiceJsonConsts.VAL_0);
		String major = IpcDataUnitWrapper.getIpcStructUint16Value(versionStruct,
				VtnServiceIpcConsts.MAJOR);
		LOG.debug("major: "+ IpcDataUnitWrapper.getIpcStructUint16Value(versionStruct,
				VtnServiceIpcConsts.MAJOR));
		String minor = IpcDataUnitWrapper.getIpcStructUint16Value(versionStruct,
				VtnServiceIpcConsts.MINOR);
		LOG.debug("minor: "+ IpcDataUnitWrapper.getIpcStructUint16Value(versionStruct,
				VtnServiceIpcConsts.MINOR));
		String revision = IpcDataUnitWrapper.getIpcStructUint16Value(versionStruct,
				VtnServiceIpcConsts.REVISION);
		LOG.debug("revision: "+ IpcDataUnitWrapper.getIpcStructUint16Value(versionStruct,
				VtnServiceIpcConsts.REVISION));
		String patchLevel = IpcDataUnitWrapper.getIpcStructUint16Value(versionStruct,
				VtnServiceIpcConsts.PATCHLEVEL);
		LOG.debug("patchLevel: "+ IpcDataUnitWrapper.getIpcStructUint16Value(versionStruct,
				VtnServiceIpcConsts.PATCHLEVEL));
		String version = VtnServiceJsonConsts.V + major + VtnServiceConsts.DOT + minor + VtnServiceConsts.DOT + revision + VtnServiceConsts.DOT + patchLevel;
		versionJson.addProperty(VtnServiceJsonConsts.VERSIONNO, version);
		final JsonArray patchJsonArray = new JsonArray();
		JsonObject patchJson = null;
		for (int i = VtnServiceJsonConsts.VAL_1; i < count; i++) {
			patchJson = new JsonObject();
			patchJson.addProperty(VtnServiceJsonConsts.PATCHNO,
					IpcDataUnitWrapper.getIpcDataUnitValue(session
							.getResponse(i)));
			patchJsonArray.add(patchJson);
		}
		versionJson.add(VtnServiceJsonConsts.PATCHES, patchJsonArray);
		response.add(VtnServiceJsonConsts.VERSION, versionJson);
		LOG.trace("Complted VersionResource#createGetResponse()");
		return response;
	}
}
