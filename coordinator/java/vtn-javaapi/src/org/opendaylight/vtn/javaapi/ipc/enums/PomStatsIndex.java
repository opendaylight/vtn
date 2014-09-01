/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.ipc.enums;

public class PomStatsIndex {
	private Integer software;
	private Integer existingFlow;
	private Integer expiredFlow;
	private Integer total;

	public final Integer getSoftware() {
		return software;
	}

	public final void setSoftware(final Integer software) {
		this.software = software;
	}

	public final Integer getExistingFlow() {
		return existingFlow;
	}

	public final void setExistingFlow(final Integer existingFlow) {
		this.existingFlow = existingFlow;
	}

	public final Integer getExpiredFlow() {
		return expiredFlow;
	}

	public final void setExpiredFlow(final Integer expiredFlow) {
		this.expiredFlow = expiredFlow;
	}

	public final Integer getTotal() {
		return total;
	}

	public final void setTotal(final Integer total) {
		this.total = total;
	}
}
