/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.ipc.enums;

public class UncSessionEnums {

	// IPC Channel Name
	public static final String UNCD_IPC_CHANNEL = "usess";

	// IPC Service Name
	public static final String UNCD_IPC_SERVICE = "usess";

	// IPC service ID
	public enum ServiceID {
		kUsessSessAdd, // add user authentication and session.
		kUsessSessDel, // delete the session.
		kUsessSessTypeDel, // delete session of specified session type.
		kUsessEnable, // authenticate enable.
		kUsessDisable, // to cancel the state of the enable.
		kUsessSessCount, // gets number of sessions.
		kUsessSessList, // gets list of session information.
		kUsessSessDetail, // gets detailed Session information.
		kUserUserPasswd, // To change the user password.
		kUserEnablePasswd, // To change the enable password.
		kUsessIpcNipcs; // number of IPC service ID.
	}

	// Type of user session.
	public enum UsessTypeE {
		USESS_TYPE_UNKNOWN("0"), // Unknown session type.
		USESS_TYPE_CLI("1"), // CLI session type.
		USESS_TYPE_CLI_DAEMON("2"), // Resident CLI session type.
		USESS_TYPE_WEB_API("3"), // WEB API session type.
		USESS_TYPE_WEB_UI("4");

		private final String value;

		private UsessTypeE(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	}

	// Type of user.
	public enum UserTypeE {
		USER_TYPE_UNKNOWN("0"), // Unknown user.
		USER_TYPE_OPER("1"), // operator user.
		USER_TYPE_ADMIN("2"); // administrator user.

		private final String value;

		private UserTypeE(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	}

	// Mode of user session
	public enum UsessModeE {
		USESS_MODE_UNKNOWN("0"), // Unknown mode.
		USESS_MODE_OPER("1"), // operator mode.
		USESS_MODE_ENABLE("2"), // enable mode.
		USESS_MODE_DEL("3"); // administrator user.

		private final String value;

		private UsessModeE(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	}

	// config mode status.
	public enum UsessConfigModeE {
		CONFIG_STATUS_NONE("0"), // Not Configuration mode.
		CONFIG_STATUS_TCLOCK("1"), // Configuration mode.
		CONFIG_STATUS_TCLOCK_PART("2");

		private final String value;

		private UsessConfigModeE(final String value) {
			this.value = value;
		}

		public String getValue() {
			return value;
		}
	}

	// error code.
	public enum UsessIpcErrE {
		USESS_E_OK(200, "Success"), // success.
		USESS_E_NG(50000, "Internal error"), // error.
		USESS_E_INVALID_SESSID(40001, "Invalid current session ID"), 
		USESS_E_NO_SUCH_SESSID(40400, "Invalid target session ID"),
		USESS_E_INVALID_PRIVILEGE(50000, "Invalid privileges"),
		USESS_E_INVALID_MODE(50000, "Invalid mode"),
		USESS_E_INVALID_SESSTYPE(50000, "Invalid session type"),
		USESS_E_INVALID_USER(40100, "Invalid user name"),
		USESS_E_INVALID_PASSWD(40100, "Invalid password"),
		USESS_E_SESS_OVER(50301, "Over the number of user sessions");

		private final String message;
		private final int code;

		private UsessIpcErrE(final int code, final String message) {
			this.code = code;
			this.message = message;
		}

		public String getMessage() {
			return message;
		}

		public int getCode() {
			return code;
		}
	}
}
