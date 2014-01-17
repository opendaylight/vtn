/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.util;

import java.io.File;

/**
 * <p>
 *   The {@code LogConfiguration} class represents configuration of the
 *   PFC-Core logging system. It is used to initialize the PFC-Core logging
 *   system.
 * </p>
 * <p>
 *   Note that the {@code LogConfiguration} class is not thread safe.
 * </p>
 *
 * @since	C10
 */
public final class LogConfiguration
{
	/**
	 * Default number of log file rotation.
	 */
	private final static int  FILE_ROTATE  = 10;

	/**
	 * Default value of log file size limit.
	 */
	private final static int  FILE_SIZE_LIMIT  = 10000000;

	/**
	 * Identifier string which is passed to the syslog.
	 */
	private String  _ident;

	/**
	 * Syslog facility.
	 */
	private LogFacility  _facility = LogFacility.LOCAL0;

	/**
	 * User-defined logging level of the trace log.
	 */
	private LogLevel  _level = LogLevel.NONE;

	/**
	 * Default logging level of the trace log.
	 */
	private LogLevel  _defaultLevel = LogLevel.INFO;

	/**
	 * Number of log file rotation.
	 */
	private int  _fileCount = FILE_ROTATE;

	/**
	 * The limit of log file size.
	 */
	private int  _fileSize = FILE_SIZE_LIMIT;

	/**
	 * Path to the log file.
	 */
	private File  _logFile;

	/**
	 * Path to the log level file.
	 */
	private File  _logLevelFile;

	/**
	 * FATAL log handler.
	 */
	private LogFatalHandler  _fatalHandler = LogSystem.getInstance();

	/**
	 * <p>
	 *   Construct a logging configuration.
	 * </p>
	 * <p>
	 *   Trace log messages are stored to the file specified by
	 *   {@code logFile}.
	 * </p>
	 * <p>
	 *   Other attributes are initialized with default value.
	 * </p>
	 * <dl>
	 *   <dt>Syslog facility</dt>
	 *   <dd>{@link LogFacility#LOCAL0}</dd>
	 *
	 *   <dt>User-defined trace log level</dt>
	 *   <dd>Undefined value ({@link LogLevel#NONE})</dd>
	 *
	 *   <dt>Default trace log level</dt>
	 *   <dd>{@link LogLevel#INFO}</dd>
	 *
	 *   <dt>Number of file rotation</dt>
	 *   <dd>10</dd>
	 *
	 *   <dt>Maximum size of log file</dt>
	 *   <dd>10000000</dd>
	 *
	 *   <dt>Path to logging level file</dt>
	 *   <dd>Undefined</dd>
	 *
	 *   <dt>FATAL log handler</dt>
	 *   <dd>
	 *     Default handler, which calls {@code System.exit(int)}.
	 *   </dd>
	 * </dl>
	 *
	 * @param ident		An identifier string of the syslog.
	 * @param logFile	An absolute path to the log file.
	 * @throws NullPointerException
	 *	{@code ident} or {@code logFile} is null.
	 * @throws IllegalArgumentException
	 *	{@code ident} is empty.
	 * @throws IllegalArgumentException
	 *	{@code logFile} is not null and not an absolute path.
	 * @throws IllegalArgumentException
	 *	{@code logFile} is not null, and path component specified by
	 *	{@code logFile} does not contain a file name.
	 * @throws IllegalArgumentException
	 *	{@code logFile} is not null, and the last path component of
	 *	{@code logFile} is "." or "..".
	 */
	public LogConfiguration(String ident, File logFile)
	{
		if (ident == null) {
			throw new NullPointerException("ident is null.");
		}
		if (ident.length() == 0) {
			throw new IllegalArgumentException("ident is empty.");
		}

		checkFile(logFile);
		_logFile = logFile;
		_ident = ident;
	}

	/**
	 * <p>
	 *   Set logging facility.
	 * </p>
	 * <p>
	 *   Only the system logger uses the logging facility value.
	 * </p>
	 *
	 * @param facility	Syslog facility to be configured.
	 * @throws NullPointerException
	 *	{@code facility} is null.
	 */
	public void setFacility(LogFacility facility)
	{
		if (facility == null) {
			throw new NullPointerException("facility is null.");
		}

		_facility = facility;
	}

	/**
	 * <p>
	 *   Configure the log file rotation.
	 * </p>
	 * <p>
	 *   This method does not affect to the logging system if the file
	 *   logging is disabled.
	 * </p>
	 *
	 * @param count		The number of log file rotation.
	 * @param size		Maximum size of the log file. The log file
	 *			will be rotated when its size exceeds the
	 *			specified size.
	 * @throws IllegalArgumentException
	 *	{@code count} is negative value.
	 * @throws IllegalArgumentException
	 *	{@code size} is less than or equal to zero.
	 */
	public void setRotation(int count, int size)
	{
		if (count < 0) {
			throw new IllegalArgumentException
				("count is negative: " + count);
		}
		if (size <= 0) {
			throw new IllegalArgumentException
				("size is less than or equal to zero: " +
				 size);
		}

		_fileCount = count;
		_fileSize = size;
	}

	/**
	 * <p>
	 *   Set file path to the logging level file, which keeps current
	 *   logging level.
	 * </p>
	 * <p>
	 *   If the logging level file is configured, the PFC-Core logging
	 *   system saves the logging level into the specified file.
	 *   The logging level is restored from the specified file on the
	 *   next initialization of the logging system.
	 * </p>
	 * <p>
	 *   The logging level file is disabled if null is specified to
	 *   {@code levelFile}.
	 * </p>
	 *
	 * @param levelFile	An absolute path to the logging level file.
	 * @throws IllegalArgumentException
	 *	{@code levelFile} is not null and it is not an absolute path.
	 * @throws IllegalArgumentException
	 *	{@code levelFile} is not null and its path component does not
	 *	contain a file name.
	 * @throws IllegalArgumentException
	 *	{@code levelFile} is not null and the last path component of
	 *	{@code levelFile} is "." or "..".
	 */
	public void setLevelFile(File levelFile)
	{
		if (levelFile != null) {
			checkFile(levelFile);
		}
		_logLevelFile = levelFile;
	}

	/**
	 * <p>
	 *   Set user-defined logging level for the trace log.
	 * </p>
	 * <p>
	 *   The specified logging level is applied by force.
	 *   Once user-defined logging level is configured, logging level
	 *   saved in the logging level file set by {@link #setLevelFile(File)}
	 *   will be ignored.
	 * </p>
	 * <p>
	 *   User-defined logging level can be invalidated by specifying
	 *   {@link LogLevel#NONE}.
	 * </p>
	 *
	 * @param level		Logging level for trace log.
	 * @throws NullPointerException
	 *	{@code level} is null.
	 */
	public void setLevel(LogLevel level)
	{
		if (level == null) {
			throw new NullPointerException("level is null.");
		}

		_level = level;
	}

	/**
	 * <p>
	 *   Set default logging level for the trace log, which is used if
	 *   the logging level is not saved in the logging level file.
	 * </p>
	 * <p>
	 *   Unlike {@link #setLevel(LogLevel)}, {@link LogLevel#NONE}
	 *   can not be specified as default logging level.
	 * </p>
	 *
	 * @param level		Logging level for trace log.
	 * @throws NullPointerException
	 *	{@code level} is null.
	 * @throws IllegalArgumentException
	 *	{@link LogLevel#NONE} is passed to {@code level}.
	 */
	public void setDefaultLevel(LogLevel level)
	{
		if (level == null) {
			throw new NullPointerException("level is null.");
		}
		if (level == LogLevel.NONE) {
			throw new IllegalArgumentException("level is NONE.");
		}

		_defaultLevel = level;
	}

	/**
	 * <p>
	 *   Set FATAL log handler.
	 * </p>
	 * <p>
	 *   Default FATAL log handler terminates the program by calling
	 *   {@code System.exit(int)}. If you want to change this behavior,
	 *   you need to register your FATAL log handler by this method.
	 *   And you can specify null to {@code handler} to disable FATAL log
	 *   handler.
	 * </p>
	 *
	 * @param handler	A FATAL log handler to be set.
	 */
	public void setFatalHandler(LogFatalHandler handler)
	{
		_fatalHandler = handler;
	}

	/**
	 * Get a syslog identifier.
	 *
	 * @return	Syslog identifier.
	 */
	String getIdent()
	{
		return _ident;
	}

	/**
	 * Get a syslog facility.
	 *
	 * @return	{@link LogFacility} which represents syslog facility.
	 */
	LogFacility getFacility()
	{
		return _facility;
	}

	/**
	 * Get user-defined logging level of trace log.
	 *
	 * @return	User-defined logging level of trace log.
	 */
	LogLevel getLevel()
	{
		return _level;
	}

	/**
	 * Get default logging level of trace log.
	 *
	 * @return	Default logging level of trace log.
	 */
	LogLevel getDefaultLevel()
	{
		return _defaultLevel;
	}

	/**
	 * Get the number of log file rotation.
	 *
	 * @return	The number of log file rotation.
	 */
	int getRotationCount()
	{
		return _fileCount;
	}

	/**
	 * Get the maximum size of the log file.
	 *
	 * @return	The maximum size of the log file.
	 */
	int getRotationSize()
	{
		return _fileSize;
	}

	/**
	 * Get path to the log file.
	 *
	 * @return	A path to the log file. null is returned if the file
	 *		logging is disabled.
	 */
	File getLogFile()
	{
		return _logFile;
	}

	/**
	 * Get path to the logging level file.
	 *
	 * @return	A path to the logging level file. null is returned if
	 *		the logging level file is disabled.
	 */
	File getLevelFile()
	{
		return _logLevelFile;
	}

	/**
	 * Get FATAL log handler.
	 *
	 * @return	A FATAL log handler. null is returned if no handler is
	 *		configured.
	 */
	LogFatalHandler getFatalHandler()
	{
		return _fatalHandler;
	}

	/**
	 * Ensure that the specified file is valid file path.
	 *
	 * @param file	A file path to be tested.
	 * @throws NullPointerException
	 *	{@code file} is null.
	 * @throws IllegalArgumentException
	 *	{@code file} is not an absolute path.
	 * @throws IllegalArgumentException
	 *	Path component specified by {@code file} does not
	 *	contain a file name.
	 * @throws IllegalArgumentException
	 *	The last path component of {@code file} is "." or "..".
	 */
	private void checkFile(File file)
	{
		if (!file.isAbsolute()) {
			throw new IllegalArgumentException
				("Not an absolute path: " + file);
		}

		String name = file.getName();
		if (name.length() == 0) {
			throw new IllegalArgumentException
				("File does not contain file name: " + file);
		}

		if (name.equals(".") || name.equals("..")) {
			throw new IllegalArgumentException
				("Invalid path component: " + file);
		}
	}
}
