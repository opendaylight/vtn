/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.openstack.beans;

/**
 * Bean class for os_fl_tbl
 */
public class FlowListBean {

	private int flId;
	private String flName;
	private int flStatus;

	public int getFlId() {
		return flId;
	}

	public void setFlId(int flId) {
		this.flId = flId;
	}

	public String getFlName() {
		return flName;
	}

	public void setFlName(String flName) {
		this.flName = flName;
	}
	
	public int getFlStatus() {
		return flStatus;
	}

	public void setFlStatus(int flStatus) {
		this.flStatus = flStatus;
	}
}
