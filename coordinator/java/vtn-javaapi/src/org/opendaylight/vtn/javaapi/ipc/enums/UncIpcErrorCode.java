/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.ipc.enums;

import java.util.HashMap;
import java.util.Map;

import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.init.VtnServiceConfiguration;
import org.opendaylight.vtn.javaapi.init.VtnServiceInitManager;
import org.opendaylight.vtn.javaapi.util.OrderedProperties;

public class UncIpcErrorCode {

	public static final int RC_SUCCESS = 0;

	private static final Map<Integer, UncErrorBean> PHYSICAL_CODES = new HashMap<Integer, UncErrorBean>();

	private static final Map<Integer, UncErrorBean> LOGICAL_CODES = new HashMap<Integer, UncErrorBean>();

	private static final Map<Integer, UncTCEnums.OperationStatus> TC_CODES = new HashMap<Integer,UncTCEnums.OperationStatus>();

	private static final Map<Integer, UncSessionEnums.UsessIpcErrE> SESSION_CODES = new HashMap<Integer, UncSessionEnums.UsessIpcErrE>();

	private static final Map<Integer, UncSYSMGEnums.NodeIpcErrorT> NODE_CODES = new HashMap<Integer, UncSYSMGEnums.NodeIpcErrorT>();

	private static final Map<Integer, String> SYSMG_CODES = new HashMap<Integer, String>();
	
	/**
	 * Read the properties of logical and physical errors
	 * Create objects for UncErrorBean and set them in Map
	 */
	public static void initializeErrorsMessages(){
		VtnServiceConfiguration configuration = VtnServiceInitManager.getConfigurationMap();
		
		/*
		 * load the logical errors and put the error bean objects in the map 
		 */
		OrderedProperties upplErrorProperties = configuration.getPhysicalErrorProperties();
		int index = 0;
		for(String errorProp : upplErrorProperties.getValueSet()){
			String values[] = errorProp.split(VtnServiceConsts.COLON);
			UncErrorBean errorCodeBean = new UncErrorBean();
			errorCodeBean.setErrorCode(values[0]);
			errorCodeBean.setJavaAPIErrorMessage(values[1]);
			errorCodeBean.setSouthboundErrorMessage(values[2]);
			errorCodeBean.setErrorCodeKey(upplErrorProperties.getKeySet().get(index));
			PHYSICAL_CODES.put(index, errorCodeBean);
			index++;
		}
		
		/*
		 * load the physical errors and put the error bean objects in the map 
		 */
		OrderedProperties upllErrorProperties = configuration.getLogicalErrorProperties();
		index = 0;
		for(String errorProp : upllErrorProperties.getValueSet()){
			String values[] = errorProp.split(VtnServiceConsts.COLON);
			UncErrorBean errorCodeBean = new UncErrorBean();
			errorCodeBean.setErrorCode(values[0]);
			errorCodeBean.setJavaAPIErrorMessage(values[1]);
			errorCodeBean.setSouthboundErrorMessage(values[2]);
			errorCodeBean.setErrorCodeKey(upllErrorProperties.getKeySet().get(index));
			LOGICAL_CODES.put(index, errorCodeBean);
			index++;
		}
		
		/*
		 * load the TC errors and put the error enums objects in the map 
		 */
		index = 0;
		for(UncTCEnums.OperationStatus operationStatus : UncTCEnums.OperationStatus.values()){
			TC_CODES.put(operationStatus.getCode(), operationStatus);
		}
		
		/*
		 * load the Session errors and put the error enums objects in the map 
		 */
		index = 0;
		for(UncSessionEnums.UsessIpcErrE operationStatus : UncSessionEnums.UsessIpcErrE.values()){
			SESSION_CODES.put(index++, operationStatus);
		}
		
		/*
		 * load the Node errors and put the error enums objects in the map 
		 */
		index = 0;
		for(UncSYSMGEnums.NodeIpcErrorT operationStatus : UncSYSMGEnums.NodeIpcErrorT.values()){
			NODE_CODES.put(index++, operationStatus);
		}
		
		/*
		 * load the Node errors and put the error enums objects in the map 
		 */
		index = 0;
		for(UncSYSMGEnums.MgmtIpcErrorT operationStatus : UncSYSMGEnums.MgmtIpcErrorT.values()){
			SYSMG_CODES.put(index++, operationStatus.getMessage());
		}
		
	}
	
	/**
	 * Get error code enum for the received result code from UPPL
	 * @param errorKey
	 * @return
	 */
	public static UncErrorBean getPhysicalError(final int errorKey){
		return PHYSICAL_CODES.get(errorKey);
	}

	/**
	 * Get error code enum for the received result code from UPLL
	 * @param errorKey
	 * @return
	 */
	public static UncErrorBean getLogicalError(final int errorKey){
		return LOGICAL_CODES.get(errorKey);
	}
	
	/**
	 * Get error message for the received result code from TC
	 * @param errorKey
	 * @return
	 */
	public static UncTCEnums.OperationStatus getTcCodes(final int errorKey) {
		return TC_CODES.get(errorKey);
	}
	
	/**
	 * Get error message for the received result code from Session
	 * @param errorKey
	 * @return
	 */
	public static UncSessionEnums.UsessIpcErrE getSessionCodes(final int errorKey) {
		return SESSION_CODES.get(errorKey);
	}
	
	/**
	 * Get error message for the received result code from Node Manager
	 * @param errorKey
	 * @return
	 */
	public static UncSYSMGEnums.NodeIpcErrorT getNodeCodes(final int errorKey) {
		return NODE_CODES.get(errorKey);
	}
	
	/**
	 * Get error message for the received result code from System Manager
	 * @param errorKey
	 * @return
	 */
	public static String getSysmgCodes(final int errorKey) {
		return SYSMG_CODES.get(errorKey);
	}

}
