/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.openstack.beans;

/**
 * Bean class for os_vrt_if_tbl
 */
public class VRouterInterfaceBean {

	private int vrtIfId;
	private String vtnName;
	private String vrtName;
	private String vrtIfName;
	private String vbrName;

	public int getVrtIfId() {
		return vrtIfId;
	}

	public void setVrtIfId(int vrtIfId) {
		this.vrtIfId = vrtIfId;
	}

	public String getVtnName() {
		return vtnName;
	}

	public void setVtnName(String vtnName) {
		this.vtnName = vtnName;
	}

	public String getVrtName() {
		return vrtName;
	}

	public void setVrtName(String vrtName) {
		this.vrtName = vrtName;
	}

	public String getVrtIfName() {
		return vrtIfName;
	}

	public void setVrtIfName(String vrtIfName) {
		this.vrtIfName = vrtIfName;
	}

	public String getVbrName() {
		return vbrName;
	}

	public void setVbrName(String vbrName) {
		this.vbrName = vbrName;
	}
}
