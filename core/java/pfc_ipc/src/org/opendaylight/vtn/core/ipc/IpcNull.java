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
 *   The {@code IpcNull} is an IPC additional data unit class which
 *   represents a NULL data.
 * </p>
 * <p>
 *   NULL data is just a placeholder in IPC additional data array, and it
 *   has no value.
 * </p>
 *
 * @since	C10
 */
public final class IpcNull extends IpcDataUnit
{
	/**
	 * <p>
	 *   Construct a NULL data.
	 * </p>
	 */
	public IpcNull()
	{
		super();
	}

	/**
	 * <p>
	 *   Return the type identifier associated with this data.
	 * </p>
	 *
	 * @return	{@link IpcDataUnit#NULL} is always returned.
	 */
	@Override
	public int getType()
	{
		return NULL;
	}

	/**
	 * <p>
	 *   Return a {@code String} which represents this value.
	 * </p>
	 *
	 * @return	{@code "null"} is always returned.
	 */
	@Override
	public String toString()
	{
		return "null";
	}

	/**
	 * <p>
	 *   Return a hash code of this value.
	 * </p>
	 *
	 * @return	{@link IpcDataUnit#NULL} is always returned as a
	 *		hash code.
	 */
	@Override
	public int hashCode()
	{
		return NULL;
	}

	/**
	 * <p>
	 *   Determine whether the specified object is identical to this
	 *   object.
	 * </p>
	 * <p>
	 *   Note that {@code IpcNull} instance is just a placeholder without
	 *   data. So this method returns {@code true} if the given object is
	 *   {@code IpcNull} instance.
	 * </p>
	 *
	 * @param obj	An object to be compared.
	 * @return	True is returned if the specified object is identical
	 *		to this object. Otherwise false is returned.
	 */
	@Override
	public boolean equals(Object obj)
	{
		return (obj instanceof IpcNull);
	}

	/**
	 * <p>
	 *   Append this data to the end of the additional data in the
	 *   specified IPC client session
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
	native void addClientOutput(long session) throws IpcException;

	/**
	 * <p>
	 *   {@code IpcNull} does not support this method.
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
			("IpcNull is not supported.");
	}

	/**
	 * <p>
	 *   {@code IpcNull} does not support this method.
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
}
