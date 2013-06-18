/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.util;

/**
 * <p>
 *   The {@code LogFacility} class defines a set of PFC-Core logging
 *   facility which specifies source of a log message. It is used to specify
 *   syslog facility.
 * </p>
 *
 * @since	C10
 */
public final class LogFacility
{
	/**
	 * The log facility associated with {@code LOG_DAEMON}.
	 */
	public final static LogFacility  DAEMON = new LogFacility(0, "daemon");

	/**
	 * The log facility associated with {@code LOG_LOCAL0}.
	 */
	public final static LogFacility  LOCAL0 = new LogFacility(1, "local0");

	/**
	 * The log facility associated with {@code LOG_LOCAL1}.
	 */
	public final static LogFacility  LOCAL1 = new LogFacility(2, "local1");

	/**
	 * The log facility associated with {@code LOG_LOCAL2}.
	 */
	public final static LogFacility  LOCAL2 = new LogFacility(3, "local2");

	/**
	 * The log facility associated with {@code LOG_LOCAL3}.
	 */
	public final static LogFacility  LOCAL3 = new LogFacility(4, "local3");

	/**
	 * The log facility associated with {@code LOG_LOCAL4}.
	 */
	public final static LogFacility  LOCAL4 = new LogFacility(5, "local4");

	/**
	 * The log facility associated with {@code LOG_LOCAL5}.
	 */
	public final static LogFacility  LOCAL5 = new LogFacility(6, "local5");

	/**
	 * The log facility associated with {@code LOG_LOCAL6}.
	 */
	public final static LogFacility  LOCAL6 = new LogFacility(7, "local6");

	/**
	 * The log facility associated with {@code LOG_LOCAL7}.
	 */
	public final static LogFacility  LOCAL7 = new LogFacility(8, "local7");

	/**
	 * <p>
	 *   An array which keeps valid facilities.
	 * </p>
	 */
	private final static LogFacility[]  _allFacilities = {
		DAEMON,
		LOCAL0,
		LOCAL1,
		LOCAL2,
		LOCAL3,
		LOCAL4,
		LOCAL5,
		LOCAL6,
		LOCAL7,
	};

	/**
	 * Internal logging facility value.
	 */
	private int  _facility;

	/**
	 * A string which represents log facility.
	 */
	private String  _name;

	/**
	 * <p>
	 *   Return a {@code LogFacility} instance associated with the
	 *   specified logging facility name.
	 * </p>
	 * <p>
	 *   Note that logging facility name is case insensitive.
	 * </p>
	 *
	 * @param name	The name of logging facility.
	 * @return	A {@code LogFacility} instance associated with the
	 *		given name. {@code null} is returned if the given
	 *		name is an invalid facility name.
	 * @throws NullPointerException
	 *	{@code name} is {@code null}.
	 */
	public static LogFacility forName(String name)
	{
		for (LogFacility fac: _allFacilities) {
			if (name.equalsIgnoreCase(fac._name)) {
				return fac;
			}
		}

		return null;
	}

	/**
	 * Construct a {@code LogFacility} instance which has the
	 * specified facility.
	 *
	 * @param facility	Internal logging facility value.
	 * @param name		Symbolic name of logging facility.
	 */
	private LogFacility(int facility, String name)
	{
		_facility = facility;
		_name = name;
	}

	/**
	 * Return internal logging facility value of this instance.
	 *
	 * @return	An internal logging facility.
	 */
	public int getFacility()
	{
		return _facility;
	}

	/**
	 * Return a string representation of this log facility.
	 *
	 * @return	A string which represents this log facility.
	 */
	@Override
	public String toString()
	{
		return _name;
	}
}
