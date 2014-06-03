/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.ipc.enums;

public class UncBaseEnums {

	
	public enum UncRespCode {

		/* System Fail over */

		UNC_RC_INTERNAL_ERR(4000, 50000, "Internal error in any of the modules"),

		/* Invalid Input Values Passed */

		UNC_RC_CONFIG_INVAL(4001, 50000, "Invalid configuiration"),

		UNC_RC_CTRLAPI_FAILURE(4002, 50000, "Controller api failed"),

		UNC_RC_CTR_CONFIG_STATUS_ERR(4003, 50000, "Controller configuration status is not confirmed"),

		UNC_RC_CTR_BUSY(4004, 50301, "Acquiring config mode failed in Controller"),

		UNC_RC_CTR_DISCONNECTED(4005, 50000, "Controller disconnected/down"),

		UNC_RC_REQ_NOT_SENT_TO_CTR(4006, 50000, "Request not sent to Controller"),

		UNC_RC_NO_SUCH_INSTANCE(4007, 40400,"Request for unknown attribute");

		private final String message;
		private final int errorCode;
		private final int code;

		private UncRespCode(final int code, final int errorCode,
				final String message) {
			this.code = code;
			this.errorCode = errorCode;
			this.message = message;
		}

		public String getMessage() {
			return message;
		}

		public int getErrorCode() {
			return errorCode;
		}
		
		public int getCode() {
			return code;
		}
	}
}
