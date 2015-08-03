/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.ipc.enums;

public class UncTCEnums {

	public static final String UNC_CHANNEL_NAME = "uncd";
	public static final String TC_SERVICE_NAME = "tc";

	public enum ServiceID {

		TC_CONFIG_SERVICES,
		TC_CANDIDATE_SERVICES,
		TC_STARTUP_DB_SERVICES,
		TC_READ_ACCESS_SERVICES,
		TC_READ_STATUS_SERVICES,
		TC_AUTO_SAVE_SERVICES,
		TC_AUDIT_SERVICES;

	}

	public enum ServiceType {
		TC_OP_CONFIG_ACQUIRE,
		TC_OP_CONFIG_RELEASE,
		TC_OP_CONFIG_ACQUIRE_TIMED,
		TC_OP_CONFIG_ACQUIRE_PARTIAL,
		TC_OP_CONFIG_ACQUIRE_FORCE,
		TC_OP_CANDIDATE_COMMIT,
		TC_OP_CANDIDATE_COMMIT_TIMED,
		TC_OP_CANDIDATE_ABORT,
		TC_OP_CANDIDATE_ABORT_TIMED,
		TC_OP_RUNNING_SAVE,
		TC_OP_CLEAR_STARTUP,
		TC_OP_READ_ACQUIRE,
		TC_OP_READ_RELEASE,
		TC_OP_READ_RUNNING_STATUS,
		TC_OP_READ_STARTUP_STATUS,
		TC_OP_AUTOSAVE_GET,
		TC_OP_AUTOSAVE_ENABLE,
		TC_OP_AUTOSAVE_DISABLE,
		TC_OP_USER_AUDIT,
		TC_OP_DRIVER_AUDIT,
		TC_OP_INVALID
	}

	public enum ConfigMode {
		TC_CONFIG_INVALID,
		TC_CONFIG_GLOBAL,
		TC_CONFIG_REAL,
		TC_CONFIG_VIRTUAL,
		TC_CONFIG_VTN
	}

	public enum OperationStatus {

		/* System Fail over */

		TC_OPER_FAILURE(-11, 50000, "Operation failure"),

		/* Invalid Input Values Passed */

		TC_OPER_INVALID_INPUT(-10, 50000, "Invalid Input Values Passed"),

		TC_OPER_SUCCESS(0, 200, "Success"),

		TC_CONFIG_NOT_PRESENT(100, 50000, "Configuration not present"),

		TC_CONFIG_PRESENT(101, 50301, "Server Busy"),

		TC_STATE_CHANGED(102, 50000, "Cluster status is changed"),

		TC_INVALID_CONFIG_ID(103, 50000, "Invalid config id"),

		TC_INVALID_OPERATION_TYPE(104, 50000, "Invalid operation type"),

		TC_INVALID_SESSION_ID(105, 50000, "Invalid session id"),

		TC_INVALID_STATE(106, 50300, "Invalid state"),

		TC_OPER_ABORT(107, 50000, "Operation abort"),

		TC_SESSION_ALREADY_ACTIVE(108, 50301, "Server Busy"),

		TC_SESSION_NOT_ACTIVE(109, 50000, "Session not active"),

		TC_SYSTEM_BUSY(110, 50301, "Server Busy"),

		TC_SYSTEM_FAILURE(111, 50000, "System failure"),

		TC_OPER_FORBIDDEN(112, 50000, "Operation forbidden in current setup"),
		
		TC_AUDIT_CANCELLED(113, 50300, "Audit may be cancelled."),
		/* Define for commit command start */
		TC_INTERNAL_SERVER_ERROR(1000, 50000, "Internal server error"),

		TC_OPER_SUCCESS_CTR_DISCONNECTED(1001, 20200,
				"Succeed, but resource is disconnected"),

		TC_OPER_SUCCESS_CTRLAPI_FAILURE(1002, 20200,
				"Succeed, but resource is disconnected or error"),

		TC_OPER_SUCCESS_CTR_CONFIG_STATUS_ERR(1003, 20200,
				"Succeed, but resource is error state"),

		TC_OPER_SUCCESS_OTHER_ERROR(1010, 20200,
				"Succeed, but internal error has occurred in resource"),

		TC_OPER_ABORT_INTERNAL_ERR(1011, 50000, "Internal server error"),

		TC_OPER_ABORT_CONFIG_INVAL(1012, 40900,
				"The configuration is not right"),

		TC_OPER_ABORT_CTR_BUSY(1013, 50300,
				"The other user is changing the configuration"),

		TC_OPER_ABORT_CTRLAPI_FAILURE(1014, 50000,
				"Resource is disconnected or error"),

		TC_OPER_ABORT_ERR_DRIVER_NOT_PRESENT(1015, 50000,
				"Internal server error"),

		TC_OPER_ABORT_OTHER_ERROR(1020, 50000, "Internal server error");
		/* Define for commit command end */

		private final String message;
		private final int code;
		private final int errorCode;

		private OperationStatus(final int code, final int errorCode,
				final String message) {
			this.code = code;
			this.errorCode = errorCode;
			this.message = message;
		}

		public String getMessage() {
			return message;
		}

		public int getCode() {
			return code;
		}

		public int getErrorCode() {
			return errorCode;
		}
	}

	public enum RequestIndex {
		TC_REQ_OP_TYPE_INDEX, TC_REQ_SESSION_ID_INDEX, TC_REQ_ARG_INDEX
	}

	public enum ResponseIndex {
		TC_RES_OP_TYPE_INDEX,
		TC_RES_SESSION_ID_INDEX,
		TC_RES_OP_STATUS_INDEX,
		TC_RES_VALUE_INDEX;
	}

	public enum CandidateOperRespIndex {
		TC_CAND_RES_OP_TYPE_INDEX,
		TC_CAND_RES_SESSION_ID_INDEX,
		TC_CAND_RES_CONFIG_ID_INDEX,
		TC_CAND_RES_OP_STATUS_INDEX;
	}

	public enum AutoSave {
		TC_AUTOSAVE_DISABLED("0"), TC_AUTOSAVE_ENABLED("1");

		private final String value;

		AutoSave(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	}
	public enum TcAuditStatus {
		TC_AUDIT_OPER_FAILURE("0"), TC_AUDIT_OPER_SUCCESS("1");

		private final String value;

		TcAuditStatus(final String value) {
			this.value = value;
		}

		public int getValue() {
			return Integer.parseInt(value);
		}
	}

	public enum TcResponseCode {
		TC_ERR_DRIVER_NOT_PRESENT(200),
		TC_INTERNAL_ERR(4000),
		TC_CONFIG_INVAL(4001),
		TC_CTRLAPI_FAILURE(4002),
		TC_CTR_CONFIG_STATUS_ERR(4003),
		TC_CTR_BUSY(4004),
		TC_CTR_DISCONNECTED(4005),
		TC_REQ_NOT_SENT_TO_CTR(4006),
		TC_NO_SUCH_INSTANCE(4007);

		private final int code;

		TcResponseCode(final int code) {
			this.code = code;
		}

		public int getCode() {
			return code;
		}
	}
}
