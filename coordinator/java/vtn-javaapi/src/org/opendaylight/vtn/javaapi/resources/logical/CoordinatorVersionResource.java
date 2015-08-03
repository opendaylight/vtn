/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.resources.logical;

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
import org.opendaylight.vtn.javaapi.validation.logical.CoordinatorVersionResourceValidator;

/**
 * The Class CoordinatorVersionResource implements get method.
 * 
 */
@UNCVtnService(path = "/coordinator_version")
public class CoordinatorVersionResource extends AbstractResource {

	/** The Constant LOG. */
	private static final Logger LOG = Logger
			.getLogger(CoordinatorVersionResource.class.getName());

	/**
	 * Instantiates a new ARP entry resource.
	 */
	public CoordinatorVersionResource() {
		super();
		LOG.trace("Start CoordinatorVersionResource#CoordinatorVersionResource()");
		setValidator(new CoordinatorVersionResourceValidator(this));
		LOG.trace("Complete CoordinatorVersionResource#CoordinatorVersionResource()");
	}

	/**
	 * Implementation of Get method of Coordinator Version API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public final int get() throws VtnServiceException {
		LOG.trace("Start CoordinatorVersionResource#get()");
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
			long start = System.currentTimeMillis();
			status = session.invoke();
			LOG.debug("The treatment of under layer cost the following time: "
					+ (System.currentTimeMillis() - start) + "(ms)");
			LOG.info("Request packet processed with status:" + status);
			if (status != UncSYSMGEnums.NodeIpcErrorT.NOMG_E_OK.ordinal()) {
				LOG.info("error occurred while performing operation");
				createNoMgErrorInfo(UncIpcErrorCode.getNodeCodes(status));
				status = UncResultCode.UNC_SERVER_ERROR.getValue();
			} else {
				LOG.info("Operation successfully performed");
				final JsonObject response = createCoordinatorVersionResponse(session);
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
		LOG.trace("Completed CoordinatorVersionResource#get()");
		return status;
	}

	/**
	 * Creating Response for get Coordinator Version API
	 * 
	 * @param ClientSession
	 *            Object
	 * @return Response Json Object
	 */
	private JsonObject createCoordinatorVersionResponse(
			final ClientSession session) throws IpcException {
		LOG.trace("Starts CoordinatorVersionResource#createCoordinatorVersionResponse()");
		final int count = session.getResponseCount();
		final JsonObject response = new JsonObject();
		final JsonObject coordinatorVersionJson = new JsonObject();
		final IpcStruct coordinatorVersionStruct = (IpcStruct) session
				.getResponse(VtnServiceJsonConsts.VAL_0);
		final String major = IpcDataUnitWrapper.getIpcStructUint16Value(
				coordinatorVersionStruct, VtnServiceIpcConsts.MAJOR);
		LOG.debug("major: "
				+ IpcDataUnitWrapper.getIpcStructUint16Value(
						coordinatorVersionStruct, VtnServiceIpcConsts.MAJOR));
		final String minor = IpcDataUnitWrapper.getIpcStructUint16Value(
				coordinatorVersionStruct, VtnServiceIpcConsts.MINOR);
		LOG.debug("minor: "
				+ IpcDataUnitWrapper.getIpcStructUint16Value(
						coordinatorVersionStruct, VtnServiceIpcConsts.MINOR));
		final String revision = IpcDataUnitWrapper.getIpcStructUint16Value(
				coordinatorVersionStruct, VtnServiceIpcConsts.REVISION);
		LOG.debug("revision: "
				+ IpcDataUnitWrapper.getIpcStructUint16Value(
						coordinatorVersionStruct, VtnServiceIpcConsts.REVISION));
		final String patchLevel = IpcDataUnitWrapper.getIpcStructUint16Value(
				coordinatorVersionStruct, VtnServiceIpcConsts.PATCHLEVEL);
		LOG.debug("patchLevel: "
				+ IpcDataUnitWrapper.getIpcStructUint16Value(
						coordinatorVersionStruct,
						VtnServiceIpcConsts.PATCHLEVEL));
		final String version = VtnServiceJsonConsts.V + major
				+ VtnServiceConsts.DOT + minor + VtnServiceConsts.DOT
				+ revision + VtnServiceConsts.DOT + patchLevel;
		coordinatorVersionJson.addProperty(VtnServiceJsonConsts.VERSION,
				version);
		final JsonArray patchJsonArray = new JsonArray();
		JsonObject patchJson = null;
		for (int i = VtnServiceJsonConsts.VAL_1; i < count; i++) {
			patchJson = new JsonObject();
			patchJson.addProperty(VtnServiceJsonConsts.PATCHNO,
					IpcDataUnitWrapper.getIpcDataUnitValue(session
							.getResponse(i)));
			patchJsonArray.add(patchJson);
		}
		coordinatorVersionJson
				.add(VtnServiceJsonConsts.PATCHES, patchJsonArray);
		response.add(VtnServiceJsonConsts.COORDINATOR_VERSION,
				coordinatorVersionJson);
		LOG.trace("Completed CoordinatorVersionResource#createCoordinatorVersionResponse()");
		return response;
	}
}
