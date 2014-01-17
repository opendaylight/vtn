/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.util;

import java.lang.ref.ReferenceQueue;
import java.lang.ref.WeakReference;
import java.util.HashMap;
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

/**
 * <p>
 *   An abstract class for logging system implementation.
 * </p>
 * <p>
 *   A subclass of this class should manage {@link Logger} instances
 *   associated with this logging system, and current logging level.
 * </p>
 */
abstract class LogSystemImpl
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
	 *   Hash map which keeps pairs of the logger name and weak reference
	 *   to the {@link Logger} instance.
	 * <p>
	 */
	private final HashMap<String, LoggerRef>  _loggers =
		new HashMap<String, LoggerRef>();

	/**
	 * <p>
	 *   {@link Logger} instances which have been collected by the GC will
	 *   be enqueued to this queue.
	 * </p>
	 */
	private final ReferenceQueue<Logger> _queue =
		new ReferenceQueue<Logger>();

	/**
	 * <p>
	 *   Weak reference to {@link Logger} instance.
	 * </p>
	 */
	final class LoggerRef extends WeakReference<Logger>
	{
		/**
		 * <p>
		 *   The name of the logger.
		 * </p>
		 * <p>
		 *   It must be held by this instance in order to remove
		 *   {@code HashMap} entry associated with the name.
		 * </p>
		 */
		private final String  _name;

		/**
		 * <p>
		 *   Set {@code true} only if this reference is removed from
		 *   the map.
		 * </p>
		 */
		private boolean  _removed;

		/**
		 * <p>
		 *   Construct a new weak reference to {@link Logger} instance.
		 * </p>
		 *
		 * @param logger	The logger instance.
		 */
		LoggerRef(Logger logger)
		{
			super(logger, _queue);
			_name = logger.getName();
		}
	}

	/**
	 * <p>
	 *   Construct a new logging system implementation.
	 * </p>
	 */
	LogSystemImpl()
	{
	}

	/**
	 * <p>
	 *   Get the logger associated with the specified name.
	 * </p>
	 * <p>
	 *   If the logger associated with the specified name is held by this
	 *   instance, this method returns it. Otherwise a new logger is
	 *   created.
	 * </p>
	 *
	 * @param name	The name of the logger.
	 * @return	The logger associated with the specified name.
	 */
	synchronized Logger getLogger(String name)
	{
		flushQueue();

		LoggerRef  lref = _loggers.get(name);
		if (lref != null) {
			Logger logger = lref.get();

			if (logger != null) {
				return logger;
			}

			// Remove reference to dead logger.
			_loggers.remove(name);

			// Turn the removed flag on.
			lref._removed = true;
			lref = null;

			// Removed reference may be queued in the reference
			// queue. So we need to drain the queue again.
			flushQueue();
		}

		// Create a new logger.
		Logger logger = newLogger(name);
		lref = new LoggerRef(logger);
		_loggers.put(name, lref);

		return logger;
	}

	/**
	 * <p>
	 *   Return the number of logger instances.
	 * </p>
	 * <p>
	 *   This method always eliminates dead loggers in the map.
	 * </p>
	 *
	 * @return	The number of active loggers.
	 */
	synchronized int getSize()
	{
		flushQueue();

		return _loggers.size();
	}

	/**
	 * <p>
	 *   Remove all map entries associated with references in the
	 *   reference queue {@link #_queue}.
	 * </p>
	 */
	private synchronized void flushQueue()
	{
		LoggerRef  lref;
		while ((lref = (LoggerRef)_queue.poll()) != null) {
			if (!lref._removed) {
				_loggers.remove(lref._name);
			}
		}
	}

	/**
	 * <p>
	 *   Return the current logging level of this implementation.
	 * </p>
	 *
	 * @return	The current logging level of this implementation.
	 */
	LogLevel getLevel()
	{
		return LogLevel.getInstance(getLogLevel());
	}

	/**
	 * <p>
	 *   Change the logging level of this implementation to the specified
	 *   value.
	 * </p>
	 * <p>
	 *   The logging level is reset to initial level if
	 *   {@link LogLevel#NONE} is specified to {@code level}.
	 * </p>
	 *
	 * @param level		New logging level.
	 * @return		{@code true} if the logging level has been
	 *			actually changed.
	 *			{@code false} if the current logging level is
	 *			 identical to {@code level}.
	 * @throws NullPointerException
	 *	{@code level} is {@code null}.
	 */
	public boolean setLevel(LogLevel level)
	{
		return setLogLevel(level.getLevel());
	}

	/**
	 * <p>
	 *   Create a new {@link Logger} instance.
	 * </p>
	 *
	 * @param name	The name of the logger instance.
	 *		{@code null} means an anonymous logger.
	 */
	abstract Logger newLogger(String name);

	/**
	 * <p>
	 *   Return the current logging level of this implementation.
	 * </p>
	 *
	 * @return	The current logging level value of this implementation.
	 */
	abstract int getLogLevel();

	/**
	 * <p>
	 *   Set the logging level of this implementation.
	 * </p>
	 *
	 * @param level		Logging level value to be set.
	 * @return		{@code true} if the level has been actually
	 *			changed.
	 *			{@code  false} if the current level is
	 *			identical to {@code level}.
	 */
	abstract boolean setLogLevel(int level);
}

/**
 * <p>
 *   Logging system implementation for the trace log.
 * </p>
 */
final class TraceLogImpl extends LogSystemImpl
{
	/**
	 * <p>
	 *   Holder class of the trace log implementation.
	 * </p>
	 */
	private final static class TraceLogHolder
	{
		/**
		 * <p>
		 *   A single global trace log implementation.
		 * </p>
		 */
		private final static TraceLogImpl  _theInstance =
			new TraceLogImpl();
	}

	/**
	 * <p>
	 *   Return the global {@code TraceLogImpl} instance.
	 * </p>
	 *
	 * @return	The {@code TraceLogImpl} instance.
	 */
	static TraceLogImpl getInstance()
	{
		return TraceLogHolder._theInstance;
	}

	/**
	 * <p>
	 *   Create a single instance of the trace log implementation.
	 * </p>
	 */
	private TraceLogImpl()
	{
	}

	/**
	 * <p>
	 *   Create a new {@link Logger} instance for the trace log.
	 * </p>
	 *
	 * @param name	The name of the logger instance.
	 *		{@code null} means an anonymous logger.
	 */
	@Override
	Logger newLogger(String name)
	{
		return new Logger.TraceLogger(name);
	}

	/**
	 * <p>
	 *   Return the current logging level of the trace log.
	 * </p>
	 *
	 * @return	The current logging level value of the trace log.
	 */
	@Override
	native int getLogLevel();

	/**
	 * <p>
	 *   Set the logging level of the trace log.
	 * </p>
	 *
	 * @param level		Logging level value to be set.
	 * @return		{@code true} if the level has been actually
	 *			changed.
	 *			{@code  false} if the current level is
	 *			identical to {@code level}.
	 */
	@Override
	native boolean setLogLevel(int level);
}
