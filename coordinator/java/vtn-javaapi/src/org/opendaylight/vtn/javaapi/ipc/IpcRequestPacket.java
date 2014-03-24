/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.ipc;

import org.opendaylight.vtn.core.ipc.IpcDataUnit;
import org.opendaylight.vtn.core.ipc.IpcStruct;
import org.opendaylight.vtn.core.ipc.IpcUint32;

/**
 * The Class IpcRequestPacket. Bean class for IPC Request Structure
 */
public class IpcRequestPacket {

	private IpcUint32 sessionId;
	private IpcUint32 configId;
	private IpcUint32 operation;
	private IpcUint32 maxRepCount;
	private IpcUint32 option1;
	private IpcUint32 option2;
	private IpcUint32 dataType;
	private IpcUint32 keyType;
	private IpcStruct keyStruct;
	private IpcStruct valStruct;
	private IpcDataUnit extraDataUnits[];

	/**
	 * Gets the session id.
	 * 
	 * @return the session id
	 */
	public final IpcUint32 getSessionId() {
		return sessionId;
	}

	/**
	 * Sets the session id.
	 * 
	 * @param sessionId
	 *            the new session id
	 */
	public final void setSessionId(final IpcUint32 sessionId) {
		this.sessionId = sessionId;
	}

	/**
	 * Gets the config id.
	 * 
	 * @return the config id
	 */
	public final IpcUint32 getConfigId() {
		return configId;
	}

	/**
	 * Sets the config id.
	 * 
	 * @param configId
	 *            the new config id
	 */
	public final void setConfigId(final IpcUint32 configId) {
		this.configId = configId;
	}

	/**
	 * Gets the operation.
	 * 
	 * @return the operation
	 */
	public final IpcUint32 getOperation() {
		return operation;
	}

	/**
	 * Sets the operation.
	 * 
	 * @param opretaion
	 *            the new operation
	 */
	public final void setOperation(final IpcUint32 operation) {
		this.operation = operation;
	}

	/**
	 * Gets the max rep count.
	 * 
	 * @return the max rep count
	 */
	public final IpcUint32 getMaxRepCount() {
		return maxRepCount;
	}

	/**
	 * Sets the max rep count.
	 * 
	 * @param maxRepCount
	 *            the new max rep count
	 */
	public final void setMaxRepCount(final IpcUint32 maxRepCount) {
		this.maxRepCount = maxRepCount;
	}

	/**
	 * Gets the option1.
	 * 
	 * @return the option1
	 */
	public final IpcUint32 getOption1() {
		return option1;
	}

	/**
	 * Sets the option1.
	 * 
	 * @param option1
	 *            the new option1
	 */
	public final void setOption1(final IpcUint32 option1) {
		this.option1 = option1;
	}

	/**
	 * Gets the option2.
	 * 
	 * @return the option2
	 */
	public final IpcUint32 getOption2() {
		return option2;
	}

	/**
	 * Sets the option2.
	 * 
	 * @param option2
	 *            the new option2
	 */
	public final void setOption2(final IpcUint32 option2) {
		this.option2 = option2;
	}

	/**
	 * Gets the data type.
	 * 
	 * @return the data type
	 */
	public final IpcUint32 getDataType() {
		return dataType;
	}

	/**
	 * Sets the data type.
	 * 
	 * @param dataType
	 *            the new data type
	 */
	public final void setDataType(final IpcUint32 dataType) {
		this.dataType = dataType;
	}

	/**
	 * Gets the key type.
	 * 
	 * @return the key type
	 */
	public final IpcUint32 getKeyType() {
		return keyType;
	}

	/**
	 * Sets the key type.
	 * 
	 * @param keyType
	 *            the new key type
	 */
	public final void setKeyType(final IpcUint32 keyType) {
		this.keyType = keyType;
	}

	/**
	 * Gets the key struct.
	 * 
	 * @return the key struct
	 */
	public final IpcStruct getKeyStruct() {
		return keyStruct;
	}

	/**
	 * Sets the key struct.
	 * 
	 * @param keyStruct
	 *            the new key struct
	 */
	public final void setKeyStruct(final IpcStruct keyStruct) {
		this.keyStruct = keyStruct;
	}

	/**
	 * Gets the val struct.
	 * 
	 * @return the val struct
	 */
	public final IpcStruct getValStruct() {
		return valStruct;
	}

	/**
	 * Sets the val struct.
	 * 
	 * @param valStruct
	 *            the new val struct
	 */
	public final void setValStruct(final IpcStruct valStruct) {
		this.valStruct = valStruct;
	}

	/**
	 * Get the array of extra IpcDataUnit required to process the request
	 * 
	 * @return
	 */
	public final IpcDataUnit[] getExtraDataUnits() {
		return extraDataUnits;
	}

	/**
	 * Set the array of extra IpcDataUnit required to process the request
	 * 
	 * @param extraDataUnits
	 */
	public final void setExtraDataUnits(final IpcDataUnit[] extraDataUnits) {
		this.extraDataUnits = extraDataUnits;
	}
}
