/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.ipc;

import java.lang.reflect.Method;
import java.util.List;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.ipc.ClientSession;
import org.opendaylight.vtn.core.ipc.IpcDataUnit;
import org.opendaylight.vtn.core.ipc.IpcException;
import org.opendaylight.vtn.core.ipc.IpcStruct;
import org.opendaylight.vtn.core.ipc.IpcUint32;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.exception.VtnServiceExceptionHandler;
import org.opendaylight.vtn.javaapi.init.VtnServiceInitManager;
import org.opendaylight.vtn.javaapi.ipc.conversion.IpcDataUnitWrapper;
import org.opendaylight.vtn.javaapi.ipc.enums.IpcRequestPacketEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncCommonEnum.UncResultCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncDataType;
import org.opendaylight.vtn.javaapi.ipc.enums.UncErrorBean;
import org.opendaylight.vtn.javaapi.ipc.enums.UncIpcErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.ipc.enums.UncKeyTypeEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncOperationEnum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncOption2Enum;
import org.opendaylight.vtn.javaapi.ipc.enums.UncStructEnum;
import org.opendaylight.vtn.javaapi.util.VtnServiceUtil;

/**
 * The Class IpcRequestProcessor. Interacts with IPC classes to send the request
 * and receive the response
 */
public class IpcRequestProcessor {

	private static final Logger LOG = Logger
			.getLogger(IpcRequestProcessor.class.getName());

	private transient final IpcRequestPacket requestPacket;
	private transient final VtnServiceExceptionHandler exceptionHandler;
	private transient IpcStructFactory stuctGenerator;
	private transient final ClientSession session;
	private JsonObject errorJson;
	private boolean noSuchInstanceFlag;
	private String requestPacketEnumName = null;
	private String serviceName = null;
	private int serviceId;

	/**
	 * Instantiates a new ipc request processor.
	 * 
	 * @param session
	 *            the session
	 * @param sessionId
	 *            the session id
	 * @param configId
	 *            the config id
	 * @param exceptionHandler
	 *            the exception handler
	 */
	public IpcRequestProcessor(final ClientSession session,
			final long sessionId, final long configId,
			final VtnServiceExceptionHandler exceptionHandler) {
		LOG.trace("Start IpcRequestProcessor#IpcRequestProcessor()");
		noSuchInstanceFlag = false;
		this.session = session;
		this.requestPacket = new IpcRequestPacket();
		this.exceptionHandler = exceptionHandler;
		// set the session id and config id to IPC request packet
		LOG.debug("session id : " + sessionId);
		requestPacket.setSessionId(new IpcUint32(sessionId));
		LOG.debug("config id : " + configId);
		requestPacket.setConfigId(new IpcUint32(configId));
		LOG.trace("Complete IpcRequestProcessor#IpcRequestProcessor()");
	}

	/**
	 * Set sessions related service information This interface is required to be
	 * invoked, if session reset is required
	 * 
	 * @return the request packet
	 */
	public final void setServiceInfo(final String serviceName,
			final int serviceId) {
		this.serviceName = serviceName;
		this.serviceId = serviceId;
	}

	/**
	 * Gets the request packet.
	 * 
	 * @return the request packet
	 */
	public final IpcRequestPacket getRequestPacket() {
		LOG.trace("Return from IpcRequestProcessor#getRequestPacket()");
		return requestPacket;
	}

	/**
	 * Creates the ipc request packet.
	 * 
	 * @param requestPacketEnum
	 *            the request packet enum
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	public final void createIpcRequestPacket(
			final IpcRequestPacketEnum requestPacketEnum,
			final JsonObject requestBody, final List<String> uriParameters)
			throws VtnServiceException {

		LOG.trace("Start IpcRequestProcessor#createIpcRequestPacket()");

		if (requestPacketEnum != null) {
			// set the values to IPC request packet, that has been received by
			// request packet enum
			requestPacket.setOperation(new IpcUint32(requestPacketEnum
					.getOperation()));
			// set max repetition as zero
			requestPacket.setMaxRepCount(new IpcUint32(0));
			requestPacket.setOption1(new IpcUint32(requestPacketEnum
					.getOption1()));
			requestPacket.setOption2(new IpcUint32(requestPacketEnum
					.getOption2()));
			requestPacket.setDataType(new IpcUint32(requestPacketEnum
					.getDataType()));
			requestPacket.setKeyType(new IpcUint32(requestPacketEnum
					.getKeyType()));
			// set the key structure
			requestPacket.setKeyStruct(getIpcStructure(
					requestPacketEnum.getKeyStruct(), requestBody,
					uriParameters));
			// set the value structure
			requestPacket.setValStruct(getIpcStructure(
					requestPacketEnum.getValStruct(), requestBody,
					uriParameters));

			/*
			 * update request packet as per the received json parameters used in
			 * read like operations
			 */
			if (requestPacketEnum.getOperation() == UncOperationEnum.UNC_OP_READ
					.ordinal()) {
				setCommonReadParams(requestBody);
			} else {
				LOG.debug("No need to update the parameters of request packet");
			}
			requestPacketEnumName = requestPacketEnum.name();
		} else {
			LOG.warning("Request Packet Enum cannot be null");
		}
		LOG.trace("Complete IpcRequestProcessor#createIpcRequestPacket()");
	}

	/**
	 * Update IPC request packet for READ operation according to JSON parameters
	 * 
	 * @param requestBody
	 * @throws VtnServiceException
	 */
	private void setCommonReadParams(final JsonObject requestBody)
			throws VtnServiceException {

		LOG.trace("Start IpcRequestProcessor#setCommonReadParams()");
		/*
		 * Update the value of max repetition count, if not received in the
		 * request then set the default value from configuration file
		 */
		setMaxRepCount(requestBody);

		/*
		 * Update the data type value received in the request json
		 */
		setDataType(requestBody);

		/*
		 * No need to update the OP type in common part Most of the API contains
		 * the OP type as normal For exception APIs OP type must set in
		 * corresponding resource classes
		 */
		setOperationType(requestBody);
		LOG.trace("Complete IpcRequestProcessor#setCommonReadParams()");
	}

	/**
	 * Update operation type according to "op" parameter's value
	 * 
	 * @param requestBody
	 */
	private void setOperationType(final JsonObject requestBody) {

		LOG.trace("Start IpcRequestProcessor#setOperationType()");
		// check if "op" parameter is received
		if (requestBody.has(VtnServiceJsonConsts.OP)) {
			// get value of "op" parameter
			final String opType = requestBody
					.getAsJsonPrimitive(VtnServiceJsonConsts.OP).getAsString()
					.trim();
			// set the op type to request packet
			if (opType.equalsIgnoreCase(VtnServiceJsonConsts.COUNT)) {
				/*
				 * No need to update option1 value on the basis of "op"
				 * parameter's value requestPacket.setOption1(new
				 * IpcUint32(UncOption1Enum.UNC_OPT1_COUNT.ordinal()));
				 */
				// operation type is read_sibling_begin_count, if op is count
				requestPacket.setOperation(new IpcUint32(
						UncOperationEnum.UNC_OP_READ_SIBLING_COUNT.ordinal()));
			} else if (opType.equalsIgnoreCase(VtnServiceJsonConsts.DETAIL)
					|| opType.equalsIgnoreCase(VtnServiceJsonConsts.NORMAL)
					|| opType.equalsIgnoreCase(VtnServiceJsonConsts.INFO)) {
				/*
				 * No need to update option1 value on the basis of "op"
				 * parameter's value requestPacket.setOption1(new
				 * IpcUint32(UncOption1Enum.UNC_OPT1_COUNT.ordinal()));
				 */

				/*
				 * update the operation from read to read_sibling or
				 * read_sinling_index
				 */
				if (requestBody.has(VtnServiceJsonConsts.INDEX)) {
					requestPacket.setOperation(new IpcUint32(
							UncOperationEnum.UNC_OP_READ_SIBLING.ordinal()));
				} else {
					// operation type is read_sibling_begin, if index is null
					requestPacket.setOperation(new IpcUint32(
							UncOperationEnum.UNC_OP_READ_SIBLING_BEGIN
									.ordinal()));
				}
			} else {
				LOG.debug("No need to change the operation type i.e. READ");
			}
		}
		LOG.trace("Complete IpcRequestProcessor#setOperationType()");
	}

	/**
	 * Update data type according to "targetdb" parameter's value
	 * 
	 * @param requestBody
	 */
	private void setDataType(final JsonObject requestBody) {

		LOG.trace("Start IpcRequestProcessor#setDataType()");
		// check if "targetdb" parameter is received
		if (requestBody.has(VtnServiceJsonConsts.TARGETDB)) {
			// get value of "targetdb" parameter
			final String dataType = requestBody
					.getAsJsonPrimitive(VtnServiceJsonConsts.TARGETDB)
					.getAsString().trim();
			// set the data type to request packet
			if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.STARTUP)) {
				requestPacket.setDataType(new IpcUint32(
						UncDataType.UNC_DT_STARTUP.ordinal()));
			} else if (dataType.equalsIgnoreCase(VtnServiceJsonConsts.RUNNING)) {
				requestPacket.setDataType(new IpcUint32(
						UncDataType.UNC_DT_RUNNING.ordinal()));
			} else if (dataType
					.equalsIgnoreCase(VtnServiceJsonConsts.CANDIDATE)) {
				requestPacket.setDataType(new IpcUint32(
						UncDataType.UNC_DT_CANDIDATE.ordinal()));
			} else {
				LOG.debug("No need to update data type from state to another one");
			}
		} else {
			LOG.debug("No need to do anything");
		}
		LOG.trace("Complete IpcRequestProcessor#setDataType()");
	}

	/**
	 * Update max repetition type according to "max_repetition" parameter's
	 * value
	 * 
	 * @param requestBody
	 * @throws VtnServiceException
	 */
	private void setMaxRepCount(final JsonObject requestBody)
			throws VtnServiceException {

		LOG.trace("Start IpcRequestProcessor#setMaxRepCount()");
		// check id "max_repetition" parameter is received
		if (requestBody.has(VtnServiceJsonConsts.MAX) &&
			 requestBody.getAsJsonPrimitive(VtnServiceJsonConsts.MAX).getAsLong() <
			 VtnServiceConsts.MAX_REP_COUNT) {
			// set the value of "max_repetition" parameter to IPC request
			// parameter
			requestPacket.setMaxRepCount(new IpcUint32(requestBody
					.getAsJsonPrimitive(VtnServiceJsonConsts.MAX).getAsString()));
		} else {
			requestPacket.setMaxRepCount(new IpcUint32(VtnServiceConsts.MAX_REP_COUNT));
		}
		LOG.trace("Complete IpcRequestProcessor#setMaxRepCount()");
	}

	/**
	 * Gets the ipc response packet, after checking the result code
	 * 
	 * @param methodype
	 * 
	 * @return the ipc response packet
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	public final IpcDataUnit[] getIpcResponsePacket()
			throws VtnServiceException {

		LOG.trace("Start IpcRequestProcessor#getIpcResponsePacket()");
		IpcDataUnit[] ipcDataUnits = null;
		try {
			// No response required to be retrieve for NO_SUCH_INSTANCE cases
			if (noSuchInstanceFlag) {
				ipcDataUnits = new IpcDataUnit[0];
				noSuchInstanceFlag = false;
				LOG.debug("Response array with size zero");
			} else {
				// in case of result code is success the create response packet
				// with key and value structures
				final int size = session.getResponseCount();
				ipcDataUnits = new IpcDataUnit[size
						- VtnServiceConsts.IPC_RESUL_CODE_INDEX - 1];
				// iterate response one by one after Result Code
				int index = 0;
				for (int i = VtnServiceConsts.IPC_RESUL_CODE_INDEX + 1; i < size; i++) {
					ipcDataUnits[index++] = session.getResponse(i);
					LOG.debug("Response at index " + i + " "
							+ session.getResponse(i));
				}
			}
		} catch (final IpcException e) {
			exceptionHandler.raise(
					Thread.currentThread().getStackTrace()[1].getClassName()
							+ VtnServiceConsts.HYPHEN
							+ Thread.currentThread().getStackTrace()[1]
									.getMethodName(),
					UncJavaAPIErrorCode.IPC_OP_ERROR.getErrorCode(),
					UncJavaAPIErrorCode.IPC_OP_ERROR.getErrorMessage(), e);
		}
		LOG.trace("Complete IpcRequestProcessor#getIpcResponsePacket()");
		return ipcDataUnits;
	}

	/**
	 * Create Json error object on the basis of result code if result code is
	 * null then do nothing
	 * 
	 * @param resultCode
	 */
	private void createErrorJson(final Integer resultCode) {
		// error code and message required to be update after confirmation
		final JsonObject error = new JsonObject();

		final int keyType = requestPacket.getKeyType().intValue();

		/*
		 * Check the error code either for logical or physical APIs
		 */
		if (keyType >= UncCommonEnum.MIN_LOGICAL_KEYTYPE
				&& keyType <= UncCommonEnum.MAX_LOGICAL_KEYTYPE) {
			// set the physical error code and error message
			final UncErrorBean errorEnum = UncIpcErrorCode
					.getLogicalError(resultCode);
			error.addProperty(VtnServiceJsonConsts.CODE,
					errorEnum.getErrorCode());
			error.addProperty(VtnServiceJsonConsts.MSG,
					errorEnum.getJavaAPIErrorMessage());
			LOG.error(errorEnum.getErrorCodeKey() + VtnServiceConsts.SPACE
					+ errorEnum.getErrorCode() + VtnServiceConsts.SPACE
					+ errorEnum.getJavaAPIErrorMessage()
					+ VtnServiceConsts.SPACE
					+ errorEnum.getSouthboundErrorMessage());
		} else if (keyType >= UncCommonEnum.MIN_PHYSICAL_KEYTYPE
				&& keyType <= UncCommonEnum.MAX_PHYSICAL_KEYTYPE) {
			// set the logical error code and error message
			final UncErrorBean errorEnum = UncIpcErrorCode
					.getPhysicalError(resultCode);
			error.addProperty(VtnServiceJsonConsts.CODE,
					errorEnum.getErrorCode());
			error.addProperty(VtnServiceJsonConsts.MSG,
					errorEnum.getJavaAPIErrorMessage());
			LOG.error(errorEnum.getErrorCodeKey() + VtnServiceConsts.SPACE
					+ errorEnum.getErrorCode() + VtnServiceConsts.SPACE
					+ errorEnum.getJavaAPIErrorMessage()
					+ VtnServiceConsts.SPACE
					+ errorEnum.getSouthboundErrorMessage());
		} else {
			error.addProperty(VtnServiceJsonConsts.CODE,
					UncCommonEnum.UncResultCode.UNC_SERVER_ERROR.getValue());
			error.addProperty(VtnServiceJsonConsts.MSG,
					UncCommonEnum.UncResultCode.UNC_SERVER_ERROR.getMessage());
			LOG.warning("Invalid Error Code Returned from IPC Server!");
		}

		errorJson = new JsonObject();
		errorJson.add(VtnServiceJsonConsts.ERROR, error);
	}

	public final JsonObject getErrorJson() {
		return errorJson;
	}

	/**
	 * Gets the exception handler.
	 * 
	 * @return the exception handler
	 */
	public final VtnServiceExceptionHandler getExceptionHandler() {
		LOG.trace("Return from IpcRequestProcessor#getExceptionHandler()");
		return exceptionHandler;
	}

	/**
	 * Gets the ipc structure.
	 * 
	 * @param structName
	 *            the struct name
	 * @param requestBody
	 *            the request body
	 * @param uriParameters
	 *            the uri parameters
	 * @return the ipc structure
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	private IpcStruct getIpcStructure(final String structName,
			final JsonObject requestBody, final List<String> uriParameters)
			throws VtnServiceException {

		LOG.trace("Start IpcRequestProcessor#getIpcStructure()");
		IpcStruct ipcStruct = null;

		if (VtnServiceUtil.isValidString(structName)
				&& !UncStructEnum.NONE.name().equalsIgnoreCase(structName)) {
			// create one structure generator
			if (stuctGenerator == null) {
				stuctGenerator = new IpcStructFactory();
			}

			try {
				Method method;
				final Class<IpcStructFactory> sourceClass = IpcStructFactory.class;
				// get the method name to get the IpcStruct object for given key
				method = sourceClass.getMethod(
						VtnServiceConsts.STRUCT_METHOD_PREFIX + structName
								+ VtnServiceConsts.STRUCT_METHOD_POSTFIX,
						JsonObject.class, List.class);
				// get IpcStruct object
				ipcStruct = (IpcStruct) method.invoke(stuctGenerator,
						requestBody, uriParameters);
			} catch (final Exception e) {
				exceptionHandler
						.raise(Thread.currentThread().getStackTrace()[1]
								.getClassName()
								+ VtnServiceConsts.HYPHEN
								+ Thread.currentThread().getStackTrace()[1]
										.getMethodName(),
								UncJavaAPIErrorCode.INTERNAL_ERROR
										.getErrorCode(),
								UncJavaAPIErrorCode.INTERNAL_ERROR
										.getErrorMessage(), e);
			}
		}
		LOG.trace("Complete IpcRequestProcessor#getIpcStructure()");
		return ipcStruct;
	}

	/**
	 * Process ipc request.
	 * 
	 * @return the int
	 * @throws VtnServiceException
	 *             the vtn service exception
	 */
	public final int processIpcRequest() throws VtnServiceException {

		LOG.trace("Start IpcRequestProcessor#processIpcRequest()");
		int status = ClientSession.RESP_FATAL;
		/*
		 * set IPC data to session object
		 */
		try {
			if (serviceName != null) {
				LOG.debug("session reset execution");
				session.reset(serviceName, serviceId);
			}
			// add IPC request packet values to session for the further
			// processing at lower layer
			LOG.debug("Session id : " + requestPacket.getSessionId());
			session.addOutput(requestPacket.getSessionId());
			LOG.debug("Config id : " + requestPacket.getConfigId());
			session.addOutput(requestPacket.getConfigId());
			LOG.debug("Operation : " + requestPacket.getOperation());
			session.addOutput(requestPacket.getOperation());
			LOG.debug("Max. Repetition Count : "
					+ requestPacket.getMaxRepCount());
			session.addOutput(requestPacket.getMaxRepCount());
			LOG.debug("Option1 : " + requestPacket.getOption1());
			session.addOutput(requestPacket.getOption1());
			LOG.debug("Option2 : " + requestPacket.getOption2());
			session.addOutput(requestPacket.getOption2());
			LOG.debug("Data Type : " + requestPacket.getDataType());
			session.addOutput(requestPacket.getDataType());
			LOG.debug("Key Type : " + requestPacket.getKeyType());
			session.addOutput(requestPacket.getKeyType());
			LOG.debug("Key Structure : " + requestPacket.getKeyStruct());
			session.addOutput(requestPacket.getKeyStruct());

			// add value structure, if required
			if (requestPacket.getValStruct() != null) {
				LOG.debug("Value Structure: " + requestPacket.getValStruct());
				session.addOutput(requestPacket.getValStruct());
			}

			/*
			 * For exceptional cases, IpcDataUnit data will be set before
			 * invoking the request
			 */
			if (requestPacket.getExtraDataUnits() != null
					&& requestPacket.getExtraDataUnits().length != 0) {
				for (final IpcDataUnit ipcDataUnit : requestPacket
						.getExtraDataUnits()) {
					LOG.debug("extra units : " + ipcDataUnit);
					session.addOutput(ipcDataUnit);
				}
			}

			// execute the operation
			long start = System.currentTimeMillis();
			status = session.invoke();
			LOG.debug("The treatment of under layer cost the following time: "
					+ (System.currentTimeMillis() - start) + "(ms)");
			if (status == ClientSession.RESP_FATAL) {
				throw new IpcException("Server Response Failure");
			} else {
				final String resultCode = IpcDataUnitWrapper
						.getIpcDataUnitValue(session
								.getResponse(VtnServiceConsts.IPC_RESUL_CODE_INDEX));
				LOG.debug("Result code received: " + resultCode);
				final int keyType = requestPacket.getKeyType().intValue();
				if (null == resultCode) {
					final JsonObject error = new JsonObject();
					error.addProperty(VtnServiceJsonConsts.CODE,Integer.toString(
						UncCommonEnum.UncResultCode.UNC_INTERNAL_SERVER_ERROR.getValue()));
					error.addProperty(VtnServiceJsonConsts.MSG,
						UncCommonEnum.UncResultCode.UNC_INTERNAL_SERVER_ERROR.getMessage());
					errorJson = new JsonObject();
					errorJson.add(VtnServiceJsonConsts.ERROR, error);

					throw new VtnServiceException(
								Thread.currentThread()
									.getStackTrace()[1].getMethodName(),
								Integer.toString(UncCommonEnum.UncResultCode
									.UNC_INTERNAL_SERVER_ERROR.getValue()),
								UncCommonEnum.UncResultCode.UNC_INTERNAL_SERVER_ERROR.getMessage());
				} else {
					if (requestPacket.getOperation().intValue() == UncOperationEnum.UNC_OP_READ_SIBLING_BEGIN
							.ordinal()
							|| requestPacket.getOperation().intValue() == UncOperationEnum.UNC_OP_READ_SIBLING
									.ordinal()
							|| requestPacket.getOperation().intValue() == UncOperationEnum.UNC_OP_READ_SIBLING_COUNT
									.ordinal()
							|| VtnServiceInitManager.getReadAsList().contains(
									requestPacketEnumName)
							|| VtnServiceInitManager.getMultiCallList().contains(
									requestPacketEnumName)) {
						if (keyType >= UncCommonEnum.MIN_LOGICAL_KEYTYPE
								&& keyType <= UncCommonEnum.MAX_LOGICAL_KEYTYPE
								&& Integer.parseInt(resultCode) == VtnServiceConsts.UPLL_RC_ERR_NO_SUCH_INSTANCE) {
							noSuchInstanceFlag = true;
							LOG.debug("No such instance case for UPLL");
						} else if (keyType >= UncCommonEnum.MIN_PHYSICAL_KEYTYPE
								&& keyType <= UncCommonEnum.MAX_PHYSICAL_KEYTYPE
								&& Integer.parseInt(resultCode) == VtnServiceConsts.UPPL_RC_ERR_NO_SUCH_INSTANCE) {
							noSuchInstanceFlag = true;
							LOG.debug("No such instance case for UPPL");
						} else {
							LOG.debug(" Either Key Type does not exists or operation is not success");
						}

						if (VtnServiceInitManager.getMultiCallList().contains(
								requestPacketEnumName)
								&& requestPacket.getOperation().intValue() == UncOperationEnum.UNC_OP_READ
										.ordinal()) {
							if (requestPacket
									.getOption2()
									.compareTo(
											IpcDataUnitWrapper
													.setIpcUint32Value((UncOption2Enum.UNC_OPT2_NEIGHBOR
															.ordinal()))) != 0) {
								noSuchInstanceFlag = false;
							}
						}
					} else if (keyType == UncKeyTypeEnum.UNC_KT_PORT.getValue()
							&& Integer.parseInt(resultCode) == VtnServiceConsts.UPPL_RC_ERR_NO_SUCH_INSTANCE) {
						noSuchInstanceFlag = true;
					} else if (keyType == UncKeyTypeEnum.UNC_KT_LOGICAL_PORT.getValue()
							&& requestPacket.getOption2().intValue() == UncOption2Enum.UNC_OPT2_BOUNDARY.ordinal()
							&& Integer.parseInt(resultCode) == VtnServiceConsts.UPPL_RC_ERR_NO_SUCH_INSTANCE) {
						noSuchInstanceFlag = true;
					} else if (keyType == UncKeyTypeEnum.UNC_KT_LOGICAL_MEMBER_PORT.getValue()
							&& requestPacket.getOption2().intValue() == UncOption2Enum.UNC_OPT2_NEIGHBOR.ordinal()
							&& Integer.parseInt(resultCode) == VtnServiceConsts.UPPL_RC_ERR_NO_SUCH_INSTANCE) {
						noSuchInstanceFlag = true;
					}

					// if return code is not success, then create the error Json for
					// received result code
					if (UncIpcErrorCode.RC_SUCCESS != Integer.parseInt(resultCode)) {
						if (noSuchInstanceFlag) {
							status = UncResultCode.UNC_SUCCESS.getValue();
						} else {
							createErrorJson(Integer.parseInt(resultCode));
							throw new VtnServiceException(
								Thread.currentThread()
									.getStackTrace()[1].getMethodName(),
								getErrorJson().get(VtnServiceJsonConsts.ERROR).getAsJsonObject()
									.get(VtnServiceJsonConsts.CODE).toString(),
								getErrorJson().get(VtnServiceJsonConsts.ERROR).getAsJsonObject()
									.get(VtnServiceJsonConsts.MSG).toString());
						}
					} else {
						status = UncResultCode.UNC_SUCCESS.getValue();
					}
				}
			}
		} catch (final IpcException e) {
			exceptionHandler.raise(
					Thread.currentThread().getStackTrace()[1].getClassName()
							+ VtnServiceConsts.HYPHEN
							+ Thread.currentThread().getStackTrace()[1]
									.getMethodName(),
					UncJavaAPIErrorCode.IPC_OP_ERROR.getErrorCode(),
					UncJavaAPIErrorCode.IPC_OP_ERROR.getErrorMessage(), e);
		}
		LOG.trace("Complete IpcRequestProcessor#processIpcRequest()");
		return status;
	}
}
