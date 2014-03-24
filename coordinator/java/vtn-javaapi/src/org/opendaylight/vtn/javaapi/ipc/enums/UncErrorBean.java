/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.ipc.enums;

public class UncErrorBean {

	private String errorCodeKey;
	private String errorCode;
	private String javaAPIErrorMessage;
	private String southboundErrorMessage;

	public final String getErrorCodeKey() {
		return errorCodeKey;
	}

	public final void setErrorCodeKey(final String errorCodeKey) {
		this.errorCodeKey = errorCodeKey;
	}

	public final String getErrorCode() {
		return errorCode;
	}

	public final void setErrorCode(final String errorCode) {
		this.errorCode = errorCode;
	}

	public final String getJavaAPIErrorMessage() {
		return javaAPIErrorMessage;
	}

	public final void setJavaAPIErrorMessage(final String javaAPIErrorMessage) {
		this.javaAPIErrorMessage = javaAPIErrorMessage;
	}

	public final String getSouthboundErrorMessage() {
		return southboundErrorMessage;
	}

	public final void setSouthboundErrorMessage(
			final String southboundErrorMessage) {
		this.southboundErrorMessage = southboundErrorMessage;
	}
}
