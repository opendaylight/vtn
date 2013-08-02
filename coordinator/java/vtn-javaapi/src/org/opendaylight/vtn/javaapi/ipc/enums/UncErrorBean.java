/*
 * Copyright (c) 2012-2013 NEC Corporation
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
	
	public String getErrorCodeKey() {
		return errorCodeKey;
	}
	public void setErrorCodeKey(String errorCodeKey) {
		this.errorCodeKey = errorCodeKey;
	}
	public String getErrorCode() {
		return errorCode;
	}
	public void setErrorCode(String errorCode) {
		this.errorCode = errorCode;
	}
	public String getJavaAPIErrorMessage() {
		return javaAPIErrorMessage;
	}
	public void setJavaAPIErrorMessage(String javaAPIErrorMessage) {
		this.javaAPIErrorMessage = javaAPIErrorMessage;
	}
	public String getSouthboundErrorMessage() {
		return southboundErrorMessage;
	}
	public void setSouthboundErrorMessage(String southboundErrorMessage) {
		this.southboundErrorMessage = southboundErrorMessage;
	}
}
