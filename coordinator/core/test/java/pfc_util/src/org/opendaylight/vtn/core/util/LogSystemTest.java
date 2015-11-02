/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.util;

import java.io.File;
import java.util.LinkedList;
import java.util.HashSet;
import java.util.HashMap;
import java.util.Map;
import java.util.Iterator;
import java.lang.reflect.Method;

import static junit.framework.TestCase.assertNotNull;

/**
 * <p>
 *   Unit test class for {@link LogSystem}.
 * </p>
 */
public class LogSystemTest extends TestBase
{
	/**
	 * An interface that specifies the condition to met.
	 */
	private interface TestCond {
		/**
		 * Determine whether the specified condition is met or not.
		 *
		 * @return  {@code true} only if the condition is met.
		 */
		boolean check();

		/**
		 * Invoked when the test has been timed out.
		 */
		void timedOut();
	}

	/**
	 * <p>
	 *   Create JUnit test case for {@link LogSystem}.
	 * </p>
	 *
	 * @param name	The test name.
	 */
	public LogSystemTest(String name)
	{
		super(name);
	}

	/**
	 * <p>
	 *   Test case of {@link LogSystem#getInstance()}.
	 * </p>
	 */
	public void testGetInstance()
	{
		LogSystem  lsys = LogSystem.getInstance();
		assertNotNull(lsys);

		for (int i = 0; i < 100; i++) {
			LogSystem  l = LogSystem.getInstance();
			assertSame(lsys, l);
		}
	}

	/**
	 * <p>
	 *   Test case of {@link LogSystem#setLevel(LogLevel)}.
	 * </p>
	 */
	public void testSetLevel()
	{
		LogLevel invalid[] = {
				LogLevel.NONE,
				LogLevel.TRACE,
				LogLevel.VERBOSE,
			};

		LogSystem lsys = LogSystem.getInstance();

		// NullPointerException must be thrown
		try {
			lsys.setLevel(null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class, null);
		}
	}

	/**
	 * <p>
	 *   Test case of {@link LogSystemImpl#getLogger(String)} for
	 *   {@link TraceLogImpl}.
	 * </p>
	 */
	public void testGetTraceLogger()
	{
		getLoggerTest(new TestTraceLoggerFactory());
	}

	/**
	 * <p>
	 *   Race condition test for creation of logger instances.
	 * </p>
	 */
	public void testCreationRace()
	{
		int  nthreads = 3;
		int  loggerCount = 1024;
		int  duration = 500;	// 500 milliseconds.

		// Create 3 threads which create trace loggers.
		TestTraceLoggerFactory tfactory = new TestTraceLoggerFactory();
		RaceCreationThread[] tt =
			new RaceCreationThread[nthreads];

		for (int i = 0; i < nthreads; i++) {
			tt[i] = new RaceCreationThread(tfactory, loggerCount,
						       i);
		}

		// Create one thread to invoke the GC periodically.
		Thread gt = new Thread() {
			public void run() {
				while (true) {
					System.gc();

					try {
						sleep(1000);
					}
					catch (InterruptedException e) {
						break;
					}
				}
			}
		};

		// Start test threads.
		for (int i = 0; i < nthreads; i++) {
			tt[i].start();
		}
		gt.start();

		try {
			Thread.sleep(duration);

			// Terminate test threads.
			gt.interrupt();
			for (int i = 0; i < nthreads; i++) {
				tt[i].interrupt();
				tt[i].join();
			}
			gt.join();
		}
		catch (Exception e) {
			fail("Unexpected exception: " + e);
		}

		// Ensure that all loggers are removed by the GC.
		final LogSystemImpl tsys = TraceLogImpl.getInstance();
		TestCond cond = new TestCond() {
			@Override
			public boolean check() {
				return (tsys.getSize() == 0);
			}

			@Override
			public void timedOut() {
				assertEquals(0, tsys.getSize());
			}
		};
		runGC(cond);
	}

	/**
	 * <p>
	 *   Ensure that logging levels are initialized correctly.
	 * </p>
	 */
	public void testLogLevel()
	{
		String tmpdir = "logsys_tmpdir";
		String logfile = tmpdir + "/logs/test.log";
		String lvlfile = tmpdir + "/level";

		// Ensure that the temporary directory does not exist.
		removePath(tmpdir);

		try {
			logLevelTest(logfile, lvlfile);
		}
		finally {
			removePath(tmpdir);
		}
	}

	/**
	 * <p>
	 *   Ensure that logging levels are initialized correctly.
	 * </p>
	 *
	 * @param logFile	The log file path.
	 * @param lvlFile	The log level file path.
	 */
	private void logLevelTest(String logFile, String lvlFile)
	{
		int[] levels = {
			LogLevel.LVL_FATAL,
			LogLevel.LVL_TRACE,
		};

		// Change log levels with default parameters.
		for (int lvl: levels) {
			LogLevelTest test = new LogLevelTest(logFile);
			test.setNewTraceLevel(lvl);
			test.invoke();
		}

		// Change log levels with default levels.
		for (int lvl: levels) {
			LogLevelTest test = new LogLevelTest(logFile);
			test.setNewTraceLevel(LogLevel.LVL_DEBUG);
			test.setDefaultTraceLevel(lvl);
			test.invoke();
		}

		// Change log levels with saving them to the log level file.
		int oldt = LogLevel.LVL_NONE;
		for (int lvl: levels) {
			LogLevelTest test = new LogLevelTest(logFile, lvlFile);
			test.setNewTraceLevel(lvl);
			test.setDefaultTraceLevel(LogLevel.LVL_NOTICE);
			if (oldt != LogLevel.LVL_NONE) {
				test.setSavedTraceLevel(oldt);
			}
			test.invoke();
			oldt = lvl;
		}

		// Specify user-defined log levels.
		oldt = LogLevel.LVL_FATAL;
		for (int lvl: levels) {
			oldt++;
			LogLevelTest test = new LogLevelTest(logFile, lvlFile);
			test.setTraceLevel(lvl);
			test.setNewTraceLevel(oldt);
			test.setDefaultTraceLevel(LogLevel.LVL_DEBUG);
			test.invoke();
		}

		// Ensure that log levels are saved in the level file.
		LogLevelTest test = new LogLevelTest(logFile, lvlFile);
		test.setSavedTraceLevel(oldt);
		test.setNewTraceLevel(oldt);
		test.invoke();

		// Reset trace log level.
		test = new LogLevelTest(logFile, lvlFile);
		test.setSavedTraceLevel(oldt);
		test.invoke();

		// No logging level is saved.
		test = new LogLevelTest(logFile, lvlFile);
		test.setDefaultTraceLevel(LogLevel.LVL_TRACE);
		test.invoke();
	}

	/**
	 * <p>
	 *   A class used to invoke {@link #logLevelTest(String[])} on a
	 *   child process.
	 * </p>
	 */
	private final class LogLevelTest
	{
		/**
		 * <p>
		 *   Command line arguments.
		 * </p>
		 */
		private final HashMap<String, String> _args =
			new HashMap<String, String>();

		/**
		 * <p>
		 *   Construct a new instance.
		 * </p>
		 *
		 * @param logFile	The log file path.
		 */
		private LogLevelTest(String logFile)
		{
			_args.put("logfile", logFile);
		}

		/**
		 * <p>
		 *   Construct a new instance with specifying the log level
		 *   file.
		 * </p>
		 *
		 * @param logFile	The log file path.
		 * @param lvlFile	The log level file path.
		 */
		private LogLevelTest(String logFile, String lvlFile)
		{
			this(logFile);

			_args.put("levelfile", lvlFile);
		}

		/**
		 * <p>
		 *   Invoke {@link #logLevelTest(String[])} on a child process.
		 * </p>
		 */
		private void invoke()
		{
			Object[] args = new Object[_args.size()];

			int i = 0;
			for (Iterator<Map.Entry<String, String>> it =
				     _args.entrySet().iterator();
			     it.hasNext(); i++) {
				Map.Entry<String, String> entry = it.next();
				String key = entry.getKey();
				String value = entry.getValue();
				args[i] = key + "=" + value;
			}

			runOnChild("logLevelTest", args);
		}

		/**
		 * <p>
		 *   Set command line option which specifies an integer value.
		 * </p>
		 *
		 * @param name	A option name.
		 * @param value	An integer associated with the given option.
		 */
		private void setInteger(String name, int value)
		{
			_args.put(name, Integer.toString(value));
		}

		/**
		 * <p>
		 *   Set a user-defined trace log level.
		 * </p>
		 *
		 * @param lvl	A user-defined trace log level value.
		 */
		private void setTraceLevel(int lvl)
		{
			setInteger("trace", lvl);
		}

		/**
		 * <p>
		 *   Set a default trace log level.
		 * </p>
		 *
		 * @param lvl	A default trace log level value.
		 */
		private void setDefaultTraceLevel(int lvl)
		{
			setInteger("deftrace", lvl);
		}

		/**
		 * <p>
		 *   Set a trace log level to be set by
		 *   {@link #logLevelTest(String[])}.
		 * </p>
		 *
		 * @param lvl	A trace log level value to be set.
		 */
		private void setNewTraceLevel(int lvl)
		{
			setInteger("newtrace", lvl);
		}

		/**
		 * <p>
		 *   Set a trace log level value saved in the log level file.
		 * </p>
		 *
		 * @param lvl	A trace log level value saved in the log level
		 *		file.
		 */
		private void setSavedTraceLevel(int lvl)
		{
			setInteger("savedtrace", lvl);
		}
	}

	/**
	 * <p>
	 *   Ensure that the {@link LogSystem} instance keeps active loggers.
	 * </p>
	 *
	 * @param factory	Factory class of the logger.
	 */
	private void getLoggerTest(final TestLoggerFactory factory)
	{
		// Create 3 loggers.
		String[] names = {
			null,
			"",
			"logger-3",
		};

		Class<?> cls = factory.getRequiredClass();
		Logger[] loggers = new Logger[names.length];
		HashSet<Logger> lset = new HashSet<Logger>();
		for (int i = 0; i < names.length; i++) {
			loggers[i] = factory.newLogger(names[i]);
			assertLogger(loggers[i], cls, names[i]);
			assertTrue(lset.add(loggers[i]));
		}

		// Ensure that loggers are kept by LogSystem.
		final LogSystemImpl impl = factory.getImpl();
		assertEquals(loggers.length, impl.getSize());

		// Run the GC.
		runGC();

		// Loggers must be still kept by LogSystem.
		assertEquals(loggers.length, impl.getSize());
		for (int i = 0; i < names.length; i++) {
			assertEquals(loggers[i], factory.newLogger(names[i]));
		}

		int size = loggers.length;
		for (int i = 0; i < loggers.length; i++) {
			// Remove the logger from HashSet.
			assertTrue(lset.remove(loggers[i]));

			// Run the GC.
			// The logger must be still active because it is
			// held by logger array.
			runGC();
			assertEquals(size, impl.getSize());
			assertEquals(loggers[i], factory.newLogger(names[i]));

			// Unlink the logger, and make the GC collect it.
			loggers[i] = null;
			size--;
			final int sz = size;
			TestCond cond = new TestCond() {
				@Override
				public boolean check() {
					return (impl.getSize() == sz);
				}

				@Override
				public void timedOut() {
					assertEquals(sz, impl.getSize());
				}
			};
			runGC(cond);
		}
		assertEquals(0, lset.size());

		// Create 3 loggers again.
		for (int i = 0; i < names.length; i++) {
			assertEquals(i, impl.getSize());
			loggers[i] = factory.newLogger(names[i]);
			assertSame(loggers[i], factory.newLogger(names[i]));
			assertLogger(loggers[i], cls, names[i]);
			assertTrue(lset.add(loggers[i]));
			assertEquals(i + 1, impl.getSize());
			assertEquals(i + 1, lset.size());
		}

		// Invalidate all loggers.
		lset.clear();
		for (int i = 0; i < names.length; i++) {
			loggers[i] = null;
		}

		// Create a logger with specifying another name.
		// This purges dead loggers.
		final String another = "another-logger";
		TestCond cond = new TestCond() {
			@Override
			public boolean check() {
				Logger logger = factory.newLogger(another);
				return (impl.getSize() == 1);
			}

			@Override
			public void timedOut() {
				runGC();
				Logger logger = factory.newLogger(another);
				assertEquals(1, impl.getSize());
			}
		};
		runGC(cond);
	}

	/**
	 * <p>
	 *   Verify the logger instance.
	 * </p>
	 *
	 * @param logger	The {@link Logger} instance to be verified.
	 * @param cls		Required class of the logger.
	 * @param name		Required name of the logger.
	 */
	private void assertLogger(Logger logger, Class<?> cls, String name)
	{
		assertEquals(cls, logger.getClass());
		assertEquals(name, logger.getName());
	}

	/**
	 * <p>
	 *   Run the GC and insert short delay.
	 * </p>
	 */
	private void runGC()
	{
		System.gc();

		try {
			Thread.sleep(5);
		}
		catch (InterruptedException e) {
		}
	}

	/**
	 * <p>
	 *   Run the GC util the given condition is met.
	 * </p>
	 *
	 * @param cond  A {@link TestCond} that specifies the condition.
	 */
	private void runGC(TestCond cond) {
		runGC();

		long cur = System.currentTimeMillis();
		long timeout = cur + 10000;
		while (!cond.check()) {
			runGC();
			cur = System.currentTimeMillis();
			if (cur > timeout) {
				cond.timedOut();
			}
		}
	}

	/**
	 * <p>
	 *   Private interface of {@link Logger} factory class.
	 * </p>
	 */
	private interface TestLoggerFactory
	{
		/**
		 * <p>
		 *   Create a new logger instance.
		 * </p>
		 *
		 * @param name	The name of the logger.
		 * @return	A new logger instance.
		 */
		public Logger newLogger(String name);

		/**
		 * <p>
		 *   Return required class of the logger.
		 * </p>
		 *
		 * @return	A class object of the logger.
		 */
		public Class<?> getRequiredClass();

		/**
		 * <p>
		 *   Get implementation of this logging system.
		 * </p>
		 *
		 * @return	A {@link LogSystemImpl} instance associated
		 *		with this logging system.
		 */
		public LogSystemImpl getImpl();
	}

	/**
	 * <p>
	 *   Factory class of {@link Logger.TraceLogger} instance.
	 * <p>
	 */
	private static class TestTraceLoggerFactory
		implements TestLoggerFactory
	{
		/**
		 * <p>
		 *   Create a new trace logger instance.
		 * </p>
		 *
		 * @param name	The name of logger instance.
		 * @return	A new trace logger instance.
		 */
		@Override
		public Logger newLogger(String name)
		{
			return Logger.getLogger(name);
		}

		/**
		 * <p>
		 *   Return required class of the logger instance.
		 * </p>
		 *
		 * @return	Class object of {@link Logger.TraceLogger}.
		 */
		@Override
		public Class<?> getRequiredClass() {
			return Logger.TraceLogger.class;
		}

		/**
		 * <p>
		 *   Get implementation of the trace log.
		 * </p>
		 *
		 * @return	A {@link LogSystemImpl} instance associated
		 *		with the trace log.
		 */
		public LogSystemImpl getImpl()
		{
			return TraceLogImpl.getInstance();
		}
	}

	/**
	 * <p>
	 *   A test thread which creates logger instances.
	 * </p>
	 */
	private class RaceCreationThread extends Thread
	{
		/**
		 * <p>
		 *   Logger factory class.
		 * </p>
		 */
		private final TestLoggerFactory  _factory;

		/**
		 * <p>
		 *   The number of named logger instances.
		 * </p>
		 */
		private final int  _count;

		/**
		 * <p>
		 *   The number of milliseconds to be inserted.
		 * </p>
		 */
		private final long  _delay;

		/**
		 * <p>
		 *   Create a new test thread to create logger instances.
		 * </p>
		 *
		 * @param factory	Factory object of the logger.
		 * @param count		The number of named loggers to be
		 *			created.
		 * @param delay		The number of milliseconds to be
		 *			inserted.
		 */
		private RaceCreationThread(TestLoggerFactory factory,
					   int count, long delay)
		{
			_factory = factory;
			_count = count;
			_delay = delay;
		}

		/**
		 * <p>
		 *   Run the test until the thread is interrupted.
		 * </p>
		 */
		public void run()
		{
			while (!isInterrupted()) {
				loggerRaceTest();

				try {
					sleep(_delay);
				}
				catch (InterruptedException e) {
					break;
				}
			}
		}

		/**
		 * <p>
		 *   Create and dispose logger instances.
		 * </p>
		 */
		private void loggerRaceTest()
		{
			Class<?> cls = _factory.getRequiredClass();

			// Get anonymous logger.
			Logger logger = _factory.newLogger(null);
			assertLogger(logger, cls, null);
			logger = null;

			// Get named logger.
			for (int i = 0; i < _count; i++) {
				String name = "logger-" + i;
				logger = _factory.newLogger(name);
				assertLogger(logger, cls, name);
				logger = null;
			}
		}
	}

	/**
	 * <p>
	 *   Ensure that logging levels are initialized correctly.
	 * </p>
	 * <p>
	 *   This method is invoked on a child process.
	 * </p>
	 *
	 * @param args	Command line arguments passed by
	 *		{@link #logLevelTest(String,String)}.
	 */
	static void logLevelTest(String[] args)
	{
		// Parse arguments.
		LevelOptionParser parser = new LevelOptionParser();
		parser.parse(args);
		File logFile = parser.getLogFile();

		LogConfiguration conf =
			new LogConfiguration("leveltest", logFile);

		LogLevel trace = parser.getTraceLevel();
		if (trace != null) {
			// User-defined trace log level is specified.
			conf.setLevel(trace);
		}

		LogLevel defTrace = parser.getDefaultTraceLevel();
		if (defTrace == null) {
			defTrace = LogLevel.INFO;
		}
		else {
			// Default trace log level is specified.
			conf.setDefaultLevel(defTrace);
		}

		File levelFile = parser.getLevelFile();
		if (levelFile != null) {
			// Log level file path is specified.
			conf.setLevelFile(levelFile);
			if (!levelFile.isFile()) {
				// Log level file does not exist.
				levelFile = null;
			}
		}

		// Initialize the logging system.
		LogSystem lsys = LogSystem.getInstance();
		lsys.initialize(conf);

		// Verify initial logging levels.
		LogLevel iniTrace = defTrace;
		if (levelFile == null) {
			// No log level file is available.
			if (trace != null) {
				iniTrace = trace;
			}
		}
		else {
			// Log levels saved in the level file are preferred to
			// default levels.
			LogLevel savedTrace = parser.getSavedTraceLevel();

			if (trace != null) {
				iniTrace = trace;
			}
			else if (savedTrace != null) {
				iniTrace = savedTrace;
			}
		}

		assertSame(iniTrace, lsys.getLevel());

		LogLevel newTrace = parser.getNewTraceLevel();
		LogLevel expected;
		boolean changed;
		if (newTrace == null) {
			// Reset trace log level.
			changed = lsys.setLevel(LogLevel.NONE);
			expected = defTrace;
		}
		else {
			// Change trace log level.
			changed = lsys.setLevel(newTrace);
			expected = newTrace;
		}

		assertSame(expected != iniTrace, changed);
		assertSame(expected, lsys.getLevel());

		lsys.shutdown();
	}
}

/**
 * <p>
 *   Simple command line option parser.
 * </p>
 */
class CommandOptionParser
{
	/**
	 * <p>
	 *   Option names.
	 * </p>
	 */
	private final String[] _options;

	/**
	 * <p>
	 *   Parsed options.
	 * </p>
	 */
	private final HashMap<String, String>  _values =
		new HashMap<String, String>();

	/**
	 * <p>
	 *   Construct a new command line option parser.
	 * </p>
	 *
	 * @param options	An array of command line options to be parsed.
	 */
	CommandOptionParser(String[] options)
	{
		_options = options;
	}

	/**
	 * <p>
	 *   Parse command line options.
	 * </p>
	 *
	 * @param args	Command line arguments.
	 */
	void parse(String[] args)
	{
		for (String arg: args) {
			parse(arg);
		}
	}

	/**
	 * <p>
	 *   Parse the specified argument.
	 * </p>
	 *
	 * @param arg	A command line argument to be parsed.
	 */
	void parse(String arg)
	{
		for (String opt: _options) {
			if (!arg.startsWith(opt)) {
				continue;
			}

			int optlen = opt.length();
			if (optlen == arg.length()) {
				// No value is specified to this option.
				_values.put(opt, null);

				return;
			}

			char c = arg.charAt(optlen);
			if (c == '=') {
				// A value is specified to this option.
				String value = arg.substring(optlen + 1);
				_values.put(opt, value);

				return;
			}
		}

		unparsedOption(arg);
	}

	/**
	 * <p>
	 *   Get a parsed value associated with the given option.
	 * </p>
	 *
	 * @param opt	Option name.
	 * @return	A value associated with the given option.
	 *		{@code null} is returned if the given option is not
	 *		parsed, or no value is specified.
	 */
	String get(String opt)
	{
		return _values.get(opt);
	}

	/**
	 * <p>
	 *   Determine whether the given option is specified or not.
	 * </p>
	 *
	 * @param opt	Option name.
	 * @return	{@code true} only if the given option is specified.
	 */
	boolean isSpecified(String opt)
	{
		return _values.containsKey(opt);
	}

	/**
	 * <p>
	 *   Invoked when an unexpected argument is found.
	 * </p>
	 *
	 * @param arg	Unparsed argument.
	 */
	void unparsedOption(String arg)
	{
		throw new IllegalArgumentException("Unknown option: " + arg);
	}
}

/**
 * <p>
 *   Command line option parser for {@link LogSystemTest#testLogLevel()}.
 * </p>
 */
class LevelOptionParser extends CommandOptionParser
{
	/**
	 * <p>
	 *   Valid options.
	 * </p>
	 */
	private final static String[]  OPTION_NAMES= {
		"trace",
		"deftrace",
		"newtrace",
		"savedtrace",
		"logfile",
		"levelfile",
	};

	/**
	 * <p>
	 *   Construct a new command line option parser for
	 *   {@link LogSystemTest#testLogLevel()}.
	 * </p>
	 */
	LevelOptionParser()
	{
		super(OPTION_NAMES);
	}

	/**
	 * <p>
	 *   Return a user-defined trace log level.
	 * </p>
	 *
	 * @return	A user-defined trace log level.
	 *		{@code null} is returned if not specified.
	 */
	LogLevel getTraceLevel()
	{
		return getLevel("trace");
	}

	/**
	 * <p>
	 *   Return a default trace log level.
	 * </p>
	 *
	 * @return	A default trace log level.
	 *		{@code null} is returned if not specified.
	 */
	LogLevel getDefaultTraceLevel()
	{
		return getLevel("deftrace");
	}

	/**
	 * <p>
	 *   Return a trace log level to be set by
	 *   {@link LogSystemTest#logLevelTest(String[])}.
	 * </p>
	 *
	 * @return	A trace log level to be set.
	 *		{@code null} is returned if not specified.
	 */
	LogLevel getNewTraceLevel()
	{
		return getLevel("newtrace");
	}

	/**
	 * <p>
	 *   Return a trace log level saved in the log level file.
	 * </p>
	 *
	 * @return	A trace log level saved in the log level file.
	 *		{@code null} is returned if not specified.
	 */
	LogLevel getSavedTraceLevel()
	{
		return getLevel("savedtrace");
	}

	/**
	 * <p>
	 *   Return the path to the log file.
	 * </p>
	 *
	 * @return	A {@code File} instance which represents the path to
	 *		the log file.
	 */
	File getLogFile()
	{
		String value = get("logfile");
		assertNotNull(value);

		return new File(value).getAbsoluteFile();
	}

	/**
	 * <p>
	 *   Return the path to the log level file.
	 * </p>
	 *
	 * @return	A {@code File} instance which represents the path to
	 *		the log level file.
	 *		{@code null} is returned if not specified.
	 */
	File getLevelFile()
	{
		String value = get("levelfile");
		File file;

		if (value == null) {
			file = null;
		}
		else {
			file = new File(value).getAbsoluteFile();
		}

		return file;
	}

	/**
	 * <p>
	 *   Return the log level specified by the given option.
	 * </p>
	 *
	 * @param name	An option name.
	 * @return	A {@link LogLevel} instance associated with the given
	 *		option name. {@code null} is returned if the given
	 *		option is not specified.
	 * @throws NumberFormatException
	 *	Illegal option value is associated with the given option name.
	 */
	private LogLevel getLevel(String name)
	{
		String value = get(name);
		LogLevel level;

		if (value == null) {
			level = null;
		}
		else {
			int lvl = Integer.decode(value);

			level = LogLevel.getInstance(lvl);
		}

		return level;
	}
}
