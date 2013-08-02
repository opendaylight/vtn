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
 *   The {@code IpcDataUnit} class is an abstract class which represents a
 *   data to be transferred via IPC connection. Additional data of IPC service
 *   is represented by an array of {@code IpcDataUnit} instances.
 * </p>
 *
 * @since	C10
 */
public abstract class IpcDataUnit
{
	/**
	 * Load native library.
	 */
	static {
		ClientLibrary.load();
	}

	/**
	 * Data type identifier associated with signed 8-bit integer.
	 */
	public final static int  INT8 = 0;

	/**
	 * Data type identifier associated with unsigned 8-bit integer.
	 */
	public final static int  UINT8 = 1;

	/**
	 * Data type identifier associated with signed 16-bit integer.
	 */
	public final static int  INT16 = 2;

	/**
	 * Data type identifier associated with unsigned 16-bit integer.
	 */
	public final static int  UINT16 = 3;

	/**
	 * Data type identifier associated with signed 32-bit integer.
	 */
	public final static int  INT32 = 4;

	/**
	 * Data type identifier associated with unsigned 32-bit integer.
	 */
	public final static int  UINT32 = 5;

	/**
	 * Data type identifier associated with signed 64-bit integer.
	 */
	public final static int  INT64 = 6;

	/**
	 * Data type identifier associated with unsigned 64-bit integer.
	 */
	public final static int  UINT64 = 7;

	/**
	 * Data type identifier associated with single precision floating
	 * point.
	 */
	public final static int  FLOAT = 8;

	/**
	 * Data type identifier associated with double precision floating
	 * point.
	 */
	public final static int  DOUBLE = 9;

	/**
	 * Data type identifier associated with IPv4 address.
	 */
	public final static int  IPV4 = 10;

	/**
	 * Data type identifier associated with IPv6 address.
	 */
	public final static int  IPV6 = 11;

	/**
	 * Data type identifier associated with string.
	 */
	public final static int  STRING = 12;

	/**
	 * Data type identifier associated with raw binary image.
	 */
	public final static int  BINARY = 13;

	/**
	 * Data type identifier associated with NULL, which is a placeholder
	 * in IPC additional data array.
	 */
	public final static int  NULL = 14;

	/**
	 * Data type identifier associated with user-defined IPC struct.
	 */
	public final static int  STRUCT = 0xff;

	/**
	 * Only internal classes can instantiate {@code IpcDataUnit}.
	 */
	IpcDataUnit()
	{
	}

	/**
	 * Return the type identifier associated with this data.
	 *
	 * @return	The type identifier of this data.
	 */
	public abstract int getType();

	/**
	 * <p>
	 *   Append this data to the end of the additional data in the
	 *  specified IPC client session.
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
	abstract void addClientOutput(long session) throws IpcException;

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
	abstract void setStruct(IpcStruct target, String name);

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
	abstract void setStruct(IpcStruct target, String name, int index);

	/**
	 * <p>
	 *   Return a string representation of the given data type identifier.
	 * </p>
	 *
	 * @param type	Data type identifier, such as {@link #INT8}.
	 * @return	A string representation of {@code type}.
	 */
	static native String getTypeName(int type);
}
