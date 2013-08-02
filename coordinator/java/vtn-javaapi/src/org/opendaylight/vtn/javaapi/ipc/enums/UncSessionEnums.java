/*
 * Copyright (c) 2012-2013 NEC Corporation
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
		kUsessSessTypeDel,// delete session of specified session type.
		kUsessEnable, // authenticate enable.
		kUsessDisable, // to cancel the state of the enable.
		kUsessSessCount, // gets number of sessions.
		kUsessSessList, // gets list of session information.
		kUsessSessDetail, // gets detailed Session information.
		kUserUserPasswd, // To change the user password.
		kUserEnablePasswd, // To change the enable password.
		kUsessIpcNipcs; // number of IPC service ID.
	}

	public enum UsessTypeE{
		USESS_TYPE_UNKNOWN("0"),     // Unknown session type.
		USESS_TYPE_CLI("1"),             // CLI session type.
		USESS_TYPE_CLI_DAEMON("2"),      // Resident CLI session type.
		USESS_TYPE_WEB_API("3"),         // WEB API session type.
		USESS_TYPE_WEB_UI("4");

		private final String value;

		private UsessTypeE(final String value){
			this.value = value;
		}

		public String getValue(){
			return value;
		}
	}

	public enum UserTypeE {
		USER_TYPE_UNKNOWN("0"),      	// Unknown user.
		USER_TYPE_OPER("1"),             // operator user.
		USER_TYPE_ADMIN("2");			// administrator user.

		private final String value;

		private UserTypeE(final String value){
			this.value = value;
		}

		public String getValue(){
			return value;
		}
	}

	// error code.
	public enum UsessIpcErrE {
		USESS_E_OK("success"), // success.
		USESS_E_NG("error"), // error.
		USESS_E_INVALID_SESSID("Invalid current session ID"), // Invalid current session ID.
		USESS_E_NO_SUCH_SESSID("Invalid target session ID"), // Invalod target session ID.
		USESS_E_INVALID_PRIVILEGE("Invalid privileges"), // Invalid privileges
		USESS_E_INVALID_MODE("Invalid target session ID"), // Invalid mode.
		USESS_E_INVALID_SESSTYPE("Invalid session type"), // Invalid session type.
		USESS_E_INVALID_USER("Invalid user name"), // Invalid user name.
		USESS_E_INVALID_PASSWD("Invalid password"), // Invalid password.
		USESS_E_SESS_OVER("Over the number of user sessions"); // Over the number of user sessions
		
		private String message;

		private UsessIpcErrE(String message) {
			this.message = message;
		}

		public String getMessage() {
			return message;
		}
	}
}
