/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.ipc.enums;

// TODO: Auto-generated Javadoc
/**
 * The Class UncSYSMGEnums.
 */
public class UncSYSMGEnums {

	// IPC Channel Name
	public static final String SYSMG_IPC_CHANNEL_NAME = "unc_sysmg";

	// IPC Service Name
	public static final String NOMG_IPC_SERVICE_NAME = "unc_nomg";
	public static final String MGMT_IPC_SERVICE_NAME = "unc_mgmt";

	// Node Manager IPC Service ID
	/**
	 * The Enum NodeMgrServiceID.
	 */
	public enum NodeMgrServiceID{
	  kNomgSoftVersionGet ,
	  kAlarmStatusListGet,
	  kAlarmClear,
	  kResourceStatusGet,
	  kNomgIpcNipcs,       //  Number of IPC Service IDs
	}
 
	// Management IPC Service ID 
	/**
	 * The Enum MgmtServiceID.
	 */
	public enum MgmtServiceID{
		 kMgmtSyslogSeveritySet ,
		 kMgmtSyslogSeverityUnset,
		 kMgmtSyslogSeverityGetSettings,
		 kMgmtTracelogSeveritySet,
		 kMgmtTracelogSeverityUnset,
		 kMgmtTracelogSeverityGetSettings,
		 kMgmtTrapEnableSet,
		 kMgmtTrapEnableUnset,
		 kMgmtTrapEnableGetSettings,
		 kMgmtTrapDestinationSet,
		 kMgmtTrapDestinationUnset,
		 kMgmtTrapDestinationGetSettings,
		 kMgmtTrapSourceSet,
		 kMgmtTrapSourceUnset,
		 kMgmtTrapSourceGetSettings,
		 kMgmtPromptSet,
		 kMgmtPromptUnset,
		 kMgmtPromptGetSettings,
		 kMgmtServiceCount;  // Number of IPC Service IDs
	   
	  
	  
	} 

	// Nomg Return code
	/**
	 * The Enum NodeMgrReturnCode.
	 */
	public enum NodeIpcErrorT{
	  NOMG_E_OK("Success"),
	  NOMG_E_NG("Error"),
	  NOMG_E_NOENT("No Entry"),
	  NOMG_E_INVAL("Invalid"),
	  NOMG_E_INITING("Initiating"),
	  NOMG_E_ABRT("Abort");
		private String message;

		/**
		 * Instantiates a new node mgr return code.
		 *
		 * @param message the message
		 */
		private NodeIpcErrorT(String message) {
			this.message = message;
		}

		/**
		 * Gets the message.
		 *
		 * @return the message
		 */
		public String getMessage() {
			return message;
		}
	} 

	// Management Return code
	/**
	 * The Enum MgmtReturnCode.
	 */
	public enum MgmtIpcErrorT{
	  MGMT_E_OK("Success"),
	  MGMT_E_NG("Error"),
	  MGMT_E_NO_ENTRY("No entry"),
	  MGMT_E_DB_ACCESS_FAILED("Database access failed"), 
	  MGMT_E_INVALID_APLNAME("Invalid name"),
	  MGMT_E_INVALID_LEVEL("Invalid level"),
	  MGMT_E_INVALID_DESTINATION("Invalid destination"),
	  MGMT_E_INVALID_COMMUNITY("Invalid community"),
	  MGMT_E_INVALID_IPADDR("Invalid IP address"),
	  MGMT_E_INVALID_PROMPT("Invalid promt");
	  
	  private String message;

		/**
		 * Instantiates a new mgmt return code.
		 *
		 * @param message the message
		 */
		private MgmtIpcErrorT(String message) {
			this.message = message;
		}

		/**
		 * Gets the message.
		 *
		 * @return the message
		 */
		public String getMessage() {
			return message;
		}
	}
	
	// Management Return code
		/**
	 * The Enum AlarmLevelT.
	 */
	public enum AlarmLevelT{
			ALM_EMERG("0"),
			ALM_ALERT("1"),
			ALM_CRITICAL("2"),
			ALM_ERROR("3"),
			ALM_WARNING("4"),
			ALM_NOTICE("5"),
			ALM_INFO("6"),
			ALM_DEBUG("7");
			
			private String value;

			/**
			 * Instantiates a new alarm level t.
			 *
			 * @param value the value
			 */
			private AlarmLevelT(final String value) {
				this.value = value;
			}

			/**
			 * Gets the value.
			 *
			 * @return the value
			 */
			public String getValue() {
				return value;
			}

		}
}
