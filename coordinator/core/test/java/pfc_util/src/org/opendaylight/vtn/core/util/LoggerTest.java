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
import java.io.FileReader;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.io.CharArrayWriter;
import java.io.PrintWriter;
import java.util.LinkedList;
import java.util.Iterator;
import java.util.IllegalFormatException;
import java.util.Random;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import static junit.framework.TestCase.assertEquals;
import static junit.framework.TestCase.assertNull;
import static junit.framework.TestCase.assertSame;
import static junit.framework.TestCase.assertTrue;
import static junit.framework.TestCase.fail;

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
	 *   A newline character for the running environment.
	 * </p>
	 */
	final static String  NEWLINE =
		System.getProperty("line.separator", "\n");

	/**
	 * <p>
	 *   A pseudo random number generator.
	 * </p>
	 */
	private final Random  _rand = new Random();

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
	 *   Ensure that logger methods implemented by {@link Logger} construct
	 *   a correct log message from a string and a throwable, and pass it
	 *   to subclass.
	 * </p>
	 */
	public void testThrowable()
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
				throwableTest(name, method);
			}
		}
	}

	/**
	 * <p>
	 *   Ensure that logger methods implemented by {@link Logger} construct
	 *   a correct log message from a format string and a throwable,
	 *   and pass it to subclass.
	 * </p>
	 */
	public void testFormatThrowable()
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
				formatThrowableTest(name, method);
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
	 *   message specified by format string.
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
	 *   Test case of logger method for each levels, which takes a string
	 *   and a throwable.
	 * </p>
	 *
	 * @param name		The name of the logger.
	 * @param method	A {@link LoggerMethod} instance.
	 */
	private void throwableTest(String name, LoggerMethod method)
	{
		// Create a dummy logger.
		DummyLogger logger = new DummyLogger(name);
		assertEquals(name, logger.getName());

		LogLevel level = method.getLevel();
		int lvl = level.getLevel();
		int index = 0;

		String[] whites = {
			NEWLINE,
			" ",
			"\f",
			"\n",
			"\r",
			"\t",
			"\u000b",	// "\v"
		};

		for (int curlvl = LogLevel.LVL_FATAL;
		     curlvl <= LogLevel.LVL_VERBOSE; curlvl++) {
			// Logger methods which takes a throwable checks the
			// current logging level, and it does not construct
			// a log message if it should not be logged.
			boolean dropped;

			if (level == LogLevel.VERBOSE && !CoreSystem.DEBUG) {
				dropped = true;
			}
			else {
				dropped = (lvl > curlvl);
			}

			// Change current logging level.
			logger.setCurrentLevel(curlvl);

			for (int widx = 0; widx < whites.length; widx++) {
				// Reset previous message.
				logger.reset();

				// Log a dummy message.
				// A format conversion specifier should be
				// embedded into the dummy message in order to
				// check whether the message is never passed
				// to Formatter.
				StringBuilder buf = new StringBuilder
					("Test log message: %s: index = ");
				buf.append(index);
				String msg = buf.toString();
				for (int n = 0; n <= widx; n++) {
					buf.append(whites[n]);
				}
				Throwable t = getThrowable(index);
				String message = buf.toString();
				method.invoke(logger, t, message);

				if (dropped) {
					// Verbose log must be dropped on
					// release build.
					assertEquals(LogLevel.LVL_NONE,
						     logger.getLastLevel());
					assertEquals(null,
						     logger.getLastName());
					assertEquals(null,
						     logger.getLastMessage());
				}
				else {
					assertEquals(lvl,
						     logger.getLastLevel());
					assertEquals(name,
						     logger.getLastName());

					buf = new StringBuilder(msg);
					buf.append('\n').append(toString(t));
					assertEquals(buf.toString(),
						     logger.getLastMessage());
				}

				// Specify null throwable.
				method.invoke(logger, (Throwable)null,
					      message);
				if (dropped) {
					assertEquals(LogLevel.LVL_NONE,
						     logger.getLastLevel());
					assertEquals(null,
						     logger.getLastName());
					assertEquals(null,
						     logger.getLastMessage());
				}
				else {
					assertEquals(lvl,
						     logger.getLastLevel());
					assertEquals(name,
						     logger.getLastName());
					assertEquals(msg,
						     logger.getLastMessage());
				}

				// Specify an empty message.
				buf = new StringBuilder();
				for (int n = 0; n <= widx; n++) {
					buf.append(whites[n]);
				}
				method.invoke(logger, t, buf.toString());

				if (dropped) {
					assertEquals(LogLevel.LVL_NONE,
						     logger.getLastLevel());
					assertEquals(null,
						     logger.getLastName());
					assertEquals(null,
						     logger.getLastMessage());
				}
				else {
					assertEquals(lvl,
						     logger.getLastLevel());
					assertEquals(name,
						     logger.getLastName());

					buf = new StringBuilder();
					buf.append('\n').append(toString(t));
					assertEquals(buf.toString(),
						     logger.getLastMessage());
				}

				index++;
			}
		}
	}

	/**
	 * <p>
	 *   Test case of logger method for each levels, which takes a log
	 *   message specified by format string and a throwable.
	 * </p>
	 *
	 * @param name		The name of the logger.
	 * @param method	A {@link LoggerMethod} instance.
	 */
	private void formatThrowableTest(String name, LoggerMethod method)
	{
		// Create a dummy logger.
		DummyLogger logger = new DummyLogger(name);
		assertEquals(name, logger.getName());

		LogLevel level = method.getLevel();
		int lvl = level.getLevel();
		int index = 0;

		String[] whites = {
			NEWLINE,
			" ",
			"\f",
			"\n",
			"\r",
			"\t",
			"\u000b",	// "\v"
		};

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

			// Change current logging level.
			logger.setCurrentLevel(curlvl);

			for (int widx = 0; widx < whites.length; widx++) {
				// Reset previous message.
				logger.reset();

				// Invoke log method with illegal format.
				StringBuilder buf = new StringBuilder
					("Test log message: %s: index = %d");
				for (int n = 0; n <= widx; n++) {
					buf.append(whites[n]);
				}
				String format = buf.toString();
				Throwable t = getThrowable(index);
				Object[] args = {
					index,
					level,
				};

				try {
					method.invoke(logger, t, format, args);
					assertTrue(dropped);
				}
				catch (IllegalFormatException e) {
					assertFalse(dropped);
				}
				catch (Exception e) {
					unexpected(e);
				}

				assertEquals(LogLevel.LVL_NONE,
					     logger.getLastLevel());
				assertEquals(null, logger.getLastName());
				assertEquals(null, logger.getLastMessage());

				args = new Object[]{
					level,
					index,
				};

				// Log a dummy message.
				method.invoke(logger, t, format, args);

				if (dropped) {
					// This message should be dropped.
					assertEquals(LogLevel.LVL_NONE,
						     logger.getLastLevel());
					assertEquals(null,
						     logger.getLastName());
					assertEquals(null,
						     logger.getLastMessage());
				}
				else {
					buf = new StringBuilder
						("Test log message: ");
					buf.append(level).append(": index = ").
						append(index).append(NEWLINE).
						append(toString(t));
					assertEquals(lvl,
						     logger.getLastLevel());
					assertEquals(name,
						     logger.getLastName());
					assertEquals(buf.toString(),
						     logger.getLastMessage());
				}

				// Specify null throwable.
				method.invoke(logger, (Throwable)null, format,
					      args);

				if (dropped) {
					assertEquals(LogLevel.LVL_NONE,
						     logger.getLastLevel());
					assertEquals(null,
						     logger.getLastName());
					assertEquals(null,
						     logger.getLastMessage());
				}
				else {
					buf = new StringBuilder
						("Test log message: ");
					buf.append(level).append(": index = ").
						append(index);
					assertEquals(lvl,
						     logger.getLastLevel());
					assertEquals(name,
						     logger.getLastName());
					assertEquals(buf.toString(),
						     logger.getLastMessage());
				}

				// Specify an empty message.
				buf = new StringBuilder();
				for (int n = 0; n <= widx; n++) {
					buf.append(whites[n]);
				}

				method.invoke(logger, t, "%s", buf.toString());

				if (dropped) {
					assertEquals(LogLevel.LVL_NONE,
						     logger.getLastLevel());
					assertEquals(null,
						     logger.getLastName());
					assertEquals(null,
						     logger.getLastMessage());
				}
				else {
					assertEquals(lvl,
						     logger.getLastLevel());
					assertEquals(name,
						     logger.getLastName());

					buf = new StringBuilder();
					buf.append('\n').append(toString(t));
					assertEquals(buf.toString(),
						     logger.getLastMessage());
				}

				index++;
			}
		}
	}

	/**
	 * <p>
	 *   Create an throwable for test.
	 * </p>
	 *
	 * @param index	An index which determines the type of throwable.
	 * @return	A throwable for test.
	 */
	private Throwable getThrowable(int index)
	{
		return getThrowable(_rand, index);
	}

	/**
	 * <p>
	 *   Create a throwable for test.
	 * </p>
	 *
	 * @param rand	A pseudo random number generator.
	 * @param index	An index which determines the type of throwable.
	 * @return	A throwable for test.
	 */
	private static Throwable getThrowable(Random rand, int index)
	{
		int mod = index % 4;
		int arg = rand.nextInt();
		Throwable ret = null;

		try {
			if (mod == 0) {
				throw new IllegalArgumentException
					("Test runtime exception: " + arg);
			}
			else if (mod == 1) {
				throw new IOException
					("Test exception: " + arg);
			}
			else if (mod == 2) {
				throw new OutOfMemoryError
					("Test error: " + arg);
			}
			else {
				throw new Throwable("Test throwable: " + arg);
			}
		}
		catch (Throwable t) {
			ret = t;
		}

		return ret;
	}

	/**
	 * <p>
	 *   Convert the given throwable into a string.
	 * </p>
	 *
	 * @param t	A throwable.
	 * @return	A string which represents the given throwable.
	 */
	private static String toString(Throwable t)
	{
		CharArrayWriter array = new CharArrayWriter();
		PrintWriter writer = new PrintWriter(array, false);
		t.printStackTrace(writer);
		writer.close();

		StringBuilder builder = new StringBuilder(array.toString());
		int len = builder.length();
		int idx = len - 1;
		for (; idx >= 0; idx--) {
			char  c = builder.charAt(idx);
			if (c != 0 && !Character.isWhitespace(c)) {
				idx++;
				break;
			}
		}

		if (idx < len) {
			builder.delete(idx, len);
		}

		return builder.toString();
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
		Throwable t = getThrowable(ctx.getRandom(), 0);
		for (LoggerMethod method: methods) {
			String msg = "A log message.";
			method.invoke(logger, msg);
			assertEquals(length, file.length());
			method.invoke(logger, t, msg);
			assertEquals(length, file.length());

			String fmt = "A formatted message: %d";
			method.invoke(logger, fmt, 0);
			assertEquals(length, file.length());
			method.invoke(logger, t, fmt, 0);
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
			assertFalse(dropped);

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
		String message = "This is a message string: %s: %d: " +
			curLevel + "." + level + ": " + time;
		int fcnt = ctx.getFatalCount();

		method.invoke(logger, message);

		// Check the fatal error counter.
		if (level == LogLevel.FATAL) {
			assertFalse(dropped);
			fcnt++;
		}
		assertEquals(fcnt, ctx.getFatalCount());

		// Check whether a log record was written to the file or not.
		String log = ctx.getNewRecord();
		if (dropped) {
			assertTrue(log == null);
		}
		else {
			assertNotNull(log);
			message = ctx.createMessage(level, logger.getName(),
						    message);
			assertEquals(message, log);
		}

		// Record a log message and null throwable.
		// The logger should ignore null throwable.
		time = System.nanoTime();
		message = "This is a message string: %s: %d: " +
			curLevel + "." + level + ": " + time;
		Throwable t = null;

		method.invoke(logger, t, message);

		if (level == LogLevel.FATAL) {
			assertFalse(dropped);
			fcnt++;
		}
		assertEquals(fcnt, ctx.getFatalCount());

		log = ctx.getNewRecord();
		if (dropped) {
			assertTrue(log == null);
		}
		else {
			assertNotNull(log);
			message = ctx.createMessage(level, logger.getName(),
						    message);
			assertEquals(message, log);
		}

		// Record a log message by specifying a message string and
		// a throwable.
		time = System.nanoTime();
		message = "This is a message string: %s: %d: " +
			curLevel + "." + level + ": " + time;
		t = getThrowable(ctx.getRandom(), (int)(time % 4));

		method.invoke(logger, t, message);

		// Check the fatal error counter.
		if (level == LogLevel.FATAL) {
			assertFalse(dropped);
			fcnt++;
		}
		assertEquals(fcnt, ctx.getFatalCount());

		// Check whether a log record was written to the file or not.
		log = ctx.getNewRecord();
		if (dropped) {
			assertTrue(log == null);
		}
		else {
			assertNotNull(log);
			message = ctx.createMessage(level, logger.getName(),
						    message, NEWLINE,
						    toString(t));
			assertEquals(message, log);
		}

		// Record a log message by specifying format string.
		String format = "This is a formatted message: %s.%s: %d";
		time = System.nanoTime();

		Object[] args = {curLevel, level, time};
		method.invoke(logger, format, args);

		// Check the fatal error counter.
		if (level == LogLevel.FATAL) {
			assertFalse(dropped);
			fcnt++;
		}
		assertEquals(fcnt, ctx.getFatalCount());

		// Check whether a log record was written to the file or not.
		log = ctx.getNewRecord();
		if (dropped) {
			assertNull(log);
		}
		else {
			String msg =  String.format(format, args);
			message = ctx.createMessage(level, logger.getName(),
						    msg);
			assertNotNull(log);
			assertEquals(message, log);
		}

		// Record a log message by specifying format string and
		// null throwable.
		// The logger should ignore null throwable.
		time = System.nanoTime();
		t = null;

		args = new Object[]{curLevel, level, time};
		method.invoke(logger, t, format, args);

		// Check the fatal error counter.
		if (level == LogLevel.FATAL) {
			assertFalse(dropped);
			fcnt++;
		}
		assertEquals(fcnt, ctx.getFatalCount());

		// Check whether a log record was written to the file or not.
		log = ctx.getNewRecord();
		if (dropped) {
			assertNull(log);
		}
		else {
			String msg =  String.format(format, args);
			message = ctx.createMessage(level, logger.getName(),
						    msg);
			assertNotNull(log);
			assertEquals(message, log);
		}

		// Record a log message by specifying format string and
		// a throwable.
		time = System.nanoTime();
		t = getThrowable(ctx.getRandom(), (int)(time % 4));

		args = new Object[]{curLevel, level, time};
		method.invoke(logger, t, format, args);

		// Check the fatal error counter.
		if (level == LogLevel.FATAL) {
			assertFalse(dropped);
			fcnt++;
		}
		assertEquals(fcnt, ctx.getFatalCount());

		// Check whether a log record was written to the file or not.
		log = ctx.getNewRecord();
		if (dropped) {
			assertNull(log);
		}
		else {
			String msg =  String.format(format, args);
			message = ctx.createMessage(level, logger.getName(),
						    msg, NEWLINE, toString(t));
			assertNotNull(log);
			assertEquals(message, log);
		}
	}
}

/**
 * Log file reader.
 */
class LogFileReader extends BufferedReader
{
	/**
	 * Regular expression that matches the first line of a log record.
	 */
	private final static Pattern  LOG_PATTERN =
		Pattern.compile("^\\d{4}-\\d{2}-\\d{2} " +
				"\\d{2}:\\d{2}:\\d{2}\\.\\d{6}: " +
				"\\[\\d+\\]: " +
				"((FATAL|ERROR|WARNING|NOTICE|INFO|DEBUG|" +
				"TRACE|VERBOSE): (.*))$");

	/**
	 * A stack for unread lines.
	 */
	private final LinkedList<String>  _unreadStack =
		new LinkedList<String>();

	/**
	 * Construct a new object.
	 *
	 * @param file	A file object corresponding to the log file.
	 * @throws FileNotFoundException
	 *	The given file does not exist, or it is unable to open.
	 */
	LogFileReader(File file) throws FileNotFoundException
	{
		super(new FileReader(file));
	}

	/**
	 *  Reads a line of text from the log file.
	 *
	 * @return	A string containing the contents of the line.
	 * @throws IOException
	 *	An I/O error occurred.
	 */
	@Override
	public String readLine() throws IOException
	{
		// Pop unread string if exists.
		return (_unreadStack.isEmpty())
			? super.readLine()
			: _unreadStack.removeFirst();
	}

	/**
	 * Read a log record.
	 *
	 * @return	A string which contains the contents of a log record.
	 *		{@code null} is returned if no log record is available.
	 * @throws IOException
	 *	An I/O error occurred.
	 */
	String readLog() throws IOException
	{
		// Read the first line of a log record.
		String line = readLine();
		if (line == null) {
			return null;
		}

		Matcher m = LOG_PATTERN.matcher(line);
		if (!m.matches()) {
			fail("Unexpected log: " + line);
		}

		StringBuilder builder = new StringBuilder(m.group(1));

		// Read the rest of lines.
		for (line = readLine(); line != null; line = readLine()) {
			m = LOG_PATTERN.matcher(line);
			if (m.matches()) {
				// This record should be returned by the
				// next call.
				unreadLine(line);
				break;
			}

			builder.append(LoggerTest.NEWLINE).append(line);
		}

		return builder.toString();
	}

	/**
	 * Put the given string to the head of file reader stream.
	 *
	 * @param line	A string containing the contents of the line.
	 */
	private void unreadLine(String line)
	{
		_unreadStack.addFirst(line);
	}
}

/**
 * <p>
 *   Context for trace log test.
 * </p>
 */
class LogTestContext implements LogFatalHandler
{
	/**
	 * <p>
	 *   A pseudo random number generator.
	 * </p>
	 */
	private final Random  _rand = new Random();

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
	 * Return a pseudo random number generator.
	 *
	 * @return	A pseudo random number generator.
	 */
	Random getRandom()
	{
		return _rand;
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
		LogFileReader reader = new LogFileReader(_file);
		Iterator<String> it = _messages.iterator();

		// Read all lines in the log file.
		String last = null;
		for (String log = reader.readLog(); log != null;
		     log = reader.readLog()) {
			if (it.hasNext()) {
				// This record should be recorded in _message.
				String msg = it.next();
				assertEquals(msg, log);
			}
			else if (last == null) {
				// Keep the latest log.
				last = log;
			}
			else {
				fail("Unexpected log: [" + log + "], last[" +
				     last + "]");
			}
		}

		if (last != null) {
			// Put a new log record into _message.
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
		StringBuilder buf = new StringBuilder(level.toString());

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
abstract class LogTestManager
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
			String log = ctx.getNewRecord();
			LogLevel tlevel = LogSystem.getInstance().getLevel();

			assertEquals(cur != level, changed);
			if (changed && tlevel.compareTo(LogLevel.INFO) >= 0) {
				// A log message which indicates the change of
				// logging level should be recorded.
				String expected = ctx.createMessage
					(LogLevel.INFO, null,
					 "Logging level for ", getTypeName(),
					 " has been changed to ", level, ".");
				assertEquals(expected, log);
			}
			else {
				assertNull(log);
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
	 *   Invoke logger method with the given log message and throwable.
	 * </p>
	 *
	 * @param logger	A logger instance.
	 * @param t		A throwable.
	 * @param message	A log message.
	 */
	abstract void invoke(Logger logger, Throwable t, String message);

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

	/**
	 * <p>
	 *   Invoke logger method with the given format string and a throwable.
	 * </p>
	 *
	 * @param logger	A logger instance.
	 * @param t		A throwable.
	 * @param format	A format string.
	 * @param args		Arguments referenced by {@code format}.
	 */
	abstract void invoke(Logger logger, Throwable t, String format,
			     Object ... args);
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
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, String message)
	{
		logger.fatal(message);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, Throwable t, String message)
	{
		logger.fatal(t, message);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, String format, Object ... args)
	{
		logger.fatal(format, args);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, Throwable t, String format, Object ... args)
	{
		logger.fatal(t, format, args);
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
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, String message)
	{
		logger.error(message);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, Throwable t, String message)
	{
		logger.error(t, message);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, String format, Object ... args)
	{
		logger.error(format, args);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, Throwable t, String format, Object ... args)
	{
		logger.error(t, format, args);
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
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, String message)
	{
		logger.warning(message);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, Throwable t, String message)
	{
		logger.warning(t, message);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, String format, Object ... args)
	{
		logger.warning(format, args);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, Throwable t, String format, Object ... args)
	{
		logger.warning(t, format, args);
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
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, String message)
	{
		logger.notice(message);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, Throwable t, String message)
	{
		logger.notice(t, message);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, String format, Object ... args)
	{
		logger.notice(format, args);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, Throwable t, String format, Object ... args)
	{
		logger.notice(t, format, args);
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
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, String message)
	{
		logger.info(message);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, Throwable t, String message)
	{
		logger.info(t, message);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, String format, Object ... args)
	{
		logger.info(format, args);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, Throwable t, String format, Object ... args)
	{
		logger.info(t, format, args);
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
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, String message)
	{
		logger.debug(message);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, Throwable t, String message)
	{
		logger.debug(t, message);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, String format, Object ... args)
	{
		logger.debug(format, args);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, Throwable t, String format, Object ... args)
	{
		logger.debug(t, format, args);
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
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, String message)
	{
		logger.trace(message);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, Throwable t, String message)
	{
		logger.trace(t, message);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, String format, Object ... args)
	{
		logger.trace(format, args);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, Throwable t, String format, Object ... args)
	{
		logger.trace(t, format, args);
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
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, String message)
	{
		logger.verbose(message);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, Throwable t, String message)
	{
		logger.verbose(t, message);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, String format, Object ... args)
	{
		logger.verbose(format, args);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	void invoke(Logger logger, Throwable t, String format, Object ... args)
	{
		logger.verbose(t, format, args);
	}
}
