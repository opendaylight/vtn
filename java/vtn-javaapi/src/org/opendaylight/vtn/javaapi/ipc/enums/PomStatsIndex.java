/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.ipc.enums;

public class PomStatsIndex {
	private Integer software;

	public Integer getSoftware() {
		return software;
	}

	public void setSoftware(final Integer software) {
		this.software = software;
	}

	public Integer getExistingFlow() {
		return existingFlow;
	}

	public void setExistingFlow(final Integer existingFlow) {
		this.existingFlow = existingFlow;
	}

	public Integer getExpiredFlow() {
		return expiredFlow;
	}

	public void setExpiredFlow(final Integer expiredFlow) {
		this.expiredFlow = expiredFlow;
	}

	public Integer getTotal() {
		return total;
	}

	public void setTotal(final Integer total) {
		this.total = total;
	}

	private Integer existingFlow;
	private Integer expiredFlow;
	private Integer total;
}
