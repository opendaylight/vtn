/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.openstack.beans;

/**
 * Bean class for os_vtn_tbl
 */
public class VtnBean {

	private int vtnId;
	private String vtnName;
	private int vtnStatus;

	public int getVtnId() {
		return vtnId;
	}

	public void setVtnId(int vtnId) {
		this.vtnId = vtnId;
	}

	public String getVtnName() {
		return vtnName;
	}

	public void setVtnName(String vtnName) {
		this.vtnName = vtnName;
	}

	public int getVtnStatus() {
		return vtnStatus;
	}

	public void setVtnStatus(int vtnStatus) {
		this.vtnStatus = vtnStatus;
	}
}
