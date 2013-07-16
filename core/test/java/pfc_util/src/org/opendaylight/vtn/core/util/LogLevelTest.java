/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.util;

import java.util.TreeSet;
import java.util.Iterator;

/**
 * <p>
 *   Unit test class for {@link LogLevel}.
 * </p>
 */
public class LogLevelTest extends TestBase
{
	/**
	 * <p>
	 *   Create JUnit test case for {@link LogLevel}.
	 * </p>
	 *
	 * @param name	The test name.
	 */
	public LogLevelTest(String name)
	{
		super(name);
	}

	/**
	 * <p>
	 *   Verify built-in {@link LogLevel} constants.
	 * </p>
	 */
	public void testConstants()
	{
		assertEquals(-1, LogLevel.LVL_NONE);
		assertEquals(0, LogLevel.LVL_FATAL);
		assertEquals(1, LogLevel.LVL_ERROR);
		assertEquals(2, LogLevel.LVL_WARNING);
		assertEquals(3, LogLevel.LVL_NOTICE);
		assertEquals(4, LogLevel.LVL_INFO);
		assertEquals(5, LogLevel.LVL_DEBUG);
		assertEquals(6, LogLevel.LVL_TRACE);
		assertEquals(7, LogLevel.LVL_VERBOSE);

		assertLogLevel(LogLevel.NONE, -1, "NONE", false);
		assertLogLevel(LogLevel.FATAL, 0, "FATAL", true);
		assertLogLevel(LogLevel.ERROR, 1, "ERROR", true);
		assertLogLevel(LogLevel.WARNING, 2, "WARNING", true);
		assertLogLevel(LogLevel.NOTICE, 3, "NOTICE", true);
		assertLogLevel(LogLevel.INFO, 4, "INFO", true);
		assertLogLevel(LogLevel.DEBUG, 5, "DEBUG", true);
		assertLogLevel(LogLevel.TRACE, 6, "TRACE", false);
		assertLogLevel(LogLevel.VERBOSE, 7, "VERBOSE", false);
	}

	/**
	 * <p>
	 *   Test case of {@link LogLevel#getInstance(int)}.
	 * </p>
	 */
	public void testGetInstance()
	{
		LogLevel  required[] = {
			LogLevel.FATAL,
			LogLevel.ERROR,
			LogLevel.WARNING,
			LogLevel.NOTICE,
			LogLevel.INFO,
			LogLevel.DEBUG,
			LogLevel.TRACE,
			LogLevel.VERBOSE,
		};

		for (int i = 0; i < required.length; i++) {
			LogLevel lvl = LogLevel.getInstance(i);

			assertSame(required[i], lvl);
		}

		// Error test.
		for (int i = -100; i <= 100; i++) {
			if (i >= 0 && i < required.length) {
				continue;
			}

			try {
				LogLevel.getInstance(i);
				needException();
			}
			catch (Exception e) {
				checkException(e,
					       IllegalArgumentException.class,
					       "Invalid level: " + i);
			}
		}
	}

	/**
	 * <p>
	 *   Test case of {@link LogLevel#compareTo(LogLevel)}.
	 * </p>
	 */
	public void testCompareTo()
	{
		LogLevel  levels[] = {
			LogLevel.NOTICE,
			LogLevel.WARNING,
			LogLevel.VERBOSE,
			LogLevel.DEBUG,
			LogLevel.FATAL,
			LogLevel.NONE,
			LogLevel.ERROR,
			LogLevel.TRACE,
			LogLevel.INFO,
		};

		TreeSet<LogLevel> set = new TreeSet<LogLevel>();
		for (int i = 0; i < levels.length; i++) {
			LogLevel  lvl = levels[i];
			assertTrue(set.add(lvl));
			assertFalse(set.add(lvl));
			assertFalse(set.add(lvl));
		}

		int prev = -2;
		for (Iterator<LogLevel> it = set.iterator(); it.hasNext();) {
			LogLevel lvl = it.next();
			int level = lvl.getLevel();
			assertTrue(prev < level);
			prev = level;
		}
	}

	/**
	 * <p>
	 *   Test case of {@link LogLevel#forName(String)}.
	 * </p>
	 */
	public void testForName()
	{
		// null name.
		try {
			LogLevel.forName(null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class);
		}

		// Valid log level names.
		LogLevel[] levels = {
			LogLevel.FATAL,
			LogLevel.ERROR,
			LogLevel.WARNING,
			LogLevel.NOTICE,
			LogLevel.INFO,
			LogLevel.DEBUG,
			LogLevel.TRACE,
			LogLevel.VERBOSE,
		};

		for (LogLevel lvl: levels) {
			String name = lvl.toString();
			assertSame(lvl, LogLevel.forName(name));
			assertSame(lvl, LogLevel.forName(name.toLowerCase()));

			int len = name.length();
			for (int i = 0; i < len; i++) {
				String prefix = name.substring(0, i);
				String c = name.substring(i, i + 1);
				String suffix = name.substring(i + 1, len); 

				String nm = prefix + c.toLowerCase() + suffix;
				assertSame(lvl, LogLevel.forName(nm));
			}
		}

		// Invalid log level names.
		String[] invalid = {
			"",
			"1",
			"bad level",
			"fine",
			"crit",
		};

		for (String name: invalid) {
			assertSame(null, LogLevel.forName(name));
		}

		for (LogLevel lvl: levels) {
			String name = lvl.toString();

			assertSame(null, LogLevel.forName(" " + name));
			assertSame(null, LogLevel.forName(name + " "));
			assertSame(null, LogLevel.forName("a" + name));
			assertSame(null, LogLevel.forName(name + "a"));

			int len = name.length();
			for (int i = 1; i <= len - 1; i++) {
				String nm = name.substring(0, i);
				assertSame(null, LogLevel.forName(nm));

				nm = nm.toLowerCase();
				assertSame(null, LogLevel.forName(nm));

				nm = name.substring(i, len);
				assertSame(null, LogLevel.forName(nm));

				nm = nm.toLowerCase();
				assertSame(null, LogLevel.forName(nm));
			}
		}
	}

	/**
	 * <p>
	 *   Verify contents of {@link LogLevel}.
	 * </p>
	 *
	 * @param lvl		{@link LogLevel} to be tested.
	 * @param level		Required log level.
	 * @param name		Required name.
	 * @param validSysLevel	True if {@code lvl} is valid syslog level.
	 */
	private void assertLogLevel(LogLevel lvl, int level, String name,
				    boolean validSysLevel)
	{
		assertEquals(level, lvl.getLevel());
		assertEquals(name, lvl.toString());
	}
}
