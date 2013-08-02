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
 *   The {@code IpcNumber} class is a base class of IPC data unit which
 *   represents a numerical data.
 * </p>
 * <p>
 *   The subclass of this class should implements {@code Comparable} because
 *   numerical value is comparable.
 * </p>
 *
 * @since	C10
 */
public abstract class IpcNumber extends IpcDataUnit
{
	/**
	 * Only internal classes can instantiate {@code IpcNumber}.
	 */
	IpcNumber()
	{
	}

	/**
	 * Get the value of this object as {@code byte} value.
	 * Note that returned value may be truncated or rounded.
	 *
	 * @return	The value of this object after conversion to
	 *		{@code byte}.
	 */
	public abstract byte byteValue();

	/**
	 * Get the value of this object as {@code short} value.
	 * Note that returned value may be truncated or rounded.
	 *
	 * @return	The value of this object after conversion to
	 *		{@code short}.
	 */
	public abstract short shortValue();

	/**
	 * Get the value of this object as {@code int} value.
	 * Note that returned value may be truncated or rounded.
	 *
	 * @return	The value of this object after conversion to
	 *		{@code int}.
	 */
	public abstract int intValue();

	/**
	 * Get the value of this object as {@code long} value.
	 * Note that returned value may be truncated or rounded.
	 *
	 * @return	The value of this object after conversion to
	 *		{@code long}.
	 */
	public abstract long longValue();

	/**
	 * Get the value of this object as {@code float} value.
	 * Note that returned value may be rounded.
	 *
	 * @return	The value of this object after conversion to
	 *		{@code float}.
	 */
	public abstract float floatValue();

	/**
	 * Get the value of this object as {@code double} value.
	 * Note that returned value may be rounded.
	 *
	 * @return	The value of this object after conversion to
	 *		{@code double}.
	 */
	public abstract double doubleValue();
}
