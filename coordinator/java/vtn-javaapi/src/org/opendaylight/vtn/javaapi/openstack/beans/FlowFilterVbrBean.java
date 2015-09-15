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
 * Bean class for os_ff_vbr_tbl
 */
public class FlowFilterVbrBean {

	private String vtnName;
	private String vbrName;
	private String vbrIfName;
	private String flName;

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

	public String getFlName() {
		return flName;
	}

	public void setFlName(String flName) {
		this.flName = flName;
	}
	
	@Override
	public boolean equals(Object obj) {
		if (obj == this) {
			return true;
		}

		if (obj instanceof FlowFilterVbrBean) {
			FlowFilterVbrBean u = (FlowFilterVbrBean)obj;
			return this.vtnName.equals(u.vtnName)
				&& this.vbrIfName.equals(u.vbrIfName)
				&& this.flName.equals(u.flName)
				&& this.vbrName.equals(u.vbrName);
		}
		return false;
	}

	@Override
	public int hashCode() {
		return Objects.hash(getClass(), vtnName, vbrName, vbrIfName,
				    flName);
	}
}
