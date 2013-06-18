/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.ipc.enums;

public class UncTCEnums {


	public static final String UNC_CHANNEL_NAME="uncd";
	public static final String TC_SERVICE_NAME="tc";

	public enum ServiceID{
		
		TC_CONFIG_SERVICES,
		TC_CANDIDATE_SERVICES,
		TC_STARTUP_DB_SERVICES,
		TC_READ_ACCESS_SERVICES,
		TC_AUTO_SAVE_SERVICES,
		TC_AUDIT_SERVICES;
		
	}

	public enum ServiceType{
		  TC_OP_CONFIG_ACQUIRE,
		  TC_OP_CONFIG_RELEASE,
		  TC_OP_CONFIG_ACQUIRE_FORCE,
		  TC_OP_CANDIDATE_COMMIT,
		  TC_OP_CANDIDATE_ABORT,
		  TC_OP_RUNNING_SAVE,
		  TC_OP_CLEAR_STARTUP,
		  TC_OP_READ_ACQUIRE,
		  TC_OP_READ_RELEASE,
		  TC_OP_AUTOSAVE_GET,
		  TC_OP_AUTOSAVE_ENABLE,
		  TC_OP_AUTOSAVE_DISABLE,
		  TC_OP_USER_AUDIT,
		  TC_OP_DRIVER_AUDIT,
		  TC_OP_INVALID
	}

public enum OperationStatus{                   

		  /* System Failover */

		  TC_OPER_FAILURE(-11,"OPeration failure"),

		  /* Invalid Input Values Passed*/

		  TC_OPER_INVALID_INPUT(-10,"Invalid Input Values Passed"),

		  TC_OPER_SUCCESS(0,"Success"),

		  TC_CONFIG_NOT_PRESENT(1,"Configuration not present"),

		  TC_CONFIG_PRESENT(2,"Config present"),

		  TC_INVALID_CONFIG_ID(3,"Invalid config id"),

		  TC_INVALID_OPERATION_TYPE(4,"Invalid operation type"),

		  TC_INVALID_SESSION_ID(5,"Invalid session id"),

		  TC_INVALID_STATE(6,"Invalid state"),

		  TC_OPER_ABORT(7,"Operation abort"),

		  TC_SESSION_ALREADY_ACTIVE(8,"Session already active"),

		  TC_SESSION_NOT_ACTIVE(9,"Session not active"),

		  TC_SYSTEM_BUSY(10,"System busy"),

		  TC_SYSTEM_FAILURE(11,"System failure");                       

		private final String message;
		private final int code;
		  
		private OperationStatus(final int code, final String message) {
			this.code =code;
			this.message = message;
		}

		public String getMessage() {
			return message;
		}

		public int getCode() {
			return code;
		}
	}

	public enum RequestIndex{
		TC_REQ_OP_TYPE_INDEX,
		TC_REQ_SESSION_ID_INDEX,
		TC_REQ_ARG_INDEX
	}

	public enum ResponseIndex{
		TC_RES_OP_TYPE_INDEX,
		TC_RES_SESSION_ID_INDEX,
		TC_RES_OP_STATUS_INDEX,
		TC_RES_VALUE_INDEX;
	}

	public enum CandidateOperRespIndex{
		TC_CAND_RES_OP_TYPE_INDEX,
		TC_CAND_RES_SESSION_ID_INDEX,
		TC_CAND_RES_CONFIG_ID_INDEX,
		TC_CAND_RES_OP_STATUS_INDEX;
	}

	

	public enum AutoSave{
		TC_AUTOSAVE_DISABLED("0"),
		TC_AUTOSAVE_ENABLED("1");

		private final String value;

		AutoSave(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	}

	
}
