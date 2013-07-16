/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.util;

import junit.framework.Test;
import junit.framework.TestSuite;

/**
 * <p>
 *   Bundle of all JUnit tests for org.opendaylight.vtn.core.util package.
 * </p>
 */
public class AllTests
{
	/**
	 * Return test suits which contains all JUnit tests for
	 * org.opendaylight.vtn.core.util package.
	 *
	 * @return	{@code Test} instance which contains tests.
	 */
	public static Test suite()
	{
		TestSuite  suite = new TestSuite("All core.util JUnit tests");

		suite.addTest(new TestSuite(LogLevelTest.class));
		suite.addTest(new TestSuite(LogFacilityTest.class));
		suite.addTest(new TestSuite(LogConfigurationTest.class));
		suite.addTest(new TestSuite(LogSystemTest.class));
		suite.addTest(new TestSuite(LoggerTest.class));
		suite.addTest(new TestSuite(HostAddressTest.class));
		suite.addTest(new TestSuite(TimeSpecTest.class));
		suite.addTest(new TestSuite(UnsignedIntegerTest.class));

		return suite;
	}
}
