/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.ipc.enums;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.exception.VtnServiceExceptionHandler;
import org.opendaylight.vtn.javaapi.init.VtnServiceInitManager;

import org.opendaylight.vtn.core.util.Logger;

public class UncIpcErrorCode {

	public static final int RC_SUCCESS = 0;

	private static final Map<Integer, UncErrorBean> PHYSICAL_CODES = new HashMap<Integer, UncErrorBean>();

	private static final Map<Integer, UncErrorBean> LOGICAL_CODES = new HashMap<Integer, UncErrorBean>();

	private static final Map<Integer, UncTCEnums.OperationStatus> TC_CODES = new HashMap<Integer, UncTCEnums.OperationStatus>();

	private static final Map<Integer, UncSessionEnums.UsessIpcErrE> SESSION_CODES = new HashMap<Integer, UncSessionEnums.UsessIpcErrE>();

	private static final Map<Integer, UncSYSMGEnums.NodeIpcErrorT> NODE_CODES = new HashMap<Integer, UncSYSMGEnums.NodeIpcErrorT>();

	private static final Map<Integer, String> SYSMG_CODES = new HashMap<Integer, String>();

	/**
	 * Read the properties of logical and physical errors Create objects for
	 * UncErrorBean and set them in Map
	 *
	 * @throws VtnServiceException   Failed to load error definition.
	 */
	public static void initializeErrorsMessages()
		throws VtnServiceException {
		Logger log = Logger.getLogger("UncIpcErrorCode");

		try {
			// Load the logical errors and put the error bean
			// objects in the map.
			loadErrorCodes(log, LOGICAL_CODES,
				       VtnServiceConsts.UPLL_ERRORS_FILEPATH,
				       VtnServiceConsts.
				       UPLL_ERROR_INITIAL_INDEX);
		
			// Load the physical errors and put the error bean
			// objects in the map.
			loadErrorCodes(log, PHYSICAL_CODES,
				       VtnServiceConsts.UPPL_ERRORS_FILEPATH,
				       VtnServiceConsts.
				       UPPL_ERROR_INITIAL_INDEX);
		}
		catch (Exception e) {
			VtnServiceInitManager.getExceptionHandler().
				raise("UncIpcErrorCode-loadErrorCodes()",
				      UncJavaAPIErrorCode.APP_CONFIG_ERROR.
				      getErrorCode(),
				      UncJavaAPIErrorCode.APP_CONFIG_ERROR.
				      getErrorMessage(), e);
		}

		/*
		 * load the TC errors and put the error enums objects in the map
		 */
		for (final UncTCEnums.OperationStatus operationStatus : UncTCEnums.OperationStatus
				.values()) {
			TC_CODES.put(operationStatus.getCode(), operationStatus);
		}

		/*
		 * load the Session errors and put the error enums objects in the map
		 */
		int index = 0;
		for (final UncSessionEnums.UsessIpcErrE operationStatus : UncSessionEnums.UsessIpcErrE
				.values()) {
			SESSION_CODES.put(index++, operationStatus);
		}

		/*
		 * load the Node errors and put the error enums objects in the map
		 */
		index = 0;
		for (final UncSYSMGEnums.NodeIpcErrorT operationStatus : UncSYSMGEnums.NodeIpcErrorT
				.values()) {
			NODE_CODES.put(index++, operationStatus);
		}

		/*
		 * load the Node errors and put the error enums objects in the map
		 */
		index = 0;
		for (final UncSYSMGEnums.MgmtIpcErrorT operationStatus : UncSYSMGEnums.MgmtIpcErrorT
				.values()) {
			SYSMG_CODES.put(index++, operationStatus.getMessage());
		}
	}

	/**
	 * Get error code enum for the received result code from UPPL
	 *
	 * @param errorKey
	 * @return
	 */
	public static UncErrorBean getPhysicalError(final int errorKey) {
		return PHYSICAL_CODES.get(errorKey);
	}

	/**
	 * Get error code enum for the received result code from UPLL
	 *
	 * @param errorKey
	 * @return
	 */
	public static UncErrorBean getLogicalError(final int errorKey) {
		return LOGICAL_CODES.get(errorKey);
	}

	/**
	 * Get error message for the received result code from TC
	 *
	 * @param errorKey
	 * @return
	 */
	public static UncTCEnums.OperationStatus getTcCodes(final int errorKey) {
		return TC_CODES.get(errorKey);
	}

	/**
	 * Get error message for the received result code from Session
	 * 
	 * @param errorKey
	 * @return
	 */
	public static UncSessionEnums.UsessIpcErrE getSessionCodes(
			final int errorKey) {
		return SESSION_CODES.get(errorKey);
	}

	/**
	 * Get error message for the received result code from Node Manager
	 * 
	 * @param errorKey
	 * @return
	 */
	public static UncSYSMGEnums.NodeIpcErrorT getNodeCodes(final int errorKey) {
		return NODE_CODES.get(errorKey);
	}

	/**
	 * Get error message for the received result code from System Manager
	 * 
	 * @param errorKey
	 * @return
	 */
	public static String getSysmgCodes(final int errorKey) {
		return SYSMG_CODES.get(errorKey);
	}

	/**
	 * Load IPC error code from the property file specified by the given
	 * resource name.
	 *
	 * @param log       A logger instance.
	 * @param map       A map to store IPC error codes.
	 * @param resource  The resource name corresponding to the property
	 *                  file.
	 * @param initialErrorIndex
	 *                  Initial error index for UPLL or UPPL.
	 * @throws FileNotFoundException
	 *     Property file was not found.
	 * @throws IOException
	 *     Failed to load property file.
	 */
	private static void loadErrorCodes(Logger log,
					   Map<Integer, UncErrorBean>map,
					   String resource,
					   int initialErrorIndex)
		throws IOException {
		// Load the give property file.
		InputStream in = UncIpcErrorCode.class.getClassLoader().
			getResourceAsStream(resource);
		if (in == null) {
			String msg =
				"Property file was not found: " + resource;
			log.error(msg);
			throw new FileNotFoundException(msg);
		}

		Properties prop = new Properties();
		try {
			prop.load(in);
		} finally {
			try {
				in.close();
			} catch (Exception e) {
				// Ignore this error.
				log.warning(e, "Failed to close IPC error " +
					    "property: name=%s, error=%s",
					    resource, e);
			}
		}

		// Add common error code(UNC_RC_ERR_DRIVER_NOT_PRESENT: 200)
		UncErrorBean commErr = new UncErrorBean();
		commErr.setErrorCodeKey("UNC_RC_ERR_DRIVER_NOT_PRESENT");
		commErr.setErrorCode("50000");
		commErr.setJavaAPIErrorMessage("Internal Server Error");
		commErr.setSouthboundErrorMessage("Resource is disconnected");
		map.put(200, commErr);

		// Construct UncErrorBean instances.
		for (int index = 0; true; index++) {
			String key = prop.getProperty("key." + index);
			if (key == null) {
				break;
			}

			String code = prop.getProperty("code." + index);
			if (code == null) {
				break;
			}

			String java = prop.getProperty("java." + index);
			if (java == null) {
				break;
			}

			String south = prop.getProperty("south." + index);
			if (south == null) {
				break;
			}

			UncErrorBean uerr = new UncErrorBean();
			uerr.setErrorCodeKey(key);
			uerr.setErrorCode(code);
			uerr.setJavaAPIErrorMessage(java);
			uerr.setSouthboundErrorMessage(south);
			if (index == 0) {
				map.put(index, uerr);
			} else if(uerr.getErrorCodeKey().equals("UPLL_RC_ERR_CTR_DISCONNECTED")) {
				//Add common error code(UNC_RC_CTR_DISCONNECTED: 4005)
				map.put(4005, uerr);
			} else if (uerr.getErrorCodeKey().equals("UPLL_RC_ERR_DRIVER_NOT_PRESENT")) {
				//Add common error code(UNC_RC_ERR_DRIVER_NOT_PRESENT: 200)
				map.put(200, uerr);
			} else {
				map.put(index + initialErrorIndex, uerr);
			}
		}
	}
}
