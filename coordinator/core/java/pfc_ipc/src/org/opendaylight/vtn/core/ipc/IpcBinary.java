/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.ipc;

import java.util.Arrays;

/**
 * <p>
 *   The {@code IpcBinary} is an IPC additional data unit class which
 *   represents a binary data.
 * </p>
 * <p>
 *   Note that {@code IpcBinary} accepts a null data.
 *   If an IPC client sends a null, the IPC server will receive a null.
 * </p>
 *
 * @since	C10
 */
public final class IpcBinary extends IpcDataUnit
{
	/**
	 * <p>
	 *   A binary data.
	 * </p>
	 */
	private final byte[]  _value;

	/**
	 * <p>
	 *   Construct a null binary data.
	 * </p>
	 */
	public IpcBinary()
	{
		_value = null;
	}

	/**
	 * <p>
	 *   Construct a binary data.
	 * </p>
	 *
	 * @param value		A value for a new object.
	 */
	public IpcBinary(byte[] value)
	{
		if (value == null) {
			_value = null;
		}
		else {
			_value = value.clone();
		}
	}

	/**
	 * <p>
	 *   Construct a binary data.
	 *   This constructor sets the specified {@code byte} array itself.
	 * </p>
	 *
	 * @param value		A value for a new object.
	 * @param dummy		A dummy parameter to distinguish this
	 *			constructor from others.
	 */
	IpcBinary(byte[] value, boolean dummy)
	{
		_value = value;
	}

	/**
	 * <p>
	 *   Return the type identifier associated with this data.
	 * </p>
	 *
	 * @return	{@link IpcDataUnit#BINARY} is always returned.
	 */
	@Override
	public int getType()
	{
		return BINARY;
	}

	/**
	 * <p>
	 *   Get the value of this object.
	 *   Note that this method may return a null.
	 * </p>
	 * <p>
	 *   A returned array is a clone of binary data kept by this object.
	 *   So this object does not affect even if returned array is modified.
	 * </p>
	 *
	 * @return	The value of this object.
	 */
	public byte[] getValue()
	{
		if (_value == null) {
			return null;
		}

		return _value.clone();
	}

	/**
	 * <p>
	 *   Return a {@code String} which represents this value.
	 *   If this object keeps a null as data, {@code "null"} is returned.
	 * </p>
	 *
	 * @return	A {@code String} which represents this value.
	 */
	@Override
	public String toString()
	{
		return Arrays.toString(_value);
	}

	/**
	 * <p>
	 *   Return a hash code of this value.
	 * </p>
	 *
	 * @return	A hash code of this value.
	 */
	@Override
	public int hashCode()
	{
		return Arrays.hashCode(_value);
	}

	/**
	 * <p>
	 *   Determine whether the specified object is identical to this object.
	 * </p>
	 *
	 * @param obj	An object to be compared.
	 * @return	True is returned if the specified object is identical
	 *		to this object. Otherwise false is returned.
	 */
	@Override
	public boolean equals(Object obj)
	{
		if (obj instanceof IpcBinary) {
			return Arrays.equals(_value, ((IpcBinary)obj)._value);
		}

		return false;
	}

	/**
	 * <p>
	 *   Append this data to the end of the additional data in the
	 *   specified IPC client session.
	 * </p>
	 * <p>
	 *   Note that this method must be called by an internal method of
	 *   {@link ClientSession}.
	 * </p>
	 *
	 * @param session	An IPC client session handle.
	 * @throws IpcResourceBusyException
	 *	Another IPC service request is being issued on the specified
	 *	session.
	 * @throws IpcShutdownException
	 *	The state of the specified session is <strong>DISCARD</strong>.
	 *	This exception indicates that the specified session is no
	 *	longer available.
	 * @throws IpcBadStateException
	 *	The state of the specified session is <strong>RESULT</strong>.
	 * @throws IpcTooBigDataException
	 *	No more additional data can not be added.
	 */
	@Override
	void addClientOutput(long session) throws IpcException
	{
		addClientOutput(session, _value);
	}

	/**
	 * <p>
	 *   {@code IpcBinary} does not support this method.
	 * </p>
	 *
	 * @param target	The target {@link IpcStruct} instance.
	 * @param name		The name of the target field in {@code target}.
	 * @throws IllegalArgumentException
	 *	Always thrown.
	 * @see	IpcStruct#set(String, IpcDataUnit)
	 */
	@Override
	void setStruct(IpcStruct target, String name)
	{
		throw new IllegalArgumentException
			("IpcBinary is not supported.");
	}

	/**
	 * <p>
	 *   {@code IpcBinary} does not support this method.
	 * </p>
	 *
	 * @param target	The target {@link IpcStruct} instance.
	 * @param name		The name of the target field in {@code target}.
	 * @param index		An array index. It is guaranteed by the caller
	 *			that a negative value is not specified.
	 * @throws IllegalArgumentException
	 *	Always thrown.
	 * @see	IpcStruct#set(String, int, IpcDataUnit)
	 */
	@Override
	void setStruct(IpcStruct target, String name, int index)
	{
		setStruct(target, name);
	}

	/**
	 * <p>
	 *   Append this data to the end of the additional data in the
	 *   specified IPC client session.
	 * </p>
	 *
	 * @param session	An IPC client session handle.
	 * @param value		A value to be added.
	 * @throws IpcResourceBusyException
	 *	Another IPC service request is being issued on the specified
	 *	session.
	 * @throws IpcShutdownException
	 *	The state of the specified session is <strong>DISCARD</strong>.
	 *	This exception indicates that the specified session is no
	 *	longer available.
	 * @throws IpcBadStateException
	 *	The state of the specified session is <strong>RESULT</strong>.
	 * @throws IpcTooBigDataException
	 *	No more additional data can not be added.
	 */
	private native void addClientOutput(long session, byte[] value)
		throws IpcException;
}
