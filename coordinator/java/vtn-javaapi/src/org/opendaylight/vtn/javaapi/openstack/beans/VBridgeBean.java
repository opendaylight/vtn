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
 * Bean class for os_vbr_tbl
 */
public class VBridgeBean {

	private int vbrId;
	private String vtnName;
	private String vbrName;
	private int vbrStatus;

	public int getVbrId() {
		return vbrId;
	}

	public void setVbrId(int vbrId) {
		this.vbrId = vbrId;
	}

	public String getVtnName() {
		return vtnName;
	}

	public void setVtnName(String vtnName) {
		this.vtnName = vtnName;
	}

	public String getVbrName() {
		return vbrName;
	}

	public void setVbrName(String vbrName) {
		this.vbrName = vbrName;
	}

	public int getVbrStatus() {
		return vbrStatus;
	}

	public void setVbrStatus(int vbrStatus) {
		this.vbrStatus = vbrStatus;
	}
}
