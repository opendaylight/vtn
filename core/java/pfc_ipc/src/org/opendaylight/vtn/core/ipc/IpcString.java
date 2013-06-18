/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.ipc;

/**
 * <p>
 *   The {@code IpcString} is an IPC additional data unit class which
 *   represents a string.
 * </p>
 * <p>
 *   Note that {@code IpcString} accepts a null data.
 *   If an IPC client sends a null, the IPC server will receive a null.
 * </p>
 *
 * @since	C10
 */
public final class IpcString extends IpcDataUnit
	implements Comparable<IpcString>
{
	/**
	 * <p>
	 *   The string contained by this data unit.
	 * </p>
	 */
	private final String  _value;

	/**
	 * <p>
	 *   Construct a null string data.
	 * </p>
	 */
	public IpcString()
	{
		_value = null;
	}

	/**
	 * <p>
	 *   Construct a string data.
	 * </p>
	 *
	 * @param value		A value for a new object.
	 */
	public IpcString(String value)
	{
		_value = value;
	}

	/**
	 * <p>
	 *   Return the type identifier associated with this data.
	 * </p>
	 *
	 * @return	{@link IpcDataUnit#STRING} is always returned.
	 */
	@Override
	public int getType()
	{
		return STRING;
	}

	/**
	 * <p>
	 *   Get the value of this object.
	 *   Note that this method may return a null.
	 * </p>
	 *
	 * @return	The value of this object.
	 */
	public String getValue()
	{
		return _value;
	}

	/**
	 * <p>
	 *   Return a {@code String} which represents this value.
	 *   If this object keeps a null string, {@code "null"} is returned.
	 * </p>
	 *
	 * @return	A {@code String} which represents this value.
	 */
	@Override
	public String toString()
	{
		return (_value == null) ? "null" : _value;
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
		return (_value == null) ? 0 : _value.hashCode();
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
		if (!(obj instanceof IpcString)) {
			return false;
		}

		String ovalue = ((IpcString)obj)._value;
		if (_value == null) {
			return (ovalue == null);
		}

		return _value.equals(ovalue);
	}

	/**
	 * <p>
	 *   Compare this object to the specified {@code IpcString} object.
	 *   Note that a null string data is treated as the smallest.
	 * </p>
	 *
	 * @param obj	An {@code IpcString} object to be compared.
	 * @return	A negative integer, zero, or a positive integer is
	 *		returned if this object is less than, equal to,
	 *		or greater than the specified object respectively.
	 */
	@Override
	public int compareTo(IpcString obj)
	{
		String v = _value;
		String ov = obj._value;

		if (v == null) {
			return (ov == null) ? 0 : -1;
		}

		if (ov == null) {
			return 1;
		}

		return v.compareTo(ov);
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
	 *   {@code IpcString} does not support this method.
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
			("IpcString is not supported.");
	}

	/**
	 * <p>
	 *   {@code IpcString} does not support this method.
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
	private native void addClientOutput(long session, String value)
		throws IpcException;
}
