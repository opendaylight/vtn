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
 * Bean class for os_vbr_if_tbl
 */
public class VBridgeInterfaceBean {

	private int vbrIfId;
	private String vtnName;
	private String vbrName;
	private String vbrIfName;
	private String mapType;
	private String logicalPortId;
	private int vbrIfStatus;

	public int getVbrIfId() {
		return vbrIfId;
	}

	public void setVbrIfId(int vbrIfId) {
		this.vbrIfId = vbrIfId;
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

	public String getVbrIfName() {
		return vbrIfName;
	}

	public void setVbrIfName(String vbrIfName) {
		this.vbrIfName = vbrIfName;
	}

	public String getMapType() {
		return mapType;
	}

	public void setMapType(String mapType) {
		this.mapType = mapType;
	}

	public void setLogicalPortId(String logicalPortId) {
		this.logicalPortId = logicalPortId;
	}

	public String getLogicalPortId() {
		return logicalPortId;
	}

	public int getVbrIfStatus() {
		return vbrIfStatus;
	}

	public void setVbrIfStatus(int vbrIfStatus) {
		this.vbrIfStatus = vbrIfStatus;
	}
}
