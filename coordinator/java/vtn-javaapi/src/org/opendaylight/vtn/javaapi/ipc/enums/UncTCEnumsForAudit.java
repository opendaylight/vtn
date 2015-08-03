/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.ipc.enums;


public class UncTCEnumsForAudit {

	public enum OperationStatus {
		INVALID_STATE(50300, "Target UNC server is not active node."),
		SYSTEM_BUSY(50300, "Unable to secure exclusion."),
		INVALID_OPERATION_TYPE(50000, "Internal error occurred(invalid operation)."),
		SYSTEM_FAILURE(50000, "Failed audit operation by fatal error."),
		AUDIT_CANCELLED(50300, "Audit may be cancelled."),
		OPER_ABORT(50000, "Failed audit operation."),
		UNKNOWN(50000, "Internal error occurred(unknown operation status).");

		private final int errorCode;
		private final String message;

		private OperationStatus(final int errorCode, final String message) {
			this.errorCode = errorCode;
			this.message = message;
		}

		public String getMessage() {
			return message;
		}

		public int getErrorCode() {
			return errorCode;
		}
	}

	public enum TcResponseCode {
		INTERNAL_ERR(50000, "Internal error occurred."),
		CONFIG_INVAL(40900, "The configuration is not right for specified controller."),
		CTR_BUSY(50300, "The other users is changing the configuration of specified controller."),
		CTR_DISCONNECTED(50000, "Specified controller is disconnected."),
		CTRLAPI_FAILURE(50000, "The configuration is not supported by specified controller."),
		CTR_CONFIG_STATUS_ERR(50300, "Configuration's status is not confirmed."),
		NO_SUCH_INSTANCE(40400, "Specified controller does not exist."),
		ERR_DRIVER_NOT_PRESENT(50000, "Driver not present."),
		UNKNOWN(50000, "Internal error occurred(unknown response code).");

		private final int errorCode;
		private final String message;

		TcResponseCode(final int errorCode, final String message) {
			this.errorCode = errorCode;
			this.message = message;
		}

		public int getErrorCode() {
			return errorCode;
		}
		
		public String getMessage() {
			return message;
		}
	}
}
