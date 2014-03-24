/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.webapi.enums;

public enum HttpErrorCodeEnum {

	UNC_BAD_REQUEST(40001, "Invalid Format"),
	UNC_UNAUTHORIZED(40100, "Unauthorized"),
	UNC_FORBIDDEN(40300, "Forbidden"),
	UNC_NOT_FOUND(40400, "Not Found"),
	UNC_CUSTOM_NOT_FOUND(40499, "Not Found"),
	UNC_METHOD_NOT_ALLOWED(40500, "Method Not Allowed"),
	UNC_NOT_ACCEPTABLE(40600, "Not Acceptable"),
	UNC_UNSUPPORTED_MEDIA_TYPE(41500, "Unsupported Media Type"),
	UNC_INTERNAL_SERVER_ERROR(50000, "Internal Server Error"),
	UNC_SERVICE_UNAVAILABLE(50301, "Server Busy"),
	UNC_STATUS_OK(200, "Server Busy");

	private final int code;
	private String message;

	private HttpErrorCodeEnum(final int code, final String messgae) {
		this.code = code;
		this.message = messgae;
	}

	public int getCode() {
		return code;
	}

	public String getMessage() {
		return message;
	}
}
