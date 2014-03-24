/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.ipc.enums;

public class UncUPPLEnums {

	public static final String UPPL_IPC_CHN_NAME = "phynwd";
	public static final String UPPL_IPC_SVC_NAME = "uppl";

	public enum SystemState {
		SYSTEM_ST_SIMPLEX, SYSTEM_ST_ACTIVE, SYSTEM_ST_STANDBY;
	}

	public enum ControllerOpStatus {
		CONTROLLER_OPER_DOWN,
		CONTROLLER_OPER_UP,
		CONTROLLER_OPER_AUDIT_WAITING,
		CONTROLLER_OPER_AUDIT_FAILED,
	}

	public enum AuditStatus {
		AUDIT_DISABLED, AUDIT_ENABLED;
	}

	public enum ServiceID {
		UPPL_SVC_CONFIGREQ, UPPL_SVC_READREQ, UPPL_SVC_GLOBAL_CONFIG;
	}

	public enum RowStatus {
		CREATED,
		UPDATED,
		DELETED,
		ROW_VALID,
		ROW_INVALID,
		APPLIED,
		NOTAPPLIED,
		PARTIALLY_APPLIED;
	}

	public enum Valid {
		INVALID, VALID, VALID_WITH_NO_VALUE, NOT_SUPPORTED, NOT_SET
	}

	public enum UncAddlOperationT {
		UNC_OP_IS_CANDIDATE_DIRTY,
		UNC_OP_IMPORT_CONTROLLER_CONFIG,
		UNC_OP_MERGE_CONTROLLER_CONFIG,
		UNC_OP_CLEAR_IMPORT_CONFIG;
	}
}
