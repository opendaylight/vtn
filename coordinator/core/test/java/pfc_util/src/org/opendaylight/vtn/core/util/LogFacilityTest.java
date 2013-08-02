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
 *   Unit test class for {@link LogFacility}.
 * </p>
 */
public class LogFacilityTest extends TestBase
{
	/**
	 * Create JUnit test case for {@link LogFacility}.
	 *
	 * @param name	The test name.
	 */
	public LogFacilityTest(String name)
	{
		super(name);
	}

	/**
	 * Verify built-in {@link LogFacility} constants.
	 */
	public void testConstants()
	{
		assertFacility(LogFacility.DAEMON, 0, "daemon");
		assertFacility(LogFacility.LOCAL0, 1, "local0");
		assertFacility(LogFacility.LOCAL1, 2, "local1");
		assertFacility(LogFacility.LOCAL2, 3, "local2");
		assertFacility(LogFacility.LOCAL3, 4, "local3");
		assertFacility(LogFacility.LOCAL4, 5, "local4");
		assertFacility(LogFacility.LOCAL5, 6, "local5");
		assertFacility(LogFacility.LOCAL6, 7, "local6");
		assertFacility(LogFacility.LOCAL7, 8, "local7");
	}

	/**
	 * <p>
	 *   Test case of {@link LogFacility#forName(String)}.
	 * </p>
	 */
	public void testForName()
	{
		// null name.
		try {
			LogFacility.forName(null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class);
		}

		// Valid facility names.
		LogFacility[] facilities = {
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

		for (LogFacility fac: facilities) {
			String name = fac.toString();
			assertSame(fac, LogFacility.forName(name));
			assertSame(fac,
				   LogFacility.forName(name.toLowerCase()));

			int len = name.length();
			for (int i = 0; i < len; i++) {
				String prefix = name.substring(0, i);
				String c = name.substring(i, i + 1);
				String suffix = name.substring(i + 1, len); 

				String nm = prefix + c.toLowerCase() + suffix;
				assertSame(fac, LogFacility.forName(nm));
			}
		}

		// Invalid log facility names.
		String[] invalid = {
			"",
			"1",
			"bad facility",
			"mail",
			"news",
		};

		for (String name: invalid) {
			assertSame(null, LogFacility.forName(name));
		}

		for (LogFacility fac: facilities) {
			String name = fac.toString();

			assertSame(null, LogFacility.forName(" " + name));
			assertSame(null, LogFacility.forName(name + " "));
			assertSame(null, LogFacility.forName("a" + name));
			assertSame(null, LogFacility.forName(name + "a"));

			int len = name.length();
			for (int i = 1; i <= len - 1; i++) {
				String nm = name.substring(0, i);
				assertSame(null, LogFacility.forName(nm));

				nm = nm.toLowerCase();
				assertSame(null, LogFacility.forName(nm));

				nm = name.substring(i, len);
				assertSame(null, LogFacility.forName(nm));

				nm = nm.toLowerCase();
				assertSame(null, LogFacility.forName(nm));
			}
		}
	}

	/**
	 * Verify contents of {@link LogFacility}.
	 *
	 * @param facility	{@link LogFacility} to be tested.
	 * @param value		Required logging facility value.
	 * @param name		Required name.
	 */
	private void assertFacility(LogFacility facility, int value,
				    String name)
	{
		assertEquals(value, facility.getFacility());
		assertEquals(name, facility.toString());
	}
}
