/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.ipc.enums;

public enum UncJavaAPIErrorCode {

	INIT_ERROR("ER001", "JavaAPI Initialization Failed"),
	CONN_INIT_ERROR("ER002", "Connection Initialization Failure"),
	IPC_CONN_ERROR("ER003", "IPC Connection Openning Failure"),
	IPC_SESS_ERROR("ER004", "IPC Session Creation Failure"),
	SESS_ERROR("ER005", "Session Creation Failure"),
	ROLLBACK_ERROR("ER006", "Rollbacking Operation Failure"),
	IPC_OP_ERROR("ER007", "IPC Operation Failure"),
	IPC_SERVER_ERROR("ER008", "IPC Server Error"),
	INTERNAL_ERROR("ER009", "VtnService Internal Failure"),

	RESOURCE_PATH_ERROR("ER010", "Set Resource Path Failure"),
	POST_ERROR("ER011", "Post Operation Failure"),
	PUT_ERROR("ER012", "Put Operation Failure"),
	GET_ERROR("ER013", "Get Operation Failure"),
	DELETE_ERROR("ER014", "Delete Operation Failure"),
	RESOURCE_NOT_FOUND_ERROR("ER015", "Resource Not Found Error"),
	VALIDATION_ERROR("ER016", "Validation Failure"),

	RESOURCE_SCAN_ERROR("ER017", "Resource Package Scanning Failure"),
	RESOURCE_LOAD_ERROR("ER018", "Resource Loading Failure"),

	COMMON_CONFIG_ERROR("ER019", "Resource Loading Failure"),
	APP_CONFIG_ERROR("ER020", "Resource Loading Failure"),
	POOL_SIZE_ERROR("ER021", "Connection Pool Size Invalid"),

	DB_CONN_INIT_ERROR("ER022", "Database Connection Initialization Failure");

	private String errorCode;
	private String errorMessage;

	private UncJavaAPIErrorCode(final String errorCode,
			final String errorMessage) {
		this.errorCode = errorCode;
		this.errorMessage = errorMessage;
	}

	public String getErrorCode() {
		return errorCode;
	}

	public String getErrorMessage() {
		return errorMessage;
	}
}
