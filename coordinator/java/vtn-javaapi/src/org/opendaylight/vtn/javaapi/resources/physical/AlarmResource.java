/*
 * Copyright (c) 2012-2015 NEC Corporation
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
import org.opendaylight.vtn.core.ipc.IpcString;
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
import org.opendaylight.vtn.javaapi.validation.physical.AlarmResourceValidator;

/**
 * The Class AlarmResource implements put and get method.
 */
@UNCVtnService(path = "/unc/alarms")
public class AlarmResource extends AbstractResource {
	private static final Logger LOG = Logger.getLogger(AlarmResource.class
			.getName());

	/**
	 * Instantiates a new Alarm API.
	 */
	public AlarmResource() {
		super();
		LOG.trace("Start AlarmResource#AlarmResource()");
		setValidator(new AlarmResourceValidator(this));
		LOG.trace("Complete AlarmResource#AlarmResource()");
	}

	/**
	 * Implementation of put method of AlarmResource API
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
		LOG.trace("Start Alarm#put()");
		ClientSession session = null;
		int status = ClientSession.RESP_FATAL;
		try {
			LOG.debug("Start Ipc framework call");
			session = getConnPool().getSession(
					UncSYSMGEnums.SYSMG_IPC_CHANNEL_NAME,
					UncSYSMGEnums.NOMG_IPC_SERVICE_NAME,
					UncSYSMGEnums.NodeMgrServiceID.kAlarmClear.ordinal(),
					getExceptionHandler());
			LOG.info("Session created successfully");
			session.addOutput(IpcDataUnitWrapper.setIpcUint64Value(requestBody
					.getAsJsonPrimitive(VtnServiceJsonConsts.ALARMNO)
					.getAsString()));
			LOG.info("Request packet created successfully");
			long start = System.currentTimeMillis();
			status = session.invoke();
			LOG.debug("The treatment of under layer cost the following time: "
					+ (System.currentTimeMillis() - start) + "(ms)");
			LOG.info("Request packet processed with status:" + status);
			if (status != UncSYSMGEnums.NodeIpcErrorT.NOMG_E_OK.ordinal()) {
				LOG.info("Error occurred while performing operation");
				createNoMgErrorInfo(UncIpcErrorCode.getNodeCodes(status));
				status = UncResultCode.UNC_SERVER_ERROR.getValue();
			} else {
				LOG.info("Opeartion successfully performed");
				status = UncResultCode.UNC_SUCCESS.getValue();
			}
			LOG.debug("Complete Ipc framework call");
		} catch (final IpcException e) {
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
		LOG.trace("Completed Alarm#put()");
		return status;
	}

	/**
	 * Implementation of get method of AlarmResource API
	 * 
	 * @param requestBody
	 *            the request Json object
	 * 
	 * @return Error code
	 * @throws VtnServiceException
	 */
	@Override
	public final int get() throws VtnServiceException {
		LOG.trace("Start AlarmResourcee#get()");
		ClientSession session = null;
		int status = ClientSession.RESP_FATAL;
		try {
			LOG.debug("Start Ipc framework call");
			session = getConnPool().getSession(
					UncSYSMGEnums.SYSMG_IPC_CHANNEL_NAME,
					UncSYSMGEnums.NOMG_IPC_SERVICE_NAME,
					UncSYSMGEnums.NodeMgrServiceID.kAlarmStatusListGet
							.ordinal(), getExceptionHandler());
			long start = System.currentTimeMillis();
			status = session.invoke();
			LOG.debug("The treatment of under layer cost the following time: "
					+ (System.currentTimeMillis() - start) + "(ms)");
			LOG.info("Request packet processed with status:" + status);
			if (status != UncSYSMGEnums.NodeIpcErrorT.NOMG_E_OK.ordinal()) {
				LOG.info("Error occurred while performing operation");
				createNoMgErrorInfo(UncIpcErrorCode.getNodeCodes(status));
				status = UncResultCode.UNC_SERVER_ERROR.getValue();
			} else {
				LOG.info("Opeartion successfully performed");
				status = UncResultCode.UNC_SUCCESS.getValue();
				final JsonObject response = createGetResponse(session);
				setInfo(response);
			}
			LOG.debug("Complete Ipc framework call");
		} catch (final VtnServiceException e) {
			LOG.info("Error occured while performing getSession operation");
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
		LOG.trace("Completed AlarmResource#get()");
		return status;
	}

	/**
	 * Creates the get response.
	 * 
	 * @param session
	 *            the session
	 * @return the json object
	 * @throws IpcException
	 *             the ipc exception
	 */

	private JsonObject createGetResponse(final ClientSession session)
			throws IpcException {
		LOG.trace("Start AlarmResource#createGetResponse()");
		final int count = session.getResponseCount();
		final JsonArray alarmJsonArray = new JsonArray();
		// LOG.info("response:"+ ipcResponseStruct.toString());
		// convert IPC Structure into Json
		final JsonObject response = new JsonObject();
		for (int i = VtnServiceJsonConsts.VAL_1; i < count; i++) {
			final JsonObject sessionDetails = new JsonObject();
			final IpcStruct ipcResponseStruct = (IpcStruct) session
					.getResponse(i++);
			// add AlarmNo to response json
			sessionDetails.addProperty(
					VtnServiceJsonConsts.ALARMNO,
					IpcDataUnitWrapper.getIpcStructUint64Value(
							ipcResponseStruct, VtnServiceJsonConsts.ALARMNO)
							.toString());
			// add TimeStamp to response json
			sessionDetails.addProperty(
					VtnServiceJsonConsts.TIMESTAMP,
					IpcDataUnitWrapper.getIpcStructInt64Value(
							ipcResponseStruct, VtnServiceIpcConsts.TIME_STAMP)
							.toString());
			// add type to response json
			if (IpcDataUnitWrapper
					.getIpcStructUint8Value(ipcResponseStruct,
							VtnServiceJsonConsts.SEVERITY).toString()
					.equals(UncSYSMGEnums.AlarmLevelT.ALM_EMERG.getValue())) {
				sessionDetails.addProperty(VtnServiceJsonConsts.SEVERITY,
						VtnServiceJsonConsts.EMERGENCY);
			} else if (IpcDataUnitWrapper
					.getIpcStructUint8Value(ipcResponseStruct,
							VtnServiceJsonConsts.SEVERITY).toString()
					.equals(UncSYSMGEnums.AlarmLevelT.ALM_ALERT.getValue())) {
				sessionDetails.addProperty(VtnServiceJsonConsts.SEVERITY,
						VtnServiceJsonConsts.ALERT);
			} else if (IpcDataUnitWrapper
					.getIpcStructUint8Value(ipcResponseStruct,
							VtnServiceJsonConsts.SEVERITY).toString()
					.equals(UncSYSMGEnums.AlarmLevelT.ALM_CRITICAL.getValue())) {
				sessionDetails.addProperty(VtnServiceJsonConsts.SEVERITY,
						VtnServiceJsonConsts.CRITICAL);
			} else if (IpcDataUnitWrapper
					.getIpcStructUint8Value(ipcResponseStruct,
							VtnServiceJsonConsts.SEVERITY).toString()
					.equals(UncSYSMGEnums.AlarmLevelT.ALM_ERROR.getValue())) {
				sessionDetails.addProperty(VtnServiceJsonConsts.SEVERITY,
						VtnServiceJsonConsts.ERROR);
			} else if (IpcDataUnitWrapper
					.getIpcStructUint8Value(ipcResponseStruct,
							VtnServiceJsonConsts.SEVERITY).toString()
					.equals(UncSYSMGEnums.AlarmLevelT.ALM_WARNING.getValue())) {
				sessionDetails.addProperty(VtnServiceJsonConsts.SEVERITY,
						VtnServiceJsonConsts.WARNING);
			} else if (IpcDataUnitWrapper
					.getIpcStructUint8Value(ipcResponseStruct,
							VtnServiceJsonConsts.SEVERITY).toString()
					.equals(UncSYSMGEnums.AlarmLevelT.ALM_NOTICE.getValue())) {
				sessionDetails.addProperty(VtnServiceJsonConsts.SEVERITY,
						VtnServiceJsonConsts.NOTICE);
			} else if (IpcDataUnitWrapper
					.getIpcStructUint8Value(ipcResponseStruct,
							VtnServiceJsonConsts.SEVERITY).toString()
					.equals(UncSYSMGEnums.AlarmLevelT.ALM_INFO.getValue())) {
				sessionDetails.addProperty(VtnServiceJsonConsts.SEVERITY,
						VtnServiceJsonConsts.INFORMATION);
			} else if (IpcDataUnitWrapper
					.getIpcStructUint8Value(ipcResponseStruct,
							VtnServiceJsonConsts.SEVERITY).toString()
					.equals(UncSYSMGEnums.AlarmLevelT.ALM_DEBUG.getValue())) {
				sessionDetails.addProperty(VtnServiceJsonConsts.SEVERITY,
						VtnServiceJsonConsts.DEBUG);
			}
			// add user name to response json
			if (IpcDataUnitWrapper
					.getIpcStructUint8Value(ipcResponseStruct,
							VtnServiceJsonConsts.TYPE).toString()
					.equals(VtnServiceJsonConsts.ONE)) {
				sessionDetails.addProperty(VtnServiceJsonConsts.TYPE,
						VtnServiceJsonConsts.OCCURRED);
			} else if (IpcDataUnitWrapper
					.getIpcStructUint8Value(ipcResponseStruct,
							VtnServiceJsonConsts.TYPE).toString()
					.equals(VtnServiceJsonConsts.TWO)) {
				sessionDetails.addProperty(VtnServiceJsonConsts.TYPE,
						VtnServiceJsonConsts.RECOVERED);
			}
			sessionDetails.addProperty(VtnServiceJsonConsts.VTNNAME,
					((IpcString) session.getResponse(i++)).toString());
			sessionDetails.addProperty(VtnServiceJsonConsts.SUMMARY,
					((IpcString) session.getResponse(i++)).toString());
			sessionDetails.addProperty(VtnServiceJsonConsts.MESSAGE,
					((IpcString) session.getResponse(i++)).toString());
			alarmJsonArray.add(sessionDetails);
		}
		response.add(VtnServiceJsonConsts.ALARMS, alarmJsonArray);
		LOG.trace("Completed AlarmResource#createGetResponse()");
		return response;
	}
}
