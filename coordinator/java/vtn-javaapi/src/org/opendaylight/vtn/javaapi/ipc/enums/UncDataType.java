/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.ipc.enums;

/**
 * The Enum UncDataType.
 */
public enum UncDataType {

	UNC_DT_INVALID, UNC_DT_STATE, /* Entity database (State and Statistics) */
	UNC_DT_CANDIDATE, /* Candidate configuration */
	UNC_DT_RUNNING, /* Running configuration */
	UNC_DT_STARTUP, /* Startup configuration */
	UNC_DT_IMPORT, /* Import configuration */
	UNC_DT_AUDIT

}
