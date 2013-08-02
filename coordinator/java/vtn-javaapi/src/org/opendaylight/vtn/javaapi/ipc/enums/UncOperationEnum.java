/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.ipc.enums;

/**
 * The Enum UncOperationEnum.
 */
public enum UncOperationEnum {

	UNC_OP_INVALID,
	UNC_OP_CREATE,
	UNC_OP_DELETE,
	UNC_OP_UPDATE,
	UNC_OP_CONTROL,
	UNC_OP_RENAME,
	UNC_OP_READ,
	UNC_OP_READ_NEXT,
	UNC_OP_READ_BULK,
	UNC_OP_READ_SIBLING,
	UNC_OP_READ_SIBLING_BEGIN,
	UNC_OP_READ_SIBLING_COUNT;
}
