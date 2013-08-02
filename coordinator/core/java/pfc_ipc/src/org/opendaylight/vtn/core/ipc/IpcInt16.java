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
 *   The {@code IpcInt16} is an IPC additional data unit class which
 *   represents a signed 16-bit integer value.
 * </p>
 *
 * @since	C10
 */
public final class IpcInt16 extends IpcNumber implements Comparable<IpcInt16>
{
	/**
	 * The signed 16-bit value.
	 */
	private final short  _value;

	/**
	 * Construct a signed 16-bit data.
	 *
	 * @param value		A value for a new object.
	 */
	public IpcInt16(short value)
	{
		_value = value;
	}

	/**
	 * <p>
	 *   Construct a new signed 16-bit data indicated by the specified
	 *   string.
	 * </p>
	 * <p>
	 *   This method uses {@link Short#decode(String)} to parse the
	 *   specified string.
	 * </p>
	 *
	 * @param str	The string to be parsed.
	 * @throws NullPointerException
	 *	{@code str} is null.
	 * @throws NumberFormatException
	 *	{@code str} does not contain parsable value.
	 * @see	Short#decode(String)
	 */
	public IpcInt16(String str)
	{
		Short s = Short.decode(str);
		_value = s.shortValue();
	}

	/**
	 * Return the type identifier associated with this data.
	 *
	 * @return	{@link IpcDataUnit#INT16} is always returned.
	 */
	@Override
	public int getType()
	{
		return INT16;
	}

	/**
	 * Get the value of this object as {@code byte} value.
	 * Note that returned value may be truncated.
	 *
	 * @return	The value of this object after conversion to
	 *		{@code byte}.
	 */
	@Override
	public byte byteValue()
	{
		return (byte)_value;
	}

	/**
	 * Get the value of this object as {@code short} value.
	 *
	 * @return	The value of this object after conversion to
	 *		{@code short}.
	 */
	@Override
	public short shortValue()
	{
		return _value;
	}

	/**
	 * Get the value of this object as {@code int} value.
	 *
	 * @return	The value of this object after conversion to
	 *		{@code int}.
	 */
	@Override
	public int intValue()
	{
		return (int)_value;
	}

	/**
	 * Get the value of this object as {@code long} value.
	 *
	 * @return	The value of this object after conversion to
	 *		{@code long}.
	 */
	@Override
	public long longValue()
	{
		return (long)_value;
	}

	/**
	 * Get the value of this object as {@code float} value.
	 * Note that returned value may be rounded.
	 *
	 * @return	The value of this object after conversion to
	 *		{@code float}.
	 */
	@Override
	public float floatValue()
	{
		return (float)_value;
	}

	/**
	 * Get the value of this object as {@code double} value.
	 * Note that returned value may be rounded.
	 *
	 * @return	The value of this object after conversion to
	 *		{@code double}.
	 */
	@Override
	public double doubleValue()
	{
		return (double)_value;
	}

	/**
	 * Return a {@code String} which represents this value.
	 *
	 * @return	A {@code String} which represents this value in
	 *		base 10.
	 */
	@Override
	public String toString()
	{
		return Integer.toString((int)_value);
	}

	/**
	 * Return a hash code of this value.
	 *
	 * @return	A hash code of this value.
	 */
	@Override
	public int hashCode()
	{
		return (int)_value;
	}

	/**
	 * Determine whether the specified object is identical to this object.
	 *
	 * @param obj	An object to be compared.
	 * @return	True is returned if the specified object is identical
	 *		to this object. Otherwise false is returned.
	 */
	@Override
	public boolean equals(Object obj)
	{
		if (obj instanceof IpcInt16) {
			return (_value == ((IpcInt16)obj)._value);
		}

		return false;
	}

	/**
	 * Compare this object to the specified {@code IpcInt16} object.
	 *
	 * @param obj	An {@code IpcInt16} object to be compared.
	 * @return	A negative integer, zero, or a positive integer is
	 *		returned if this object is less than, equal to,
	 *		or greater than the specified object respectively.
	 */
	@Override
	public int compareTo(IpcInt16 obj)
	{
		return _value - obj._value;
	}

	/**
	 * Append this data to the end of the additional data in the
	 * specified IPC client session
	 *
	 * Note that this method must be called by an internal method of
	 * {@link ClientSession}.
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
	 * Set this value to the specified IPC structure field.
	 * This method is called by {@link IpcStruct#set(String, IpcDataUnit)}.
	 *
	 * @param target	The target {@link IpcStruct} instance.
	 * @param name		The name of the target field in {@code target}.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a valid field name.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @see	IpcStruct#set(String, IpcDataUnit)
	 */
	@Override
	void setStruct(IpcStruct target, String name)
	{
		target.setShort(name, -1, INT16, _value);
	}

	/**
	 * Set this value into an array element at the specified IPC
	 * structure field.
	 * This method is called by
	 * {@link IpcStruct#set(String, int, IpcDataUnit)}.
	 *
	 * @param target	The target {@link IpcStruct} instance.
	 * @param name		The name of the target field in {@code target}.
	 * @param index		An array index. It is guaranteed by the caller
	 *			that a negative value is not specified.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	{@code index} is greater than or equal to its length.
	 * @see	IpcStruct#set(String, int, IpcDataUnit)
	 */
	@Override
	void setStruct(IpcStruct target, String name, int index)
	{
		target.setShort(name, index, INT16, _value);
	}

	/**
	 * Append this data to the end of the additional data in the
	 * specified IPC client session
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
	private native void addClientOutput(long session, short value)
		throws IpcException;
}
