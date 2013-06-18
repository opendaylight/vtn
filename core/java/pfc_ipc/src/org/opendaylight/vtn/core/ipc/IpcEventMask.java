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
 *   An instance of {@code IpcEventMask} class represents an IPC event mask.
 * </p>
 * <p>
 *   IPC event mask is a set of IPC event types, which determines type of
 *   an IPC event. An {@code IpcEventMask} instance is used to determine
 *   IPC event to be delivered to {@link IpcEventHandler}.
 * </p>
 * <p>
 *   Note that this class is not synchronized.
 *   Concurrent access to an {@code IpcEventMask} from multiple threads
 *   results in undefined behavior.
 * </p>
 *
 * @since	C10
 * @see		IpcEvent
 */
public final class IpcEventMask implements Cloneable
{
	/**
	 * <p>
	 *   IPC event mask value which contains all supported IPC event types.
	 * </p>
	 */
	final static long  MASK_FILLED = -1L;

	/**
	 * <p>
	 *   IPC event mask value which contains no IPC event type.
	 * </p>
	 */
	final static long  MASK_EMPTY = 0L;

	/**
	 * <p>
	 *   The value of this IPC event mask.
	 * </p>
	 */
	private long  _mask;

	/**
	 * <p>
	 *   Create mask value from the specified IPC event type.
	 * </p>
	 *
	 * @param type	An IPC event type.
	 * @throws IllegalArgumentException
	 *	{@code type} is not a valid IPC event type.
	 */
	private static long createMask(int type)
	{
		if (type < IpcEvent.MIN_TYPE || type > IpcEvent.MAX_TYPE) {
			throw new IllegalArgumentException
				("Invalid IPC event type: " + type);
		}

		return (1L << type);
	}

	/**
	 * <p>
	 *   Construct an {@code IpcEventMask} instance, which contains all
	 *   supported IPC event types.
	 * </p>
	 */
	public IpcEventMask()
	{
		_mask = MASK_FILLED;
	}

	/**
	 * <p>
	 *   Construct an {@code IpcEventMask} which contains only the specified
	 *   IPC event type.
	 * </p>
	 *
	 * @param type	An IPC event type to be set in a new IPC event mask.
	 * @throws IllegalArgumentException
	 *	{@code type} is not a valid IPC event type.
	 */
	public IpcEventMask(int type)
	{
		_mask = createMask(type);
	}

	/**
	 * <p>
	 *   Construct an {@code IpcEventMask} which has the specified mask
	 *   value.
	 * </p>
	 *
	 * @param mask	The IPC event mask value.
	 */
	IpcEventMask(long mask)
	{
		_mask = mask;
	}

	/**
	 * <p>
	 *   Return a clone of this IPC event mask.
	 * </p>
	 *
	 * @return	A clone of this IPC event mask.
	 */
	@Override
	public Object clone()
	{
		try {
			return super.clone();
		}
		catch (CloneNotSupportedException e) {
			// This should never happen.
			throw new InternalError
				("Unable to clone IpcEventMask.");
		}
	}

	/**
	 * <p>
	 *   Determine whether the specified object is identical to this object.
	 * </p>
	 *
	 * @param obj	An object to be compared.
	 * @return	{@code true} is returned if the specified object is
	 *		identical to this object. Otherwise {@code false} is
	 *		returned.
	 */
	@Override
	public boolean equals(Object obj)
	{
		if (obj instanceof IpcEventMask) {
			return (_mask == ((IpcEventMask)obj)._mask);
		}

		return false;
	}

	/**
	 * <p>
	 *   Return a hash code of this object.
	 * </p>
	 *
	 * @return	A hash code of this object.
	 */
	@Override
	public int hashCode()
	{
		return (int)(_mask ^ (_mask >>> 32));
	}

	/**
	 * <p>
	 *   Add the specified IPC event type to this IPC event mask.
	 * </p>
	 *
	 * @param type	An IPC event type to be added.
	 * @throws IllegalArgumentException
	 *	{@code type} is not a valid IPC event type.
	 */
	public void add(int type)
	{
		_mask |= createMask(type);
	}

	/**
	 * <p>
	 *   Remove the specified IPC event type from this IPC event mask.
	 * </p>
	 *
	 * @param type	An IPC event type to be removed.
	 * @throws IllegalArgumentException
	 *	{@code type} is not a valid IPC event type.
	 */
	public void remove(int type)
	{
		long mask = createMask(type);

		_mask &= ~mask;
	}

	/**
	 * <p>
	 *   Clear event masks in this object, and set all specified event
	 *   types.
	 * </p>
	 * <p>
	 *   If no argument is specified, this method make this event mask
	 *   empty.
	 * </p>
	 *
	 * @param args	Event types to be set to this event mask.
	 * @throws NullPointerException
	 *	{@code args} is {@code null}.
	 * @throws IllegalArgumentException
	 *	{@code args} contains at least one invalid event type.
	 */
	public void set(Integer ... args)
	{
		long mask = MASK_EMPTY;

		for (Integer arg: args) {
			mask |= createMask(arg.intValue());
		}

		_mask = mask;
	}

	/**
	 * <p>
	 *   Determine whether this IPC event mask contains the specified IPC
	 *   event type or not.
	 * </p>
	 *
	 * @param type	An IPC event type to be tested.
	 * @return	{@code true} if this IPC event mask contains
	 *		{@code type}. Otherwise {@code false}.
	 * @throws IllegalArgumentException
	 *	{@code type} is not a valid IPC event type.
	 */
	public boolean contains(int type)
	{
		long mask = createMask(type);

		return ((_mask & mask) != MASK_EMPTY);
	}

	/**
	 * <p>
	 *   Add all supported IPC event types to this IPC event mask.
	 * </p>
	 */
	public void fill()
	{
		_mask = MASK_FILLED;
	}

	/**
	 * <p>
	 *   Make this IPC event mask empty.
	 * </p>
	 */
	public void clear()
	{
		_mask = MASK_EMPTY;
	}

	/**
	 * <p>
	 *   Determine whether this IPC event mask contains all supported IPC
	 *   event types.
	 * </p>
	 *
	 * @return	{@code true} if this IPC event contains all supported
	 *		IPC event types. Otherwise {@code false}.
	 */
	public boolean isFilled()
	{
		return (_mask == MASK_FILLED);
	}

	/**
	 * <p>
	 *   Determine whether this IPC event mask is empty, which means it
	 *   contains no IPC event type.
	 * </p>
	 *
	 * @return	{@code true} if this IPC event mask is empty.
	 *		Otherwise {@code false}.
	 */
	public boolean isEmpty()
	{
		return (_mask == MASK_EMPTY);
	}

	/**
	 * <p>
	 *   Return current value of this IPC event mask.
	 * </p>
	 *
	 * @return	The value of this IPC event mask.
	 */
	long getMask()
	{
		return _mask;
	}
}
