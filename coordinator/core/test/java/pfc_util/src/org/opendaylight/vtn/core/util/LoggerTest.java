/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.util;

import java.io.File;
import java.io.FileReader;
import java.io.BufferedReader;
import java.util.Date;
import java.util.LinkedList;
import java.util.Iterator;
import java.util.IllegalFormatException;
import junit.framework.Assert;
import org.opendaylight.vtn.core.CoreSystem;

/**
 * <p>
 *   Unit test class for {@link Logger}.
 * </p>
 */
public class LoggerTest extends TestBase
{
	/**
	 * <p>
	 *   Create JUnit test case for {@link Logger}.
	 * </p>
	 *
	 * @param name	The test name.
	 */
	public LoggerTest(String name)
	{
		super(name);
	}

	/**
	 * <p>
	 *   Ensure that logger methods implemented by {@link Logger} pass
	 *   correct arguments to subclass.
	 * </p>
	 */
	public void testLogger()
	{
		String[] names = {
			null,
			"name-1",
			"name-2",
		};

		LoggerMethod[] methods = {
			new FatalLoggerMethod(),
			new ErrorLoggerMethod(),
			new WarningLoggerMethod(),
			new NoticeLoggerMethod(),
			new InfoLoggerMethod(),
			new DebugLoggerMethod(),
			new TraceLoggerMethod(),
			new VerboseLoggerMethod(),
		};

		for (String name: names) {
			for (LoggerMethod method: methods) {
				loggerTest(name, method);
			}
		}
	}

	/**
	 * <p>
	 *   Ensure that logger methods implemented by {@link Logger} construct
	 *   a formatted message, and pass it to subclass.
	 * </p>
	 */
	public void testFormatLogger()
	{
		String[] names = {
			null,
			"name-1",
			"name-2",
		};

		LoggerMethod[] methods = {
			new FatalLoggerMethod(),
			new ErrorLoggerMethod(),
			new WarningLoggerMethod(),
			new NoticeLoggerMethod(),
			new InfoLoggerMethod(),
			new DebugLoggerMethod(),
			new TraceLoggerMethod(),
			new VerboseLoggerMethod(),
		};

		for (String name: names) {
			for (LoggerMethod method: methods) {
				formatTest(name, method);
			}
		}
	}

	/**
	 * <p>
	 *   Ensure that logs can be recorded to the log file.
	 * </p>
	 */
	public void testLogFile()
	{
		String tmpdir = "logfile_tmpdir";

		try {
			runOnChild("logFileTest", "testLogFile", tmpdir);
		}
		finally {
			removePath(tmpdir);
		}
	}

	/**
	 * <p>
	 *   Test case of logger method for each levels, which takes a string.
	 * </p>
	 *
	 * @param name		The name of the logger.
	 * @param method	A {@link LoggerMethod} instance.
	 */
	private void loggerTest(String name, LoggerMethod method)
	{
		// Create a dummy logger.
		DummyLogger logger = new DummyLogger(name);
		assertEquals(name, logger.getName());

		LogLevel level = method.getLevel();
		int lvl = level.getLevel();
		int index = 0;

		// Logger method which takes a string always passes a log
		// message to logImpl(), irrespective of current logging level.
		boolean dropped = (level == LogLevel.VERBOSE &&
				   !CoreSystem.DEBUG);

		for (int curlvl = LogLevel.LVL_FATAL;
		     curlvl <= LogLevel.LVL_VERBOSE; curlvl++) {
			// Reset previous message.
			logger.reset();

			// Change current logging level.
			logger.setCurrentLevel(curlvl);

			// Log a dummy message.
			// A format conversion specifier should be embedded
			// into the dummy message in order to check whether
			// the message is never passed to Formatter.
			String message = "Test log message: %s: index = " +
				index;
			index++;
			method.invoke(logger, message);

			if (dropped) {
				// Verbose log must be dropped on release
				// build.
				assertEquals(LogLevel.LVL_NONE,
					     logger.getLastLevel());
				assertEquals(null, logger.getLastName());
				assertEquals(null, logger.getLastMessage());
			}
			else {
				assertEquals(lvl, logger.getLastLevel());
				assertEquals(name, logger.getLastName());
				assertEquals(message, logger.getLastMessage());
			}
		}
	}

	/**
	 * <p>
	 *   Test case of logger method for each levels, which takes a log
	 *   message specified format string.
	 * </p>
	 *
	 * @param name		The name of the logger.
	 * @param method	A {@link LoggerMethod} instance.
	 */
	private void formatTest(String name, LoggerMethod method)
	{
		// Create a dummy logger.
		DummyLogger logger = new DummyLogger(name);
		assertEquals(name, logger.getName());

		LogLevel level = method.getLevel();
		int lvl = level.getLevel();
		int index = 0;

		for (int curlvl = LogLevel.LVL_FATAL;
		     curlvl <= LogLevel.LVL_VERBOSE; curlvl++) {
			// Formatter logger methods checks the current logging
			// level, and it does not construct a log message
			// if it should not be logged.
			boolean dropped;

			if (level == LogLevel.VERBOSE && !CoreSystem.DEBUG) {
				dropped = true;
			}
			else {
				dropped = (lvl > curlvl);
			}

			// Reset previous message.
			logger.reset();

			// Change current logging level.
			logger.setCurrentLevel(curlvl);

			// Invoke log method with illegal format.
			String format = "Test log message: %s: index = %d";
			int idx = index;
			index++;

			Object[] args = {
				idx,
				level,
			};

			try {
				method.invoke(logger, format, args);
				assertTrue(dropped);
			}
			catch (IllegalFormatException e) {
				assertFalse(dropped);
			}
			catch (Exception e) {
				unexpected(e);
			}

			assertEquals(LogLevel.LVL_NONE, logger.getLastLevel());
			assertEquals(null, logger.getLastName());
			assertEquals(null, logger.getLastMessage());

			args = new Object[]{
				level,
				idx,
			};

			// Log a dummy message.
			method.invoke(logger, format, args);

			if (dropped) {
				// This message should be dropped.
				assertEquals(LogLevel.LVL_NONE,
					     logger.getLastLevel());
				assertEquals(null, logger.getLastName());
				assertEquals(null, logger.getLastMessage());
			}
			else {
				String message = "Test log message: " +
					level + ": index = " + idx;
				assertEquals(lvl, logger.getLastLevel());
				assertEquals(name, logger.getLastName());
				assertEquals(message, logger.getLastMessage());
			}
		}
	}

	/**
	 * <p>
	 *   Test logger class which ensures that {@link Logger} methods
	 *   specifies correct arguments to logger method.
	 * </p>
	 */
	private static class DummyLogger extends Logger
	{
		/**
		 * <p>
		 *   Logging level passed to the last call.
		 * </p>
		 */
		private int  _lastLevel = LogLevel.LVL_NONE;

		/**
		 * <p>
		 *   The logger name passed to the last call.
		 * </p>
		 */
		private String  _lastName;

		/**
		 * <p>
		 *   The log message passed to the last call.
		 * </p>
		 */
		private String  _lastMessage;

		/**
		 * <p>
		 *   The current logging level value.
		 * </p>
		 */
		private int  _currentLevel = LogLevel.LVL_INFO;

		/**
		 * <p>
		 *   Create a dummy logger instance.
		 * </p>
		 *
		 * @param name	The name of the logger.
		 */
		private DummyLogger(String name)
		{
			super(name);
		}

		/**
		 * <p>
		 *   Return the current logging level value.
		 * </p>
		 *
		 * @return	The current logging level value.
		 */
		@Override
		int getCurrentLevel()
		{
			return _currentLevel;
		}

		/**
		 * <p>
		 *   Preserve arguments passed by {@link Logger}.
		 * </p>
		 *
		 * @param level		Logging level of the message.
		 * @param name		Module name.
		 * @param msg		Log message.
		 */
		@Override
		void logImpl(int level, String name, String msg)
		{
			_lastLevel = level;
			_lastName = name;
			_lastMessage = msg;
		}

		/**
		 * <p>
		 *   Set current logging level value.
		 * </p>
		 *
		 * @param lvl	New logging level value.
		 */
		private void setCurrentLevel(int lvl)
		{
			_currentLevel = lvl;
		}

		/**
		 * <p>
		 *   Return the logging level passed to the last call.
		 * </p>
		 *
		 * @return	The logging level value.
		 */
		private int getLastLevel()
		{
			return _lastLevel;
		}

		/**
		 * <p>
		 *   Return the logging module name passed to the last call.
		 * </p>
		 *
		 * @return	The logging module name.
		 */
		private String getLastName()
		{
			return _lastName;
		}

		/**
		 * <p>
		 *   Return the log message passed to the last call.
		 * </p>
		 *
		 * @return	The log message.
		 */
		private String getLastMessage()
		{
			return _lastMessage;
		}

		/**
		 * <p>
		 *   Reset received message.
		 * </p>
		 */
		private void reset()
		{
			_lastLevel = LogLevel.LVL_NONE;
			_lastName = null;
			_lastMessage = null;
		}
	}

	/**
	 * <p>
	 *   Ensure that logs can be recorded to the log file.
	 * </p>
	 * <p>
	 *   This method is invoked on a child process.
	 * </p>
	 *
	 * @param args		Arguments passed by the parent.
	 * @throws Exception	An unexpected exception was thrown.
	 */
	static void logFileTest(String[] args) throws Exception
	{
		String ident = args[0];
		String tmpdir = args[1];

		// Ensure that the given temporary directory does not exist.
		removePath(tmpdir);

		// Initialize the logging system.
		LogSystem lsys = LogSystem.getInstance();
		String path = tmpdir + "/logdir/trace.log";
		File file = new File(path).getAbsoluteFile();
		LogConfiguration conf = new LogConfiguration(ident, file);

		LogTestContext ctx = new LogTestContext(file);
		conf.setFatalHandler(ctx);

		lsys.initialize(conf);
		assertTrue(file.isFile());

		String[] names = {
			null,
			"name-1",
		};

		LoggerMethod[] methods = {
			new FatalLoggerMethod(),
			new ErrorLoggerMethod(),
			new WarningLoggerMethod(),
			new NoticeLoggerMethod(),
			new InfoLoggerMethod(),
			new DebugLoggerMethod(),
			new TraceLoggerMethod(),
			new VerboseLoggerMethod(),
		};

		// Run tests.
		TraceLogTestManager tmgr = new TraceLogTestManager();
		for (String name: names) {
			for (LoggerMethod method: methods) {
				// Write trace logs.
				logFileTest(tmgr, ctx, name, method);
			}
		}

		// Change the trace log level to FATAL.
		tmgr.setLevel(LogLevel.FATAL, ctx);

		// Ensure that the trace log level is not changed.
		assertSame(LogLevel.FATAL, tmgr.getLevel());

		long length = file.length();
		assertTrue(length != 0);

		// Finalize the logging system.
		lsys.shutdown();

		// After shutdown() call, any logging request should be
		// ignored.
		Logger logger = Logger.getLogger();
		for (LoggerMethod method: methods) {
			method.invoke(logger, "A log message.");
			assertEquals(length, file.length());

			method.invoke(logger, "A formatted message: %d", 0);
			assertEquals(length, file.length());
		}
	}

	/**
	 * <p>
	 *   Ensure that logs can be recorded to the log file.
	 * </p>
	 *
	 * @param mgr		Logging test manager.
	 * @param ctx		Context for logging test.
	 * @param name		The name of the logger.
	 * @param method	A {@link LoggerMethod} instance.
	 * @throws Exception	An unexpected exception was thrown.
	 */
	static void logFileTest(LogTestManager mgr, LogTestContext ctx,
				String name, LoggerMethod method)
		throws Exception
	{
		// Create a trace logger.
		Logger logger = mgr.getLogger(name);

		// Run tests with changing current logging level.
		for (int curlvl = LogLevel.LVL_FATAL;
		     curlvl <= LogLevel.LVL_VERBOSE; curlvl++) {
			logFileTest(mgr, ctx, logger, method, curlvl);
		}
	}

	/**
	 * <p>
	 *   Ensure that logs can be recorded to the log file.
	 * </p>
	 *
	 * @param mgr		Logging test manager.
	 * @param ctx		Context for logging test.
	 * @param logger	A {@link Logger} instance.
	 * @param method	A {@link LoggerMethod} instance.
	 * @param curlvl	A logging level value to be set to the current
	 *			logging level.
	 * @throws Exception	An unexpected exception was thrown.
	 */
	static void logFileTest(LogTestManager mgr, LogTestContext ctx,
				Logger logger, LoggerMethod method, int curlvl)
		throws Exception
	{
		LogLevel level = method.getLevel();
		int lvl = level.getLevel();

		// Change logging level.
		LogLevel curLevel = LogLevel.getInstance(curlvl);
		curLevel = mgr.setLevel(curLevel, ctx);
		curlvl = curLevel.getLevel();

		// Determine whether a log of the specified level should be
		// dropped or not.
		boolean dropped;

		if (!mgr.isSupported(level)) {
			// Unsupported type of log is always ignored.
			dropped = true;
		}
		else if (level == LogLevel.VERBOSE && !CoreSystem.DEBUG) {
			dropped = true;
		}
		else {
			dropped = (lvl > curlvl);
		}

		// Try to record a null message.
		Class<?> cls = NullPointerException.class;
		try {
			method.invoke(logger, null);
			assertTrue(dropped);
		}
		catch (Exception e) {
			assertFalse(dropped);

			// This must be a NullPointerException thrown by
			// JNI library.
			checkException(e, cls, "message is null.");
		}

		try {
			method.invoke(logger, null, 1, 2, 3);
			assertTrue(dropped);
		}
		catch (Exception e) {
			// This must be a NullPointerException thrown by
			// Java VM.
			checkException(e, cls);
		}

		assertNull(ctx.getNewRecord());

		// Below code will throw an IllegalFormatException when a
		// log message is constructed.
		try {
			method.invoke(logger, "foo: %s, %d", "test");
			assertTrue(dropped);
		}
		catch (IllegalFormatException e) {
			assertFalse(dropped);
		}
		catch (Exception e) {
			unexpected(e);
		}

		assertNull(ctx.getNewRecord());

		// Record a log message by specifying a message string.
		// A format conversion specifier should be embedded into the
		// message in order to check whether the message is never
		// passed to Formatter.
		long time = System.nanoTime();
		String message = "This is a message string: " + curLevel +
			"." + level + ": " + time;
		int fcnt = ctx.getFatalCount();

		method.invoke(logger, message);

		// Check the fatal error counter.
		if (level == LogLevel.FATAL) {
			assertFalse(dropped);
			fcnt++;
		}
		assertEquals(fcnt, ctx.getFatalCount());

		// Check whether a log record was written to the file or not.
		String line = ctx.getNewRecord();
		if (dropped) {
			assertTrue(line == null);
		}
		else {
			assertNotNull(line);
			message = ctx.createMessage(level, logger.getName(),
						    message);
			assertTrue(line.endsWith(message));
		}

		// Record a log message by specifying format string.
		String format = "This is a formatted message: %s.%s: %d";
		time = System.nanoTime();
		fcnt = ctx.getFatalCount();

		Object[] args = {curLevel, level, time};
		method.invoke(logger, format, args);

		// Check the fatal error counter.
		if (level == LogLevel.FATAL) {
			assertFalse(dropped);
			fcnt++;
		}
		assertEquals(fcnt, ctx.getFatalCount());

		// Check whether a log record was written to the file or not.
		line = ctx.getNewRecord();
		if (dropped) {
			assertNull(line);
		}
		else {
			String msg =  String.format(format, args);
			message = ctx.createMessage(level, logger.getName(),
						    msg);
			assertNotNull(line);
			assertTrue(line.endsWith(message));
		}
	}
}

/**
 * <p>
 *   Context for trace log test.
 * </p>
 */
class LogTestContext extends Assert implements LogFatalHandler
{
	/**
	 * <p>
	 *   Log file.
	 * </p>
	 */
	private final File  _file;

	/**
	 * <p>
	 *   The number of fatal logs.
	 * </p>
	 */
	private int  _fatalCount;

	/**
	 * <p>
	 *   A list which keeps all messages in the log file.
	 * </p>
	 */
	private final LinkedList<String>  _messages = new LinkedList<String>();

	/**
	 * <p>
	 *   Construct a new logging test context.
	 * </p>
	 *
	 * @param file	Log file.
	 */
	LogTestContext(File file)
	{
		_file = file;
	}

	/**
	 * <p>
	 *   Return the number of fatal error.
	 * </p>
	 *
	 * @return	The number of fatal error.
	 */
	int getFatalCount()
	{
		return _fatalCount;
	}

	/**
	 * <p>
	 *   A handler which is invoked when a FATAL level message is logged.
	 * </p>
	 */
	public void fatalError()
	{
		_fatalCount++;
	}

	/**
	 * <p>
	 *   Return a log record newly added to the log file.
	 * </p>
	 * <p>
	 *   This method keeps all records returned by this method.
	 *   If a new log record is written to the log file, this method
	 *   must be called to record it.
	 * </p>
	 *
	 * @return	A log record newly added to the log file.
	 *		{@code null} is returned if no line is added.
	 * @throws Exception
	 *	An unexpected exception was thrown.
	 */
	String getNewRecord() throws Exception
	{
		BufferedReader reader = new BufferedReader
			(new FileReader(_file));
		Iterator<String> it = _messages.iterator();

		// Read all lines in the log file.
		String last = null;
		for (String line = reader.readLine(); line != null;
		     line = reader.readLine()) {
			if (it.hasNext()) {
				// This line should be recorded in _message.
				String msg = it.next();
				assertEquals(msg, line);
			}
			else if (last == null) {
				// This must be the last line in the log file.
				last = line;
			}
			else {
				fail("Unexpected log: [" + line + "], last[" +
				     last + "]");
			}
		}

		if (last != null) {
			// Record a new line in _message.
			_messages.add(last);
		}

		reader.close();

		return last;
	}

	/**
	 * <p>
	 *   Create an expected log message.
	 * </p>
	 * <p>
	 *   Note that returned message does not contains timestamp and
	 *   kernel thread ID.
	 * </p>
	 *
	 * @param level	Level of the log message.
	 * @param name	Name of the logger.
	 * @param msgs	Array of object to be added to the messagel
	 */
	String createMessage(LogLevel level, String name, Object ... msgs)
	{
		StringBuffer buf = new StringBuffer(level.toString());

		buf.append(": ");
		if (name != null) {
			buf.append(name);
			buf.append(": ");
		}

		for (Object m: msgs) {
			buf.append(m);
		}

		return buf.toString();
	}
}

/**
 * <p>
 *   Abstract class which handles the logging system.
 * </p>
 */
abstract class LogTestManager extends Assert
{
	/**
	 * <p>
	 *   A symbolic name of the log type.
	 * </p>
	 */
	private final String  _typeName;

	/**
	 * <p>
	 *   Construct a new log manager instance.
	 * </p>
	 *
	 * @param typeName	A symbolic name of the log type.
	 */
	LogTestManager(String typeName)
	{
		_typeName = typeName;
	}

	/**
	 * <p>
	 *   Return a symbolic name of the log type.
	 * </p>
	 *
	 * @return	A symbolic name of the log type.
	 */
	String getTypeName()
	{
		return _typeName;
	}

	/**
	 * <p>
	 *   Get a {@link Logger} instance.
	 * </p>
	 *
	 * @param name	Name of the logger.
	 * @return	A {@link Logger} instance associated with {@code name}.
	 */
	abstract Logger getLogger(String name);

	/**
	 * <p>
	 *   Set the current logging level.
	 * </p>
	 *
	 * @param level	A new logging level.
	 * @param ctx	Context for logging test.
	 * @return	A new logging level applied actually is returned.
	 * @throws Exception
	 *	An unexpected exception was thrown.
	 */
	LogLevel setLevel(LogLevel level, LogTestContext ctx) throws Exception
	{
		LogLevel cur = getLevel();

		if (isSupported(level)) {
			boolean changed = setLevel(level);
			String line = ctx.getNewRecord();
			LogLevel tlevel = LogSystem.getInstance().getLevel();

			assertEquals(cur != level, changed);
			if (changed && tlevel.compareTo(LogLevel.INFO) >= 0) {
				// A log message which indicates the change of
				// logging level should be recorded.
				String expected = ctx.createMessage
					(LogLevel.INFO, null,
					 "Logging level for ", getTypeName(),
					 " has been changed to ", level, ".");
				assertTrue(line.endsWith(expected));
			}
			else {
				assertNull(line);
			}
		}
		else {
			// Unsupported level can not be set to the logging
			// level.
			Class<?> cls = IllegalArgumentException.class;
			String msg = "Invalid syslog level: " + level;

			try {
				setLevel(level);
				TestBase.needException();
			}
			catch (Exception e) {
				TestBase.checkException(e, cls, msg);
			}

			level = cur;
		}

		assertSame(level, getLevel());

		return level;
	}

	/**
	 * <p>
	 *   Set the current logging level.
	 * </p>
	 *
	 * @param level	A new logging level.
	 */
	abstract boolean setLevel(LogLevel level);

	/**
	 * <p>
	 *   Return the current logging level.
	 * </p>
	 *
	 * @return	The current logging level.
	 */
	abstract LogLevel getLevel();

	/**
	 * <p>
	 *   Determine whether this log type supports the given log level
	 *   or not.
	 * </p>
	 *
	 * @param level	A logging level to be tested.
	 * @return	{@code true} only if the given logging level is
	 *		supported.
	 */
	abstract boolean isSupported(LogLevel level);
}

/**
 * <p>
 *   Manager class which handles the trace log.
 * </p>
 */
final class TraceLogTestManager extends LogTestManager
{
	/**
	 * <p>
	 *   Construct a new log manager instance.
	 * </p>
	 */
	TraceLogTestManager()
	{
		super("trace log");
	}

	/**
	 * <p>
	 *   Get a {@link Logger} instance.
	 * </p>
	 *
	 * @param name	Name of the logger.
	 * @return	A {@link Logger} instance associated with {@code name}.
	 */
	@Override
	Logger getLogger(String name)
	{
		return Logger.getLogger(name);
	}

	/**
	 * <p>
	 *   Set the current logging level.
	 * </p>
	 *
	 * @param level	A new logging level.
	 * @return	{@code true} if the logging level has been actually
	 *		changed.
	 *		{@code false} if the current logging level is
	 *		identical to {@code level}.
	 */
	@Override
	boolean setLevel(LogLevel level)
	{
		return LogSystem.getInstance().setLevel(level);
	}

	/**
	 * <p>
	 *   Return the current logging level.
	 * </p>
	 *
	 * @return	The current logging level.
	 */
	@Override
	LogLevel getLevel()
	{
		return LogSystem.getInstance().getLevel();
	}

	/**
	 * <p>
	 *   Determine whether this log type supports the given log level
	 *   or not.
	 * </p>
	 *
	 * @param level	A logging level to be tested.
	 * @return	{@code true} only if the given logging level is
	 *		supported.
	 */
	@Override
	boolean isSupported(LogLevel level)
	{
		// Trace log supports all log levels.
		return true;
	}
}

/**
 * <p>
 *   Abstract class which represents a logging method in {@link Logger}.
 * </p>
 */
abstract class LoggerMethod
{
	/**
	 * <p>
	 *   Log level of this method.
	 * </p>
	 */
	private final LogLevel  _level;

	/**
	 * <p>
	 *   Construct a new logger method.
	 * </p>
	 *
	 * @param level	Log level of the method.
	 */
	LoggerMethod(LogLevel level)
	{
		_level = level;
	}

	/**
	 * <p>
	 *   Return a log level of this method.
	 * </p>
	 *
	 * @return	Log level of this method.
	 */
	LogLevel getLevel()
	{
		return _level;
	}

	/**
	 * <p>
	 *   Invoke logger method with the given log message.
	 * </p>
	 *
	 * @param logger	A logger instance.
	 * @param message	A log message.
	 */
	abstract void invoke(Logger logger, String message);

	/**
	 * <p>
	 *   Invoke logger method with the given format string.
	 * </p>
	 *
	 * @param logger	A logger instance.
	 * @param format	A format string.
	 * @param args		Arguments referenced by {@code format}.
	 */
	abstract void invoke(Logger logger, String format, Object ... args);
}

/**
 * <p>
 *   Logger interface to record a fatal level log.
 * </p>
 */
final class FatalLoggerMethod extends LoggerMethod
{
	/**
	 * <p>
	 *   Construct a new fatal error logger method.
	 * </p>
	 */
	FatalLoggerMethod()
	{
		super(LogLevel.FATAL);
	}

	/**
	 * <p>
	 *   Invoke logger method with the given log message.
	 * </p>
	 *
	 * @param logger	A logger instance.
	 * @param message	A log message.
	 */
	@Override
	void invoke(Logger logger, String message)
	{
		logger.fatal(message);
	}

	/**
	 * <p>
	 *   Invoke logger method with the given format string.
	 * </p>
	 *
	 * @param logger	A logger instance.
	 * @param format	A format string.
	 * @param args		Arguments referenced by {@code format}.
	 */
	@Override
	void invoke(Logger logger, String format, Object ... args)
	{
		logger.fatal(format, args);
	}
}

/**
 * <p>
 *   Logger interface to record an error level log.
 * </p>
 */
final class ErrorLoggerMethod extends LoggerMethod
{
	/**
	 * <p>
	 *   Construct a new error logger method.
	 * </p>
	 */
	ErrorLoggerMethod()
	{
		super(LogLevel.ERROR);
	}

	/**
	 * <p>
	 *   Invoke logger method with the given log message.
	 * </p>
	 *
	 * @param logger	A logger instance.
	 * @param message	A log message.
	 */
	@Override
	void invoke(Logger logger, String message)
	{
		logger.error(message);
	}

	/**
	 * <p>
	 *   Invoke logger method with the given format string.
	 * </p>
	 *
	 * @param logger	A logger instance.
	 * @param format	A format string.
	 * @param args		Arguments referenced by {@code format}.
	 */
	@Override
	void invoke(Logger logger, String format, Object ... args)
	{
		logger.error(format, args);
	}
}

/**
 * <p>
 *   Logger interface to record a warning level log.
 * </p>
 */
final class WarningLoggerMethod extends LoggerMethod
{
	/**
	 * <p>
	 *   Construct a new warning logger method.
	 * </p>
	 */
	WarningLoggerMethod()
	{
		super(LogLevel.WARNING);
	}

	/**
	 * <p>
	 *   Invoke logger method with the given log message.
	 * </p>
	 *
	 * @param logger	A logger instance.
	 * @param message	A log message.
	 */
	@Override
	void invoke(Logger logger, String message)
	{
		logger.warning(message);
	}

	/**
	 * <p>
	 *   Invoke logger method with the given format string.
	 * </p>
	 *
	 * @param logger	A logger instance.
	 * @param format	A format string.
	 * @param args		Arguments referenced by {@code format}.
	 */
	@Override
	void invoke(Logger logger, String format, Object ... args)
	{
		logger.warning(format, args);
	}
}

/**
 * <p>
 *   Logger interface to record a notice level log.
 * </p>
 */
final class NoticeLoggerMethod extends LoggerMethod
{
	/**
	 * <p>
	 *   Construct a new notice logger method.
	 * </p>
	 */
	NoticeLoggerMethod()
	{
		super(LogLevel.NOTICE);
	}

	/**
	 * <p>
	 *   Invoke logger method with the given log message.
	 * </p>
	 *
	 * @param logger	A logger instance.
	 * @param message	A log message.
	 */
	@Override
	void invoke(Logger logger, String message)
	{
		logger.notice(message);
	}

	/**
	 * <p>
	 *   Invoke logger method with the given format string.
	 * </p>
	 *
	 * @param logger	A logger instance.
	 * @param format	A format string.
	 * @param args		Arguments referenced by {@code format}.
	 */
	@Override
	void invoke(Logger logger, String format, Object ... args)
	{
		logger.notice(format, args);
	}
}

/**
 * <p>
 *   Logger interface to record a info level log.
 * </p>
 */
final class InfoLoggerMethod extends LoggerMethod
{
	/**
	 * <p>
	 *   Construct a new info logger method.
	 * </p>
	 */
	InfoLoggerMethod()
	{
		super(LogLevel.INFO);
	}

	/**
	 * <p>
	 *   Invoke logger method with the given log message.
	 * </p>
	 *
	 * @param logger	A logger instance.
	 * @param message	A log message.
	 */
	@Override
	void invoke(Logger logger, String message)
	{
		logger.info(message);
	}

	/**
	 * <p>
	 *   Invoke logger method with the given format string.
	 * </p>
	 *
	 * @param logger	A logger instance.
	 * @param format	A format string.
	 * @param args		Arguments referenced by {@code format}.
	 */
	@Override
	void invoke(Logger logger, String format, Object ... args)
	{
		logger.info(format, args);
	}
}

/**
 * <p>
 *   Logger interface to record a debug level log.
 * </p>
 */
final class DebugLoggerMethod extends LoggerMethod
{
	/**
	 * <p>
	 *   Construct a new debug logger method.
	 * </p>
	 */
	DebugLoggerMethod()
	{
		super(LogLevel.DEBUG);
	}

	/**
	 * <p>
	 *   Invoke logger method with the given log message.
	 * </p>
	 *
	 * @param logger	A logger instance.
	 * @param message	A log message.
	 */
	@Override
	void invoke(Logger logger, String message)
	{
		logger.debug(message);
	}

	/**
	 * <p>
	 *   Invoke logger method with the given format string.
	 * </p>
	 *
	 * @param logger	A logger instance.
	 * @param format	A format string.
	 * @param args		Arguments referenced by {@code format}.
	 */
	@Override
	void invoke(Logger logger, String format, Object ... args)
	{
		logger.debug(format, args);
	}
}

/**
 * <p>
 *   Logger interface to record a trace level log.
 * </p>
 */
final class TraceLoggerMethod extends LoggerMethod
{
	/**
	 * <p>
	 *   Construct a new trace logger method.
	 * </p>
	 */
	TraceLoggerMethod()
	{
		super(LogLevel.TRACE);
	}

	/**
	 * <p>
	 *   Invoke logger method with the given log message.
	 * </p>
	 *
	 * @param logger	A logger instance.
	 * @param message	A log message.
	 */
	@Override
	void invoke(Logger logger, String message)
	{
		logger.trace(message);
	}

	/**
	 * <p>
	 *   Invoke logger method with the given format string.
	 * </p>
	 *
	 * @param logger	A logger instance.
	 * @param format	A format string.
	 * @param args		Arguments referenced by {@code format}.
	 */
	@Override
	void invoke(Logger logger, String format, Object ... args)
	{
		logger.trace(format, args);
	}
}

/**
 * <p>
 *   Logger interface to record a verbose level log.
 * </p>
 */
final class VerboseLoggerMethod extends LoggerMethod
{
	/**
	 * <p>
	 *   Construct a new verbose logger method.
	 * </p>
	 */
	VerboseLoggerMethod()
	{
		super(LogLevel.VERBOSE);
	}

	/**
	 * <p>
	 *   Invoke logger method with the given log message.
	 * </p>
	 *
	 * @param logger	A logger instance.
	 * @param message	A log message.
	 */
	@Override
	void invoke(Logger logger, String message)
	{
		logger.verbose(message);
	}

	/**
	 * <p>
	 *   Invoke logger method with the given format string.
	 * </p>
	 *
	 * @param logger	A logger instance.
	 * @param format	A format string.
	 * @param args		Arguments referenced by {@code format}.
	 */
	@Override
	void invoke(Logger logger, String format, Object ... args)
	{
		logger.verbose(format, args);
	}
}
