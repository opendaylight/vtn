/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.util;

import org.opendaylight.vtn.core.CoreSystem;

/**
 * <p>
 *   The {@code Logger} class is an abstract class of front-end interface of
 *   the PFC-Core logging system. An instance of this class is used to log
 *   messages.
 * </p>
 * <p>
 *   If you want to log messages to the trace log, you need to obtain a
 *   logger instance by {@link #getLogger()} or {@link #getLogger(String)}.
 * </p>
 * <blockquote>
 *   <pre>
 *     // Obtain an anonymous logger.
 *     Logger  logger = Logger.getLogger();
 *
 *     // Obtain a named logger.
 *     // Specified name is embedded to messages.
 *     Logger  logger = Logger.getLogger("MyClass");
 *   </pre>
 * </blockquote>
 * <p>
 *   Messages can be logged via instance method of the logger.
 * </p>
 * <blockquote>
 *   <pre>
 *     // Log an informational message.
 *     logger.info("Information");
 *
 *     // Log a debugging message specified by the format string and
 *     // arguments. A log message is constructed by
 *     // String.format(String, Object...).
 *     logger.debug("Debug message: %s, %d", "string argument", 10);
 *
 *     // Log a fatal message.
 *     // This invokes FATAL log handler if registered.
 *     logger.fatal("Fatal error.");
 *   </pre>
 * </blockquote>
 * <p>
 *   Note that you must initialize the PFC-Core logging system by
 *   {@link LogSystem#initialize(LogConfiguration)} before logging message.
 * </p>
 *
 * @since	C10
 */
public abstract class Logger
{
	/**
	 * <p>
	 *   Load native library.
	 * </p>
	 */
	static {
		PfcUtil.load();
	}

	/**
	 * <p>
	 *   Name of this logger instance.
	 * </p>
	 */
	final String  _name;

	/**
	 * <p>
	 *   Obtain an anonymous trace logger instance.
	 * </p>
	 *
	 * @return	An anonymous trace logger instance.
	 */
	public static Logger getLogger()
	{
		return getLogger(null);
	}

	/**
	 * <p>
	 *   Obtain a trace logger instance for the specified name.
	 * </p>
	 * <p>
	 *   {@code name} should be the name of the sub-component.
	 *   The specified name will be embedded into all messages logged
	 *   via returned logger.
	 * </p>
	 * <p>
	 *   An anonymous logger instance is returned if {@code name} is
	 *   {@code null}.
	 * </p>
	 *
	 * @param name	The name of the logger.
	 * @return	An anonymous trace logger instance.
	 */
	public static Logger getLogger(String name)
	{
		return TraceLogImpl.getInstance().getLogger(name);
	}

	/**
	 * <p>
	 *   Create a logger instance.
	 * </p>
	 *
	 * @param name		The name of this logger.
	 *			{@code null} means an anonymous logger.
	 */
	Logger(String name)
	{
		_name = name;
	}

	/**
	 * <p>
	 *   Return the name of this logger instance.
	 * </p>
	 *
	 * @return	The name of this logger instance.
	 *		{@code null} is returned if this instance is an
	 *		anonymous logger.
	 */
	public String getName()
	{
		return _name;
	}

	/**
	 * <p>
	 *   Log a fatal error message.
	 * </p>
	 * <ul>
	 *   <li>
	 *     A FATAL log handler is invoked if registered.
	 *   </li>
	 *   <li>
	 *     This method does nothing if the logging system is not
	 *     initialized.
	 *   </li>
	 * </ul>
	 *
	 * @param message		A message to be logged.
	 * @throws NullPointerException
	 *	{@code message} is {@code null}.
	 * @see LogSystem#initialize(LogConfiguration)
	 * @see LogFatalHandler
	 */
	public void fatal(String message)
	{
		log(LogLevel.LVL_FATAL, message);
	}

	/**
	 * <p>
	 *   Log a fatal error message.
	 * </p>
	 * <ul>
	 *   <li>
	 *     A FATAL log handler is invoked if registered.
	 *   </li>
	 *   <li>
	 *     A log message is constructed by
	 *     {@link String#format(String,Object...)}.
	 *   </li>
	 *   <li>
	 *     This method does nothing if the logging system is not
	 *     initialized.
	 *   </li>
	 * </ul>
	 *
	 * @param format	A format string.
	 * @param args		Arguments referenced by {@code format}.
	 * @throws NullPointerException
	 *	{@code format} is {@code null}.
	 * @throws IllegalFormatException
	 *	{@code format} contains an illegal syntax.
	 * @see LogSystem#initialize(LogConfiguration)
	 * @see LogFatalHandler
	 */
	public void fatal(String format, Object ... args)
	{
		log(LogLevel.LVL_FATAL, format, args);
	}

	/**
	 * <p>
	 *   Log an error message.
	 * </p>
	 * <p>
	 *   This method does nothing if the logging system is not initialized.
	 * </p>
	 *
	 * @param message		A message to be logged.
	 * @throws NullPointerException
	 *	{@code message} is {@code null}.
	 * @see LogSystem#initialize(LogConfiguration)
	 */
	public void error(String message)
	{
		log(LogLevel.LVL_ERROR, message);
	}

	/**
	 * <p>
	 *   Log an error message.
	 * </p>
	 * <ul>
	 *   <li>
	 *     A log message is constructed by
	 *     {@link String#format(String,Object...)}.
	 *   </li>
	 *   <li>
	 *     This method does nothing if the logging system is not
	 *     initialized.
	 *   </li>
	 * </ul>
	 *
	 * @param format	A format string.
	 * @param args		Arguments referenced by {@code format}.
	 * @throws NullPointerException
	 *	{@code format} is {@code null}.
	 * @throws IllegalFormatException
	 *	{@code format} contains an illegal syntax.
	 * @see LogSystem#initialize(LogConfiguration)
	 */
	public void error(String format, Object ... args)
	{
		log(LogLevel.LVL_ERROR, format, args);
	}

	/**
	 * <p>
	 *   Log a warning message.
	 * </p>
	 * <p>
	 *   This method does nothing if the logging system is not initialized.
	 * </p>
	 *
	 * @param message		A message to be logged.
	 * @throws NullPointerException
	 *	{@code message} is {@code null}.
	 * @see LogSystem#initialize(LogConfiguration)
	 */
	public void warning(String message)
	{
		log(LogLevel.LVL_WARNING, message);
	}

	/**
	 * <p>
	 *   Log a warning message.
	 * </p>
	 * <ul>
	 *   <li>
	 *     A log message is constructed by
	 *     {@link String#format(String,Object...)}.
	 *   </li>
	 *   <li>
	 *     This method does nothing if the logging system is not
	 *     initialized.
	 *   </li>
	 * </ul>
	 *
	 * @param format	A format string.
	 * @param args		Arguments referenced by {@code format}.
	 * @throws NullPointerException
	 *	{@code format} is {@code null}.
	 * @throws IllegalFormatException
	 *	{@code format} contains an illegal syntax.
	 * @see LogSystem#initialize(LogConfiguration)
	 */
	public void warning(String format, Object ... args)
	{
		log(LogLevel.LVL_WARNING, format, args);
	}

	/**
	 * <p>
	 *   Log a normal but significant message.
	 * </p>
	 * <p>
	 *   This method does nothing if the logging system is not initialized.
	 * </p>
	 *
	 * @param message		A message to be logged.
	 * @throws NullPointerException
	 *	{@code message} is {@code null}.
	 * @see LogSystem#initialize(LogConfiguration)
	 */
	public void notice(String message)
	{
		log(LogLevel.LVL_NOTICE, message);
	}

	/**
	 * <p>
	 *   Log a normal but significant message.
	 * </p>
	 * <ul>
	 *   <li>
	 *     A log message is constructed by
	 *     {@link String#format(String,Object...)}.
	 *   </li>
	 *   <li>
	 *     This method does nothing if the logging system is not
	 *     initialized.
	 *   </li>
	 * </ul>
	 *
	 * @param format	A format string.
	 * @param args		Arguments referenced by {@code format}.
	 * @throws NullPointerException
	 *	{@code format} is {@code null}.
	 * @throws IllegalFormatException
	 *	{@code format} contains an illegal syntax.
	 * @see LogSystem#initialize(LogConfiguration)
	 */
	public void notice(String format, Object ... args)
	{
		log(LogLevel.LVL_NOTICE, format, args);
	}

	/**
	 * <p>
	 *   Log an informational message.
	 * </p>
	 * <p>
	 *   This method does nothing if the logging system is not initialized.
	 * </p>
	 *
	 * @param message		A message to be logged.
	 * @throws NullPointerException
	 *	{@code message} is {@code null}.
	 * @see LogSystem#initialize(LogConfiguration)
	 */
	public void info(String message)
	{
		log(LogLevel.LVL_INFO, message);
	}

	/**
	 * <p>
	 *   Log an informational message.
	 * </p>
	 * <ul>
	 *   <li>
	 *     A log message is constructed by
	 *     {@link String#format(String,Object...)}.
	 *   </li>
	 *   <li>
	 *     This method does nothing if the logging system is not
	 *     initialized.
	 *   </li>
	 * </ul>
	 *
	 * @param format	A format string.
	 * @param args		Arguments referenced by {@code format}.
	 * @throws NullPointerException
	 *	{@code format} is {@code null}.
	 * @throws IllegalFormatException
	 *	{@code format} contains an illegal syntax.
	 * @see LogSystem#initialize(LogConfiguration)
	 */
	public void info(String format, Object ... args)
	{
		log(LogLevel.LVL_INFO, format, args);
	}

	/**
	 * <p>
	 *   Log a debugging message.
	 * </p>
	 * <p>
	 *   This method does nothing if the logging system is not initialized.
	 * </p>
	 *
	 * @param message		A message to be logged.
	 * @throws NullPointerException
	 *	{@code message} is {@code null}.
	 * @see LogSystem#initialize(LogConfiguration)
	 */
	public void debug(String message)
	{
		log(LogLevel.LVL_DEBUG, message);
	}

	/**
	 * <p>
	 *   Log a debugging message.
	 * </p>
	 * <ul>
	 *   <li>
	 *     A log message is constructed by
	 *     {@link String#format(String,Object...)}.
	 *   </li>
	 *   <li>
	 *     This method does nothing if the logging system is not
	 *     initialized.
	 *   </li>
	 * </ul>
	 *
	 * @param format	A format string.
	 * @param args		Arguments referenced by {@code format}.
	 * @throws NullPointerException
	 *	{@code format} is {@code null}.
	 * @throws IllegalFormatException
	 *	{@code format} contains an illegal syntax.
	 * @see LogSystem#initialize(LogConfiguration)
	 */
	public void debug(String format, Object ... args)
	{
		log(LogLevel.LVL_DEBUG, format, args);
	}

	/**
	 * <p>
	 *   Log a trace level message.
	 * </p>
	 * <ul>
	 *   <li>
	 *     This method does nothing if the logging system is not
	 *     initialized.
	 *   </li>
	 *   <li>
	 *     This method does nothing if the logger instance is bound to
	 *     the system logger because it does not support trace level
	 *     message.
	 *   </li>
	 * </ul>
	 *
	 * @param message		A message to be logged.
	 * @throws NullPointerException
	 *	{@code message} is {@code null}.
	 * @see LogSystem#initialize(LogConfiguration)
	 */
	public void trace(String message)
	{
		log(LogLevel.LVL_TRACE, message);
	}

	/**
	 * <p>
	 *   Log a trace level message.
	 * </p>
	 * <ul>
	 *   <li>
	 *     A log message is constructed by
	 *     {@link String#format(String,Object...)}.
	 *   </li>
	 *   <li>
	 *     This method does nothing if the logging system is not
	 *     initialized.
	 *   </li>
	 * </ul>
	 *
	 * @param format	A format string.
	 * @param args		Arguments referenced by {@code format}.
	 * @throws NullPointerException
	 *	{@code format} is {@code null}.
	 * @throws IllegalFormatException
	 *	{@code format} contains an illegal syntax.
	 * @see LogSystem#initialize(LogConfiguration)
	 */
	public void trace(String format, Object ... args)
	{
		log(LogLevel.LVL_TRACE, format, args);
	}

	/**
	 * <p>
	 *   Log a verbose message.
	 * </p>
	 * <ul>
	 *   <li>
	 *     This method does nothing if the logging system is not
	 *     initialized.
	 *   </li>
	 *   <li>
	 *     This method does nothing on release build.
	 *   </li>
	 *   <li>
	 *     This method does nothing if the logger instance is bound to
	 *     the system logger because it does not support verbose message.
	 *   </li>
	 * </ul>
	 *
	 * @param message		A message to be logged.
	 * @throws NullPointerException
	 *	{@code message} is {@code null}.
	 * @see LogSystem#initialize(LogConfiguration)
	 */
	public void verbose(String message)
	{
		if (CoreSystem.DEBUG) {
			log(LogLevel.LVL_VERBOSE, message);
		}
	}

	/**
	 * <p>
	 *   Log a verbose message.
	 * </p>
	 * <ul>
	 *   <li>
	 *     A log message is constructed by
	 *     {@link String#format(String,Object...)}.
	 *   </li>
	 *   <li>
	 *     This method does nothing on release build.
	 *   </li>
	 *   <li>
	 *     This method does nothing if the logging system is not
	 *     initialized.
	 *   </li>
	 * </ul>
	 *
	 * @param format	A format string.
	 * @param args		Arguments referenced by {@code format}.
	 * @throws NullPointerException
	 *	{@code format} is {@code null}.
	 * @throws IllegalFormatException
	 *	{@code format} contains an illegal syntax.
	 * @see LogSystem#initialize(LogConfiguration)
	 */
	public void verbose(String format, Object ... args)
	{
		if (CoreSystem.DEBUG) {
			log(LogLevel.LVL_VERBOSE, format, args);
		}
	}

	/**
	 * <p>
	 *   Log the specified message to the PFC-Core logging system
	 * </p>
	 *
	 * @param lvl		Logging level value of the message.
	 * @param message	A message to be logged.
	 * @throws NullPointerException
	 *	{@code level} or {@code message} is {@code null}.
	 */
	private void log(int lvl, String message)
	{
		logImpl(lvl, _name, message);
	}

	/**
	 * <p>
	 *   Log the message specified by the given format to the PFC-Core
	 *   logging system
	 * </p>
	 *
	 * @param lvl		Logging level value of the message.
	 * @param format	A format string.
	 * @param args		Arguments referenced by {@code format}.
	 * @throws NullPointerException
	 *	{@code level} or {@code format} is {@code null}.
	 * @throws IllegalFormatException
	 *	{@code format} contains an illegal syntax.
	 */
	private void log(int lvl, String format, Object[] args)
	{
		int cur = getCurrentLevel();

		if (lvl <= cur) {
			logImpl(lvl, _name, String.format(format, args));
		}
	}

	/**
	 * <p>
	 *   Return the current logging level value.
	 * </p>
	 *
	 * @return	The current logging level value.
	 */
	abstract int getCurrentLevel();

	/**
	 * <p>
	 *   Log the specified message to the PFC-Core logging system.
	 * </p>
	 *
	 * @param level		Logging level of the message.
	 * @param name		Module name.
	 * @param msg		Log message.
	 */
	abstract void logImpl(int level, String name, String msg);

	/**
	 * <p>
	 *   Implementation of {@link Logger} class which logs messages to the
	 *   trace log.
	 * </p>
	 */
	static final class TraceLogger extends Logger
	{
		/**
		 * <p>
		 *   Create a trace logger instance.
		 * </p>
		 *
		 * @param name	The name of this logger.
		 *		{@code null} or an empty string means
		 *		anonymous logger.
		 */
		TraceLogger(String name)
		{
			super(name);
		}

		/**
		 * <p>
		 *   Return the current logging level value of the trace log.
		 * </p>
		 *
		 * @return	The current logging level value.
		 */
		@Override
		int getCurrentLevel()
		{
			return TraceLogImpl.getInstance().getLogLevel();
		}

		/**
		 * <p>
		 *   Log the specified message to the trace log.
		 * </p>
		 *
		 * @param level		Logging level of the message.
		 * @param name		Module name.
		 * @param msg		Log message.
		 */
		@Override
		native void logImpl(int level, String name, String msg);
	}
}
