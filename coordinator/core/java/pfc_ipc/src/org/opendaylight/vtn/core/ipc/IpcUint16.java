/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.ipc;

import org.opendaylight.vtn.core.util.UnsignedInteger;

/**
 * <p>
 *   The {@code IpcUint16} is an IPC additional data unit class which
 *   represents an unsigned 16-bit integer value.
 * </p>
 *
 * @since	C10
 */
public final class IpcUint16 extends IpcNumber implements Comparable<IpcUint16>
{
	/**
	 * The maximum value of an unsigned 16-bit integer.
	 */
	public final static int  MAX_VALUE = 0xffff;

	/**
	 * The unsigned 16-bit value.
	 */
	private final int  _value;

	/**
	 * Construct an unsigned 16-bit data from the specified {@code short}
	 * value.
	 * The specified value is treated as unsigned integer.
	 *
	 * @param value		A value for a new object.
	 */
	public IpcUint16(short value)
	{
		_value = UnsignedInteger.intValue(value);
	}

	/**
	 * <p>
	 *   Construct an unsigned 16-bit data from the specified {@code int}
	 *   value.
	 * </p>
	 * <p>
	 *   Note that the specified value must be in valid range of
	 *   unsigned 16-bit integer. That is, it must be greater than or
	 *   equal to zero, and be less than or equal to {@link #MAX_VALUE}.
	 * </p>
	 *
	 * @param value		A value for a new object.
	 * @throws IllegalArgumentException
	 *	The specified value is out of valid range.
	 */
	public IpcUint16(int value)
	{
		if (value < 0 || value > MAX_VALUE) {
			throw new IllegalArgumentException
				("Out of valid range: " + value);
		}

		_value = value;
	}

	/**
	 * <p>
	 *   Construct a new unsigned 16-bit data indicated by the specified
	 *   string.
	 * </p>
	 * <p>
	 *   This method uses {@link UnsignedInteger#parseShort(String)}
	 *   to parse the specified string.
	 * </p>
	 *
	 * @param str	The string to be parsed.
	 * @throws NullPointerException
	 *	{@code str} is null.
	 * @throws NumberFormatException
	 *	{@code str} does not contain parsable value.
	 * @see	UnsignedInteger#parseShort(String)
	 */
	public IpcUint16(String str)
	{
		short s = UnsignedInteger.parseShort(str);
		_value = UnsignedInteger.intValue(s);

		assert _value <= MAX_VALUE: "Too large value: " + _value;
	}

	/**
	 * Return the type identifier associated with this data.
	 *
	 * @return	{@link IpcDataUnit#UINT16} is always returned.
	 */
	@Override
	public int getType()
	{
		return UINT16;
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
	 * Note that returned value must be treated as unsigned integer.
	 *
	 * @return	The value of this object after conversion to
	 *		{@code short}.
	 * @see	UnsignedInteger
	 */
	@Override
	public short shortValue()
	{
		return (short)_value;
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
		return _value;
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
		return Integer.toString(_value);
	}

	/**
	 * Return a hash code of this value.
	 *
	 * @return	A hash code of this value.
	 */
	@Override
	public int hashCode()
	{
		return _value;
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
		if (obj instanceof IpcUint16) {
			return (_value == ((IpcUint16)obj)._value);
		}

		return false;
	}

	/**
	 * Compare this object to the specified {@code IpcUint16} object.
	 *
	 * @param obj	An {@code IpcUint16} object to be compared.
	 * @return	A negative integer, zero, or a positive integer is
	 *		returned if this object is less than, equal to,
	 *		or greater than the specified object respectively.
	 */
	@Override
	public int compareTo(IpcUint16 obj)
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
		addClientOutput(session, (short)_value);
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
		target.setShort(name, -1, UINT16, (short)_value);
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
		target.setShort(name, index, UINT16, (short)_value);
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
