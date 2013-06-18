/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.ipc.enums;

public class UncCommonEnum {

	/*
	 * Minimum and Maximum Key Type values for logical and Physical APIs
	 */
	public static final int MIN_LOGICAL_KEYTYPE = 0x001;
	public static final int MAX_LOGICAL_KEYTYPE = 0x1ff;
	public static final int MIN_PHYSICAL_KEYTYPE = 0x200;
	public static final int MAX_PHYSICAL_KEYTYPE = 0x3ff;

	/* Enumeration for Configuration Status. */
	public enum ConfigStatus{
		UNC_CS_UNKNOWN,
		UNC_CS_APPLIED,
		UNC_CS_PARTAILLY_APPLIED,
		UNC_CS_NOT_APPLIED,
		UNC_CS_INVALID,
		UNC_CS_NOT_SUPPORTED
	}

	/* Enumeration for Valid flag. */
	public enum ValidFlag{
		UNC_VF_INVALID,
		UNC_VF_VALID,
		UNC_VF_VALID_NO_VALUE,
		UNC_VF_NOT_SOPPORTED
	}

	/* Controller type enum. */
	public enum ControllerType{
		UNC_CT_UNKNOWN,
		UNC_CT_OPENFLOW,
		UNC_CT_OVERLAY,
		UNC_CT_LEGACY
	}

	/* Operation Status */
	public enum OperationStatus {
		UPLL_OPER_STATUS_UP(1),
		UPLL_OPER_STATUS_DOWN(2),
		UPLL_OPER_STATUS_UNKNOWN(3);

		private final int value;

		private OperationStatus(final int value) {
			this.value = value;
		}

		public int getValue() {
			return value;
		}
	}

	/* JavaAPI return code */
	public enum UncResultCode {

		UNC_SUCCESS(200,"Success"),
		UNC_CLIENT_ERROR(400,"Validation error for parameter: "),
		UNC_SERVER_ERROR(500, "IPC server error");

		private final int value;
		private final String message;

		private UncResultCode(final int value, final String message){
			this.value = value;
			this.message = message;
		}

		public int getValue() {
			return value;
		}
		
		public String getMessage() {
			return message;
		}
	}
}
