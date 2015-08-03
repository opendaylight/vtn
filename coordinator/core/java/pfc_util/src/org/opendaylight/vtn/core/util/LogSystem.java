/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.util;

import java.util.Date;
import java.io.File;

/**
 * <p>
 *   The {@code LogSystem} class is a global manager class of the PFC-Core
 *   logging system. The PFC-Core logging system can be configured via
 *   a single instance returned by {@link #getInstance()}.
 * </p>
 * <p>
 *   Note that the PFC-Core logging system must be initialized by
 *   {@link #initialize(LogConfiguration)} before logging any message.
 * </p>
 *
 * @since	C10
 */
public final class LogSystem implements LogFatalHandler
{
	/**
	 * <p>
	 *   Exit status passed to {@code System.exit(int)} by the default
	 *   FATAL log handler.
	 * </p>
	 */
	public final static int  FATAL_EXIT_STATUS = 70;

	/**
	 * <p>
	 *   Holder class of the logging system instance.
	 * </p>
	 */
	private final static class LogSystemHolder
	{
		/**
		 * <p>
		 *   A single global logging system instance.
		 * </p>
		 */
		private final static LogSystem  _theInstance = new LogSystem();
	}

	/**
	 * <p>
	 *   Logging system implementation for the trace log.
	 * </p>
	 * <p>
	 *   The trace log is expected to be used frequently.
	 *   That is why the implementation of the trace log is cached into
	 *   this instance.
	 * </p>
	 */
	private final LogSystemImpl  _traceLog;

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
	 *   Create a single instance of the logging manager.
	 * </p>
	 */
	private LogSystem()
	{
		_traceLog = TraceLogImpl.getInstance();
	}

	/**
	 * <p>
	 *   Return the global {@code LogSystem} instance.
	 * </p>
	 *
	 * @return	The {@code LogSystem} instance.
	 */
	public static LogSystem getInstance()
	{
		return LogSystemHolder._theInstance;
	}

	/**
	 * <p>
	 *   Initialize the PFC-Core logging system.
	 * </p>
	 * <p>
	 *   This method must be called before logging any message.
	 *   Note that this method does nothing if the logging system is
	 *   already initialized.
	 * </p>
	 *
	 * @param conf	A log configuration object.
	 * @throws NullPointerException
	 *	{@code conf} is {@code null}.
	 * @throws IllegalStateException
	 *	The logging system is already initialized by another VM.
	 */
	public void initialize(LogConfiguration conf)
	{
		String ident = conf.getIdent();
		int level = conf.getLevel().getLevel();
		int defLevel = conf.getDefaultLevel().getLevel();
		int facility = conf.getFacility().getFacility();
		int rcount = conf.getRotationCount();
		int rsize = conf.getRotationSize();

		File logFile = conf.getLogFile();
		String logDir = logFile.getParent();
		String logName = logFile.getName();

		File levelFile = conf.getLevelFile();
		String levelDir, levelName;
		if (levelFile == null) {
			levelDir = null;
			levelName = null;
		}
		else {
			levelDir = levelFile.getParent();
			levelName = levelFile.getName();
		}

		LogFatalHandler fatalHandler = conf.getFatalHandler();

		initializeLog(ident, facility, level, defLevel, logDir,
			      logName, levelDir, levelName, rcount, rsize,
			      fatalHandler);
	}

	/**
	 * <p>
	 *   Finalize the logging system.
	 * </p>
	 * <p>
	 *   Any messages are dropped after the call of this method.
	 * </p>
	 */
	public void shutdown()
	{
		finalizeLog();
	}

	/**
	 * <p>
	 *   Return the current logging level of the trace log.
	 * </p>
	 *
	 * @return	The current logging level of the trace log.
	 */
	public LogLevel getLevel()
	{
		return _traceLog.getLevel();
	}

	/**
	 * <p>
	 *   Change the logging level for the trace log to the specified
	 *   value.
	 * </p>
	 * <p>
	 *   The logging level is reset to the default level if
	 *   {@link LogLevel#NONE} is specified to {@code level}.
	 * </p>
	 *
	 * @param level		New logging level.
	 * @return		{@code true} only if the logging level has been
	 *			actually changed.
	 * @throws NullPointerException
	 *	{@code level} is {@code null}.
	 * @see	LogConfiguration#setDefaultLevel(LogLevel)
	 */
	public boolean setLevel(LogLevel level)
	{
		return _traceLog.setLevel(level);
	}

	/**
	 * <p>
	 *   Finalize the global instance.
	 * </p>
	 */
	@Override
	protected void finalize()
	{
		shutdown();
	}

	/**
	 * <p>
	 *   Default FATAL log handler.
	 * </p>
	 * <p>
	 *   This method calls {@code System.exit(int)} with specifying
	 *   {@link #FATAL_EXIT_STATUS}.
	 * </p>
	 */
	@Override
	public void fatalError()
	{
		Logger log = _traceLog.getLogger("LogSystem");
		log.error("Terminated by FATAL log handler.");
		System.exit(FATAL_EXIT_STATUS);
	}

	/**
	 * <p>
	 *   Initialize the PFC-Core logging system.
	 * </p>
	 *
	 * @param ident		Syslog identifier.
	 * @param level		User-defined logging level for file log.
	 * @param facility	Logging facility value.
	 * @param defLevel	Default logging level for file log.
	 * @param logDir	Path to parent directory of the log file.
	 * @param logName	Name of the log file.
	 * @param levelDir	Path to parent directory of the logging level
	 *			file.
	 * @param levelName	Name of the logging level file.
	 * @param rcount	The number of log file rotation.
	 * @param rsize		The limit of the log file size.
	 * @param fatalHandler	FATAL log handler.
	 */
	private native void initializeLog(String ident, int facility,
					  int level, int defLevel, 
					  String logDir, String logName,
					  String levelDir, String levelName,
					  int rcount, int rsize,
					  LogFatalHandler fatalHandler);

	/**
	 * <p>
	 *   Shut down the logging system.
	 * </p>
	 */
	private native void finalizeLog();
}
