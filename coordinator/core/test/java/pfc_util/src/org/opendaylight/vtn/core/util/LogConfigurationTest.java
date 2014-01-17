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
 *   Unit test class for {@link LogConfiguration}.
 * </p>
 */
public class LogConfigurationTest extends TestBase
{
	/**
	 * Create JUnit test case for {@link LogConfiguration}.
	 *
	 * @param name	The test name.
	 */
	public LogConfigurationTest(String name)
	{
		super(name);
	}

	/**
	 * Verify default value of {@link LogConfiguration}.
	 */
	public void testDefault()
	{
		// Specifying log file.
		{
			String ident = "conf-test-1";
			File   file = new File("/foo/bar/test.log");
			LogConfiguration conf =
				new LogConfiguration(ident, file);

			checkDefault(conf, ident, file);
		}

		// Without log file.
		{
			String ident = "conf-test-2";
			try {
				LogConfiguration conf =
					new LogConfiguration(ident, null);
			}
			catch (NullPointerException e) {}
			catch (Exception e) {
				unexpected(e);
			}
		}
	}

	/**
	 * Test case of error handling in
	 * {@link LogConfiguration#LogConfiguration(String,File)}.
	 */
	public void testCtorError()
	{
		LogConfiguration conf;

		// ident is null.
		try {
			conf = new LogConfiguration(null, null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class,
				       "ident is null.");
		}

		// ident is empty.
		try {
			conf = new LogConfiguration("", null);
			needException();
		}
		catch (Exception e) {
			checkException(e, IllegalArgumentException.class,
				       "ident is empty.");
		}

		// Log file path is relative.
		String[] path = {
			"foo/bar/log.txt",
			"log.txt",
			"log/../../../test.log",
		};
		for (int i = 0; i < path.length; i++) {
			File  file = new File(path[i]);
			try {
				conf = new LogConfiguration("ident", file);
				needException(file);
			}
			catch (Exception e) {
				checkException
					(e, IllegalArgumentException.class,
					 "Not an absolute path: " + file);
			}
		}

		// Root directory.
		{
			File file = new File("/");
			try {
				conf = new LogConfiguration("ident", file);
				needException(file);
			}
			catch (Exception e) {
				checkException
					(e, IllegalArgumentException.class,
					 "File does not contain file name: " +
					 file);
			}
		}

		// Invalid path component.
		path = new String[] {
			"/foo/bar/..",
			"/somewhere/.",
		};
		for (int i = 0; i < path.length; i++) {
			File  file = new File(path[i]);
			try {
				conf = new LogConfiguration("ident", file);
				needException(file);
			}
			catch (Exception e) {
				checkException
					(e, IllegalArgumentException.class,
					 "Invalid path component: " + file);
			}
		}
	}

	/**
	 * Test case of {@link LogConfiguration#setFacility(LogFacility)}.
	 */
	public void testSetFacility()
	{
		File  f = new File("/foo/bar/log.txt");
		LogConfiguration conf = new LogConfiguration("ident", f);
		
		try {
			conf.setFacility(null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class,
				       "facility is null.");
		}

		LogFacility[] facl = {
			LogFacility.DAEMON,
			LogFacility.LOCAL0,
			LogFacility.LOCAL1,
			LogFacility.LOCAL2,
			LogFacility.LOCAL3,
			LogFacility.LOCAL4,
			LogFacility.LOCAL5,
			LogFacility.LOCAL6,
			LogFacility.LOCAL7,
		};

		for (int i = 0; i < facl.length; i++) {
			conf.setFacility(facl[i]);
			assertEquals(facl[i], conf.getFacility());
		}
	}

	/**
	 * Test case of {@link LogConfiguration#setRotation(int,int)}.
	 */
	public void testSetRotation()
	{
		File  f = new File("/foo/bar/log.txt");
		LogConfiguration conf = new LogConfiguration("ident", f);

		// Negative count.
		for (int count = -50; count < 0; count++) {
			try {
				conf.setRotation(count, 10000);
				needException();
			}
			catch (Exception e) {
				checkException(e,
					       IllegalArgumentException.class,
					       "count is negative: " + count);
			}
		}

		// Invalid file size.
		for (int size = -100; size <= 0; size++) {
			try {
				conf.setRotation(10, size);
				needException();
			}
			catch (Exception e) {
				checkException(e,
					       IllegalArgumentException.class,
					       "size is less than or equal " +
					       "to zero: " + size);
			}
		}

		for (int count = 0; count <= 50; count++) {
			for (int size = 1; size <= 1000000; size += 8000) {
				conf.setRotation(count, size);
				assertEquals(count, conf.getRotationCount());
				assertEquals(size, conf.getRotationSize());
			}
		}
	}

	/**
	 * Test case of {@link LogConfiguration#setLevelFile(File)}.
	 */
	public void testSetLevelFile()
	{
		// Relative path.
		String[] path = {
			"foo/bar/log.txt",
			"log.txt",
			"log/../../../test.log",
		};

		File  f = new File("/foo/bar/log.txt");
		LogConfiguration conf = new LogConfiguration("ident", f);
		for (int i = 0; i < path.length; i++) {
			File  file = new File(path[i]);
			try {
				conf.setLevelFile(file);
				needException(file);
			}
			catch (Exception e) {
				checkException
					(e, IllegalArgumentException.class,
					 "Not an absolute path: " + file);
			}
		}

		// Root directory.
		{
			File file = new File("/");
			try {
				conf.setLevelFile(file);
				needException(file);
			}
			catch (Exception e) {
				checkException
					(e, IllegalArgumentException.class,
					 "File does not contain file name: " +
					 file);
			}
		}

		// Invalid path component.
		path = new String[] {
			"/foo/bar/..",
			"/somewhere/.",
		};
		for (int i = 0; i < path.length; i++) {
			File  file = new File(path[i]);
			try {
				conf.setLevelFile(file);
				needException(file);
			}
			catch (Exception e) {
				checkException
					(e, IllegalArgumentException.class,
					 "Invalid path component: " + file);
			}
		}

		// Valid file path.
		path = new String[] {
			"/foo/bar/log.txt",
			"/somewhere/log/test.log.",
			null,
		};

		for (int i = 0; i < path.length; i++) {
			File  file = (path[i] == null)
				? null : new File(path[i]);
			conf.setLevelFile(file);
			assertEquals(file, conf.getLevelFile());
		}
	}

	/**
	 * Test case of {@link LogConfiguration#setLevel(LogLevel)}.
	 */
	public void testSetLevel()
	{
		File  f = new File("/foo/bar/log.txt");
		LogConfiguration conf = new LogConfiguration("ident", f);

		try {
			conf.setLevel(null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class,
				       "level is null.");
		}

		assertEquals(LogLevel.NONE, conf.getLevel());

		LogLevel[] levels = {
			LogLevel.NONE,
			LogLevel.FATAL,
			LogLevel.ERROR,
			LogLevel.WARNING,
			LogLevel.NOTICE,
			LogLevel.INFO,
			LogLevel.DEBUG,
			LogLevel.TRACE,
			LogLevel.VERBOSE,
		};

		for (int i = 0; i < levels.length; i++) {
			conf.setLevel(levels[i]);
			assertEquals(levels[i], conf.getLevel());
			assertEquals(LogLevel.INFO, conf.getDefaultLevel());
		}
	}

	/**
	 * Test case of {@link LogConfiguration#setDefaultLevel(LogLevel)}.
	 */
	public void testSetDefaultLevel()
	{
		File  f = new File("/foo/bar/log.txt");
		LogConfiguration conf = new LogConfiguration("ident", f);

		try {
			conf.setDefaultLevel(null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class,
				       "level is null.");
		}

		try {
			conf.setDefaultLevel(LogLevel.NONE);
			needException();
		}
		catch (Exception e) {
			checkException(e, IllegalArgumentException.class,
				       "level is NONE.");
		}

		assertEquals(LogLevel.INFO, conf.getDefaultLevel());

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

		for (int i = 0; i < levels.length; i++) {
			conf.setDefaultLevel(levels[i]);
			assertEquals(levels[i], conf.getDefaultLevel());
			assertEquals(LogLevel.NONE, conf.getLevel());
		}
	}

	/**
	 * Test case of
	 * {@link LogConfiguration#setFatalHandler(LogFatalHandler)}.
	 */
	public void testSetFatalHandler()
	{
		File  f = new File("/foo/bar/log.txt");
		LogConfiguration conf = new LogConfiguration("ident", f);
		LogFatalHandler handler = new LogFatalHandler() {
			public void fatalError() {}
		};

		conf.setFatalHandler(handler);
		assertEquals(handler, conf.getFatalHandler());

		conf.setFatalHandler(null);
		assertEquals(null, conf.getFatalHandler());
	}

	/**
	 * Verify default value of {@link LogConfiguration} fields.
	 *
	 * @param conf	The configuration object to be tested.
	 * @param ident	Required log identifier.
	 * @param file	Required log file.
	 */
	private void checkDefault(LogConfiguration conf, String ident,
				  File file)
	{
		assertEquals(ident, conf.getIdent());
		assertEquals(LogFacility.LOCAL0, conf.getFacility());
		assertEquals(LogLevel.NONE, conf.getLevel());
		assertEquals(LogLevel.INFO, conf.getDefaultLevel());
		assertEquals(10, conf.getRotationCount());
		assertEquals(10000000, conf.getRotationSize());
		assertEquals(file, conf.getLogFile());
		assertEquals(null, conf.getLevelFile());
		assertEquals(LogSystem.getInstance(), conf.getFatalHandler());
	}
}
