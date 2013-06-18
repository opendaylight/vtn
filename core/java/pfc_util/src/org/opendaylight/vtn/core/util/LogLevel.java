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
 *   The {@code LogLevel} class defines a set of PFC-Core logging
 *   levels which is used to control output to PFC-Core logging system.
 * </p>
 * <p>
 *   The logging levels are specified by ordered integers. Lower level value
 *   means more severe situation.
 * </p>
 *
 * @since	C10
 */
public final class LogLevel implements Comparable<LogLevel>
{
	/**
	 * <p>
	 *   Pseudo logging level value which represents undefined value.
	 * </p>
	 */
	final static int  LVL_NONE  = -1;

	/**
	 * <p>
	 *   The logging level value which indicates fatal error condition.
	 * </p>
	 */
	final static int  LVL_FATAL  = 0;

	/**
	 * <p>
	 *   The logging level value which indicates error condition.
	 * </p>
	 */
	final static int  LVL_ERROR  = 1;

	/**
	 * <p>
	 *   The logging level value which indicates warning condition.
	 * </p>
	 */
	final static int  LVL_WARNING  = 2;

	/**
	 * <p>
	 *   The logging level value which indicates significant condition.
	 * </p>
	 */
	final static int  LVL_NOTICE  = 3;

	/**
	 * <p>
	 *   The logging level value which indicates informational message.
	 * </p>
	 */
	final static int  LVL_INFO  = 4;

	/**
	 * <p>
	 *   The logging level value which indicates debugging message.
	 * </p>
	 */
	final static int  LVL_DEBUG  = 5;

	/**
	 * <p>
	 *   The logging level which indicates trace level message.
	 * </p>
	 */
	final static int  LVL_TRACE  = 6;

	/**
	 * <p>
	 *   The logging level value which indicates verbose-level message.
	 * </p>
	 */
	final static int  LVL_VERBOSE  = 7;

	/**
	 * <p>
	 *   Pseudo logging level which represents undefined value.
	 * </p>
	 */
	public final static LogLevel  NONE =
		new LogLevel("NONE", LVL_NONE);

	/**
	 * <p>
	 *   The logging level which indicates fatal error condition.
	 * </p>
	 * <p>
	 *   A program may be terminated if a FATAL message is logged.
	 * </p>
	 */
	public final static LogLevel  FATAL =
		new LogLevel("FATAL", LVL_FATAL);

	/**
	 * <p>
	 *   The logging level which indicates error condition.
	 * </p>
	 */
	public final static LogLevel  ERROR =
		new LogLevel("ERROR", LVL_ERROR);

	/**
	 * <p>
	 *   The logging level which indicates warning condition.
	 * </p>
	 */
	public final static LogLevel  WARNING =
		new LogLevel("WARNING", LVL_WARNING);

	/**
	 * <p>
	 *   The logging level which indicates significant condition.
	 * </p>
	 */
	public final static LogLevel  NOTICE =
		new LogLevel("NOTICE", LVL_NOTICE);

	/**
	 * <p>
	 *   The logging level which indicates informational message.
	 * </p>
	 */
	public final static LogLevel  INFO =
		new LogLevel("INFO", LVL_INFO);

	/**
	 * <p>
	 *   The logging level which indicates debugging message.
	 * </p>
	 */
	public final static LogLevel  DEBUG =
		new LogLevel("DEBUG", LVL_DEBUG);

	/**
	 * <p>
	 *   The logging level which indicates trace level message.
	 * </p>
	 */
	public final static LogLevel  TRACE =
		new LogLevel("TRACE", LVL_TRACE);

	/**
	 * <p>
	 *   The logging level which indicates verbose-level message.
	 * </p>
	 * <p>
	 *   Any verbose level message is dropped on release build.
	 * </p>
	 */
	public final static LogLevel  VERBOSE =
		new LogLevel("VERBOSE", LVL_VERBOSE);

	/**
	 * <p>
	 *   An array which keeps valid logging levels.
	 * </p>
	 * <p>
	 *   The log level value must be used as array index.
	 * </p>
	 */
	private final static LogLevel[]  _allLevels = {
		FATAL,
		ERROR,
		WARNING,
		NOTICE,
		INFO,
		DEBUG,
		TRACE,
		VERBOSE,
	};

	/**
	 * <p>
	 *   Symbolic name of this log level.
	 * </p>
	 */
	private final String  _name;

	/**
	 * <p>
	 *   Log level value.
	 * </p>
	 */
	private final int  _level;

	/**
	 * <p>
	 *   Return a {@code LogLevel} instance associated with the specified
	 *   logging level name.
	 * </p>
	 * <p>
	 *   Note that logging level name is case insensitive.
	 * </p>
	 *
	 * @param name	The name of logging level.
	 * @return	A {@code LogLevel} instance associated with the
	 *		given name. {@code null} is returned if the given
	 *		name is an invalid logging level name.
	 * @throws NullPointerException
	 *	{@code name} is {@code null}.
	 */
	public static LogLevel forName(String name)
	{
		for (LogLevel lvl: _allLevels) {
			if (name.equalsIgnoreCase(lvl._name)) {
				return lvl;
			}
		}

		return null;
	}

	/**
	 * <p>
	 *   Return a {@code LogLevel} instance associated with the specified
	 *   logging level value.
	 * </p>
	 *
	 * @return	A {@code LogLevel} instance.
	 * @throws IllegalArgumentException
	 *	An invalid level value is specified to {@code level}.
	 */
	static LogLevel getInstance(int level)
	{
		try {
			return _allLevels[level];
		}
		catch (ArrayIndexOutOfBoundsException e) {
			throw new IllegalArgumentException
				("Invalid level: " + level);
		}
	}

	/**
	 * <p>
	 *   Construct a {@code LogLevel} instance which has the specified
	 *   logging level value.
	 * </p>
	 *
	 * @param name		Symbolic name of logging level.
	 * @param level		Value of logging level.
	 */
	private LogLevel(String name, int level)
	{
		_name = name;
		_level = level;
	}

	/**
	 * <p>
	 *   Return the logging level value.
	 * </p>
	 *
	 * @return	The logging level value.
	 */
	public int getLevel()
	{
		return _level;
	}

	/**
	 * <p>
	 *   Compare a given logging level with this level.
	 * </p>
	 *
	 * @param lvl	A {@code LogLevel} instance to be compared.
	 * @return	A negative value, zero, or a positive integer
	 *		if this level is less than, equal to, or greater than
	 *		the specified level respectively.
	 */
	@Override
	public int compareTo(LogLevel lvl)
	{
		return _level - lvl._level;
	}

	/**
	 * <p>
	 *   Return a string representation of this log level.
	 * </p>
	 *
	 * @return	A string which represents this log level.
	 */
	@Override
	public String toString()
	{
		return _name;
	}
}
