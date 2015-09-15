/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.openstack.beans;

import java.util.Objects;

/**
 * Bean class for os_ff_vrt_tbl
 */
public class FlowFilterVrtBean {

	private String vtnName;
	private String vrtName;
	private String vrtIfName;
	private String flName;
	private String vbrName;

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

	public String getFlName() {
		return flName;
	}

	public void setFlName(String flName) {
		this.flName = flName;
	}

	public String getVbrName() {
		return vbrName;
	}

	public void setVbrName(String vbrName) {
		this.vbrName = vbrName;
	}
	
	@Override
	public boolean equals(Object obj) {   
		if (obj == this) {
			return true;
		}

		if (obj instanceof FlowFilterVrtBean) {
			FlowFilterVrtBean u = (FlowFilterVrtBean)obj;
			return this.vtnName.equals(u.vtnName)
				&& this.vrtName.equals(u.vrtName)
				&& this.vrtIfName.equals(u.vrtIfName)
				&& this.flName.equals(u.flName)
				&& this.vbrName.equals(u.vbrName);
		}
		return false;
	}

	@Override
	public int hashCode() {
		return Objects.hash(getClass(), vtnName, vrtName, vrtIfName,
				    flName, vbrName);
	}
}
